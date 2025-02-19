/* unzip.c -- IO for uncompress .zip files using zlib
Version 1.01, May 8th, 2004

Copyright (C) 1998-2004 Gilles Vollant

Read unzip.h for more info
 */

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"
#include "unzip.h"

#ifdef STDC
  #include <stddef.h>
  #include <string.h>
  #include <stdlib.h>
#endif
#ifdef NO_ERRNO_H
  extern int errno;
#else
  #include <errno.h>
#endif

/* compile with -Dlocal if your debugger can't find static symbols */


#ifndef CASESENSITIVITYDEFAULT_NO
  #if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES)
    #define CASESENSITIVITYDEFAULT_NO
  #endif
#endif


#ifndef UNZ_BUFSIZE
  #define UNZ_BUFSIZE (32768)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
  #define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
  #define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
  #define TRYFREE(p) {if (p) free(p);}
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)

/* unz_file_info_interntal contain internal info about a file in zipfile*/

typedef struct unz_file_info_internal_s
{
  DWORD offset_curfile; /* relative offset of local header 4 bytes */
} unz_file_info_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
when reading and decompress it */

typedef struct
{
  char *read_buffer; /* internal buffer for compressed data */
  z_stream stream; /* zLib stream structure for inflate */
  DWORD pos_in_zipfile; /* position in byte on the zipfile, for fseek*/
  DWORD stream_initialised; /* flag set if stream structure is initialised*/
  DWORD offset_local_extrafield; /* offset of the local extra field */
  DWORD size_local_extrafield; /* size of the local extra field */
  DWORD pos_local_extrafield; /* position in the local extra field in read*/
  DWORD crc32; /* crc32 of all data uncompressed */
  DWORD crc32_wait; /* crc32 we must obtain after decompress all */
  DWORD rest_read_compressed; /* number of byte to be decompressed */
  DWORD rest_read_uncompressed; /*number of byte to be obtained after decomp*/
  zlib_filefunc_def z_filefunc;
  void *filestream; /* io structore of the zipfile */
  DWORD compression_method; /* compression method (0==store) */
  DWORD byte_before_the_zipfile; /* byte before the zipfile, (>0 for sfx)*/
  int raw;
} file_in_zip_read_info_s;


/* unz_s contain internal information about the zipfile
 */
typedef struct
{
  zlib_filefunc_def z_filefunc;
  void *filestream; /* io structore of the zipfile */
  unz_global_info gi; /* public global information */
  DWORD byte_before_the_zipfile; /* byte before the zipfile, (>0 for sfx)*/
  DWORD num_file; /* number of the current file in the zipfile*/
  DWORD pos_in_central_dir; /* pos of the current file in the central dir*/
  DWORD current_file_ok; /* flag about the usability of the current file*/
  DWORD central_pos; /* position of the beginning of the central dir*/
  DWORD size_central_dir; /* size of the central directory  */
  DWORD offset_central_dir; /* offset of start of central directory with respect to the starting disk number */
  unz_file_info cur_file_info; /* public info about the current file in zip*/
  unz_file_info_internal cur_file_info_internal; /* private info about it*/
  file_in_zip_read_info_s *pfile_in_zip_read; /* structure about the current file if we are decompressing it */
  int encrypted;

  #ifndef NOUNCRYPT
    DWORD keys[3]; /* keys defining the pseudo-random sequence */
    const DWORD *pcrc_32_tab;
  #endif

} unz_s;


#ifndef NOUNCRYPT
  #include "crypt.h"
#endif

/* ===========================================================================
Read a byte from a gz_stream; update next_in and avail_in. Return EOF
for end of file.
IN assertion: the stream s has been sucessfully opened for reading.
 */


static int unzlocal_getByte(const zlib_filefunc_def *pzlib_filefunc_def, void *filestream, int *pi)
{
  BYTE c;
  int err = (int)ZREAD(*pzlib_filefunc_def, filestream, &c, 1);

  if (err == 1)
  {
    *pi = (int)c;
    return UNZ_OK;
  }
  else
  {
    if (ZERROR(*pzlib_filefunc_def, filestream))
    {
      return UNZ_ERRNO;
    }
    else
    {
      return UNZ_EOF;
    }
  }
}


/* ===========================================================================
Reads a long in LSB order from the given gz_stream. Sets
 */

static int unzlocal_getShort(const zlib_filefunc_def *pzlib_filefunc_def, void *filestream, DWORD *pX)
{
  DWORD x;
  int i;
  int err;

  err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);

  x = (DWORD)i;

  if (err == UNZ_OK)
  {
    err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
  }

  x += ((DWORD)i) << 8;

  if (err == UNZ_OK)
  {
    *pX = x;
  }
  else
  {
    *pX = 0;
  }

  return err;
}

//-------------------------------------------------------------------------


static int unzlocal_getLong(const zlib_filefunc_def *pzlib_filefunc_def, void *filestream, DWORD *pX)
{
  DWORD x;
  int i;
  int err;

  err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
  x = (DWORD)i;

  if (err == UNZ_OK)
  {
    err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
  }
  x += ((DWORD)i) << 8;

  if (err == UNZ_OK)
  {
    err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
  }
  x += ((DWORD)i) << 16;

  if (err == UNZ_OK)
  {
    err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
  }
  x += ((DWORD)i) << 24;

  if (err == UNZ_OK)
  {
    *pX = x;
  }
  else
  {
    *pX = 0;
  }
  return err;
}


//-------------------------------------------------------------------------


#ifdef CASESENSITIVITYDEFAULT_NO
  #define CASESENSITIVITYDEFAULTVALUE 2
#else
  #define CASESENSITIVITYDEFAULTVALUE 1
#endif

#ifndef STRCMPCASENOSENTIVEFUNCTION
  #define STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_internal
#endif

/*
Compare two filename (fileName1,fileName2).
If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
or strcasecmp)
If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
(like 1 on Unix, 2 on Windows)
*/
/*
extern int ZEXPORT unzStringFileNameCompare(const char *fileName1, const char *fileName2, int iCaseSensitivity)
{
  if (iCaseSensitivity == 0)
  {
    iCaseSensitivity = CASESENSITIVITYDEFAULTVALUE;
  }

  if (iCaseSensitivity == 1)
  {
    return strcmp(fileName1, fileName2);
  }

  return stricmp(fileName1, fileName2);
}
*/
//-------------------------------------------------------------------------

#ifndef BUFREADCOMMENT
  #define BUFREADCOMMENT (0x400)
#endif

/*
Locate the Central directory of a zipfile (at the end, just before
the global comment)
 */

static DWORD unzlocal_SearchCentralDir(const zlib_filefunc_def *pzlib_filefunc_def, void *filestream)
{
  BYTE *buf;
  DWORD uSizeFile;
  DWORD uBackRead;
  DWORD uMaxBack = 0xffff; /* maximum size of global comment */
  DWORD uPosFound = 0;

  if (ZSEEK(*pzlib_filefunc_def, filestream, 0, SEEK_END) != 0)
  {
    return 0;
  }


  uSizeFile = ZTELL(*pzlib_filefunc_def, filestream);

  if (uMaxBack > uSizeFile)
  {
    uMaxBack = uSizeFile;
  }

  buf = (BYTE*)ALLOC(BUFREADCOMMENT + 4);
  if (buf == NULL)
  {
    return 0;
  }

  uBackRead = 4;
  while (uBackRead < uMaxBack)
  {
    DWORD uReadSize, uReadPos;
    int i;
    if (uBackRead + BUFREADCOMMENT > uMaxBack)
    {
      uBackRead = uMaxBack;
    }
    else
    {
      uBackRead += BUFREADCOMMENT;
    }
    uReadPos = uSizeFile - uBackRead;

    uReadSize = ((BUFREADCOMMENT + 4) < (uSizeFile - uReadPos)) ? (BUFREADCOMMENT + 4): (uSizeFile - uReadPos);
    if (ZSEEK(*pzlib_filefunc_def, filestream, uReadPos, SEEK_SET) != 0)
    {
      break;
    }

    if (ZREAD(*pzlib_filefunc_def, filestream, buf, uReadSize) != uReadSize)
    {
      break;
    }

    for (i = (int)uReadSize - 3; (i--) > 0;)
    if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) && ((*(buf + i + 2)) == 0x05) && ((*(buf + i + 3)) == 0x06))
    {
      uPosFound = uReadPos + i;
      break;
    }

    if (uPosFound != 0)
    {
      break;
    }
  }
  TRYFREE(buf);
  return uPosFound;
}

/*
Open a Zip file. path contain the full pathname (by example,
on a Windows NT computer "c:\\test\\zlib114.zip" or on an Unix computer
"zlib/zlib114.zip".
If the zipfile cannot be opened (file doesn't exist or in not valid), the
return value is NULL.
Else, the return value is a unzFile Handle, usable with other function
of this unzip package.
 */
extern unzFile ZEXPORT unzOpen2(const char *path, zlib_filefunc_def *pzlib_filefunc_def)
{
  unz_s us;
  unz_s *s;
  DWORD central_pos, uL;

  DWORD number_disk; /* number of the current dist, used for
  spaning ZIP, unsupported, always 0*/
  DWORD number_disk_with_CD; /* number the the disk with central dir, used
  for spaning ZIP, unsupported, always 0*/
  DWORD number_entry_CD; /* total number of entries in
  the central dir
  (same than number_entry on nospan) */

  int err = UNZ_OK;

  if (pzlib_filefunc_def == NULL)
  {
    fill_fopen_filefunc(&us.z_filefunc);
  }
  else
  {
    us.z_filefunc =  *pzlib_filefunc_def;
  }

  us.filestream = (*(us.z_filefunc.zopen_file))(us.z_filefunc.opaque, path, ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_EXISTING);
  if (us.filestream == NULL)
  {
    return NULL;
  }

  central_pos = unzlocal_SearchCentralDir(&us.z_filefunc, us.filestream);
  if (central_pos == 0)
  {
    err = UNZ_ERRNO;
  }

  if (ZSEEK(us.z_filefunc, us.filestream, central_pos, SEEK_SET) != 0)
  {
    err = UNZ_ERRNO;
  }

  /* the signature, already checked */
  if (unzlocal_getLong(&us.z_filefunc, us.filestream, &uL) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  /* number of this disk */
  if (unzlocal_getShort(&us.z_filefunc, us.filestream, &number_disk) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  /* number of the disk with the start of the central directory */
  if (unzlocal_getShort(&us.z_filefunc, us.filestream, &number_disk_with_CD) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  /* total number of entries in the central dir on this disk */
  if (unzlocal_getShort(&us.z_filefunc, us.filestream, &us.gi.number_entry) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  /* total number of entries in the central dir */
  if (unzlocal_getShort(&us.z_filefunc, us.filestream, &number_entry_CD) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if ((number_entry_CD != us.gi.number_entry) || (number_disk_with_CD != 0) || (number_disk != 0))
  {
    err = UNZ_BADZIPFILE;
  }

  /* size of the central directory */
  if (unzlocal_getLong(&us.z_filefunc, us.filestream, &us.size_central_dir) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  /* offset of start of central directory with respect to the
  starting disk number */
  if (unzlocal_getLong(&us.z_filefunc, us.filestream, &us.offset_central_dir) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  /* zipfile comment length */
  if (unzlocal_getShort(&us.z_filefunc, us.filestream, &us.gi.size_comment) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if ((central_pos < us.offset_central_dir + us.size_central_dir) && (err == UNZ_OK))
  {
    err = UNZ_BADZIPFILE;
  }

  if (err != UNZ_OK)
  {
    ZCLOSE(us.z_filefunc, us.filestream);
    return NULL;
  }

  us.byte_before_the_zipfile = central_pos - (us.offset_central_dir + us.size_central_dir);
  us.central_pos = central_pos;
  us.pfile_in_zip_read = NULL;
  us.encrypted = 0;


  s = (unz_s*)ALLOC(sizeof(unz_s));
  *s = us;
  unzGoToFirstFile((unzFile)s);
  return (unzFile)s;
}

//-------------------------------------------------------------------------

extern unzFile ZEXPORT unzOpen(CONST TCHAR *path)
{
#ifdef UNICODE
    return unzOpenW(path);
#else
    return unzOpenA(path);
#endif
}


extern unzFile ZEXPORT unzOpenA(const char *path)
{
  return unzOpen2(path, NULL);
}

extern unzFile ZEXPORT unzOpenW(const wchar_t *path)
{
    char  pathA[_MAX_PATH];
    // ���ַ�ת���ɶ��ַ��ļ����ַ���
    WideCharToMultiByte(
        CP_ACP, 
        0, 
        path,
        (INT)wcslen(path) + 1,
        pathA,
        MAX_PATH,
        NULL,
        NULL);
    return unzOpenA(pathA);
}
/*
Close a ZipFile opened with unzipOpen.
If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
return UNZ_OK if there is no problem. */

extern int ZEXPORT unzClose(unzFile file)
{
  unz_s *s;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }
  s = (unz_s*)file;

  if (s->pfile_in_zip_read != NULL)
  {
    unzCloseCurrentFile(file);
  }

  ZCLOSE(s->z_filefunc, s->filestream);
  TRYFREE(s);
  return UNZ_OK;
}


/*
Write info about the ZipFile in the *pglobal_info structure.
No preparation of the structure is needed
return UNZ_OK if there is no problem. */
/*
extern int ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info *pglobal_info)
{
  unz_s *s;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;
  *pglobal_info = s->gi;
  return UNZ_OK;
}
*/

/*
Translate date/time from Dos format to tm_unz (readable more easilty)
 */

static void unzlocal_DosDateToTmuDate(DWORD ulDosDate, tm_unz *ptm)
{
  DWORD uDate;
  uDate = (DWORD)(ulDosDate >> 16);
  ptm->tm_mday = (DWORD)(uDate &0x1f);
  ptm->tm_mon = (DWORD)((((uDate) &0x1E0) / 0x20) - 1);
  ptm->tm_year = (DWORD)(((uDate &0x0FE00) / 0x0200) + 1980);

  ptm->tm_hour = (DWORD)((ulDosDate &0xF800) / 0x800);
  ptm->tm_min = (DWORD)((ulDosDate &0x7E0) / 0x20);
  ptm->tm_sec = (DWORD)(2 *(ulDosDate &0x1f));
}

/*
Get Info about the current file in the zipfile, with internal only info
 */

static int unzlocal_GetCurrentFileInfoInternal(unzFile file, unz_file_info *pfile_info, unz_file_info_internal *pfile_info_internal, char *szFileName, DWORD fileNameBufferSize, void *extraField, DWORD extraFieldBufferSize, char *szComment, DWORD commentBufferSize)
{
  unz_s *s;
  unz_file_info file_info;
  unz_file_info_internal file_info_internal;
  int err = UNZ_OK;
  DWORD uMagic;
  long lSeek = 0;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;

  if (ZSEEK(s->z_filefunc, s->filestream, s->pos_in_central_dir + s->byte_before_the_zipfile, SEEK_SET) != 0)
  {
    err = UNZ_ERRNO;
  }

  /* we check the magic */

  if (err == UNZ_OK)
  {
    if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uMagic) != UNZ_OK)
    {
      err = UNZ_ERRNO;
    }
    else if (uMagic != 0x02014b50)
    {
      err = UNZ_BADZIPFILE;
    }
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.version) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.version_needed) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.flag) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.compression_method) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.dosDate) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  unzlocal_DosDateToTmuDate(file_info.dosDate, &file_info.tmu_date);

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.crc) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.compressed_size) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.uncompressed_size) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.size_filename) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.size_file_extra) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.size_file_comment) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.disk_num_start) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.internal_fa) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.external_fa) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info_internal.offset_curfile) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  lSeek += file_info.size_filename;

  if ((err == UNZ_OK) && (szFileName != NULL))
  {
    DWORD uSizeRead;
    if (file_info.size_filename < fileNameBufferSize)
    {
      *(szFileName + file_info.size_filename) = '\0';
      uSizeRead = file_info.size_filename;
    }
    else
    {
      uSizeRead = fileNameBufferSize;
    }

    if ((file_info.size_filename > 0) && (fileNameBufferSize > 0))
    if (ZREAD(s->z_filefunc, s->filestream, szFileName, uSizeRead) != uSizeRead)
    {
      err = UNZ_ERRNO;
    }
    lSeek -= uSizeRead;
  }

  if ((err == UNZ_OK) && (extraField != NULL))
  {
    DWORD uSizeRead;
    if (file_info.size_file_extra < extraFieldBufferSize)
    {
      uSizeRead = file_info.size_file_extra;
    }
    else
    {
      uSizeRead = extraFieldBufferSize;
    }

    if (lSeek != 0)
    {
      if (ZSEEK(s->z_filefunc, s->filestream, lSeek, SEEK_CUR) == 0)
      {
        lSeek = 0;
      }
      else
      {
        err = UNZ_ERRNO;
      }
    }

    if ((file_info.size_file_extra > 0) && (extraFieldBufferSize > 0))
    {
      if (ZREAD(s->z_filefunc, s->filestream, extraField, uSizeRead) != uSizeRead)
      {
        err = UNZ_ERRNO;
      }
    }

    lSeek += file_info.size_file_extra - uSizeRead;
  }
  else
  {
    lSeek += file_info.size_file_extra;
  }


  if ((err == UNZ_OK) && (szComment != NULL))
  {
    DWORD uSizeRead;

    if (file_info.size_file_comment < commentBufferSize)
    {
      *(szComment + file_info.size_file_comment) = '\0';
      uSizeRead = file_info.size_file_comment;
    }
    else
    {
      uSizeRead = commentBufferSize;
    }

    if (lSeek != 0)
    {
      if (ZSEEK(s->z_filefunc, s->filestream, lSeek, SEEK_CUR) == 0)
      {
        lSeek = 0;
      }
      else
      {
        err = UNZ_ERRNO;
      }
    }

    if ((file_info.size_file_comment > 0) && (commentBufferSize > 0))
    {
      if (ZREAD(s->z_filefunc, s->filestream, szComment, uSizeRead) != uSizeRead)
      {
        err = UNZ_ERRNO;
      }
    }

    lSeek += file_info.size_file_comment - uSizeRead;
  }
  else
  {
    lSeek += file_info.size_file_comment;
  }

  if ((err == UNZ_OK) && (pfile_info != NULL))
  {
    *pfile_info = file_info;
  }

  if ((err == UNZ_OK) && (pfile_info_internal != NULL))
  {
    *pfile_info_internal = file_info_internal;
  }

  return err;
}



/*
Write info about the ZipFile in the *pglobal_info structure.
No preparation of the structure is needed
return UNZ_OK if there is no problem.
 */

extern int ZEXPORT unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info, TCHAR *szFileName, DWORD fileNameBufferSize, void *extraField, DWORD extraFieldBufferSize, TCHAR *szComment, DWORD commentBufferSize)
{
#ifdef UNICODE
    return unzGetCurrentFileInfoW(file, pfile_info, szFileName, fileNameBufferSize, extraField, extraFieldBufferSize, szComment, commentBufferSize);
#else
    return unzGetCurrentFileInfoA(file, pfile_info, szFileName, fileNameBufferSize, extraField, extraFieldBufferSize, szComment, commentBufferSize);
#endif
}

extern int ZEXPORT unzGetCurrentFileInfoA(unzFile file, unz_file_info *pfile_info, char *szFileName, DWORD fileNameBufferSize, void *extraField, DWORD extraFieldBufferSize, char *szComment, DWORD commentBufferSize)
{
  return unzlocal_GetCurrentFileInfoInternal(file, pfile_info, NULL, szFileName, fileNameBufferSize, extraField, extraFieldBufferSize, szComment, commentBufferSize);
}

extern int ZEXPORT unzGetCurrentFileInfoW(unzFile file, unz_file_info *pfile_info, wchar_t *szFileName, DWORD fileNameBufferSize, void *extraField, DWORD extraFieldBufferSize, wchar_t *szComment, DWORD commentBufferSize)
{
    char  szFileNameA[_MAX_PATH];
    char  szCommentA[_MAX_PATH];
    // ���ַ�ת���ɶ��ַ��ļ����ַ���
    WideCharToMultiByte(
        CP_ACP, 
        0, 
        szFileName,
        (INT)wcslen(szFileName) + 1,
        szFileNameA,
        MAX_PATH,
        NULL,
        NULL);
    WideCharToMultiByte(
        CP_ACP, 
        0, 
        szComment,
        (INT)wcslen(szComment) + 1,
        szCommentA,
        MAX_PATH,
        NULL,
        NULL);
    return unzGetCurrentFileInfoA(file, pfile_info, szFileNameA, fileNameBufferSize, extraField, extraFieldBufferSize, szCommentA, commentBufferSize);
}

/*
Set the current file of the zipfile to the first file.
return UNZ_OK if there is no problem
 */

extern int ZEXPORT unzGoToFirstFile(unzFile file)
{
  int err = UNZ_OK;
  unz_s *s;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;
  s->pos_in_central_dir = s->offset_central_dir;
  s->num_file = 0;
  err = unzlocal_GetCurrentFileInfoInternal(file, &s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0);
  s->current_file_ok = (err == UNZ_OK);
  return err;
}

/*
Set the current file of the zipfile to the next file.
return UNZ_OK if there is no problem
return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
 */

extern int ZEXPORT unzGoToNextFile(unzFile file)
{
  unz_s *s;
  int err;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;

  if (!s->current_file_ok)
  {
    return UNZ_END_OF_LIST_OF_FILE;
  }

  if (s->gi.number_entry != 0xffff)
  {
    /* 2^16 files overflow hack */
    if (s->num_file + 1 == s->gi.number_entry)
    {
      return UNZ_END_OF_LIST_OF_FILE;
    }
  }

  s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename + s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment;
  s->num_file++;
  err = unzlocal_GetCurrentFileInfoInternal(file, &s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0);
  s->current_file_ok = (err == UNZ_OK);
  return err;
}


extern int ZEXPORT unzGetCurrentFileID(unzFile file, DWORD *file_num, DWORD *pos_in_central_dir)
{
  unz_s *s;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;
  *file_num = s->num_file;
  *pos_in_central_dir = s->pos_in_central_dir;
  return UNZ_OK;
}

extern int ZEXPORT unzGoToFileID(unzFile file, DWORD file_num, DWORD pos_in_central_dir)
{
  unz_s *s;
  int err;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }
  
  s = (unz_s*)file;
  s->num_file = file_num;
  s->pos_in_central_dir = pos_in_central_dir;
  err = unzlocal_GetCurrentFileInfoInternal(file, &s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0);
  s->current_file_ok = (err == UNZ_OK);
  return err;
}


/*
Try locate the file szFileName in the zipfile.
For the iCaseSensitivity signification, see unzipStringFileNameCompare

return value :
UNZ_OK if the file is found. It becomes the current file.
UNZ_END_OF_LIST_OF_FILE if the file is not found
 */



//-------------------------------------------------------------------------


/*
// Unzip Helper Functions - should be here?
///////////////////////////////////////////
 */

/*
Read the local header of the current zipfile
Check the coherency of the local header and info in the end of central
directory about this file
store in *piSizeVar the size of extra info in local header
(filename and size of extra field data)
 */

static int unzlocal_CheckCurrentFileCoherencyHeader(unz_s *s, DWORD *piSizeVar, DWORD *poffset_local_extrafield, DWORD *psize_local_extrafield)
{
  DWORD uMagic, uData, uFlags;
  DWORD size_filename;
  DWORD size_extra_field;
  int err = UNZ_OK;

  *piSizeVar = 0;
  *poffset_local_extrafield = 0;
  *psize_local_extrafield = 0;

  if (ZSEEK(s->z_filefunc, s->filestream, s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile, SEEK_SET) != 0)
  {
    return UNZ_ERRNO;
  }


  if (err == UNZ_OK)
  {
    if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uMagic) != UNZ_OK)
    {
      err = UNZ_ERRNO;
    }
    else if (uMagic != 0x04034b50)
    {
      err = UNZ_BADZIPFILE;
    }
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uFlags) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }
  else if ((err == UNZ_OK) && (uData != s->cur_file_info.compression_method))
  {
    err = UNZ_BADZIPFILE;
  }

  if ((err == UNZ_OK) && (s->cur_file_info.compression_method != 0) && (s->cur_file_info.compression_method != Z_DEFLATED))
  {
    err = UNZ_BADZIPFILE;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
  // date/time 
  {
    err = UNZ_ERRNO;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
  // crc 
  {
    err = UNZ_ERRNO;
  }
  else if ((err == UNZ_OK) && (uData != s->cur_file_info.crc) && ((uFlags &8) == 0))
  {
    err = UNZ_BADZIPFILE;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
  // size compr 
  {
    err = UNZ_ERRNO;
  }
  else if ((err == UNZ_OK) && (uData != s->cur_file_info.compressed_size) && ((uFlags &8) == 0))
  {
    err = UNZ_BADZIPFILE;
  }

  if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
  // size uncompr 
  {
    err = UNZ_ERRNO;
  }
  else if ((err == UNZ_OK) && (uData != s->cur_file_info.uncompressed_size) && ((uFlags &8) == 0))
  {
    err = UNZ_BADZIPFILE;
  }

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &size_filename) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }
  else if ((err == UNZ_OK) && (size_filename != s->cur_file_info.size_filename))
  {
    err = UNZ_BADZIPFILE;
  }

  *piSizeVar += (DWORD)size_filename;

  if (unzlocal_getShort(&s->z_filefunc, s->filestream, &size_extra_field) != UNZ_OK)
  {
    err = UNZ_ERRNO;
  }

  *poffset_local_extrafield = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + size_filename;
  *psize_local_extrafield = (DWORD)size_extra_field;

  *piSizeVar += (DWORD)size_extra_field;

  return err;
}

/*
Open for reading data the current file in the zipfile.
If there is no error and the file is opened, the return value is UNZ_OK.
 */

extern int ZEXPORT unzOpenCurrentFile3(unzFile file, int *method, int *level, int raw, const char *password)
{
  int err = UNZ_OK;
  DWORD iSizeVar;
  unz_s *s;
  file_in_zip_read_info_s *pfile_in_zip_read_info;
  DWORD offset_local_extrafield; // offset of the local extra field 
  DWORD size_local_extrafield; // size of the local extra field 

  #ifndef NOUNCRYPT
    char source[12];
  #else
    if (password != NULL)
    {
      return UNZ_PARAMERROR;
    }
  #endif

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }
  s = (unz_s*)file;
  if (!s->current_file_ok)
  {
    return UNZ_PARAMERROR;
  }

  if (s->pfile_in_zip_read != NULL)
  {
    unzCloseCurrentFile(file);
  }

  if (unzlocal_CheckCurrentFileCoherencyHeader(s, &iSizeVar, &offset_local_extrafield, &size_local_extrafield) != UNZ_OK)
  {
    return UNZ_BADZIPFILE;
  }

  pfile_in_zip_read_info = (file_in_zip_read_info_s*)ALLOC(sizeof(file_in_zip_read_info_s));

  if (pfile_in_zip_read_info == NULL)
  {
    return UNZ_INTERNALERROR;
  }

  pfile_in_zip_read_info->read_buffer = (char*)ALLOC(UNZ_BUFSIZE);
  pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
  pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
  pfile_in_zip_read_info->pos_local_extrafield = 0;
  pfile_in_zip_read_info->raw = raw;

  if (pfile_in_zip_read_info->read_buffer == NULL)
  {
    TRYFREE(pfile_in_zip_read_info);
    return UNZ_INTERNALERROR;
  }

  pfile_in_zip_read_info->stream_initialised = 0;

  if (method != NULL)
  {
    *method = (int)s->cur_file_info.compression_method;
  }

  if (level != NULL)
  {
    *level = 6;
    switch (s->cur_file_info.flag &0x06)
    {
      case 6:
        *level = 1;
        break;
      case 4:
        *level = 2;
        break;
      case 2:
        *level = 9;
        break;
    }
  }

  if ((s->cur_file_info.compression_method != 0) && (s->cur_file_info.compression_method != Z_DEFLATED))
  {
    err = UNZ_BADZIPFILE;
  }

  pfile_in_zip_read_info->crc32_wait = s->cur_file_info.crc;
  pfile_in_zip_read_info->crc32 = 0;
  pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
  pfile_in_zip_read_info->filestream = s->filestream;
  pfile_in_zip_read_info->z_filefunc = s->z_filefunc;
  pfile_in_zip_read_info->byte_before_the_zipfile = s->byte_before_the_zipfile;

  pfile_in_zip_read_info->stream.total_out = 0;

  if ((s->cur_file_info.compression_method == Z_DEFLATED) && (!raw))
  {
    pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
    pfile_in_zip_read_info->stream.zfree = (free_func)0;
    pfile_in_zip_read_info->stream.opaque = (void*)0;
    pfile_in_zip_read_info->stream.next_in = (Bytef*)0;
    pfile_in_zip_read_info->stream.avail_in = 0;

    err = inflateInit2(&pfile_in_zip_read_info->stream,  - MAX_WBITS);
    if (err == Z_OK)
    {
      pfile_in_zip_read_info->stream_initialised = 1;
    }
    else
    {
      return err;
    }
    /* windowBits is passed < 0 to tell that there is no zlib header.
     * Note that in this case inflate *requires* an extra "dummy" byte
     * after the compressed stream in order to complete decompression and
     * return Z_STREAM_END.
     * In unzip, i don't wait absolutely Z_STREAM_END because I known the
     * size of both compressed and uncompressed data
     */
  }

  pfile_in_zip_read_info->rest_read_compressed = s->cur_file_info.compressed_size;
  pfile_in_zip_read_info->rest_read_uncompressed = s->cur_file_info.uncompressed_size;


  pfile_in_zip_read_info->pos_in_zipfile = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar;

  pfile_in_zip_read_info->stream.avail_in = (DWORD)0;

  s->pfile_in_zip_read = pfile_in_zip_read_info;

  #ifndef NOUNCRYPT
    if (password != NULL)
    {
      int i;
      s->pcrc_32_tab = get_crc_table();
      init_keys(password, s->keys, s->pcrc_32_tab);
      if (ZSEEK(s->z_filefunc, s->filestream, s->pfile_in_zip_read->pos_in_zipfile + s->pfile_in_zip_read->byte_before_the_zipfile, SEEK_SET) != 0)
      {
        return UNZ_INTERNALERROR;
      }
      if (ZREAD(s->z_filefunc, s->filestream, source, 12) < 12)
      {
        return UNZ_INTERNALERROR;
      }

      for (i = 0; i < 12; i++)
      {
        zdecode(s->keys, s->pcrc_32_tab, source[i]);
      }

      s->pfile_in_zip_read->pos_in_zipfile += 12;
      s->encrypted = 1;
    }
  #endif


  return UNZ_OK;
}

//-------------------------------------------------------------------------

extern int ZEXPORT unzOpenCurrentFile(unzFile file)
{
  return unzOpenCurrentFile3(file, NULL, NULL, 0, NULL);
}

//-------------------------------------------------------------------------

extern int ZEXPORT unzOpenCurrentFilePassword(unzFile file, const TCHAR *password)
{
#ifdef UNICODE
    return unzOpenCurrentFilePasswordW(file, password);
#else
    return unzOpenCurrentFilePasswordA(file, password);
#endif
}


extern int ZEXPORT unzOpenCurrentFilePasswordA(unzFile file, const char *password)
{
  return unzOpenCurrentFile3(file, NULL, NULL, 0, password);
}

extern int ZEXPORT unzOpenCurrentFilePasswordW(unzFile file, const wchar_t *password)
{
    char  passwordA[_MAX_PATH];
    // ���ַ�ת���ɶ��ַ��ļ����ַ���
    WideCharToMultiByte(
        CP_ACP, 
        0, 
        password,
        (INT)wcslen(password) + 1,
        passwordA,
        MAX_PATH,
        NULL,
        NULL);
    return unzOpenCurrentFilePasswordA(file, passwordA);
}

//-------------------------------------------------------------------------

/*
Read bytes from the current file.
buf contain buffer where data must be copied
len the size of buf.

return the number of byte copied if somes bytes are copied
return 0 if the end of file was reached
return <0 with error code if there is an error
(UNZ_ERRNO for IO error, or zLib error for uncompress error)
 */
extern int ZEXPORT unzReadCurrentFile(unzFile file, void *buf, DWORD len)
{
  int err = UNZ_OK;
  DWORD iRead = 0;
  unz_s *s;

  file_in_zip_read_info_s *pfile_in_zip_read_info;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;
  pfile_in_zip_read_info = s->pfile_in_zip_read;

  if (pfile_in_zip_read_info == NULL)
  {
    return UNZ_PARAMERROR;
  }

  if ((pfile_in_zip_read_info->read_buffer == NULL))
  {
    return UNZ_END_OF_LIST_OF_FILE;
  }

  if (len == 0)
  {
    return 0;
  }

  pfile_in_zip_read_info->stream.next_out = (BYTE*)buf;
  pfile_in_zip_read_info->stream.avail_out = (DWORD)len;

  if (len > pfile_in_zip_read_info->rest_read_uncompressed)
  {
    pfile_in_zip_read_info->stream.avail_out = (DWORD)pfile_in_zip_read_info->rest_read_uncompressed;
  }

  while (pfile_in_zip_read_info->stream.avail_out > 0)
  {
    if ((pfile_in_zip_read_info->stream.avail_in == 0) && (pfile_in_zip_read_info->rest_read_compressed > 0))
    {
      DWORD uReadThis = UNZ_BUFSIZE;
      if (pfile_in_zip_read_info->rest_read_compressed < uReadThis)
      {
        uReadThis = (DWORD)pfile_in_zip_read_info->rest_read_compressed;
      }
      if (uReadThis == 0)
      {
        return UNZ_EOF;
      }
      if (ZSEEK(pfile_in_zip_read_info->z_filefunc, pfile_in_zip_read_info->filestream, pfile_in_zip_read_info->pos_in_zipfile + pfile_in_zip_read_info->byte_before_the_zipfile, SEEK_SET) != 0)
      {
        return UNZ_ERRNO;
      }
      if (ZREAD(pfile_in_zip_read_info->z_filefunc, pfile_in_zip_read_info->filestream, pfile_in_zip_read_info->read_buffer, uReadThis) != uReadThis)
      {
        return UNZ_ERRNO;
      }

      #ifndef NOUNCRYPT
        if (s->encrypted)
        {
          DWORD i;
          for (i = 0; i < uReadThis; i++)
          {
            pfile_in_zip_read_info->read_buffer[i] = zdecode(s->keys, s->pcrc_32_tab, pfile_in_zip_read_info->read_buffer[i]);
          }
        }
      #endif

      pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

      pfile_in_zip_read_info->rest_read_compressed -= uReadThis;

      pfile_in_zip_read_info->stream.next_in = (BYTE*)pfile_in_zip_read_info->read_buffer;
      pfile_in_zip_read_info->stream.avail_in = (DWORD)uReadThis;
    }

    if ((pfile_in_zip_read_info->compression_method == 0) || (pfile_in_zip_read_info->raw))
    {
      DWORD uDoCopy, i;

      if ((pfile_in_zip_read_info->stream.avail_in == 0) && (pfile_in_zip_read_info->rest_read_compressed == 0))
      {
        return (iRead == 0) ? UNZ_EOF : iRead;
      }

      if (pfile_in_zip_read_info->stream.avail_out < pfile_in_zip_read_info->stream.avail_in)
      {
        uDoCopy = pfile_in_zip_read_info->stream.avail_out;
      }
      else
      {
        uDoCopy = pfile_in_zip_read_info->stream.avail_in;
      }

      for (i = 0; i < uDoCopy; i++)
      {
        *(pfile_in_zip_read_info->stream.next_out + i) = *(pfile_in_zip_read_info->stream.next_in + i);
      }

      pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32, pfile_in_zip_read_info->stream.next_out, uDoCopy);
      pfile_in_zip_read_info->rest_read_uncompressed -= uDoCopy;
      pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
      pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
      pfile_in_zip_read_info->stream.next_out += uDoCopy;
      pfile_in_zip_read_info->stream.next_in += uDoCopy;
      pfile_in_zip_read_info->stream.total_out += uDoCopy;
      iRead += uDoCopy;
    }
    else
    {
      DWORD uTotalOutBefore, uTotalOutAfter;
      const BYTE *bufBefore;
      DWORD uOutThis;
      int flush = Z_SYNC_FLUSH;

      uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
      bufBefore = pfile_in_zip_read_info->stream.next_out;

      err = inflate(&pfile_in_zip_read_info->stream, flush);

      uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
      uOutThis = uTotalOutAfter - uTotalOutBefore;

      pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32, bufBefore, (DWORD)(uOutThis));

      pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;

      iRead += (DWORD)(uTotalOutAfter - uTotalOutBefore);

      if (err == Z_STREAM_END)
      {
        return (iRead == 0) ? UNZ_EOF : iRead;
      }

      if (err != Z_OK)
      {
        break;
      }
    }
  }

  if (err == Z_OK)
  {
    return iRead;
  }
  return err;
}


/*
Give the current position in uncompressed data
 */
/*
extern z_off_t ZEXPORT unztell(unzFile file)
{
  unz_s *s;
  file_in_zip_read_info_s *pfile_in_zip_read_info;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;
  pfile_in_zip_read_info = s->pfile_in_zip_read;

  if (pfile_in_zip_read_info == NULL)
  {
    return UNZ_PARAMERROR;
  }

  return (z_off_t)pfile_in_zip_read_info->stream.total_out;
}
*/

/*
return 1 if the end of file was reached, 0 elsewhere
 */
/*
extern int ZEXPORT unzeof(unzFile file)
{
  unz_s *s;
  file_in_zip_read_info_s *pfile_in_zip_read_info;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;
  pfile_in_zip_read_info = s->pfile_in_zip_read;

  if (pfile_in_zip_read_info == NULL)
  {
    return UNZ_PARAMERROR;
  }

  if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
*/


/*
Read extra field from the current file (opened by unzOpenCurrentFile)
This is the local-header version of the extra field (sometimes, there is
more info in the local-header version than in the central-header)

if buf==NULL, it return the size of the local extra field that can be read

if buf!=NULL, len is the size of the buffer, the extra header is copied in
buf.
the return value is the number of bytes copied in buf, or (if <0)
the error code
 */
/*
extern int ZEXPORT unzGetLocalExtrafield(unzFile file, void *buf, DWORD len)
{
  unz_s *s;
  file_in_zip_read_info_s *pfile_in_zip_read_info;
  DWORD read_now;
  DWORD size_to_read;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;

  pfile_in_zip_read_info = s->pfile_in_zip_read;

  if (pfile_in_zip_read_info == NULL)
  {
    return UNZ_PARAMERROR;
  }

  size_to_read = (pfile_in_zip_read_info->size_local_extrafield - pfile_in_zip_read_info->pos_local_extrafield);

  if (buf == NULL)
  {
    return (int)size_to_read;
  }

  if (len > size_to_read)
  {
    read_now = (DWORD)size_to_read;
  }
  else
  {
    read_now = (DWORD)len;
  }

  if (read_now == 0)
  {
    return 0;
  }

  if (ZSEEK(pfile_in_zip_read_info->z_filefunc, pfile_in_zip_read_info->filestream, pfile_in_zip_read_info->offset_local_extrafield + pfile_in_zip_read_info->pos_local_extrafield, SEEK_SET) != 0)
  {
    return UNZ_ERRNO;
  }

  if (ZREAD(pfile_in_zip_read_info->z_filefunc, pfile_in_zip_read_info->filestream, buf, read_now) != read_now)
  {
    return UNZ_ERRNO;
  }

  return (int)read_now;
}
*/
/*
Close the file in zip opened with unzipOpenCurrentFile
Return UNZ_CRCERROR if all the file was read but the CRC is not good
 */

extern int ZEXPORT unzCloseCurrentFile(unzFile file)
{
  int err = UNZ_OK;

  unz_s *s;
  file_in_zip_read_info_s *pfile_in_zip_read_info;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;
  pfile_in_zip_read_info = s->pfile_in_zip_read;

  if (pfile_in_zip_read_info == NULL)
  {
    return UNZ_PARAMERROR;
  }

  if ((pfile_in_zip_read_info->rest_read_uncompressed == 0) && (!pfile_in_zip_read_info->raw))
  {
    if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
    {
      err = UNZ_CRCERROR;
    }
  }

  TRYFREE(pfile_in_zip_read_info->read_buffer);
  pfile_in_zip_read_info->read_buffer = NULL;

  if (pfile_in_zip_read_info->stream_initialised)
  {
    inflateEnd(&pfile_in_zip_read_info->stream);
  }

  pfile_in_zip_read_info->stream_initialised = 0;
  TRYFREE(pfile_in_zip_read_info);

  s->pfile_in_zip_read = NULL;

  return err;
}


/*
Get the global comment string of the ZipFile, in the szComment buffer.
uSizeBuf is the size of the szComment buffer.
return the number of byte copied or an error code <0
 */
/*
extern int ZEXPORT unzGetGlobalComment(unzFile file, char *szComment, DWORD uSizeBuf)
{
  //int err = UNZ_OK;
  unz_s *s;
  DWORD uReadThis;

  if (file == NULL)
  {
    return UNZ_PARAMERROR;
  }

  s = (unz_s*)file;

  uReadThis = uSizeBuf;

  if (uReadThis > s->gi.size_comment)
  {
    uReadThis = s->gi.size_comment;
  }

  if (ZSEEK(s->z_filefunc, s->filestream, s->central_pos + 22, SEEK_SET) != 0)
  {
    return UNZ_ERRNO;
  }

  if (uReadThis > 0)
  {
    *szComment = '\0';

    if (ZREAD(s->z_filefunc, s->filestream, szComment, uReadThis) != uReadThis)
    {
      return UNZ_ERRNO;
    }
  }

  if ((szComment != NULL) && (uSizeBuf > s->gi.size_comment))
  {
    *(szComment + s->gi.size_comment) = '\0';
  }
  return (int)uReadThis;
}
*/
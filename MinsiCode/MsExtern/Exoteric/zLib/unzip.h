/* unzip.h -- IO for uncompress .zip files using zlib
Version 1.01, May 8th, 2004

Copyright (C) 1998-2004 Gilles Vollant

This unzip package allow extract file from .ZIP file, compatible with PKZip 2.04g
WinZip, InfoZip tools and compatible.
Encryption and multi volume ZipFile (span) are not supported.
Old compressions used by old PKZip 1.x are not supported


I WAIT FEEDBACK at mail info@winimage.com
Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution

Condition of use and distribution are the same than zlib :

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.


 */

/* for more info about .ZIP format, see
http://www.info-zip.org/pub/infozip/doc/appnote-981119-iz.zip
http://www.info-zip.org/pub/infozip/doc/
PkWare has also a specification at :
ftp://ftp.pkware.com/probdesc.zip
 */

#ifndef _unz_H
  #define _unz_H

  #ifdef __cplusplus
    extern "C"
    {
    #endif

    #ifndef _ZLIB_H
      #include "zlib.h"
    #endif

    #ifndef _ZLIBIOAPI_H
      #include "ioapi.h"
    #endif

    #if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
      /* like the STRICT of WIN32, we define a pointer that cannot be converted
      from (void*) without cast */
      typedef struct TagunzFile__
      {
        int unused;
      } unzFile__;
      typedef unzFile__ *unzFile;
    #else
      typedef void *unzFile;
    #endif


    #define UNZ_OK                          (0)
    #define UNZ_END_OF_LIST_OF_FILE         (-100)
    #define UNZ_ERRNO                       (Z_ERRNO)
    #define UNZ_EOF                         (0)
    #define UNZ_PARAMERROR                  (-102)
    #define UNZ_BADZIPFILE                  (-103)
    #define UNZ_INTERNALERROR               (-104)
    #define UNZ_CRCERROR                    (-105)

    /* tm_unz contain date/time info */
    typedef struct tm_unz_s
    {
      DWORD tm_sec; /* seconds after the minute - [0,59] */
      DWORD tm_min; /* minutes after the hour - [0,59] */
      DWORD tm_hour; /* hours since midnight - [0,23] */
      DWORD tm_mday; /* day of the month - [1,31] */
      DWORD tm_mon; /* months since January - [0,11] */
      DWORD tm_year; /* years - [1980..2044] */
    } tm_unz;

    /* unz_global_info structure contain global data about the ZIPfile
    These data comes from the end of central dir */
    typedef struct unz_global_info_s
    {
      DWORD number_entry; /* total number of entries in
      the central dir on this disk */
      DWORD size_comment; /* size of the global comment of the zipfile */
    } unz_global_info;


    /* unz_file_info contain information about a file in the zipfile */
    typedef struct unz_file_info_s
    {
      DWORD version; /* version made by                 2 bytes */
      DWORD version_needed; /* version needed to extract       2 bytes */
      DWORD flag; /* general purpose bit flag        2 bytes */
      DWORD compression_method; /* compression method              2 bytes */
      DWORD dosDate; /* last mod file date in Dos fmt   4 bytes */
      DWORD crc; /* crc-32                          4 bytes */
      DWORD compressed_size; /* compressed size                 4 bytes */
      DWORD uncompressed_size; /* uncompressed size               4 bytes */
      DWORD size_filename; /* filename length                 2 bytes */
      DWORD size_file_extra; /* extra field length              2 bytes */
      DWORD size_file_comment; /* file comment length             2 bytes */

      DWORD disk_num_start; /* disk number start               2 bytes */
      DWORD internal_fa; /* internal file attributes        2 bytes */
      DWORD external_fa; /* external file attributes        4 bytes */

      tm_unz tmu_date;
    } unz_file_info;

    extern int ZEXPORT unzStringFileNameCompare OF((const char *fileName1, const char *fileName2, int iCaseSensitivity));
    /*
    Compare two filename (fileName1,fileName2).
    If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
    If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
    or strcasecmp)
    If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
    (like 1 on Unix, 2 on Windows)
     */

    extern unzFile ZEXPORT unzOpen(const TCHAR *path);
    extern unzFile ZEXPORT unzOpenA(const char *path);
    extern unzFile ZEXPORT unzOpenW(const wchar_t *path);
        /*
    Open a Zip file. path contain the full pathname (by example,
    on a Windows XP computer "c:\\zlib\\zlib113.zip" or on an Unix computer
    "zlib/zlib113.zip".
    If the zipfile cannot be opened (file don't exist or in not valid), the
    return value is NULL.
    Else, the return value is a unzFile Handle, usable with other function
    of this unzip package.
     */

    extern unzFile ZEXPORT unzOpen2(const char *path, zlib_filefunc_def *pzlib_filefunc_def);
    /*
    Open a Zip file, like unzOpen, but provide a set of file low level API
    for read/write the zip file (see ioapi.h)
     */

    extern int ZEXPORT unzClose(unzFile file);
    /*
    Close a ZipFile opened with unzipOpen.
    If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
    return UNZ_OK if there is no problem. */

    extern int ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info *pglobal_info);
    /*
    Write info about the ZipFile in the *pglobal_info structure.
    No preparation of the structure is needed
    return UNZ_OK if there is no problem. */


    extern int ZEXPORT unzGetGlobalComment(unzFile file, char *szComment, DWORD uSizeBuf);
    /*
    Get the global comment string of the ZipFile, in the szComment buffer.
    uSizeBuf is the size of the szComment buffer.
    return the number of byte copied or an error code <0
     */


    /***************************************************************************/
    /* Unzip package allow you browse the directory of the zipfile */

    extern int ZEXPORT unzGoToFirstFile(unzFile file);
    /*
    Set the current file of the zipfile to the first file.
    return UNZ_OK if there is no problem
     */

    extern int ZEXPORT unzGoToNextFile(unzFile file);
    /*
    Set the current file of the zipfile to the next file.
    return UNZ_OK if there is no problem
    return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
     */

    extern int ZEXPORT unzLocateFile(unzFile file, const char *szFileName, int iCaseSensitivity);
    /*
    Try locate the file szFileName in the zipfile.
    For the iCaseSensitivity signification, see unzStringFileNameCompare

    return value :
    UNZ_OK if the file is found. It becomes the current file.
    UNZ_END_OF_LIST_OF_FILE if the file is not found
     */

    extern int ZEXPORT unzGetCurrentFileID(unzFile file, DWORD *file_num, DWORD *pos_in_central_dir);
    extern int ZEXPORT unzGoToFileID(unzFile file, DWORD file_num, DWORD pos_in_central_dir);

    /* ****************************************** */
    /* Ryan supplied functions */
    /* unz_file_info contain information about a file in the zipfile */
    typedef struct unz_file_pos_s
    {
      DWORD pos_in_zip_directory; /* offset in zip file directory */
      DWORD num_of_file; /* # of file */
    } unz_file_pos;

    extern int ZEXPORT unzGetFilePos(unzFile file, unz_file_pos *file_pos);

    extern int ZEXPORT unzGoToFilePos(unzFile file, unz_file_pos *file_pos);

    /* ****************************************** */

    extern int ZEXPORT unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info, TCHAR *szFileName, DWORD fileNameBufferSize, void *extraField, DWORD extraFieldBufferSize, TCHAR *szComment, DWORD commentBufferSize);
    extern int ZEXPORT unzGetCurrentFileInfoA(unzFile file, unz_file_info *pfile_info, char *szFileName, DWORD fileNameBufferSize, void *extraField, DWORD extraFieldBufferSize, char *szComment, DWORD commentBufferSize);
    extern int ZEXPORT unzGetCurrentFileInfoW(unzFile file, unz_file_info *pfile_info, wchar_t *szFileName, DWORD fileNameBufferSize, void *extraField, DWORD extraFieldBufferSize, wchar_t *szComment, DWORD commentBufferSize);
    /*
    Get Info about the current file
    if pfile_info!=NULL, the *pfile_info structure will contain somes info about
    the current file
    if szFileName!=NULL, the filemane string will be copied in szFileName
    (fileNameBufferSize is the size of the buffer)
    if extraField!=NULL, the extra field information will be copied in extraField
    (extraFieldBufferSize is the size of the buffer).
    This is the Central-header version of the extra field
    if szComment!=NULL, the comment string of the file will be copied in szComment
    (commentBufferSize is the size of the buffer)
     */

    /***************************************************************************/
    /* for reading the content of the current zipfile, you can open it, read data
    from it, and close it (you can close it before reading all the file)
     */

    extern int ZEXPORT unzOpenCurrentFile(unzFile file);
    /*
    Open for reading data the current file in the zipfile.
    If there is no error, the return value is UNZ_OK.
     */

    extern int ZEXPORT unzOpenCurrentFilePassword(unzFile file, const TCHAR *password);
    extern int ZEXPORT unzOpenCurrentFilePasswordA(unzFile file, const char *password);
    extern int ZEXPORT unzOpenCurrentFilePasswordW(unzFile file, const wchar_t *password);
        /*
    Open for reading data the current file in the zipfile.
    password is a crypting password
    If there is no error, the return value is UNZ_OK.
     */

    extern int ZEXPORT unzOpenCurrentFile2(unzFile file, int *method, int *level, int raw);
    /*
    Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
     *method will receive method of compression, *level will receive level of
    compression
    note : you can set level parameter as NULL (if you did not want known level,
    but you CANNOT set method parameter as NULL
     */

    extern int ZEXPORT unzOpenCurrentFile3(unzFile file, int *method, int *level, int raw, const char *password);
    /*
    Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
     *method will receive method of compression, *level will receive level of
    compression
    note : you can set level parameter as NULL (if you did not want known level,
    but you CANNOT set method parameter as NULL
     */


    extern int ZEXPORT unzCloseCurrentFile(unzFile file);
    /*
    Close the file in zip opened with unzOpenCurrentFile
    Return UNZ_CRCERROR if all the file was read but the CRC is not good
     */

    extern int ZEXPORT unzReadCurrentFile(unzFile file, void *buf, DWORD len);
    /*
    Read bytes from the current file (opened by unzOpenCurrentFile)
    buf contain buffer where data must be copied
    len the size of buf.

    return the number of byte copied if somes bytes are copied
    return 0 if the end of file was reached
    return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
     */

    extern z_off_t ZEXPORT unztell(unzFile file);
    /*
    Give the current position in uncompressed data
     */

    extern int ZEXPORT unzeof(unzFile file);
    /*
    return 1 if the end of file was reached, 0 elsewhere
     */

    extern int ZEXPORT unzGetLocalExtrafield(unzFile file, void *buf, DWORD len);
    /*
    Read extra field from the current file (opened by unzOpenCurrentFile)
    This is the local-header version of the extra field (sometimes, there is
    more info in the local-header version than in the central-header)

    if buf==NULL, it return the size of the local extra field

    if buf!=NULL, len is the size of the buffer, the extra header is copied in
    buf.
    the return value is the number of bytes copied in buf, or (if <0)
    the error code
     */

    /***************************************************************************/

    /* Get the current file offset */
    extern DWORD ZEXPORT unzGetOffset(unzFile file);

    /* Set the current file offset */
    extern int ZEXPORT unzSetOffset(unzFile file, DWORD pos);



    #ifdef __cplusplus
    }
  #endif

#endif /* _unz_H */

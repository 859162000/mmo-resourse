///////////////////////////////////////////////////////////
//  CStrConverter.cpp
//  实现对象：	Class CStrConverter
//  创建日期：	06-9月-2006 10:37:37
//  原始作者：	陈先进
///////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "CStrConverter.h"

/*
*功能：将以十六进制字符串转化为long
*参数：val需要转化的字符串
*返回值：转化的十进制数
*/		
long CStrConverter::parseStringToDec(const char* val)
{
	int len = (int)strlen(val);
	int i = 0;
	long sum=0;
	int Temp_TiElem = 0;

	while(i<len)
	{
		char t = val[len-i-1];
		if(t>='a' && t<='f')
		{
			Temp_TiElem = t - 87;
		}
		else
		{
			Temp_TiElem = (int)t - 48;
		}	

		sum += Temp_TiElem<<(4*i);					
		++i;
	}
	return sum;
}
/*
*功能：将以字符串转化为float
*参数：val需要转化的字符串
*返回值：转化的float
*/

float CStrConverter::parseFloat(const char* val)
{
	//float f = atof(val);
	return (float)atof(val);
}
/*
*功能：将以字符串转化为long
*参数：val需要转化的字符串
*返回值：转化的int
*/			
long  CStrConverter::parseLong(const char* val)
{
	/*int i;
	i = strlen(val);

	if(val[0] == '-')
	{
       return 
	}
	else
	{*/
		return atol(val);
	//}
	
}



/*
*功能：将以字符串转化为int
*参数：val需要转化的字符串
*返回值：转化的int
*/			
int  CStrConverter::parseInt(const char* val)
{
	return atoi(val);
}

/*
*功能：将以排列方式字符串转化为long
*参数：val需要转化的字符串
*返回值：转化的long值
*/
long CStrConverter::parseRankStyle(const char* val)
{                                            

	if(!strcmp(val,"ALIGNMENT_HORIZONTAL_LEFT|ALIGNMENT_VERTICAL_TOP"))
	{
		return 0;
	}
	else if(!strcmp(val,"ALIGNMENT_HORIZONTAL_LEFT|ALIGNMENT_VERTICAL_CENTER"))
	{
		return 4;
	}                
	else if(!strcmp(val,"ALIGNMENT_HORIZONTAL_LEFT|ALIGNMENT_VERTICAL_BOTTOM"))
	{
		return 8;
	}
	if(!strcmp(val,"ALIGNMENT_HORIZONTAL_CENTER|ALIGNMENT_VERTICAL_TOP"))
	{
		return 1;
	}
	else if(!strcmp(val,"ALIGNMENT_HORIZONTAL_CENTER|ALIGNMENT_VERTICAL_CENTER"))
	{
		return 5;
	}                
	else if(!strcmp(val,"ALIGNMENT_HORIZONTAL_CENTER|ALIGNMENT_VERTICAL_BOTTOM"))
	{
		return 9;
	}
	if(!strcmp(val,"ALIGNMENT_HORIZONTAL_RIGHT|ALIGNMENT_VERTICAL_TOP"))
	{
		return 2;
	}
	else if(!strcmp(val,"ALIGNMENT_HORIZONTAL_RIGHT|ALIGNMENT_VERTICAL_CENTER"))
	{
		return 6;
	}  
	else if(!strcmp(val,"ALIGNMENT_HORIZONTAL_RIGHT|ALIGNMENT_VERTICAL_BOTTOM"))
	{
		return 10;
	} 
	else 
	{
		return 0;
	}


}
/*
*功能：将以字符串转化为bool
*参数：val需要转化的字符串
*返回值：转化的bool
*/
bool CStrConverter::parseBool(const char* val)
{
	if(!strcmp(val,"true") || !strcmp(val,"true"))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CStrConverter::parseList(char* val,list<long>& lTemp)
{
	if (strlen(val)>0)
	{
		char *pCh;
		pCh = strtok(val,",");
		lTemp.push_back(atoi(pCh));
		while (pCh!=NULL)
		{
			pCh = strtok(NULL,",");
			if (pCh==NULL)
			{
				break;
			}
			lTemp.push_back(atoi(pCh));			
		}
	}

}
const char *CStrConverter::ParseBoolToString(const bool b)
{
	return b==true ? "true":"false";
}

string CStrConverter::GetLeftSplitStr(const std::string& val,size_t SplitPos,string& SplitStrRight)
{
	//截断区间为[1,val.length-1]
	if(SplitPos<1 || SplitPos >= val.length())
	{
		SplitStrRight="";
		return val;
	}
	if(!CanSplit(val,SplitPos))
	{
		SplitPos--;
	}
	SplitStrRight = val.substr(SplitPos+1);
	return val.substr(0,SplitPos+1);
}

bool CStrConverter::CanSplit(const std::string& str,size_t pos)
{
	if(pos<1 || pos>=str.length())
		return false;
	if((uchar)str[pos] <127 )
	{
		return true;
	}
	int c=1;
	for(size_t i= pos -1; (uchar) str[i] >=127 && i>=0;--i)
	{
		++c;
	}
	return c % 2 ==0;
}

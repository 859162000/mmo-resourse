// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif						

#include <stdio.h>
#include <tchar.h>

#include <process.h>
#include <Windows.h>
#include <assert.h>
#include "time.h"

#include <string>
#include <fstream>
#include <list>
#include <map>
#include <set>
#include <memory>


#include <vector>
using namespace std;


#define SHRINK_SCALE 8
#include "..\MemManage\MemTools.h"



class BBB
{
public:
	char arr[128];
	string str;
};


// TODO: 在此处引用程序需要的其他头文件

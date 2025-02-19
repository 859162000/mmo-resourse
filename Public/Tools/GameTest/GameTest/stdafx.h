// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 项目特定的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 从 Windows 标头中排除不常使用的资料
#endif

// 如果您必须使用下列所指定的平台之前的平台，则修改下面的定义。
// 有关不同平台的相应值的最新信息，请参考 MSDN。
#ifndef WINVER				// 允许使用 Windows 95 和 Windows NT 4 或更高版本的特定功能。
#define WINVER 0x0500		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif

#ifndef _WIN32_WINNT		// 允许使用 Windows NT 4 或更高版本的特定功能。
#define _WIN32_WINNT 0x0500		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif						

#ifndef _WIN32_WINDOWS		// 允许使用 Windows 98 或更高版本的特定功能。
#define _WIN32_WINDOWS 0x0410 //为 Windows Me 及更新版本改变为适当的值。
#endif

#ifndef _WIN32_IE			// 允许使用 IE 4.0 或更高版本的特定功能。
#define _WIN32_IE 0x0400	//为 IE 5.0 及更新版本改变为适当的值。
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常被安全忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心和标准组件
#include <afxext.h>         // MFC 扩展
#include <afxdisp.h>        // MFC 自动化类

#include <afxdtctl.h>		// Internet Explorer 4 公共控件的 MFC 支持
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// Windows 公共控件的 MFC 支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "lua.hpp"
#include "../../../DefPublic.h"
#include "../../../ServerPublic/ServerPublic.h"
#include "../../../Nets/MemServer.h"
#include <process.h>
#include "../../../Common/GoodsDef.h"
#include "../../../Common/PlayerDef.h"
#include "../../../Common/MonsterDef.h"
#include "../../../Common/DepotDef.h"
#include "../../../Common/OrganizingDef.h"
#include "../../../Common/BounsDef.h"
#include "../../../Common/SkillRelated.h"

#include "../Utility/PlayerDef.h"
#include "../Utility/GoodsExDef.h"

// 标准模板库
#include <list>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <deque>
using namespace std;

#include <hash_map>
using namespace stdext;

#pragma warning(disable:4244)


//color define
#define RED			RGB(127,0,0)
#define GREEN		RGB(0,127,0)
#define BLUE		RGB(0,0,127)
#define LIGHTRED	RGB(255,0,0)
#define LIGHTGREEN  RGB(0,255,0)
#define LIGHTBLUE   RGB(0,0,255)
#define BLACK		RGB(0,0,0)
#define	WHITE		RGB(255,255,255)
#define	GRAY		RGB(240,240,240)

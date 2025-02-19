#include "StdAfx.h"
#include <sstream>

#include ".\game.h"
#include "..\nets\Servers.h"
#include "..\nets\ServerClient.h"
#include "..\nets\netlog\Message.h"

#include "..\public\WordsFilter.h"
#include "..\public\dupliregionsetup.h"
#include "..\public\DataBlockAllocator.h"
#include "..\..\..\dbaccess\MyAdoBase.h"
#include "../LogApp/LogManage.h"
#include "..\LogApp\SMClient.h"
#include "..\public\Singleton.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const DWORD g_ms = 40;

// 调试用
DWORD g_lCurTime = 0;
DWORD g_lLSMessageTime = 0;
DWORD g_lGSMessageTime = 0;
DWORD g_lCSMessageTime = 0;

CGame* g_pGame= NULL;		// 游戏客户端数据

//_ConnectionPtr g_Connection = NULL;		// ADO主连接

HANDLE g_hGameThreadExitEvent=NULL;		// 主线程退出事件
bool g_bGameThreadExit=false;		// 是否退出主线程

HANDLE	hConnectThread=NULL;			// 连接线程句柄 
bool	bConnectThreadExit=false;		// 是否退出连接线程

HANDLE hConnectThread1=NULL;
bool	bConnectThreadExit1=false;

vector<HANDLE> g_vPlayerLoadThreads;		// 玩家读数据库线程
bool volatile g_bPlayerLoadThreadsExit = false;


HANDLE g_hSavingThread = NULL; // 存盘线程句柄

// 默认不显示存盘信息
extern bool g_bShowSaveInfo = false;
void ShowSaveInfo(LPCSTR szInfo, ...)
{
	if(g_bShowSaveInfo)
	{
		char str[256];
		va_list va;
		va_start(va,szInfo);
		vsprintf(str,szInfo,va);
		va_end(va);

		AddLogText(const_cast<char*>(str));
	}
}

unsigned volatile RELOAD_PROFILE_FLAGS = 0x0;	// 重度配置文件标示
// define const


// end define

void reload_profiles()
{
	// 重读旗标检测
	if( !RELOAD_PROFILE_FLAGS ) return;

	// 做位检测，是否需要重读，结果为1时，并清除标志位。
#define TEST(x,y) \
	(x)&(y)?((x)&=~(y)),1:0

#undef TEST
}


// 创建游戏客户端数据
BOOL CreateGame()
{
	g_pGame = new CGame;
	return true;
}

// 释放游戏客户端数据
BOOL DeleteGame()
{
	if (g_pGame)
	{
		delete g_pGame;
		g_pGame = NULL;
	}
	return true;
}

// 获取游戏客户端数据
CGame* GetGame()
{
	return g_pGame;
}


CGame::CGame(void)
:m_pGameLogManage(NULL)
,m_AsynchronousExecute(TRUE)
,m_bCreateTable(FALSE)
,m_pDBAllocator(NULL)
,m_dwtick(0)
{
	s_pNetServer = NULL;

	GetSetup()->strName = "LogServer";
	GetSetup()->dwListenPort = 8100;	
}

CGame::~CGame(void)
{
}

bool CGame::LoadServerResource()
{
	CHAR* strPath = new CHAR[MAX_PATH];
	string strWorpath("");
	if(::GetCurrentDirectory(MAX_PATH,strPath))
	{
		strWorpath=string(strPath);
	}
	delete []strPath;

	// 初始化数据包
	extern CClientResource * g_pDefaultClientResource;
	SAFE_DELETE(g_pDefaultClientResource);
	string strFilesInfo("FilesInfo.ril");
	g_pDefaultClientResource = new CClientResource(GAME_RES,strWorpath,strFilesInfo);
	g_pDefaultClientResource->LoadEx();
	AddLogText("Load package file OK!");
	return true;
}

bool CGame::LoadSetup()
{
	// 读取全局配置
	string strTemp;
	ifstream stream;
	stream.open("setup.ini");
	if( stream.is_open() )
	{
		stream  >> strTemp >> GetSetup()->dwNumber
			>> strTemp >> GetSetup()->strName
			>> strTemp >> GetSetup()->dwListenPort
			>> strTemp >> GetSetup()->dwDataBlockNum
			>> strTemp >> GetSetup()->dwDataBlockSize
			>> strTemp >> GetSetup()->dwFreeMsgNum

			>> strTemp >> GetSetup()->dwFreeSockOperNum
			>> strTemp >> GetSetup()->dwFreeIOOperNum
			>> strTemp >> GetSetup()->lIOOperDataBufNum

			>> strTemp >> GetSetup()->bCheckNet
			>> strTemp >> GetSetup()->dwBanIPTime
			>> strTemp >> GetSetup()->dwMaxMsgLen			
			>> strTemp >> GetSetup()->lMaxConnectNum
			>> strTemp >> GetSetup()->lMaxClientsNum
			>> strTemp >> GetSetup()->lPendingWrBufNum
			>> strTemp >> GetSetup()->lPendingRdBufNum
			>> strTemp >> GetSetup()->lMaxSendSizePerSecond
			>> strTemp >> GetSetup()->lMaxRecvSizePerSecond
			>> strTemp >> GetSetup()->lMaxBlockedSendMsgNum
			>> strTemp >> GetSetup()->lConPendingWrBufNum
			>> strTemp >> GetSetup()->lConPendingRdBufNum
			>> strTemp >> GetSetup()->lConMaxSendSizePerSecond
			>> strTemp >> GetSetup()->lConMaxRecvSizePerSecond
			>> strTemp >> GetSetup()->lConMaxBlockedSendMsgNum

			>> strTemp >> GetSetup()->dwRefeashInfoTime
			>> strTemp >> GetSetup()->dwSaveInfoTime
			// 超过这个时间的LoginList的Player会不清除掉
			>> strTemp >> GetSetup()->dwReleaseLoginPlayerTime
			// 日志系统数据库服务器配置
			//>> strTemp >> GetSetup()->bUseLogSys		//是否起用日志系统的数据库服务器
			//>> strTemp >> GetSetup()->strLogSysProvider	
			//>> strTemp >> GetSetup()->strLogSysServer
			//>> strTemp >> GetSetup()->strLogSysDatabase
			//>> strTemp >> GetSetup()->strLogSysUID
			//>> strTemp >> GetSetup()->strLogSysPWD

			>> strTemp >> GetSetup()->dwMinDbOptThreadNum		//! 最小DB操作线程保持数
			>> strTemp >> GetSetup()->dwMaxDbOptThreadNum		//! 最大DB操作线程数
			>> strTemp >> GetSetup()->dwThreadNumAdjustSpace	//! 自动调整线程数的间隔时间
			; //end ;

		stream.close();

		strTemp = "LogServer";
		strTemp += "[";
		strTemp += GetSetup()->strName;
		strTemp += "3010]-StarDust";

		//查看此程序是否在运行中
		HWND hWnd = FindWindow(NULL,strTemp.c_str());
		if (hWnd != NULL)
		{
			char strMsgBox[256]="";
			sprintf(strMsgBox,"%s process running!",strTemp.c_str());
			MessageBox(g_hWnd,_T(strMsgBox), "error", MB_OK);
			return false;
		}

		SendMessage(g_hWnd, WM_SETTEXT, 0, (LPARAM)strTemp.c_str() );
		return true;
	}

	else	// 从加密文件读取
	{
		int length = GetFileLength("setup.dat");

		FILE *fp;
		fp = fopen("setup.dat", "rb");
		if (fp == NULL)
		{
			return false;
		}

		char *strIn = new char[length+1];
		char *strOut = new char[(length+1)*2];
		if( strIn == NULL ) return false;
		memset(strIn, 0, length+1);
		memset(strOut, 0, (length+1)*2);

		fread(strIn, length, 1, fp);
		fclose(fp);

		IniDecoder(strIn, strOut, length);

		stringstream streamSetup;

		streamSetup << strOut;
		streamSetup.seekg(0, std::ios_base::beg);

		streamSetup  
			>> strTemp >> GetSetup()->dwNumber
			>> strTemp >> GetSetup()->strName
			>> strTemp >> GetSetup()->dwListenPort
			>> strTemp >> GetSetup()->dwDataBlockNum
			>> strTemp >> GetSetup()->dwDataBlockSize
			>> strTemp >> GetSetup()->dwFreeMsgNum

			>> strTemp >> GetSetup()->dwFreeSockOperNum
			>> strTemp >> GetSetup()->dwFreeIOOperNum
			>> strTemp >> GetSetup()->lIOOperDataBufNum

			>> strTemp >> GetSetup()->bCheckNet
			>> strTemp >> GetSetup()->dwBanIPTime
			>> strTemp >> GetSetup()->dwMaxMsgLen			
			>> strTemp >> GetSetup()->lMaxConnectNum
			>> strTemp >> GetSetup()->lMaxClientsNum
			>> strTemp >> GetSetup()->lPendingWrBufNum
			>> strTemp >> GetSetup()->lPendingRdBufNum
			>> strTemp >> GetSetup()->lMaxSendSizePerSecond
			>> strTemp >> GetSetup()->lMaxRecvSizePerSecond
			>> strTemp >> GetSetup()->lMaxBlockedSendMsgNum
			>> strTemp >> GetSetup()->lConPendingWrBufNum
			>> strTemp >> GetSetup()->lConPendingRdBufNum
			>> strTemp >> GetSetup()->lConMaxSendSizePerSecond
			>> strTemp >> GetSetup()->lConMaxRecvSizePerSecond
			>> strTemp >> GetSetup()->lConMaxBlockedSendMsgNum

			>> strTemp >> GetSetup()->dwRefeashInfoTime
			>> strTemp >> GetSetup()->dwSaveInfoTime
			// 超过这个时间的LoginList的Player会不清除掉
			>> strTemp >> GetSetup()->dwReleaseLoginPlayerTime
			// 日志系统数据库服务器配置
			//>> strTemp >> GetSetup()->bUseLogSys		//是否起用日志系统的数据库服务器
			//>> strTemp >> GetSetup()->strLogSysProvider	
			//>> strTemp >> GetSetup()->strLogSysServer
			//>> strTemp >> GetSetup()->strLogSysDatabase
			//>> strTemp >> GetSetup()->strLogSysUID
			//>> strTemp >> GetSetup()->strLogSysPWD

			>> strTemp >> GetSetup()->dwMinDbOptThreadNum		//! 最小DB操作线程保持数
			>> strTemp >> GetSetup()->dwMaxDbOptThreadNum		//! 最大DB操作线程数
			>> strTemp >> GetSetup()->dwThreadNumAdjustSpace	//! 自动调整线程数的间隔时间
			; //end ;

		strTemp = "LogServer";
		strTemp += "[";
		strTemp += GetSetup()->strName;
		strTemp += "]";

		//查看此程序是否在运行中
		HWND hWnd = FindWindow(NULL,strTemp.c_str());
		if (hWnd != NULL)
		{
			char strMsgBox[256]="";
			sprintf(strMsgBox,"%s 程序正在运行中!",strTemp.c_str());
			MessageBox(g_hWnd,_T(strMsgBox), "error", MB_OK);
			return false;
		}

		SendMessage(g_hWnd, WM_SETTEXT, 0, (LPARAM)strTemp.c_str() );

		return true;
	}

	MessageBox(g_hWnd, "Can't find file setup.ini", "message", MB_OK);
	return false;
}

void OutMemInfo(const char *pFileName, const char *pTxt)
{
	if(NULL == pTxt)
		return;

	if (NULL != pFileName)
	{
		char strFile[MAX_PATH];
		SYSTEMTIME stTime;
		GetLocalTime(&stTime);
		_snprintf(strFile, MAX_PATH, "%s\\MemPoolInfo_%04d-%02d-%02d.txt", pFileName, stTime.wYear, stTime.wMonth, stTime.wDay);

		FILE *fp;
		if( (fp=fopen(strFile, "a+")) == NULL )
		{
			return;
		}

		fseek(fp, 0, SEEK_END);
		char strTime[MAX_PATH];
		_snprintf(strTime, MAX_PATH, "%d:%d:%d\r\n", stTime.wHour, stTime.wMinute, stTime.wSecond);
		fwrite(strTime, strlen(strTime), 1, fp);
		fwrite(pTxt, strlen(pTxt), 1, fp);
		fclose(fp);
	}
}

// 游戏初始化
BOOL CGame::Init()
{
	CreateDirectory( "log",	NULL);

	SET_OUT_INFO_SETUP(OutMemInfo, "log\\");

	CHAR* strPath = new CHAR[MAX_PATH];
	string strWorpath("");
	if(::GetCurrentDirectory(MAX_PATH,strPath))
	{
		strWorpath=string(strPath);
	}
	delete []strPath;

	// 初始化数据包

	string strFilesInfo("FilesInfo.ril");
	CClientResource::InitialInstance();
	CClientResource* pClientResource = CClientResource::GetInstance();
	pClientResource->Initial(GAME_RES,strWorpath,strFilesInfo);

	
	//m_pGameLogManage->AddLog(1, "select * from aaa;");

	//! test end 

	srand((unsigned)time(NULL));
	random(100);

	PutDebugString("LogServer start!");

	//装载资源
	LoadServerResource();

	if (LoadSetup()==false)
	{
		return false;
	}

	//! test
	if(NULL == m_pGameLogManage)
	{
		m_pGameLogManage = new LogManage();
	}
	else
	{
#ifdef _DEBUG
		throw;
#endif
	}
	if (!m_pGameLogManage->Init())
	{
		return false;
	}

	//! 初始化异步执行对象
	GetAsynchronousExecute().SetWorkParam(
		GetSetup()->dwMinDbOptThreadNum, 
		GetSetup()->dwMaxDbOptThreadNum, 
		GetSetup()->dwThreadNumAdjustSpace
		);
	GetAsynchronousExecute().BeginWork(GetSetup()->dwMinDbOptThreadNum);

	if( !LoadServerSetup("serverSetup.ini") )
	{
		return false;
	}

	//分配数据块
	m_pDBAllocator = new CDataBlockAllocator(m_Setup.dwDataBlockNum,m_Setup.dwDataBlockSize);
	CSMClient::GetSM()->LoadMapFileSetup(m_pDBAllocator);

	//// 进程使用ADO接口初始Connection string
	CMyAdoBase::Initialize("", "", "", "", "");
	//	GetSetup()->strSqlUserName, GetSetup()->strSqlPassWord);



	//装载无效文字系统
	//CWordsFilter::GetInstance()->Initial(string("setup/InvalidStr.ini"));


	//
	CMessage::Initial(m_pDBAllocator,m_Setup.dwFreeMsgNum);
	// 初始化服务器网络
	CMySocket::MySocketInit();

	// 创建服务器端
	if( !InitNetServer() )
	{
		MessageBox(g_hWnd, "Can't init NetServer!", "Message", MB_OK);
		return false;
	}

	if(!StartAccept())
	{
		return false;
	}
	CSMClient::GetSM()->InitOk();
	return true;
}

// 读取游戏服务器配置
bool CGame::LoadServerSetup(const char *filename)
{
	ifstream stream;
	stream.open(filename);
	if( !stream.is_open() )
	{
		AddLogText("Load %s lost!", filename);
		return false;
	}
	
	m_setClientIP.clear();
	string strIP;
	stream >> strIP;
	while (!stream.eof())
	{
		//strIP.clear();
		stream >> strIP;
		//! 转换IP
		DWORD dwIP = inet_addr(strIP.c_str());   
		//dwIP = htonl(dwIP);
		if(0 != dwIP)
			m_setClientIP.insert(dwIP);
	}
	stream.close();

	AddLogText("Load %s succeed!", filename);
	return true;
}

void CGame::OnGameServerConnect(DWORD dwIP, LONG lSocket)
{
	set<DWORD>::iterator ite = m_setClientIP.find(dwIP);
	if(m_setClientIP.end() == ite)
	{
		char szTmp[128] = {0};
		UCHAR arrIp[4] = {0};
		memmove(arrIp, &dwIP, sizeof(LONG));
		AddLogText("Nonlicet client[%u.%u.%u.%u] connect, It been kicked!!!", (ULONG)arrIp[0], (ULONG)arrIp[1], (ULONG)arrIp[2], (ULONG)arrIp[3]);

		s_pNetServer->DisConn(lSocket);
	}
}

// 读取所有的脚本文件
void FindScriptFile(const char* strFile, list<string>* plistFileName)
{
	char strPath[MAX_PATH];
	GetFilePath(strFile, strPath);

	WIN32_FIND_DATA stFindData;
	HANDLE hFind = FindFirstFile(strFile, &stFindData);
	if (hFind==INVALID_HANDLE_VALUE)
	{
		return;
	}

	BOOL bNext = true;
	while(1)
	{
		bNext = FindNextFile(hFind, &stFindData);
		if (bNext==0)
		{
			break;
		}

		// 是目录
		if (stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (stricmp(stFindData.cFileName, "..")!=0)
			{
				char str[MAX_PATH];
				sprintf(str, "%s%s/*.*", strPath, stFindData.cFileName);
				FindScriptFile(str, plistFileName);
			}
		}
		else	// 文件
		{
			char strExtName[MAX_PATH];
			GetFileExtName(stFindData.cFileName, strExtName);
			if (stricmp(strExtName, ".script")==0)
			{
				char str[MAX_PATH];
				sprintf(str, "%s%s", strPath, stFindData.cFileName);
				strlwr(str);	//  转换成小写
				plistFileName->push_back(str);
			}
		}
	}

	FindClose(hFind);
}








// 游戏释放
BOOL CGame::Release()
{
	PutDebugString("LogServer Exiting...");

	// 关闭网络连接
	if( GetNetServer() )
	{
		s_pNetServer->Exit();

		SAFE_DELETE(s_pNetServer);
	}

	// 关闭存盘线程句柄
	WaitForSingleObject(g_hSavingThread, INFINITE);
	CloseHandle(g_hSavingThread);
	CSMClient::GetSM()->ExitOk();
	CMySocket::MySocketClearUp();

	CMessage::Release();

	GetNetSessionManager()->Release();

	// 关闭玩家数据库载入线程
	for(DWORD i=0; i<g_vPlayerLoadThreads.size(); ++i)
	{
		TerminateThread(g_vPlayerLoadThreads[i],0);
		CloseHandle(g_vPlayerLoadThreads[i]);
	}

	//! 先关闭日志对象
	if(NULL != m_pGameLogManage)
		m_pGameLogManage->Exit();
	//! 再等待剩余任务执行完成
	GetAsynchronousExecute().Exit();
	//! 最后销毁游戏日志管理对象
	if(NULL != m_pGameLogManage)
		m_pGameLogManage->Release();

	SAFE_DELETE(m_pGameLogManage);

	//释放资源管理器
	CClientResource* pClientResource = CClientResource::GetInstance();
	if(NULL != pClientResource)
		pClientResource->Release();
	CClientResource::ReleaseInstance();

	CMyAdoBase::Uninitalize();
	CSMClient::GetSM()->ReleaseSM();
	SAFE_DELETE(g_pDefaultClientResource);

	SAFE_DELETE(m_pDBAllocator);

	OUT_RUNNING_INFO(true);

	PutDebugString("LogServer Exited!");
	return true;
}



//初始化游戏网络服务器端，建立网络服务器端，装载数据
BOOL CGame::InitNetServer(void)
{
	s_pNetServer = new CServer;
	//做为服务器身份启动
	long lRet = s_pNetServer->Start(2,m_pDBAllocator,
		m_Setup.dwFreeSockOperNum,m_Setup.dwFreeIOOperNum,m_Setup.lIOOperDataBufNum,
		m_Setup.bCheckNet,m_Setup.dwBanIPTime,
		m_Setup.dwMaxMsgLen,m_Setup.lMaxConnectNum,m_Setup.lMaxClientsNum,
		m_Setup.lPendingWrBufNum,m_Setup.lPendingRdBufNum,
		m_Setup.lMaxSendSizePerSecond,m_Setup.lMaxRecvSizePerSecond,
		m_Setup.lMaxBlockedSendMsgNum);
	if( !lRet )
	{
		MessageBox(g_hWnd, "Start NetServer Error", "Message", MB_OK);
		return FALSE;
	}
	CMessage::SetServer(s_pNetServer);
	AddLogText("netServer initializtion succeed!");
	return TRUE;
}

//开始作为服务器监听
bool CGame::StartAccept()
{
	if(NULL == s_pNetServer)	return false;
	BOOL bRet = s_pNetServer->Host(m_Setup.dwListenPort,NULL,NF_Server_Client,SOCK_STREAM);
	if(!bRet)
	{
		MessageBox(g_hWnd, "Start Host Error", "Message", MB_OK);
		return false;
	}
	AddLogText("LogServer begin listen!");
	return true;
}



long g_lRebackMessageCount = 0;
// 消息处理
BOOL CGame::ProcessMessage()
{

	g_lCurTime = timeGetTime();
	long lMessageCount = GetNetServer()->GetRecvMessages()->GetSize();
	for(int i=0; i<lMessageCount; i++)
	{
		CMessage* pMsg = (CMessage*)GetNetServer()->GetRecvMessages()->PopMessage();
		if( pMsg )
		{
			//ShowMessage("4:%x:%x",pMsg, pMsg->GetClientSocket());

			pMsg->Run();
			CBaseMessage::FreeMsg(pMsg);
		}
	}
	CSMClient::GetSM()->ProcessMessage();
	return TRUE;
}

// 游戏主循环
BOOL CGame::MainLoop()
{
	// 统计每个步骤的耗时
	static DWORD dwRefeashTextTime = 0,
		dwAITime = 0,
		dwMessageTime = 0,
		dwNetSessionTime = 0,
		dwFactionWarSysTime = 0,
		dwTimerTime = 0,
		dwProcessPlayerDataQueueTime = 0,
		dwSessionFactoryTime = 0,
		dwSavePointTime = 0;

	static bool bStart = true;
	if (bStart)
	{
		CSMClient::GetSM()->SendAITick(true);
		bStart = false;
	}
	static DWORD dwCurTime = 0;

	static DWORD ai_tick, now, old = timeGetTime(), lastSaveTime=timeGetTime(), lastRefeashInfoTime = now, lastReleaseLoginPlayerTime = 0;
	now = timeGetTime();
	m_dwtick++;
	ai_tick++;

	///////////////////////////////////////////////////////////
//#define MY_TEST
#ifdef MY_TEST

	//! 测试代码
	static LONG lSendNum = 0;
	if(NULL != m_pGameLogManage && 200000 > lSendNum)
	{
		for (LONG i = 0; i < 500; ++i)
		{
			char szSQL[256] = {0};
			sprintf(szSQL, "insert into table_1(_data)values(N'说大话反抗技术%d')", rand());
			m_pGameLogManage->AddLog(2000, szSQL);
		}
		lSendNum += 500;
	}

#endif
	///////////////////////////////////////////////////////////

	// 刷新显示
	dwCurTime = timeGetTime();
	static LONG lUpdateCount = 0;
	if (now - lastRefeashInfoTime > GetSetup()->dwRefeashInfoTime)
	{
		lastRefeashInfoTime = now;

		if(NULL!= m_pGameLogManage)
		{
			string strShowInfo("Log Info:\r\n");

			DWORD dwAllCount = 0;
			for (map<LONG, map<string, LONG>>::iterator ite = m_pGameLogManage->m_WriteCount.begin();
				ite != m_pGameLogManage->m_WriteCount.end(); 
				++ite)
			{
				//! 数据库编号
				char szDbNo[16] = {0};
				sprintf(szDbNo, "%d", ite->first);
				strShowInfo += "	DB No. = " + string(szDbNo) + "\r\n";

				strShowInfo += "		LOG TYPE			ITEM COUNT\r\n";
				for (map<string,LONG>::iterator iteDB = ite->second.begin(); iteDB != ite->second.end(); ++iteDB)
				{
					char szTmp[512] = {0};
					sprintf(szTmp, "		%s		%d\r\n", iteDB->first.c_str(), iteDB->second);
					strShowInfo += string(szTmp);
					dwAllCount += iteDB->second;
				}

			}

			char szInfo[512] = {0};

			sprintf(szInfo, "写入数据库日志总数已有[%u]条！\r\n", dwAllCount);

			RefeashInfoText((szInfo + strShowInfo).c_str());
		}

		OUT_RUNNING_INFO(false);
	}
	dwRefeashTextTime += timeGetTime() - dwCurTime;

	reload_profiles();


	dwSavePointTime += timeGetTime() - dwCurTime;


	// 消息处理
	dwCurTime = timeGetTime();
	ProcessMessage();
	dwMessageTime += timeGetTime() - dwCurTime;

	if(m_bCreateTable)
	{
		m_bCreateTable = FALSE;
		m_pGameLogManage->CreateTableInDb(false);
	}
	if(NULL != m_pGameLogManage)
		m_pGameLogManage->OnOutTime();
	//网络会话
	dwCurTime = timeGetTime();
	GetNetSessionManager()->Run();
	dwNetSessionTime += timeGetTime() - dwCurTime;	

	// 桢数控制
	static DWORD new_timer = timeGetTime();
	static DWORD old_timer = new_timer;
	static DWORD Old_OrgRunTime = timeGetTime();
	new_timer = timeGetTime();

	Old_OrgRunTime = new_timer;

	new_timer = timeGetTime();
	if (new_timer - old_timer < g_ms)
	{
		WaitForSingleObject(GetCurrentProcess(), g_ms - (new_timer - old_timer));
	}
	old_timer += g_ms;
	if ((long)new_timer - (long)old_timer > 1000)
	{
		OutputDebugString("warning!!! 1 second not call AI()\n");
		old_timer = timeGetTime();
	}

#ifdef _DEBUG
	if( now - old > 600000 )
	{
		char str[128];
		sprintf(str, "600秒调用了 %d 次AI\n", ai_tick);
		OutputDebugString(str);
		ai_tick = 0;
		old = now;
	}
#endif
	new_timer = timeGetTime();
	if( new_timer - lastReleaseLoginPlayerTime > GetSetup()->dwReleaseLoginPlayerTime )
	{
		lastReleaseLoginPlayerTime = new_timer;
	}

	//检查资源装载情况
	static CClientResource *pClientResource = CClientResource::GetInstance();
	pClientResource->Run();

	return true;
}


// 获取GameServer的数据
CGame::tagGameServer* CGame::GetGameServer(const char* strIP, DWORD dwPort)
{
	for(map<DWORD, tagGameServer>::iterator it=s_mapGameServer.begin(); it!=s_mapGameServer.end(); it++)
	{
		tagGameServer *p = &it->second;
		if (p->strIP==strIP && p->dwPort==dwPort)
		{
			return p;
		}
	}
	return NULL;
}


//当某个GameServer断开连接的时候
void CGame::OnGameServerClose(long lSocket)
{
	if (NULL == m_pGameLogManage)
	{
		return;
	}
	pair<long, long> pairInfo = m_pGameLogManage->GetNoAndIpBySockte(lSocket);
	UCHAR szIP[4] = {0};
	if(m_setClientIP.end() != m_setClientIP.find(pairInfo.second))
	{
		memmove(szIP, &(pairInfo.second), sizeof(pairInfo.second)); 
		AddLogText("!!!!!Client Server lost!!!!!!!!![ClientNo = %d], [ClientIP = %u.%u.%u.%u]", 
			pairInfo.first, szIP[0], szIP[1], szIP[2], szIP[3]);
	}
	// 在GetGame()->GetPlayerList()中清除在该GameServer上的player，并通知LoginSever
	OnGameServerLost(pairInfo.first);
	m_pGameLogManage->ClientLost(lSocket);
}

// 当某个GameServer断开时
void CGame::OnGameServerLost(DWORD dwIndex)
{

}


void CGame::SendMsg2GameServer(DWORD dwIndex,CMessage* pMsg)
{
	if(pMsg)
	{
		pMsg->SendGSID(dwIndex);
	}
}

//! [2007-7-9 add]
void CGame::SendAllGameServer(CMessage* pMsg)
{
	if(pMsg)
	{
		map<DWORD, tagGameServer>::iterator iteGS = s_mapGameServer.begin();
		for (; iteGS != s_mapGameServer.end(); ++iteGS)
		{
			long dwIndex = iteGS->first;
			pMsg->SendGSID(dwIndex);
		}
	}
}

// 游戏主线程
unsigned __stdcall GameThreadFunc(void* pArguments)
{
#ifndef _DEBUG
	__try
	{
#endif
		CreateGame();
		BOOL ret = GetGame()->Init();

		if (ret)
		{
			while(1)
			{
				// 结束线程
				if( g_bGameThreadExit )
				{
					break;
				}

				if( !GetGame()->MainLoop() )
				{
					break;
				}
			}
		}

		if(GetGame())
			GetGame()->Release();
		DeleteGame();

#ifndef _DEBUG
	}
	__except(_Sword3::CrashFilter(GetExceptionInformation(),GetExceptionCode()))
	{
		//	MessageBox(g_hWnd, "程序出现错误，请查看最新的错误报告文件", "Error", MB_OK);

		PutDebugString("process exceptional stop, please look dump log!");
		AddErrorLogText("process exceptional stop, please look dump log!");

		if(GetGame())
			GetGame()->Release();
		DeleteGame();

		Sleep(5000);

		WinExec("logserver.exe", SW_SHOW);

		PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		SetEvent(g_hGameThreadExitEvent);
		_endthreadex( 0 );

		return 0;
	}
#endif

	SetEvent(g_hGameThreadExitEvent);
	PostMessage(g_hWnd, WM_CLOSE, 1,0);

	_endthreadex( 0 );
	return 0;
}




extern DWORD g_dwThisSaveStartTime;		// 本次存盘开始时间


long CGame::GetConnectedGameServerCount()
{
	long lCnt=0;
	map<DWORD,tagGameServer>::iterator it=s_mapGameServer.begin();
	for(;it!=s_mapGameServer.end();it++)
	{
		if(it->second.bConnected)
		{
			lCnt++;
		}
	}
	return lCnt;
}


void CGame::CreateTableInDB(void)
{
	m_bCreateTable = TRUE;
	if (NULL == m_pGameLogManage)
	{
		return;
	}
}

void CGame::CreateTableInSqlFile(void)
{
	if (NULL == m_pGameLogManage)
	{
		return;
	}
	m_pGameLogManage->CreateTableInDb(true, "");
}

void CGame::OutTableInfoToFile(void)
{
	if (NULL == m_pGameLogManage)
	{
		return;
	}
	m_pGameLogManage->OutTableInfoToFile();
}

void CGame::OnClientLogin(CMessage *pMsg)
{
	if (NULL == m_pGameLogManage)
	{
		return;
	}
	
	//! 不管客户端编号是否存在，都向客户端发送DB表格信息和限制条件
	vector<BYTE> tmpTableData;
	m_pGameLogManage->TableAddToByteArray(tmpTableData);
	CMessage msg(MSG_LOG20_SERVER_TABLE);
	msg.Add((long)tmpTableData.size());
	msg.AddEx((void*)&(tmpTableData[0]), tmpTableData.size());
	msg.SendToSocket(pMsg->GetSocketID());

	vector<BYTE> tmpConditionData;
	m_pGameLogManage->ConditionAddToByteArray(tmpConditionData);
	CMessage msgCondition(MSG_LOG20_SERVER_CONDITION);
	msgCondition.Add((long)tmpConditionData.size());
	msgCondition.AddEx((void*)&(tmpConditionData[0]), tmpConditionData.size());
	msgCondition.SendToSocket(pMsg->GetSocketID());

	//! 验证客户端编号是否存在
	long lClientNo = pMsg->GetLong();
	long lClientIp = pMsg->GetIP();
	BOOL isAffirm = m_pGameLogManage->AffirmClientNo(lClientNo, lClientIp, pMsg->GetSocketID());
	if (isAffirm)
	{
		CMessage::MapGSIDSocket(lClientNo,pMsg->GetSocketID());
	}
	CMessage msgAffirm(MSG_LOG20_SERVER_AFFIRM);
	msgAffirm.Add((isAffirm)? 1L : 0L);
	msgAffirm.SendToSocket(pMsg->GetSocketID());
}

void CGame::ReLoadConfig(void)
{
	vector<BYTE> tmpConditionData;
	m_pGameLogManage->ReLoadControlConfig();
	m_pGameLogManage->ConditionAddToByteArray(tmpConditionData);
	CMessage msgCondition(MSG_LOG20_SERVER_CONDITION);
	msgCondition.Add((long)tmpConditionData.size());
	msgCondition.AddEx((void*)&(tmpConditionData[0]), tmpConditionData.size());
	msgCondition.SendAll();
}

void CGame::OnLog(CMessage *pMsg)
{
	long lClientNo	= pMsg->GetLong();
	long lTableType = pMsg->GetLong();
	long lDataSize	= pMsg->GetLong();

	vector<BYTE> vData(lDataSize + 1);
	pMsg->GetEx(&vData[0], lDataSize);
	//m_pGameLogManage->AddLog(lClientNo, szData);
	m_pGameLogManage->AddData(lClientNo, lTableType, &vData[0]);

}

void CGame::OnStrSqlLog(CMessage *pMsg)
{
	long lClientNo	= pMsg->GetLong();

	char szSql[MAX_SQL_STRING_SIZE] = {0};
	pMsg->GetStr(szSql, MAX_SQL_STRING_SIZE);

	m_pGameLogManage->AddLog(lClientNo, szSql);
}

void OnClientMessage(CMessage* pMsg)
{
	switch(pMsg->GetType())
	{
	case MSG_02LOG_SERVER_LOGIN:
		{
			GetGame()->OnClientLogin(pMsg);
		}
		break;
	case MSG_02LOG_SERVER_LOG:
		{
			GetGame()->OnLog(pMsg);
		}
		break;
	case MSG_02LOG_SERVER_LOG_SQLString:
		{
			GetGame()->OnStrSqlLog(pMsg);
		}
		break;
	}
}

AsynchronousExecute::MyOverlapped* AsynchronousExecute::CreateMyOverlapped(VOID)
{
	return (MyOverlapped*)M_ALLOC(sizeof(MyOverlapped));
}
VOID AsynchronousExecute::RestoreMyOverlapped(MyOverlapped** ppMyOverlapped)
{
	if(NULL != ppMyOverlapped)
		M_FREE(*ppMyOverlapped, sizeof(MyOverlapped));
}
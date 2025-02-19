#pragma once
#include "..\..\..\nets\net\message.h"
#include "..\..\..\LogClient\server\LogClient.h"
#include "..\..\..\LogClient\server\LogicLogInterface.h"


void AddLogText(char* msg, ...);		// 加入一条记录
void AddErrorLogText(char* msg, ...);	// 加入一条错误的记录

enum eNetFlag
{
	NF_SERVER_CLIENT	=	0x001,
	NF_GATE_CLIENT		=	0x002,
	NF_LSERVER_CLIENT	=	0x010,
	NF_CSERVER_CLIENT	=	0x100,
	NF_DBSERVER_CLIENT	=	0x1000,
	NF_LOGSERVER_CLIENT =	0x10000,
};

enum eServerInitSendMsgFlag
{
	NULL_SERVER_INITSENDFLAG	= 0,
	DB_SERVER_INITSENDFLAG		= 1,
	LOGIN_SERVER_INITSENDFLAG	= 2,
	LOG_SERVER_INITSENDFLAG		= 4,
};

// 游戏全局控制
class CGame
{
public:
	CGame(void);
	virtual ~CGame(void);

	BOOL Init();
	// 初始化堆栈文件名
	BOOL InitStackFileName();
	BOOL Release();
	BOOL MainLoop();
	BOOL AI();
	BOOL ProcessMessage();

public:
	bool LoadServerResource();


/////////////////////////////////////////////////////////////////////////
	// 游戏配置
	/////////////////////////////////////////////////////////////////////////
public:
	struct tagSetup
	{
		string	strServerIP;			// ServerIP
		DWORD	dwServerPort;			// ServerPort

		long	lUseLogServer;		// 是否使用LogServer；0=不使用，1=使用
		string	strLogIP;			// LogServerIP
		DWORD	dwLogPort;			// LogServerPort
		string  strErrFile;			// 错误文件路径+文件名
		string  strLogFile;			// 本地日志文件路径+文件名（SQL格式）
		string  strTxtLogFile;		// 本地日志文件路径+文件名（明文格式）
		DWORD	dClientNo;			// 客户端编号

		DWORD	dwNumber;			// 服务器编号(0-7)
		string	strName;			// 服务器名字
		string	strLocalIP;			// 本地IP 
		DWORD	dwListenPort;		// 监听port
		
		string	strSqlConType;		//sql server connection type
		string	strSqlServerIP;		//sql server ip address
		string  strSqlUserName;		//sql server user name
		string  strSqlPassWord;		//password
		string  strDBName;			//database name

		//Client网络服务配置参数
		DWORD	dwDataBlockNum;			//数据块数量
		DWORD	dwDataBlockSize;		//数据块大小
		DWORD	dwFreeMsgNum;			//预分配的消息数量

		DWORD	dwFreeSockOperNum;		//网络命令操作预分配数量
		DWORD	dwFreeIOOperNum;		//完成端口上IO操作预分配数量
		long	lIOOperDataBufNum;		//默认IO操作的DataBuf数量

		bool	bCheckNet;				// 是否对网络进行检测
		DWORD	dwBanIPTime;			// 禁止IP的时间
		DWORD	dwMaxMsgLen;			// 允许传输的最大消息长度		
		long	lMaxConnectNum;			// 客户端的最大连接数量
		long	lMaxClientsNum;			// 最大的客户端发送缓冲区大小
		long	lPendingWrBufNum;		// 最大的发送IO操作Buf总大小
		long	lPendingRdBufNum;		// 最大的接受IO操作Buf总大小
		long	lMaxSendSizePerSecond;	// 向客户端每秒发送的最大字节数
		long	lMaxRecvSizePerSecond;	// 从客户端接受的每秒最大字节数
		long	lMaxBlockedSendMsgNum;	// 最大阻塞的发送消息数量
		long	lConPendingWrBufNum;	// 客户端最大的发送IO操作Buf总大小
		long	lConPendingRdBufNum;	// 客户端最大的接受IO操作Buf总大小
		long	lConMaxSendSizePerSecond;//向服务器发送的每秒最大字节数
		long	lConMaxRecvSizePerSecond;//从服务器接受的每秒最大字节数
		long	lConMaxBlockedSendMsgNum;// 最大阻塞的发送消息数量
		
		DWORD	dwRefeashInfoTime;		// 刷新显示时间
		DWORD	dwSaveInfoTime;			// 保存一次信息的时间间隔	

		DWORD	dwGappID;				//内存映射客户端ID

		tagSetup()
		{
			dwRefeashInfoTime	= 1000;
			dwSaveInfoTime		= 60000;
		}
	};

private:
	tagSetup m_tgSetup;

	// 每次运新产生的记录堆栈的文件名字
	char m_pszRunStackFile[MAX_PATH];

public:
	tagSetup* GetSetup() {return &m_tgSetup;}

	bool LoadSetup();
	bool LoadServerSetup(const char* pszFileName);
	bool LoadMapFileSetup();							// 装载内存影射文件配置

	const char* GetStackFileName() { return m_pszRunStackFile; }

//////////////////////////////////////////////////////////////////////////
//	网络部分
//////////////////////////////////////////////////////////////////////////
public:
	// Client信息
	struct tagClientInfo
	{
		bool			bConnected;			// 该服务器是否已经连接上
		DWORD			dwIndex;			// 服务器编号
		string			strIP;				// IP
		DWORD			dwPort;				// 端口
	};

public:
	bool InitNetServer();			// 初始化游戏网络服务器端，建立网络服务器端，装载数据
	bool StartAccept();				// 开始作为服务器监听

	bool StartConnectServer();		// 作为客户端连接相应的服务器
	bool ReConnectServer();			// 重新连接服务器
	void CreateConnectServerThread();
	void OnLostServer();			// 与服务器断开连接
	bool StartConnectLogServer();	// 连接日志服务器
	bool ReConnectLogServer();
	void CreateConnectLogThread();	// 创建连接日志服务器线程
	void OnLostLogServer();			


	CServer*	GetNetServer()		{ return s_pNetServer; }
	long		GetServerSocket()	{ return m_lServerSocket; }	 

	void		SetServerInitSendFlag(eServerInitSendMsgFlag flag);
	

private:
	CDataBlockAllocator	*m_pDBAllocator;
	CServer				*s_pNetServer;					// 做为Client的服务器端
	long				m_lServerSocket;				// 连接Server的客户端		
	long				m_lServerInitSendFlag;			// 对应的服务器发送标示
	CRITICAL_SECTION	m_csServerInitSendFlag;


	map<DWORD, tagClientInfo>		s_mapClientInfo;	// 服务器列表<dwIndex, tagClientInfo>

	//////////////////////////////////////////////////////////////////////////
	//	LogClient
	//////////////////////////////////////////////////////////////////////////
public:
	//!		响应DB信息
	void	InitLogClient(CMessage *pMsg);
	//!		响应确认信息
	void	LogServerAffirm(CMessage *pMsg);
	//!		响应筛选条件信息
	void	LogServerCondition(CMessage *pMsg);
	
	//!		
	LogicLogInterface*	GetLogicLogInterface();

public:
	LogClient			&GetLogClient(void){return m_LogClient;}
private:

	LogClient			m_LogClient;
	LogicLogInterface	*m_pLogicLogInterface;

};

BOOL CreateGame();				// 创建游戏
BOOL DeleteGame();				// 释放游戏
CGame* GetGame();				// 获取游戏
LogicLogInterface*	GetGameLogInterface();			// 获取游戏日志逻辑接口对象

// 游戏主线程
unsigned __stdcall GameThreadFunc(void *pArguments);
unsigned __stdcall ConnectServerFunc( void* pArguments );	// 连接Server的线程、当Server断开时启用，连同时候关闭
unsigned __stdcall ConnectLogServerFunc(void* pArguments);	// 连接LogServer的线程

extern HANDLE g_hGameThreadExitEvent;		// 主线程退出事件
extern bool	g_bGameThreadExit;				// 是否退出主线程

// YaYaDisk.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "YaYaDisk.h"
#include "YaYaDiskDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CYaYaDiskApp

BEGIN_MESSAGE_MAP(CYaYaDiskApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CYaYaDiskApp 构造

CYaYaDiskApp::CYaYaDiskApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CYaYaDiskApp 对象

CYaYaDiskApp theApp;


// CYaYaDiskApp 初始化

BOOL CYaYaDiskApp::InitInstance()
{
	CWinApp::InitInstance();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CYaYaDiskDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

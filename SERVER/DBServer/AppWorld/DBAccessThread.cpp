#include "stdafx.h"
#include "DBAccessThread.h"
#include "DBAdoBase.h"
#include <process.h>
#include "DBentityManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DBAccessThread::DBAccessThread(string strProvider, 
						 string strDataSource, 
						 string strInitialCatalog,
						 string strUserID, 
						 string strPassword
						 )
:
m_Running(false)
{
	m_pConnPtr = new DBAdoBase(string(	"Provider=" + strProvider +
									"; Server=" + strDataSource + 
									"; Database=" + strInitialCatalog + 
									"; UID=" + strUserID + 
									"; PWD=" + strPassword + 
									"; OLE DB Services=-1; Driver={SQL Server}"
									));
	m_strInitialCatalog = strInitialCatalog;
}

//-----------------------------------------------------------------------------------------------------------------
DBAccessThread::~DBAccessThread(void)
{
	delete m_pConnPtr;
}

_ConnectionPtr DBAccessThread::GetConnPtr(void)
{
	return m_pConnPtr->GetConnPtr();
}

bool DBAccessThread::OpenCn(void)
{
	return m_pConnPtr->OpenCn();
}
DBAdoBase* DBAccessThread::GetAdoBasePtr(void)
{
	return m_pConnPtr;
}
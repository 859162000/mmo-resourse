#include "stdafx.h"


#include "CGStallDefaultPage.h"
#include "Log.h"
#include "Player.h"
#include "GamePlayer.h"
#include "Scene.h"
#include "Obj_Human.h"


uint CGStallDefaultPageHandler::Execute( CGStallDefaultPage* pPacket, Player* pPlayer )
{
__ENTER_FUNCTION

	GamePlayer* pGamePlayer = (GamePlayer*)pPlayer ;
	Assert( pGamePlayer ) ;

	Obj_Human* pHuman = pGamePlayer->GetHuman() ;
	Assert( pHuman ) ;

	Scene* pScene = pHuman->getScene() ;
	if( pScene==NULL )
	{
		Assert(FALSE) ;
		return PACKET_EXE_ERROR ;
	}
	//检查线程执行资源是否正确
	Assert( MyGetCurrentThreadID()==pScene->m_ThreadID ) ;

	pHuman->m_StallBox.SetFirstPage( (UINT)pPacket->GetDefaultPage() );

	g_pLog->FastSaveLog( LOG_FILE_1, "CGStallDefaultPageHandler::ObjName=%s"
		,pHuman->GetName());

	return PACKET_EXE_CONTINUE ;

__LEAVE_FUNCTION

	return PACKET_EXE_ERROR ;
}

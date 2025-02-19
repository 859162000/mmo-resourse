
#include "stdafx.h"
#include "..\nets\netserver\message.h"
//#include "..\nets\netserver\MyServerClient.h"
//#include "..\nets\netserver\MyNetServer.h"
//#include "..\nets\netserver\MyServerClient.h"
#include "..\goods\cgoods.h"
#include "..\..\gameserver\game.h"
#include "..\player.h"
#include "..\OrganizingSystem\GameFaction.h"
#include "..\OrganizingSystem\GameUnion.h"
#include "..\OrganizingSystem\GameOrganizingCtrl.h"
#include "..\..\..\..\public\Date.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void OnDBSaveMessage(CMessage* pMsg)
{
	switch(pMsg->GetType())
	{
	case MSG_DB2S_SAVE_PLAYER://保存玩家数据
		{
			//取消定时器
			long MsgTimerID = pMsg->GetLong();
			GetGame()->DelMsgTimer(MsgTimerID);
			AddLogText("从DBS反馈保存Player数据消息 [%d]OK！", MsgTimerID);
		}
		break;
	case MSG_DB2S_SAVE_FACTION://保存玩家数据
		{
			//取消定时器
			long MsgTimerID = pMsg->GetLong();
			GetGame()->DelMsgTimer(MsgTimerID);
			AddLogText("从DBS反馈保存faction数据消息 [%d]OK！", MsgTimerID);
		}
		break;
	case MSG_DB2S_SAVE_CONFEDERATION://保存玩家数据
		{
			//取消定时器
			long MsgTimerID = pMsg->GetLong();
			GetGame()->DelMsgTimer(MsgTimerID);
			AddLogText("从DBS反馈保存union数据消息 [%d]OK！", MsgTimerID);
		}
		break;
	case MSG_S2DB_SAVE_PLAYER_GOOD://保存玩家物品
		{
			pMsg->SetType(MSG_W2S_OTHER_DELGOODS);
			pMsg->SendAll();
		}
		break;
	case MSG_S2W_PLAYER_DELSKILL://gm跨服务器删除一个玩家的技能,该消息转发给下面连接的gameserver
		{
			pMsg->SetType(MSG_W2S_OTHER_DELSKILL);
			pMsg->SendAll();
		}
		break;
	case MSG_S2W_PLAYER_SETLEVEL://gm跨服务器设置一个玩家的等级,该消息转发给下面连接的gameserver
		{
			pMsg->SetType(MSG_W2S_OTHER_SETLEVEL);
			pMsg->SendAll();
		}
		break;	

	}
}

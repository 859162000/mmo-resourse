
/*
*	file:		PlayerDepot.h
*	Brief:		仓库类
*	detail:		封装了仓库相关的验证和物品容器操作
*				
*
*				
*				
*				
*	Authtor:	张科智
*	Datae:		2008-01-22
*/

#pragma once

#include "Container\CAmountLimitGoodsContainer.h"
#include "Container\CAmountLimitGoodsShadowContainer.h"
#include "Container\CVolumeLimitGoodsContainer.h"
#include "Container\CSubpackContainer.h"
#include "Container\CWallet.h"
#include "Container\GemContainer.h"
#include "Container\CSilverWallet.h"

#include "goods/CGoods.h"
#include "../../public/DepotDef.h"
#include "PlayerPwd.h"



//! ------------------------------------类申明------------------------------------
class PlayerDepot
{
public:
	PlayerDepot(CPlayer *pPlayer);
	~PlayerDepot(void);

private:
	PlayerDepot(const PlayerDepot&){};
	PlayerDepot&operator=(const PlayerDepot& r){};


	//! 程序交互
	//!---------------------------------------------------------------------------------------------------------
public:
	//!							编码
	void						CodeToDataBlock_ForClient			(DBWriteSet& setWriteDB);
	//!							编码仓库信息头
	void						CodeHeadToDataBlock_ForClient		(DBWriteSet& setWriteDB);
	//!							编码
	void						CodeToDataBlock_ForServer			(DBWriteSet& setWriteDB);
	//!							仓库设置编码
	void						CodeSetupToDataBlock				(DBWriteSet& setWriteDB);
	//!							解码
	void						DecodeFromDataBlock_FromServer		(DBReadSet& setReadDB);

	//!							绑定角色
	void						SetOwner							(CPlayer *pPlayer);
	//!							清除装载的物品
	void						Clear								(void);
	//!							释放对象
	void						Release								(void);


	//! 逻辑功能
	//!---------------------------------------------------------------------------------------------------------
public:
	//!							响应玩家离开游戏
	void						OnPlayerLeave						(void);
	//!							找到容器对象
	CGoodsContainer*			FindContainer						(long eContainerId, bool bNotTest = false);
	//!							判断仓库容器是否满了
	BOOL						IsFull								(long eContainerId);
	//!							建立仓库操作Session
	bool						CreateSession						(ULONG uRadius, long lPosX, long lPosY);
	//!							测试会话，若失败则锁定仓库
	bool						TestSession							(void);

	//!							测试仓库是否可用，不包含TestSession
	bool						IsEnable							(void);
	//!							检查能量衰减物品
	void						CheckGuardianGoods					(std::vector<CGoods*>&GoodsList);

private:
	//!							设置一个附加栏位的开启或关闭，返回开启需要的金钱
	long						SetSecondary						(bool bOpen);

	//!							结束会话
	void						EndSession							(void);


	//! 消息响应
	//!---------------------------------------------------------------------------------------------------------
public:
	//!							响应密码输入
	void						OnInputPwdRequest					(CMessage *pMsg);
	//!							响应仓库锁定请求
	void						OnLockRequest						(CMessage *pMsg);
	//!							响应修改密码请求
	void						OnChangePwdRequest					(CMessage *pMsg);
	//!							响应购买子背包请求
	void						OnBuySubpackRequest					(CMessage *pMsg);



	//! 静态成员
	//!---------------------------------------------------------------------------------------------------------
public:
	//!							得到配置
	static	tagDepotSetup*		GetDepotSetup						(void);

private:

	static	tagDepotSetup		c_DepotSetup;


	//! 
	//!---------------------------------------------------------------------------------------------------------
private:
	//!											主仓库
	CVolumeLimitGoodsContainer					m_Primary;
	//!											主仓库2
	//CVolumeLimitGoodsContainer					m_Primary2;
	//!											附加仓库栏位（5个子背包栏位）
	CSubpackContainer							m_Secondary;
	//!											可用的的子背包栏位
	bool										m_arrHlodSubpack[DEPOT_SUBPACK_NUM];

	//!											金币仓库
	CWallet										m_GoldDepot;
	//!											银币仓库（由不用的宝石仓库对象改，对象名称不变）
	CSilverWallet/*GemContainer*/				m_GemDepot;


private:
	//!											绑定的玩家
	CPlayer										*m_pPlayer;

	//!											仓库密码
	CPlayerPwd									m_Pwd;

	//!											合法对话半径
	ULONG										m_uSessionRadius;
	//!											对话中心X
	long										m_lSessionPosX;
	//!											对话中心Y
	long										m_lSessionPosY;

};
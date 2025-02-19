#include "StdAfx.h"
#include "mmsystem.h"
#include ".\moveshape.h"
#include "serverRegion.h"
#include "Area.h"
#include "..\..\Setup\GMList.h"
#include "..\nets\netserver\message.h"
#include "..\nets\MySocket.h"
#include "Player.h"
#include "Pet.h"
#include "goods/cgoods.h"
#include "monster.h"
#include "../setup/GlobeSetup.h"
#include "../setup/LogSystem.h"
#include "../setup/hitlevelsetup.h"

#include "BuffSkill/BaseModule.h"
#include "pksys.h"
#include "ai\aifactory.h"
#include "..\..\..\Public\SkillRelated.h"
#include "BuffSkill/NewSkillFactory.h"
#include "AntiCheat\CPlayerManager.h"


//#define  _NEW_STATE_
//! 向客户端发送浮点类型的移动坐标
//#define ENABLE_FOALT_POS

CMoveShape::CMoveShape(void)
{		
	m_bAttackAble = true;
	m_bIsGod = false;
	m_pAI=NULL;


	m_dwLastSendMoveSendTime = timeGetTime();

	m_lMoveableCount			= 0;
	m_lCanFightCount			= 0;
	m_lWeakCount				= 0;

	
	

	m_ScriptTimers.clear();	
	ZeroMemory(&m_KilledMeAttackInfo,sizeof(tagAttackInformation));

	m_bRun						= true;
	m_bAntiInvisibleable        = true;			//不可反隐形
	m_bInvisible				= true;			//可见状态	
	m_bPretendDead				= true;			//非装死	
	m_bCanFight					= true;			//可攻击标志符			
	m_bMoveable					= true;			//可移动标志符
	m_bUseRabbSkill				= true;			//可使用法师技能
	m_bUseFighterSkill			= true;			//可使用战士技能
	m_bUseItem					= true;			//可使用物品
	m_bInstruction				= true;			//可使用指令
	m_bUnAttack					= true;			//不被攻击标志符
	m_bAddHp					= true;			//hp可增加标志符
	m_bForceAttack				= true;			//不强制攻击
	m_bRandomMove				= true;		    //不随机跑动
	m_bHide						= true;			//不隐蔽
	m_bRide						= true;			//非坐骑状态
	m_bWeakness					= true;			//虚弱状态

	m_lChangeModelType			= 0;	
	m_lChangeModelID			= 0;

	//buff动态参数
	m_BuffParam.clear();
	//debuff动态参数
	m_DeBuffParam.clear();
	//主动技能动态参数
	m_ActiveSkillParam.clear();


	//对象键值
	m_ActiveID = 0;	
	m_FixedReinforce.first	= 1.0f;
	m_FixedReinforce.second = 1.0f;
	//任务保护相关
	m_BeneficiaryType=(OBJECT_TYPE)0;
	m_bRoleProtect				=false;
	//采集相关
	m_dwCollectionID=0;
	m_dwCollectTimes=0;
	m_bIsBeenCollect=FALSE;
	m_bCanBeenCollect=FALSE;
	m_dwActStartTime	  = 0;
	m_dwActTime			  = 0;
	m_lDelEventTimerID    =-1;
	m_lUpdateAttrTimerID = -1;
	m_lBeneficiaryTimerID = -1;
	//攻击者
	m_lAttackerType = 0;
	m_guidAttacker = NULL_GUID;
	//选择目标
	m_lTargetType  = 0;
	m_guidTarget   = NULL_GUID;	
	m_SelectSelfPlayers.clear();	
}



CMoveShape::~CMoveShape(void)
{
	//清除脚本定时器
	ClearScriptTimer();	
}

//  [7/4/2007 chenxianjin]
//设置使用战士技能标志符
void CMoveShape::SetUseFightSkill(bool b)
{
	m_bUseFighterSkill = b;
}

//设置使用法师技能标志符
void CMoveShape::SetUseRabbSkill(bool b)
{
	m_bUseRabbSkill = b;
}

//设置使用道具标志符
void CMoveShape::SetUseItem(bool b)
{
	m_bUseItem = b;
}


bool CMoveShape::IsVisible(CMoveShape* pTarget)
{
	if(NULL == pTarget)
		return m_bInvisible;
	return ( !pTarget->GetAntiInvisibleable() || 	// 非反隐
		GetInvisibleState() );			// 隐形	
}

// [070611 AHC]
long CMoveShape::GetValue(const char* strName)
{
	long ret = INVILID_VALUE;

	mapShapeNameValueItr itr = m_mapShapeNameValue.begin();
	itr = m_mapShapeNameValue.find(strName);
	if (itr != m_mapShapeNameValue.end())
	{
		switch (itr->second.lType)
		{
		case 0:	// byte
			ret = (long)*(itr->second.plValue);
			break;

		case 1:	// word
			ret = (long)*(itr->second.pwValue);
			break;

		case 2:	// dword
			ret = (long)*(itr->second.pdwValue);
			break;

		case 3:	// float
			ret = (long)*(itr->second.pfValue);
			break;

		case 4:	// long
			ret = *(itr->second.pValue);
			break;

		case 5:	// short
			ret = (long)*(itr->second.psValue);

		case 6:	// bool
			ret = (long)*(itr->second.pbValue);
			break;
		}
	}
	return ret;
}

long CMoveShape::SetValue(const char* strName, long dwValue)
{
	long ret = INVILID_VALUE;

	mapShapeNameValueItr itr = m_mapShapeNameValue.begin();
	itr = m_mapShapeNameValue.find(strName);
	if (itr != m_mapShapeNameValue.end())
	{
		switch (itr->second.lType)
		{
		case 0:	// byte
			{
				*(itr->second.plValue) = (BYTE)dwValue;
				ret = dwValue;
			}
			break;
		case 1:	// word
			{
				*(itr->second.pwValue) = (WORD)dwValue;
				ret = dwValue;
			}
			break;
		case 2:	// dword
			{
				*(itr->second.pdwValue) = (DWORD)dwValue;
				ret = dwValue;
			}
			break;
		case 3:	// float
			{
				*(itr->second.pfValue) = (float)dwValue;
				ret = dwValue;
			}
			break;
		case 4:	// long
			{
				*(itr->second.pValue) = (long)dwValue;
				ret = dwValue;
			}
			break;
		case 5:	// short
			{
				*(itr->second.psValue) = (short)dwValue;
				ret = dwValue;
			}
			break;
		case 6:	// bool
			{
				*(itr->second.pbValue) = dwValue==0?false:true;
				ret = dwValue;
			}
			break;
		default:
			break;
		}
	}

	return ret;
}


//当属性发生改变时候调用
void CMoveShape::OnPropertyValueChanged(const char* strName)
{
}

// 初始化
void CMoveShape::InitNameValueMap()
{
	m_mapShapeNameValue.clear();

	tagPty st;// 0: byte 1:WORD 2:DWORD	3:float 4:long 5:short 6:bool
	st.lType=3; st.pfValue=&m_fSpeed;										st.attrEnumValue = S_ATTR_SPEED;	   		m_mapShapeNameValue["fSpeed"] = st;	
	st.lType=1; st.pwValue=&m_wState;										st.attrEnumValue = S_ATTR_SHAPESTATE;	   		m_mapShapeNameValue["wShapeState"] = st;	
	st.lType=6; st.pbValue=&m_bInvisible;			st.lCount=0;			st.attrEnumValue = S_ATTR_INVISIBLETIME;   		m_mapShapeNameValue["bInvisible"] = st;				//隐形
	st.lType=6; st.pbValue=&m_bHide;				st.lCount=0;			st.attrEnumValue = S_ATTR_HIDE;   				m_mapShapeNameValue["bHide"] = st;			    	//隐蔽
	st.lType=6; st.pbValue=&m_bPretendDead;			st.lCount=0;			st.attrEnumValue = S_ATTR_PRETENDDEADTIME; 		m_mapShapeNameValue["bPretendDead"]=st;				//假死
	st.lType=4; st.pValue=&m_lChangeModelType;		st.lCount=0;			st.attrEnumValue = S_ATTR_CHANGEMODELTYPE; 		m_mapShapeNameValue["lChangeModelType"]=st;			//改变类型
	st.lType=4; st.pValue=&m_lChangeModelID;		st.lCount=0;			st.attrEnumValue = S_ATTR_CHANGEMODELID;   		m_mapShapeNameValue["lChangeModelID"]=st;			//改变模型id
	st.lType=6; st.pbValue=&m_bInstruction;	    	st.lCount=0;			st.attrEnumValue = S_ATTR_USEINSTRUCTION;  		m_mapShapeNameValue["bUnUseInstruction"]=st;		//使用指令
	st.lType=6; st.pbValue=&m_bMoveable;			st.lCount=0;			st.attrEnumValue = S_ATTR_MOVEABLE;		   		m_mapShapeNameValue["bMoveable"]=st;			//移动
	st.lType=6; st.pbValue=&m_bCanFight;	    	st.lCount=0;			st.attrEnumValue = S_ATTR_FIGHTABLE;	   		m_mapShapeNameValue["bFightable"]=st;				//可否攻击
	st.lType=6; st.pbValue=&m_bUseFighterSkill;		st.lCount=0;			st.attrEnumValue = S_ATTR_USEFIGHTSKILL;   		m_mapShapeNameValue["bUseFighterSkill"]=st;			//使用战士技能
	st.lType=6; st.pbValue=&m_bUseRabbSkill;		st.lCount=0;			st.attrEnumValue = S_ATTR_USERABBSKILL;	   		m_mapShapeNameValue["bUseRabbSkill"]=st;			//使用法师技能
	st.lType=6; st.pbValue=&m_bUseItem;	        	st.lCount=0;			st.attrEnumValue = S_ATTR_USEITEM;			    m_mapShapeNameValue["bUseItem"]=st;					//使用道具
	st.lType=6; st.pbValue=&m_bAntiInvisibleable;	st.lCount=0;			st.attrEnumValue = S_ATTR_ANTIINVISIBLEABLE;	m_mapShapeNameValue["bAntiInvisibleable"]=st;		//反隐形
	st.lType=6; st.pbValue=&m_bAddHp;				st.lCount=0;			st.attrEnumValue = S_ATTR_ADDHP;				m_mapShapeNameValue["bAddHp"]=st;		            //不能增加HP
	st.lType=6; st.pbValue=&m_bUnAttack;			st.lCount=0;			st.attrEnumValue = S_ATTR_UNATTACK;				m_mapShapeNameValue["bUnAttack"]=st;		        //不能被攻击
	st.lType=6; st.pbValue=&m_bForceAttack;			st.lCount=0;			st.attrEnumValue = S_ATTR_FORCEATTACK;			m_mapShapeNameValue["bForceAttack"]=st;				//强制攻击
	st.lType=6; st.pbValue=&m_bRandomMove;			st.lCount=0;			st.attrEnumValue = S_ATTR_RAMDOMMOVE;			m_mapShapeNameValue["bRandomMove"]=st;		        //随机跑动
	st.lType=6; st.pbValue=&m_bRide;				st.lCount=0;			st.attrEnumValue = S_ATTR_NOTRIDE;				m_mapShapeNameValue["bRide"]=st;					//坐骑状态

	st.lType=2; st.pdwValue=&m_dwCollectionID;			st.lCount=0;			st.attrEnumValue = S_ATTR_COLLECTIONID;				m_mapShapeNameValue["dwCollectionID"]=st;		        //采集物id
	st.lType=2; st.pdwValue=&m_dwCollectTimes;			st.lCount=0;			st.attrEnumValue = S_ATTR_COLLECTIONTIMES;			m_mapShapeNameValue["dwCollectionTimes"]=st;		    //能采集的次数
	st.lType=6; st.pbValue=&m_bIsBeenCollect;			st.lCount=0;			st.attrEnumValue = S_ATTR_ISBEENCOLLECT;			m_mapShapeNameValue["bIsBeenCollect"]=st;		        //是否正在被采集
	st.lType=6; st.pbValue=&m_bCanBeenCollect;			st.lCount=0;			st.attrEnumValue = S_ATTR_CANBEENCOLLECT;			m_mapShapeNameValue["bCanBeenCollect"]=st;		        //能否被采集

}
bool  CMoveShape::SetAttribute(std::string& attrName, long value)
{
	if(INVILID_VALUE != SetValue(attrName.c_str(), value))
	{
		// 脏属性数据标记
		return true;
	}

	return false;
}
bool  CMoveShape::SetAttribute(char* attrName, long value)
{
	if(NULL == attrName)
		return false;

	if(INVILID_VALUE != SetValue(attrName, value))
	{
		return true;
	}

	return false;
}
long CMoveShape::GetAttribute(std::string attrName)
{
	long value = INVILID_VALUE;

	value = GetValue(attrName.c_str());

	return value;
}

bool  CMoveShape::ChangeAttribute(std::string& attrName, long value)
{
	// 脏属性数据标记
	long curValue = 0;
	long rightValue = GetValue(attrName.c_str());

	curValue = rightValue + value;
	mapShapeNameValueItr itr = m_mapShapeNameValue.begin();
	itr = m_mapShapeNameValue.find(attrName);
	if (itr != m_mapShapeNameValue.end())
	{	
#ifdef _DUMP
		char strDebug[256];
		_snprintf(strDebug, 256, "修改的属性是%s当前值为%d.\n",attrName.c_str(),curValue);
		PutDebugString(strDebug);
#endif
		if(SetValue(attrName.c_str(), curValue+rightValue) != INVILID_VALUE)
			return true;
	}
	return false;
}


VOID CMoveShape::SetFightable( bool b )
{
	if( b )
	{
		-- m_lCanFightCount;
	}
	else 
	{
		if( m_lCanFightCount < 0 ) m_lCanFightCount = 0;
		++ m_lCanFightCount;
	}

	if( m_lCanFightCount <= 0 )
	{
		m_bCanFight = TRUE;
	}
	else 
		m_bCanFight = FALSE;
}

VOID CMoveShape::SetKilledMeAttackInfo( LONG lType, CGUID guid )
{
	m_KilledMeAttackInfo.lAttackerType			= lType;
	m_KilledMeAttackInfo.AttackerID				= guid;
}

void CMoveShape::SetWeakNessState(bool bWeak)
{

	if( bWeak )
	{
		-- m_lWeakCount;
	}
	else 
	{
		if( m_lWeakCount < 0 ) m_lWeakCount = 0;
		++ m_lWeakCount;

	}

	if( m_lWeakCount <= 0 )
	{
		m_bWeakness = TRUE;		
	}
	else 
		m_bWeakness = FALSE;


}

VOID CMoveShape::SetMoveable( BOOL b )
{


	if( b )
	{
		-- m_lMoveableCount;
	}
	else 
	{
		if( m_lMoveableCount < 0 ) m_lMoveableCount = 0;
		++ m_lMoveableCount;

	}

	if( m_lMoveableCount <= 0 )
	{
		m_bMoveable = TRUE;		
	}
	else 
	{
		m_bMoveable = FALSE;

	}
}

//设置bool属性
BOOL  CMoveShape::SetBoolAttribute(const std::string& attrName, long value)
{

	return SetBoolAttribute(attrName, static_cast<BOOL>(value));	
}

BOOL CMoveShape::SetBoolAttribute(const string &attrName, BOOL b)
{
	mapShapeNameValueItr itr = m_mapShapeNameValue.find(attrName);

	if(itr != m_mapShapeNameValue.end()) // 找到该变量
	{
		if( b )
		{
			--itr->second.lCount;			
		}
		else 
		{		
			if(itr->second.lCount<0)
				itr->second.lCount = 0;
			++itr->second.lCount ;			
		}


		if( itr->second.lCount  <= 0 )
		{
			*(itr->second.pbValue) = true;			
		}
		else 
			*(itr->second.pbValue) = false;

		if (attrName=="bRandomMove" && *(itr->second.pbValue)==false)
		{
			if (GetType()==TYPE_PLAYER)
			{
				GetAI()->StartActiveAct();
			}

		}
		if (attrName=="bMoveable" && *(itr->second.pbValue)==false)
		{
			SetPosition(GetTileX(),GetTileY());
		}

		AddAttributeToDirty(attrName);
	}

	//属性更新
	return TRUE;
}


VOID CMoveShape::InitSkills()
{
	//##清除所有的技能
	ClearSkills();	
}

VOID CMoveShape::ClearSkills()
{
	ClearPassiveSkill();	 
	EndAllActiveSkill();	
}

// 异常状态的AI
VOID CMoveShape::UpdateAbnormality()
{	
	/**## 首先清除掉NULL的指针 */
	BOOL bNeedUpdateProperties = FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//	Shape战斗处理
//////////////////////////////////////////////////////////////////////////////////////////////////


// 目标死亡判定
BOOL CMoveShape::IsDied()
{	
	if(GetState() == STATE_DIED ||
		GetHP() ==0 )
	{
		return TRUE;
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
// 行走
//////////////////////////////////////////////////////////////////////////
// 改变坐标位置
// 改变阻挡的位置
void CMoveShape::SetPosXY(float fX,float fY )
{
	CServerRegion* pRegion = dynamic_cast<CServerRegion*>(GetFather());
	if(NULL == pRegion)	return;

	if( (int)fX < 0 || (int)fX >= pRegion->GetWidth() || (int)fY < 0 || (int)fY >= pRegion->GetHeight() )
	{
		return; 
	}

	long lOldX = GetTileX();
	long lOldY = GetTileY();

	CShape::SetPosXY(fX, fY);

	if(GetType() == TYPE_PLAYER || GetType() == TYPE_MONSTER)
	{
		//减少格子上的Shape数量fY
		pRegion->DecCellObjNum(lOldX,lOldY);
		//增加格子上的Shape数量fY
		pRegion->IncCellObjNum(fX,fY);
	}

	long lNewTileX = static_cast<long>(fX);
	long lNewTileY = static_cast<long>(fY);
	// 区域切换
	long nx = lNewTileX / AREA_WIDTH;
	long ny = lNewTileY / AREA_HEIGHT;
	if( GetArea() && ( GetArea()->GetX() != nx || GetArea()->GetY() != ny ))
	{
		pRegion->OnShapeChangeArea(this,nx,ny);
	}
	else
	{
		SetChangeState(0);
	}

	//触发位置改变接口
	if(lOldX != lNewTileX || lOldY != lNewTileX)
	{
		OnPosChanged();
	}

}

BOOL	CMoveShape::IsInAct()
{
	DWORD dwCurrTime=timeGetTime();

	if (dwCurrTime>(m_dwActStartTime+m_dwActTime))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

// 获取目标方向
long CMoveShape::GetDestDir(long sx, long sy, long dx, long dy)
{

	long x = sx - dx;
	long y = sy - dy;

	// 向左
	if( x > 0 )
	{
		if( y > 0 )
		{
			return DIR_LEFTUP;
		}
		else if( y < 0 )
		{
			return DIR_LEFTDOWN;
		}
		else
		{
			return DIR_LEFT;
		}
	}
	else if( x < 0 )	// 向右
	{
		if( y > 0 )
		{
			return DIR_RIGHTUP;
		}
		else if( y < 0 )
		{
			return DIR_RIGHTDOWN;
		}
		else
		{
			return DIR_RIGHT;
		}
	}
	else	// 纵向
	{
		if( y > 0 )
		{
			return DIR_UP;
		}
		else
		{
			return DIR_DOWN;
		}
	}

	return 0;
}

VOID CMoveShape::ForceMove( LONG lDestX, LONG lDestY, DWORD dwTime )
{
	//##首先验证X,Y是否属于场景的范围内
	LONG lVerifiedDestX = lDestX;
	LONG lVerifiedDestY = lDestY;
	CServerRegion* pRegion = dynamic_cast<CServerRegion*>( GetFather() );
	if( pRegion )
	{
		if( lDestX >= pRegion -> GetWidth() )
		{
			lVerifiedDestX = pRegion -> GetWidth() - 1;
		}
		if( lDestX < 0 )
		{
			lVerifiedDestX = 0;
		}
		if( lDestY >= pRegion -> GetHeight() )
		{
			lVerifiedDestY = pRegion -> GetWidth() - 1;
		}
		if( lDestY < 0 )
		{
			lVerifiedDestY = 0;
		}
	}
	else
	{
		return;
	}

	CMessage msg( MSG_S2C_SHAPE_FORCEMOVE );
	msg.Add( GetExID() );
	msg.Add( GetType() );
	msg.Add( lVerifiedDestX );
	msg.Add( lVerifiedDestY );
	msg.SendToAround( this );

	//##移到终点
	SetPosXY( lVerifiedDestX+0.5f, lVerifiedDestY+0.5f );

	//##AI的延迟
	CBaseAI* pAI = GetAI();
	if( pAI )
	{

	}
}
//结束被动技能
bool CMoveShape::EndPassiveSkill(DWORD dwID,DWORD dwLv)
{
	for (itModu it=m_PassiveSkillParam.begin(); it!=m_PassiveSkillParam.end();)
	{
		if (dwID==it->second->GetID())
		{
			if(dwLv>it->second->GetLv())
			{
				CBaseModule *pBaseModule = CNewSkillFactory::GetBaseModule(it->second);
				if(pBaseModule)
				{
					pBaseModule->End();				
					CNewSkillFactory::RealeaseBaseMoudule(it->second);
					it = m_PassiveSkillParam.erase(it);	
				}
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			++it;
		}
	}
	return true;
}

//添加被动技能
void CMoveShape::AddPassSkill(DWORD dwID, DWORD dwLv)
{
	CNewSkill *pSkillProperty = CNewSkillFactory::QueryNewSkillProperty(dwID);

	if (NULL!=pSkillProperty)
	{
		CNewSkill::tagSkill *ptgSkill = pSkillProperty->GetNewSkill(dwLv);
		if (ptgSkill)
		{			
			if(pSkillProperty->GetSkillType()==ModuType_Skill_Passive)
			{				
				//开启被动技能
				stModuParam *pPassiveSkill = AddPassiveSkill(dwID,dwLv);
				if (pPassiveSkill)
				{				
					m_PassiveSkillParam[pPassiveSkill->GetGlobleID()] = pPassiveSkill;					
				}
			}
		}
	}
}

//创建动态参数
stModuParam  *CMoveShape::InitModuParam(DWORD dwSkillID, DWORD dwLevel, DWORD dwGlobleID)
{
	stModuParam *pModuParam = MP_NEW stModuParam;
	pModuParam->SetGlobleID(dwGlobleID);
	CNewSkill *pSkillProperties = CNewSkillFactory::QueryNewSkillProperty(dwSkillID);
	CNewSkillFactory::InitModuParam(pSkillProperties,pModuParam,dwLevel);	
	return pModuParam;

}

//启动被动技能动态参数
stModuParam *CMoveShape::AddPassiveSkill(DWORD dwSkillID, DWORD dwLevel)
{
	++m_ActiveID;
	if (!EndPassiveSkill(dwSkillID,dwLevel))
	{
		return NULL;
	}

	stModuParam *pSkillParam = InitModuParam(dwSkillID, dwLevel, m_ActiveID);

	if(pSkillParam == NULL)	
	{						
		return NULL;
	}		
	pSkillParam->Reset(this,this);	

	return pSkillParam;
}

//启动被动技能
BOOL CMoveShape::OnBeginSkill( DWORD dwID,DWORD dwLv )
{
	CNewSkill *pSkillProperty = CNewSkillFactory::QueryNewSkillProperty(dwID);

	if (NULL!=pSkillProperty)
	{
		CNewSkill::tagSkill *ptgSkill = pSkillProperty->GetNewSkill(dwLv);
		if (ptgSkill)
		{			
			if(pSkillProperty->GetSkillType()==ModuType_Skill_Passive)
			{				
				//开启被动技能
				stModuParam *pSkillParam = AddPassiveSkill(dwID,dwLv);

				if (pSkillParam)
				{
					CBaseModule *pPassiveSkill = CNewSkillFactory::CreateModule(dwID,dwLv,pSkillParam,pSkillParam->GetGlobleID(),0,1);

					if(pPassiveSkill && pPassiveSkill->Begin(0,0,GetType(),GetExID())!=MR_Modu_End) 
					{
						m_PassiveSkillParam[pSkillParam->GetGlobleID()] = pSkillParam;
						return TRUE;
					}
					else
					{					
						CNewSkillFactory::RealeaseBaseMoudule(pSkillParam);
					}
				}
			}
		}
	}
	return FALSE;
}
//添加主动技能的动态参数
void CMoveShape::AddActiveSkill(stModuParam *pSkillParam)
{	
	if (pSkillParam)
	{
		if( pSkillParam->GetModType()==ModuType_Skill_Active)
		{		
			m_ActiveSkillParam[pSkillParam->GetGlobleID()] = pSkillParam;
		}		
	}	
}

//初始化主动技能动态参数
void CMoveShape::InitSkillInstance(DWORD dwSkillID,DWORD dwLevel,long lGoodsIndex,long lContainerID,long lPos,CGUID GooodsGuid,long lType)
{
	++m_ActiveID;
	stModuParam *pModuParam = InitModuParam(dwSkillID, dwLevel, m_ActiveID);
	if (pModuParam)
	{
		CNewSkillFactory::CreateModule(dwSkillID,dwLevel,pModuParam,m_ActiveID,0,lType);
		pModuParam->Reset(this,this);
		pModuParam->SetGoodGuid(GooodsGuid);
		pModuParam->SetGoodsIndex(lGoodsIndex);
		pModuParam->SetContainerID(lContainerID);
		pModuParam->SetPos(lPos);	
		m_pInstanceSkill = pModuParam;
	}
}

//添加主支技能
long CMoveShape::AddActiveSkill(DWORD dwSkillID, DWORD dwLevel,CMoveShape *pTarget,long lGoodsIndex,long lContainerID,long lPos,CGUID GoodsGuid)
{
	return m_ActiveID;
}

//获取所有状态个数
long CMoveShape::GetAllStateNum()
{
	long lStateNum=0;
	lStateNum = m_BuffParam.size() + m_DeBuffParam.size();
	return lStateNum;
}

//通过状态id和参数名称得到状态的相关参数
long CMoveShape::GetStateParamByID(DWORD dwID,string strParam)	
{
	long lNum=0;
	mapModules mpTempModule;
	for (itModu it=m_BuffParam.begin(); it!=m_BuffParam.end(); ++it)
	{
		if (dwID==it->second->GetID())
		{
			mpTempModule[it->first] = it->second;
			++lNum;
		}
	}
	for (itModu it=m_DeBuffParam.begin(); it!=m_DeBuffParam.end(); ++it)
	{
		if (dwID==it->second->GetID())
		{
			mpTempModule[it->first] = it->second;
			++lNum;
		}
	}

	if (strParam=="Num")
	{
		return lNum;
	}
	else if (strParam=="Lv")
	{
		long lLv=0;
		for (itModu it=mpTempModule.begin(); it!=mpTempModule.end(); ++it)
		{
			if (lLv<it->second->GetLv())
			{
				lLv = it->second->GetLv();
			}			
		}
		return lLv;
	}
	else
	{
		return 0;
	}
}
//通过状态索引和参数名称得到状态的相关参数
long CMoveShape::GetStateParamByIndex(DWORD dwIndex,string strParam)
{
	long lNum=0;
	mapModules mpTempModule;
	for (itModu it=m_BuffParam.begin(); it!=m_BuffParam.end(); ++it)
	{
		++lNum;
		if (lNum==dwIndex)
		{
			if (strParam=="ID")
			{
				return it->second->GetID();
			}
			else if (strParam=="Lv")
			{
				return it->second->GetLv();
			}

		}		
	}
	for (itModu it=m_DeBuffParam.begin(); it!=m_DeBuffParam.end(); ++it)
	{
		++lNum;
		if (lNum==dwIndex)
		{
			if (strParam=="ID")
			{
				return it->second->GetID();
			}
			else if (strParam=="Lv")
			{
				return it->second->GetLv();
			}

		}		
	}
	return 0;
}

//删除状态
void CMoveShape::RemoveState(itModu &it)
{	
	CBaseModule *pState = CNewSkillFactory::GetBaseModule((*it).second);
	if (pState)
	{
		ModuleType eMType = (*it).second->GetModType();	
		EndState(pState);
		
		if(eMType==ModuType_Buff)
		{
			it = m_BuffParam.erase(it);		
		} 
		else
		{	
			it = m_DeBuffParam.erase(it);		
		}	
	}
}
//删除状态
void CMoveShape::RemoveState(mapModules &mpBaseModule, DWORD dwStateID, DWORD dwLevel,DWORD dwCount)	
{
	int nCount = 0;
	itModu it = mpBaseModule.begin();
	for(;it!=mpBaseModule.end();)
	{		
		if (it->second->GetID()==dwStateID && (it->second->GetLv()==dwLevel || dwLevel==0))
		{				
			if (it->second)
			{
 				RemoveState(it);
				++nCount;
				if (dwCount>0 && nCount>=dwCount)
				{
					return;
				}
				else
				{
					continue;
				}
			}

		}	
		++it;
	}	  
}
//删除状态
void CMoveShape::DelNewState(DWORD dwStateID, DWORD dwLevel,DWORD dwCount)
{
	CNewSkill *pSkillProperty = CNewSkillFactory::QueryNewSkillProperty(dwStateID);

	if (pSkillProperty && pSkillProperty->GetSkillType()==ModuType_Buff)
	{
		RemoveState(m_BuffParam,dwStateID,dwLevel,dwCount);
	}
	else if (pSkillProperty && pSkillProperty->GetSkillType()==ModuType_DeBuff)
	{
		RemoveState(m_DeBuffParam,dwStateID,dwLevel,dwCount);
	}	
}
//移除对象
void CMoveShape::RemoveBaseModule(stModuParam* pModuParam)
{
	mapModules *mapBaseModule;
	CBaseModule *pBaseModule = CNewSkillFactory::GetBaseModule(pModuParam);

	if(pModuParam->GetModType()==ModuType_Skill_Active)
	{
		GetAI()->OnSkillEnd(pModuParam->GetAddSub());

		mapBaseModule = &m_ActiveSkillParam;

	}
	else if (pModuParam->GetModType()==ModuType_Buff)
	{
		SubStateCount(pModuParam);
		mapBaseModule = &m_BuffParam;	

		OnDelBuff(pModuParam->GetID(), pModuParam->GetLv());
	} 
	else if (pModuParam->GetModType()==ModuType_DeBuff)
	{
		SubStateCount(pModuParam);
		mapBaseModule = &m_DeBuffParam;

		OnDelBuff(pModuParam->GetID(), pModuParam->GetLv());
	}
	else if (pModuParam->GetModType()==ModuType_Skill_Passive)
	{	
		GetAI()->OnSkillEnd(pModuParam->GetAddSub());

		mapBaseModule = &m_PassiveSkillParam;
	}

	if ((*mapBaseModule).size()>0)
	{
		ReleaseBaseModule(pBaseModule->GetParam(),*mapBaseModule);
	}	
}
//移除对象
void CMoveShape::ReleaseBaseModule(stModuParam* pModuParam,mapModules &mpBaseModule)
{
	if (NULL == pModuParam)
		return;
	pModuParam->ProcNumSub();
	mpBaseModule.erase(pModuParam->GetGlobleID());
	CNewSkillFactory::RealeaseBaseMoudule(pModuParam);
}

//移除ActiveSkill
void CMoveShape::RemoveActive(CBaseModule* pActiveSkill)
{
	if (pActiveSkill==NULL)
	{
		return;
	}
	GetAI()->OnSkillEnd(pActiveSkill->GetParam()->GetAddSub());

	CNewSkillFactory::RealeaseBaseMoudule(pActiveSkill->GetParam());
}

//添加状态计数器
void	CMoveShape::AddStateCount(stModuParam *pState)
{
	if (NULL == pState)
	{
		return;
	}
	tagModuleKey tgState;
	tgState.dwID = pState->GetID();
	tgState.dwLvl = pState->GetLv();

	map<tagModuleKey,long,tagKeycmp>::iterator it = m_mpStateCount.find(tgState);
	if (it!=m_mpStateCount.end())
	{
		++((*it).second);
	}
	else
	{
		m_mpStateCount[tgState] = 1;
	}
}
//减状态计数器
void	CMoveShape::SubStateCount(stModuParam *pState)
{
	if (NULL == pState)
	{
		return;
	}
	tagModuleKey tgState;
	tgState.dwID = pState->GetID();
	tgState.dwLvl = pState->GetLv();

	map<tagModuleKey,long,tagKeycmp>::iterator it = m_mpStateCount.find(tgState);
	if (it!=m_mpStateCount.end())
	{
		if ((*it).second >= 1)
		{
			--((*it).second);
			if ((*it).second<=0)
			{
				it = m_mpStateCount.erase(it);
			}
		}
	}	
}
//获取状态数量
long	CMoveShape::GetStateCount(stModuParam *pState)
{
	tagModuleKey tgState;
	tgState.dwID = pState->GetID();
	tgState.dwLvl = pState->GetLv();
	map<tagModuleKey,long,tagKeycmp>::iterator it = m_mpStateCount.find(tgState);
	if (it!=m_mpStateCount.end())
	{
		return (*it).second;
	}
	return 0;

}


//根据ID结束状态
void CMoveShape::EndBuff(DWORD dwStateID)
{
	//结束Buff
	itModu it= m_BuffParam.begin();
	for(;it!=m_BuffParam.end();)
	{
		if (dwStateID==(*it).second->GetID())
		{
			CBaseModule *pState = CNewSkillFactory::GetBaseModule((*it).second);
			if (pState)
			{
				EndState(pState);
				it = m_BuffParam.erase(it);
			}			
		}
		else
		{
			++it;
		}
	}
}
//清除被动技能
VOID CMoveShape::ClearPassiveSkill()
{	

	itModu it=m_PassiveSkillParam.begin();
	for(;it!=m_PassiveSkillParam.end();)
	{	
#ifdef _RUNSTACKINFO1_
		char pszStatckInfo[1024]="";
		long lID = (*it).second->GetID();	
		_snprintf(pszStatckInfo,1024,"Name:%s,CMoveShape::ClearPassiveSkill(ID:%d) Start",GetGame(),lID);
		CMessage::AsyWriteFile(GetGame()->GetStatckFileName(),pszStatckInfo);
#endif
		CNewSkillFactory::RealeaseBaseMoudule((*it).second);		
		it = m_PassiveSkillParam.erase(it);

#ifdef _RUNSTACKINFO1_		
		_snprintf(pszStatckInfo,1024,"Name:%s,CMoveShape::ClearPassiveSkill(ID:%d) End",GetGame(),lID);
		CMessage::AsyWriteFile(GetGame()->GetStatckFileName(), pszStatckInfo);
#endif
	}
	m_PassiveSkillParam.clear();
}
//结束所有主动技能
void CMoveShape::EndAllActiveSkill()
{
	itModu it=m_ActiveSkillParam.begin();
	for(;it!=m_ActiveSkillParam.end();)
	{	
#ifdef _RUNSTACKINFO1_
		char pszStatckInfo[1024]="";
		long lID = (*it).second->GetID();	
		_snprintf(pszStatckInfo,1024,"CMoveShape::EndAllActiveSkill(ID:%d) Start",lID);
		CMessage::AsyWriteFile(GetGame()->GetStatckFileName(),pszStatckInfo);
#endif

		CBaseModule *pActiveSkill = CNewSkillFactory::GetBaseModule((*it).second);
		if (pActiveSkill)
		{
#ifdef _RUNSTACKINFO1_			
			CMessage::AsyWriteFile(GetGame()->GetStatckFileName(), "this->GetAI()->OnSkillEnd Start");
#endif
			this->GetAI()->OnSkillEnd((*it).second->GetAddSub());	
#ifdef _RUNSTACKINFO1_			
			CMessage::AsyWriteFile(GetGame()->GetStatckFileName(), "this->GetAI()->OnSkillEnd End");
#endif
			pActiveSkill->End();	
#ifdef _RUNSTACKINFO1_			
			CMessage::AsyWriteFile(GetGame()->GetStatckFileName(), "pActiveSkill->End() End");
#endif
		}
		CNewSkillFactory::RealeaseBaseMoudule((*it).second); 		
		it = m_ActiveSkillParam.erase(it);	

#ifdef _RUNSTACKINFO1_		
		_snprintf(pszStatckInfo,1024,"CMoveShape::EndAllActiveSkill(ID:%d) End",lID);
		CMessage::AsyWriteFile(GetGame()->GetStatckFileName(), pszStatckInfo);
#endif
	}
	m_ActiveSkillParam.clear();
}


//结束状态
void CMoveShape::EndState(CBaseModule *pBaseModule)
{

	if (pBaseModule)
	{
		pBaseModule->GetParam()->ProcNumSub();
		pBaseModule->End();				
		SubStateCount(pBaseModule->GetParam());
		OnDelBuff(pBaseModule->GetID(),pBaseModule->GetLv());
		CNewSkillFactory::RealeaseBaseMoudule(pBaseModule->GetParam());
	}
}

//结束DEBUFF
void CMoveShape::EndDeBuff()
{
	itModu it= m_DeBuffParam.begin();

	for(;it!=m_DeBuffParam.end();)
	{
		CNewSkillProperties *pSkillProperty = CNewSkillFactory::QueryNewSkillProperty((*it).second->GetID());
		CBaseModule *pState = CNewSkillFactory::GetBaseModule((*it).second);

		if (pSkillProperty && pSkillProperty->GetSystem()==0 && pState) 
		{
			EndState(pState);
			it = m_DeBuffParam.erase(it);			
			continue;
		}
		++it;
	}
}

//结束状态
void CMoveShape::EndState(mapModules &mapState)													
{
	itModu it= mapState.begin();	
	for(;it!=mapState.end();)
	{
		CNewSkillProperties *pSkillProperty = CNewSkillFactory::QueryNewSkillProperty((*it).second->GetID());
		CBaseModule *pState = CNewSkillFactory::GetBaseModule((*it).second);
		if (pState && pSkillProperty && pSkillProperty->GetDeadEnd() == 0 && pSkillProperty->GetSystem()==0) 
		{
			EndState(pState);				
			it = mapState.erase(it);			
			continue;
		}
		++it;
	}	
}

//结束所有的状态
void CMoveShape::EndBuffAndDebuff()
{	
	//结束buff
	EndState(m_BuffParam);
	//结束debuff
	EndState(m_DeBuffParam);
}

//清除状态
void CMoveShape::ClearState(mapModules &mapState)
{
	itModu it= mapState.begin();	
	for(;it!=mapState.end();++it)
	{

		SubStateCount((*it).second);

		CNewSkillFactory::RealeaseBaseMoudule((*it).second);			
	}
	mapState.clear();
}



//清除所有状态
void CMoveShape::ClearAllBuff()
{
	//清除BUFF
	ClearState(m_BuffParam);
	//清除DEBUFF
	ClearState(m_DeBuffParam);	
}

//启动状态
void CMoveShape::StartUpState(mapModules &mapState)
{
	itModu it= mapState.begin();
	for(;it!=mapState.end();)
	{
		CBaseModule* pState = CNewSkillFactory::GetBaseModule((*it).second);	
		if (pState && (*it).second->GetbRun()==false)
		{
			if(pState->Begin() == MR_Step_End)
			{
				(*it).second->SetbRun(true);				
				++it;
			}
			else
			{			
				SubStateCount(pState->GetParam());
				CNewSkillFactory::RealeaseBaseMoudule((*it).second);
				it = mapState.erase(it);
			}			
		}
		else
		{		
			++it;
		}
	}
}

//重新开始所有的状态
void CMoveShape::StartAllBuff()
{
	//启动Buff
	StartUpState(m_BuffParam);
	//启动DeBuff
	StartUpState(m_DeBuffParam);
}

//增益处理
VOID CMoveShape::OnBeenAddAppendSkill(CMoveShape *pAttacker)
{
	if (NULL == pAttacker)
	{
		return;
	}
	if( IsDied() ) return;
	//PK值设定
	if (pAttacker->GetExID()!=GetExID())
	{
		GetPKSys()->SetPKValue(pAttacker,this,1);
	}
}
bool	CMoveShape::OnIncreaseHp(long &lHp)
{
	string str("Hp");
	if (GetHP()==GetMaxHP())
	{
		return false;
	}

	long lCurrentHp = GetHP()+lHp;
	if (lCurrentHp > GetMaxHP())
	{
		lCurrentHp = GetMaxHP();
		lHp = GetMaxHP() - GetHP();
	}
	SetAttribute(str,lCurrentHp);
	UpdateAttribute(1);
	return true;
}

//减益处理
long CMoveShape::OnBeenAttacked( CMoveShape* pAttacker,long lHp,long lDamageResult)
{
	if(pAttacker==NULL)
		return 0;

	//if(TYPE_PLAYER == GetType() && TYPE_PLAYER == pAttacker->GetType())
	//	GetGame()->GetLogicLogInterface()->logT130_player_attack_log((CPlayer*)pAttacker, (CPlayer*)this);

	//如果已经死亡就返回
	if(IsDied() || pAttacker->IsDied()) 
		return 0;

	SetAttacker(pAttacker->GetType(),pAttacker->GetExID());
	if (lHp>0 && lDamageResult!=DAMAGE_RESULT_DamageEx
		&& lDamageResult!=DAMAGE_RESULT_DotaDamage)
	{		
		ProduceEvent(ET_BeAttack,0,0,0,0,pAttacker);//
		ProduceEvent(ET_BeenDamage);
		pAttacker->ProduceEvent(ET_Damage);
	}

	DWORD dwHp = GetHP();
	if( dwHp > lHp)
	{
		if( TYPE_PLAYER == this->GetType() && TYPE_PLAYER == pAttacker->GetType() )// 是玩家攻击
			((CPlayer*)this)->SendNotifyWarningMsg(false, (CPlayer*)pAttacker);

		//pk值设定
		if (pAttacker->GetExID()!=GetExID())
		{
			GetPKSys()->SetPKValue(pAttacker,this,2);
		}

		// 添加反外挂PK状态判定，@Expter 20090602
		if (TYPE_PLAYER == GetType())
		{
			/// 玩家间PK
			if (TYPE_PLAYER == pAttacker->GetType())
			{
				/// 玩家间PK,触发反外挂,一个pk事件
				GetInst(AntiCheat::CPlayerManager).PlayerFight(dynamic_cast<CPlayer*>(this));	
				GetInst(AntiCheat::CPlayerManager).PlayerFight(dynamic_cast<CPlayer*>(pAttacker));
			}
			/// 如果守卫，先判断是怪物
			else if (TYPE_MONSTER == pAttacker->GetType())
			{
				CMonster * pGuardAttacker = dynamic_cast<CMonster*>(pAttacker);
				/// 如果守卫
				if (pGuardAttacker->GetNpcType() == NT_Guard)
				{
					/// 玩家与守卫PK,触发反外挂,一个pk事件
					GetInst(AntiCheat::CPlayerManager).PlayerFight(dynamic_cast<CPlayer*>(this));	
				}
			}
		}
		// 反外挂：主动攻击增加战斗活跃度
		if( pAttacker->GetType() == TYPE_PLAYER )
		{
			GetInst( AntiCheat::CPlayerManager ).PlayerAttack( static_cast<CPlayer*>( pAttacker ) );
		}

		string str("Hp");
		SetAttribute(str,dwHp-lHp);
		UpdateAttribute(1);

		//##受伤后的AI处理，主要是怪物使用。
		if (pAttacker!=NULL)
		{
			if (GetAI())
			{
				GetAI()->WhenBeenHurted(pAttacker->GetType(),pAttacker->GetExID(),lHp);
			}
			OnBeenHurted( pAttacker->GetType(),pAttacker->GetExID()) ;
		}
	}
	else
	{
		if( TYPE_PLAYER == this->GetType() && TYPE_PLAYER == pAttacker->GetType() )// 是玩家攻击
			((CPlayer*)this)->SendNotifyWarningMsg(true, (CPlayer*)pAttacker);

		SetState(STATE_DIED);
		SetHP(0);	
		SetKillerInfo(pAttacker->GetExID(),pAttacker->GetType());
		pAttacker->ProduceEvent(ET_Kill);//注册杀死事件
		ProduceEvent(ET_BeenKilled);    //注册被杀死事件
		ProduceEvent(ET_EventBreak);
		return 1;
	}
	return 0;
}

//被杀的相关处理
void	CMoveShape::OnBeenKilled(CMoveShape* pAttacker)
{
	//##被杀的处理, 报告凶手
	if (pAttacker!=NULL)
	{
		OnBeenHurted( pAttacker->GetType(),pAttacker->GetExID()) ;
		OnBeenMurdered(  pAttacker->GetType(), pAttacker->GetExID() );
	}		
	//死亡
	ChangeStateTo((CShape::eSTATE)STATE_DIED);
	SetAction( ACT_ALREADYDIED );
	//停止所有技能
	StopAllSkills();
	//结束所有状态
	EndBuffAndDebuff();	

	if (NULL != pAttacker)
		SetKilledMeAttackInfo(pAttacker->GetType(),pAttacker->GetExID());

	if (GetAI() &&  pAttacker!=NULL)
	{
		GetAI()->WhenBeenKilled(pAttacker->GetType(),pAttacker->GetExID());		
	}
}

//设置攻击者的类型和guid
void CMoveShape::SetAttacker(long lType,const CGUID guid)
{
	m_lAttackerType = lType;
	m_guidAttacker = guid;
}

//根据键值获取主动技能对象
CBaseModule *CMoveShape::GetActiveSkill(long lActiveID)
{

	itModu it = m_ActiveSkillParam.find(lActiveID);
	if (it!=m_ActiveSkillParam.end())
	{
		return CNewSkillFactory::GetBaseModule((*it).second);
	}
	return NULL;
}

//保存buff数据用到的编解码
bool CMoveShape::AddBuffToByteArray( vector<BYTE>* pByteArray)
{
	vector<BYTE> TemptByteArray;TemptByteArray.clear();
	//buff
	DWORD dwSize = m_BuffParam.size()+m_DeBuffParam.size();
	_AddToByteArray(&TemptByteArray,dwSize);

	itModu it= m_BuffParam.begin();
	for(; it!=m_BuffParam.end(); it++)
	{

		_AddToByteArray(&TemptByteArray,(long)(*it).second->GetID());
		_AddToByteArray(&TemptByteArray,(long)(*it).second->GetLv());

		if (NULL==(*it).second->GetFirer())
		{
			_AddToByteArray(&TemptByteArray,(long)0);
			_AddToByteArray(&TemptByteArray,NULL_GUID);
		}
		else
		{
			_AddToByteArray(&TemptByteArray,(long)(*it).second->GetFirerType());
			_AddToByteArray(&TemptByteArray,(*it).second->GetFirerID());
		}
		(*it).second->AddToByteArray(&TemptByteArray);
	}
	it= m_DeBuffParam.begin();
	for(; it!=m_DeBuffParam.end(); it++)
	{
		_AddToByteArray(&TemptByteArray,(long)(*it).second->GetID());
		_AddToByteArray(&TemptByteArray,(long)(*it).second->GetLv());
		if (NULL==(*it).second->GetFirer())
		{
			_AddToByteArray(&TemptByteArray,(long)0);
			_AddToByteArray(&TemptByteArray,NULL_GUID);
		}
		else
		{
			_AddToByteArray(&TemptByteArray,(long)(*it).second->GetFirerType());
			_AddToByteArray(&TemptByteArray,(*it).second->GetFirerID());
		}
		(*it).second->AddToByteArray(&TemptByteArray);
	}


	//压
	DWORD dwDataSize = TemptByteArray.size();
	_AddToByteArray(pByteArray,dwDataSize);
	_AddToByteArray(pByteArray,&TemptByteArray[0],dwDataSize);
	return true;
}


bool CMoveShape::CodeToDataBlock_ForClient( DBWriteSet& setWriteDB, bool bExData)
{
	CShape::CodeToDataBlock_ForClient( setWriteDB,bExData );	
	setWriteDB.AddToByteArray(GetSpeedChangedRatio());  //速度的变化比率
	setWriteDB.AddToByteArray(static_cast<BYTE>(GetIsRun()));	//移动方式
	setWriteDB.AddToByteArray(static_cast<BYTE>(GetHide()));				//隐蔽
	setWriteDB.AddToByteArray(static_cast<BYTE>(GetInvisibleState()));		//隐形	
	setWriteDB.AddToByteArray(static_cast<BYTE>(GetPretendDeadState()));	//装死
	setWriteDB.AddToByteArray(static_cast<BYTE>(GetUnAttack()));			//不被攻击标志符
	setWriteDB.AddToByteArray(static_cast<BYTE>(GetIsRide()));				//非坐骑状态

	if(bExData)
	{
		setWriteDB.AddToByteArray(static_cast<BYTE>(GetAntiInvisibleable()));  //是否反隐形
		setWriteDB.AddToByteArray(static_cast<BYTE>(IsFightable()));			//攻击标志符	
		setWriteDB.AddToByteArray(static_cast<BYTE>(CanMove()));				//移动标志符
		setWriteDB.AddToByteArray(static_cast<BYTE>(GetUseRabbSkill()));		//使用法师技能
		setWriteDB.AddToByteArray(static_cast<BYTE>(GetUseFightSkill()));		//使用战士技能
		setWriteDB.AddToByteArray(static_cast<BYTE>(GetInstruction()));		//使用指令
		setWriteDB.AddToByteArray(static_cast<BYTE>(GetForceAttack()));		//强制攻击
		setWriteDB.AddToByteArray(static_cast<BYTE>(GetRandomMove()));		    //随机跑动
	}

	setWriteDB.AddToByteArray(GetChangeModelType());
	setWriteDB.AddToByteArray(GetChangeModelID());
	setWriteDB.AddToByteArray( (BYTE)IsDied());
	////添加buff
	if(!bExData)
	{
		CodeBuffToDataBlock_ForClient(setWriteDB);
		////添加debuff
		CodeDeBuffToDataBlocky_ForClient(setWriteDB);
	}
	if(bExData)
		CodeOnOffSkillToDataBlock_ForClient(setWriteDB);
	return true;
}

bool CMoveShape::CodeBuffToDataBlock_ForClient( DBWriteSet& setWriteDB)
{
	DWORD dwSize = m_BuffParam.size();
	setWriteDB.AddToByteArray(dwSize);

	itModu it= m_BuffParam.begin();
	for(;it!=m_BuffParam.end();it++)
	{		
		if ((*it).second!=NULL)
		{
			(*it).second->CodeToDataBlockForClient(setWriteDB,ModuType_Buff);
		}
	}
	return true;
}

bool CMoveShape::CodeDeBuffToDataBlocky_ForClient( DBWriteSet& setWriteDB)
{
	DWORD dwSize = m_DeBuffParam.size();
	setWriteDB.AddToByteArray(dwSize);

	itModu it= m_DeBuffParam.begin();
	for(;it!=m_DeBuffParam.end();it++)
	{		
		if ((*it).second!=NULL)
		{
			(*it).second->CodeToDataBlockForClient(setWriteDB,ModuType_DeBuff);
		}		
	}
	return true;
}

bool CMoveShape::CodeActSkillToDataBlock_ForClient( DBWriteSet& setWriteDB)
{
	if (m_ActiveSkillParam.size()>0)
	{
		setWriteDB.AddToByteArray((BYTE)1);
	} 
	else
	{
		setWriteDB.AddToByteArray((BYTE)0);
		return false;
	}

	setWriteDB.AddToByteArray((long)(m_ActiveSkillParam.size()));
	for (itModu it=m_ActiveSkillParam.begin(); 
		it!=m_ActiveSkillParam.end(); ++it)
	{
		setWriteDB.AddToByteArray((long)(it->first));//键值
		it->second->CodeToDataBlockForClient(setWriteDB,ModuType_Skill_Active);
	}
	return true;
}

bool CMoveShape::CodeOnOffSkillToDataBlock_ForClient( DBWriteSet& setWriteDB)
{
	DWORD dwSize = 0;
	setWriteDB.AddToByteArray(dwSize);	
	return true;
}

bool CMoveShape::CodeBuffToDataBlock(DBWriteSet& setWriteDB)
{
	vector<BYTE> TemptByteArray;TemptByteArray.clear();
	//buff
	DWORD dwSize = m_BuffParam.size()+m_DeBuffParam.size();
	_AddToByteArray(&TemptByteArray,dwSize);
	itModu it= m_BuffParam.begin();
	for(;it!=m_BuffParam.end();it++)
	{

		_AddToByteArray(&TemptByteArray,(long)(*it).second->GetID());
		_AddToByteArray(&TemptByteArray,(long)(*it).second->GetLv());

		if (NULL==(*it).second->GetFirer())
		{
			_AddToByteArray(&TemptByteArray,(long)0);
			_AddToByteArray(&TemptByteArray,NULL_GUID);
		}
		else
		{
			_AddToByteArray(&TemptByteArray,(long)(*it).second->GetFirerType());
			_AddToByteArray(&TemptByteArray,(*it).second->GetFirerID());
		}
		(*it).second->AddToByteArray(&TemptByteArray);
	}
	it= m_DeBuffParam.begin();
	for(;it!=m_DeBuffParam.end();it++)
	{
		_AddToByteArray(&TemptByteArray,(long)(*it).second->GetID());
		_AddToByteArray(&TemptByteArray,(long)(*it).second->GetLv());
		if (NULL==(*it).second->GetFirer())
		{
			_AddToByteArray(&TemptByteArray,(long)0);
			_AddToByteArray(&TemptByteArray,NULL_GUID);
		}
		else
		{
			_AddToByteArray(&TemptByteArray,(long)(*it).second->GetFirerType());
			_AddToByteArray(&TemptByteArray,(*it).second->GetFirerID());
		}
		(*it).second->AddToByteArray(&TemptByteArray);
	}

	//压
	DWORD dwDataSize = TemptByteArray.size();
	setWriteDB.AddToByteArray(dwDataSize);
	setWriteDB.AddToByteArray(&TemptByteArray[0],dwDataSize);
	return true;
}

bool CMoveShape::DecodeBuffFromDataBlock(DBReadSet& setReadDB)
{
	//总数据长度
	DWORD dwDataSize = setReadDB.GetDwordFromByteArray();
	if(dwDataSize==0)	return true;

	//解压buff
	DWORD dwSize = setReadDB.GetDwordFromByteArray();
	for(DWORD i=0;i<dwSize;i++)
	{
		long lID =  setReadDB.GetLongFromByteArray();
		long lLvl = setReadDB.GetLongFromByteArray();
		int nType = setReadDB.GetLongFromByteArray();
		CGUID guid;
		setReadDB.GetBufferFromByteArray(guid);


		long nProceNum = setReadDB.GetLongFromByteArray();
		long lTimerNum = setReadDB.GetLongFromByteArray();

		list<stTimerParam*> Timers;
		for(int n=0;n<lTimerNum;n++)
		{
			stTimerParam *pTimerParam = MP_NEW stTimerParam;
			pTimerParam->nCalledCount = 0;
			pTimerParam->lDestType = GetType();
			pTimerParam->DestID = GetExID();
			pTimerParam->lTimerID = -1;
			pTimerParam->lFlag = setReadDB.GetLongFromByteArray();
			pTimerParam->lFuture = setReadDB.GetDwordFromByteArray();
			pTimerParam->nCount = setReadDB.GetDwordFromByteArray();
			Timers.push_back(pTimerParam);
		}

		CNewSkill *pSkillProperty = CNewSkillFactory::QueryNewSkillProperty(lID);


		if (pSkillProperty && (pSkillProperty->GetType()==ModuType_Buff || pSkillProperty->GetType()==ModuType_DeBuff))
		{
			++m_ActiveID;
			//得到一个动态参数
			stModuParam* pStateParam = InitModuParam(lID, lLvl, m_ActiveID);
			if (pStateParam)
			{
				list<stTimerParam*>::iterator it = Timers.begin();
				if (pStateParam->GetTime())
				{
					time_t t_Current = time(NULL);
					long lFuture = ((*it)->lFuture - t_Current)*1000;
					if (lFuture<0)
					{
						(*it)->lFuture = 0;
					}
					else
					{
						(*it)->lFuture = lFuture;
					}
				}

				pStateParam->Reset(this,NULL);
				pStateParam->DecodeFromDataBlock(nProceNum,Timers);
				

				if(pStateParam->GetModType() == ModuType_Buff)
				{
					AddStateCount(pStateParam);
					m_BuffParam[pStateParam->GetGlobleID()] = pStateParam;				
				}
				else if(pStateParam->GetModType() == ModuType_DeBuff)	
				{
					AddStateCount(pStateParam);
					m_DeBuffParam[pStateParam->GetGlobleID()] = pStateParam;			
				}
			}		
		}
	}
	return true;
}

VOID CMoveShape::ChangeStateTo(CShape::eSTATE state)
{
	SetState( state );

	CMessage msg( MSG_S2C_SHAPE_CHANGESTATE );
	msg.Add( GetType() );
	msg.Add( GetExID() );
	msg.Add( static_cast<LONG>(state) );
	msg.SendToAround( this );

	if (state==STATE_FIGHT)
	{
		ProduceEvent(ET_Fighting);
	} 
	else if (state==STATE_PEACE)
	{
		ProduceEvent(ET_Peace);
	}
}

VOID CMoveShape::OnEnterRegion(bool bIsRelive)
{
	CRegion* pRegion = dynamic_cast<CRegion*>( GetFather() );

	if( pRegion == NULL )
	{

		assert( 0 );
		return;
	}

	//##重置计数器
	m_lMoveableCount	= 0;
	m_lCanFightCount	= 0;
	m_bMoveable			= TRUE;
	m_bCanFight			= TRUE;	
}

//结束被动技能
VOID	CMoveShape::StopPassiveSkills()
{	
	itModu it=m_PassiveSkillParam.begin();
	for (; it!=m_PassiveSkillParam.end(); ++it)
	{		
		CBaseModule *pSkill = CNewSkillFactory::GetBaseModule((*it).second);
		EndSkill(pSkill);
	}
	m_PassiveSkillParam.clear();
}

//结束技能
VOID	CMoveShape::StopSkills(long lID,long lLv,long lType)
{
	if (lType==ModuType_Skill_Active)
	{
		StopActiveSkill(lID,lLv);
	}
	else if (lType==ModuType_Skill_Passive)
	{
		StopPassiveSkill(lID,lLv);
	}
}

//结束技能
void CMoveShape::EndSkill(CBaseModule *pBaseModule)
{
	if (pBaseModule)
	{
		pBaseModule->End();
		CNewSkillFactory::RealeaseBaseMoudule(pBaseModule->GetParam());
	}
}

//结束主动技能
VOID CMoveShape::StopActiveSkill(long lID,long lLv)
{
	//tagModuleKey tgKey;
	itModu it=m_ActiveSkillParam.begin();
	for (; it!=m_ActiveSkillParam.end();)
	{	
		CBaseModule *pSkill = CNewSkillFactory::GetBaseModule((*it).second);
		if (pSkill)
		{
			if (lLv==0)
			{
				if (lID=it->second->GetID())
				{				
					EndSkill(pSkill);
					it = m_ActiveSkillParam.erase(it);
				}	
				else
				{
					++it;
				}
			}
			else
			{
				if (lID==it->second->GetID() && lLv==it->second->GetLv())
				{
					EndSkill(pSkill);
					it = m_ActiveSkillParam.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		else
		{
			++it;
		}
	}
}
//停止被动技能
VOID CMoveShape::StopPassiveSkill(long lID,long lLv)
{
	itModu it=m_PassiveSkillParam.begin();
	for (; it!=m_PassiveSkillParam.end();)
	{	
		CBaseModule *pSkill = CNewSkillFactory::GetBaseModule((*it).second);
		if (pSkill)
		{
			if (lLv==0)
			{
				if (lID=it->second->GetID())
				{				
					EndSkill(pSkill);
					it = m_PassiveSkillParam.erase(it);
				}
				else
				{
					++it;
				}
			}
			else
			{
				if (lID==it->second->GetID() && lLv==it->second->GetLv())
				{
					EndSkill(pSkill);
					it = m_PassiveSkillParam.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		else
		{
			++it;
		}
	}
}


VOID CMoveShape::StopAllSkills()
{	
	itModu it=m_ActiveSkillParam.begin();
	for(;it!=m_ActiveSkillParam.end();)
	{	
		CBaseModule *pSkill = CNewSkillFactory::GetBaseModule((*it).second);
		if (pSkill)
		{
			EndSkill(pSkill);
			it = m_ActiveSkillParam.erase(it);
		}
		else
		{
			++it;
		}
	}	
}
//释放主动技能参数
VOID CMoveShape::ClearActiveSkills()
{
	if (m_ActiveSkillParam.size()>0)
	{
		itModu it=m_ActiveSkillParam.begin();
		for(;it!=m_ActiveSkillParam.end();++it)
		{		
			CNewSkillFactory::RealeaseBaseMoudule((*it).second); 				
		}
		m_ActiveSkillParam.clear();
	}
}

VOID CMoveShape::ClearAllStates(bool bIsDead)
{

}

VOID CMoveShape::StartAllStates(bool bIsRelive)
{
	CRegion* pRegion = dynamic_cast<CRegion*>( GetFather() );	
}


VOID CMoveShape::OnChangeStates()
{
	CMessage msg( MSG_S2C_SKILL_STATE_CHANGE );
	msg.Add( GetType() );
	msg.Add( GetExID() );
	msg.Add( GetHP() );
	msg.Add( static_cast<WORD>(0) );
	msg.Add( static_cast<WORD>(0) );
	msg.Add( static_cast<WORD>(0) );
	msg.SendToAround( this );
}

BOOL CMoveShape::IsFaceTo( CMoveShape* pShape )
{
	if( pShape )
	{
		if( Distance( pShape ) == 1 )
		{
			LONG lX = GetTileX();
			LONG lY = GetTileY();
			GetDirPos( GetDir(), lX, lY );

			if( lX != pShape -> GetTileX() ||
				lY != pShape -> GetTileY() )
			{
				return FALSE;
			}

			lX = pShape -> GetTileX();
			lY = pShape -> GetTileY();
			pShape -> GetDirPos( pShape -> GetDir(), lX, lY );

			if( lX != GetTileX() ||
				lY != GetTileY() )
			{
				return FALSE;
			}

			//##检测全部通过。返回真
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

VOID CMoveShape::OnMove(float fNewX, float fNewY)
{
	ProduceEvent(ET_Move);
	ProduceEvent(ET_EventBreak);

	//添加测试代码
	CRegion* p=dynamic_cast<CRegion*>(GetFather());
	if (fNewX<0 || fNewX> p->GetWidth() || fNewY<0 || fNewY> p->GetHeight())
	{
		char str[512]="";
		_snprintf(str, 512, "x=%f,y=%f\n",fNewX,fNewY);
		PutStringToFile("越界记录",str);
	}

	SetDir(GetLineDir(GetTileX(),GetTileY(),(long)fNewX,(long)fNewY));

	CMessage msg(MSG_S2C_SHAPE_MOVE);
	msg.Add( GetExID() );
#ifdef ENABLE_FOALT_POS
	msg.Add( fNewX );
	msg.Add( fNewY );
#else
	msg.Add( (short)fNewX );
	msg.Add( (short)fNewY );
#endif
	if(GetType() == TYPE_MONSTER )
		msg.SendToAround(this);
	else
		msg.SendAroundLimitPlayer(this,NULL_GUID,true);		
}

VOID CMoveShape::OnStopMove()
{
	CMessage msg(MSG_S2C_SHAPE_STOPMOVE);
	msg.Add( GetExID() );
#ifdef ENABLE_FOALT_POS
	msg.Add( GetPosX() );
	msg.Add( GetPosY() );
#else
	msg.Add( (short)GetTileX() );
	msg.Add( (short)GetTileY() );
#endif
	msg.Add( (BYTE)GetDir() );
	msg.SendToAround(this);
}	

VOID CMoveShape::OnCannotMove()
{
}

VOID CMoveShape::SetPosition( LONG lX, LONG lY )
{

	//##先判断是否可以过去.然后再过去
	CServerRegion* pRegion = dynamic_cast<CServerRegion*>( GetFather() );
	if( pRegion )
	{
		if( lX < pRegion -> GetWidth() && 
			lX >= 0 &&
			lY < pRegion -> GetHeight() &&
			lY >= 0 && 
			pRegion -> GetBlock(lX, lY) == CRegion::BLOCK_NO )
		{			
			CMessage msg( MSG_S2C_SHAPE_SETPOS );
			msg.Add( GetType() );
			msg.Add( GetExID() );
			msg.Add( lX );
			msg.Add( lY );
			msg.Add( static_cast<LONG>(0) );
			msg.SendToAround( this );

			SetPosXY( lX+0.5f, lY+0.5f );
		}
	}
}
VOID CMoveShape::SetSlipPos(LONG lX, LONG lY,float fSpeed,float fDir)
{
	//##先判断是否可以过去.然后再过去
	CServerRegion* pRegion = dynamic_cast<CServerRegion*>( GetFather() );
	if( pRegion )
	{
		if( lX < pRegion -> GetWidth() && 
			lX >= 0 &&
			lY < pRegion -> GetHeight() &&
			lY >= 0 && 
			pRegion -> GetBlock(lX, lY) == CRegion::BLOCK_NO )
		{
			ProduceEvent(ET_Move);
			ProduceEvent(ET_EventBreak);
			CMessage msg( MSG_S2C_SHAPE_SLIPPOS );
			msg.Add( GetType() );
			msg.Add( GetExID() );
			msg.Add( lX );
			msg.Add( lY );
			msg.Add( fSpeed );
			msg.Add( fDir);
			msg.SendToAround( this );

			SetPosXY( lX+0.5f, lY+0.5f );
		}
	}
}

BOOL CMoveShape::AddSkill( const char* strName, LONG lLevel )
{
	if( strName == NULL ) 
		return FALSE;	
	return TRUE;
}

BOOL CMoveShape::AddSkill( tagSkillID tID, LONG lLevel )
{	
	return true;
}

BOOL CMoveShape::DelSkill( long lSkillID ,long lSkillLv )
{	
	return TRUE;
}

BOOL CMoveShape::SetSkillLevel( const char* strName, LONG lLevel )
{
	return AddSkill( strName, lLevel );
}

//定制一个脚本定时器
long CMoveShape::RegisterScriptTimer(long lFutureTime,long lInterTime,
									 const char* pszScriptName,long lUserData)
{
	long lTimerID = -1;
	if(NULL == pszScriptName)	return lTimerID;
	tagScriptTimer scripttimer = {lUserData,pszScriptName,lInterTime>0?true:false};
	tagTimerVar* pTimvar = CreateTimerVar(3);
	pTimvar->TimerType = ChildObjType_RunScript;
	lTimerID = TIMER_SCHEDULE(this,(void*)pTimvar,timeGetTime()+lFutureTime,lInterTime);
	if(lTimerID != -1)
	{
		m_ScriptTimers[lTimerID]=scripttimer;
	}
	else
	{
		PutDebugString("定制脚本定时器是出错!");
	}
	return lTimerID;
}
//注销一个脚本定时器
long CMoveShape::UnRegisterScriptTimer(long lUserData,long lTimerID)
{
	long lTimerNum = 0;
	itST it = m_ScriptTimers.begin();
	for(;it != m_ScriptTimers.end();)
	{
		tagScriptTimer &scripttimer = (*it).second;
		if( scripttimer.lUserData == lUserData &&
			(lTimerID == -1 ||  (*it).first == lTimerID) )
		{
			TIMER_CANCEL((*it).first);
			it = m_ScriptTimers.erase(it);
			lTimerNum++;
		}
		else
		{
			it++;
		}
	}
	return lTimerNum;
}

//脚本定时器触发
void CMoveShape::OnScripttTimeOut(long lTimerID)
{
	itST it = m_ScriptTimers.find(lTimerID);
	if(it != m_ScriptTimers.end())
	{
		//执行脚本
		tagScriptTimer &scripttimer = (*it).second;
		stRunScript sScript;
		sScript.pszFileName = const_cast<char*>(scripttimer.strScript.c_str());
		sScript.pRegion = dynamic_cast<CRegion*>(GetFather());
		sScript.srcShape	= this;
		sScript.desShape = NULL;
		RunScript( &sScript, const_cast<CHAR*>( GetGame()->GetScriptFileData(sScript.pszFileName) ) );
		if(!scripttimer.bLoop)
			m_ScriptTimers.erase(it);
	}
}
//清除所有脚本定时器
void CMoveShape::ClearScriptTimer()
{
	itST it = m_ScriptTimers.begin();
	for(;it != m_ScriptTimers.end();it++)
	{
		TIMER_CANCEL((*it).first);
	}
	m_ScriptTimers.clear();
}

void CMoveShape::StartUpState(CMoveShape *pUser, stModuParam *pModuParam, mapModules &mapState)
{
	CNewSkillProperties *pNSkillProperty = CNewSkillFactory::QueryNewSkillProperty(pModuParam->GetID());
	if (NULL==pNSkillProperty || NULL==pModuParam)
	{
		return;
	}
	DWORD dwID		= pModuParam->GetID();
	DWORD dwLv		= pModuParam->GetLv();
	DWORD dwGlobleID= pModuParam->GetGlobleID();


	itModu it =	mapState.begin();
	for(;it!=mapState.end();)
	{
		stModuParam *pOldState =it->second;

		if(pNSkillProperty->GetOnly()==1)
		{
			//类型唯一
			CNewSkillProperties *pSkillProperty = CNewSkillFactory::QueryNewSkillProperty(pOldState->GetID());
			if (pSkillProperty)
			{
				if( pNSkillProperty->GetOnlyType()==pSkillProperty->GetOnlyType())
				{
					if (pOldState->GetID()==dwID && dwLv<pOldState->GetLv() )
					{	
						//新加状态有原状ID相同且等级低就直接返回
						CNewSkillFactory::RealeaseBaseMoudule(pModuParam);
						return;
					}
					else
					{
						//删除原有同类型的状态（类型唯一）
						RemoveState(it);
						continue;
					}
				}			
			}	
			++it;
		}
		else if (pNSkillProperty->GetNameOnly()==1)
		{
			if (pOldState->GetID()==dwID)
			{	
				if (dwLv>=pOldState->GetLv())							
				{	
					//删除原有相同或低等级的同ID状态（名称唯一）
					RemoveState(it);
					continue;
				}			
			}			
			++it;

		}
		else if (pNSkillProperty->GetRoleOnly()==1)
		{
			//角色唯一			
			if (pOldState->GetID()==dwID)
			{				
				if (pModuParam->GetFirerID()==pOldState->GetFirerID())							
				{
					if (dwLv>=pOldState->GetLv())
					{
						//删除原有相同或低等级的同ID状态（角色唯一）
						RemoveState(it);
						continue;
					}								
				}				
			}					
			++it;			
		}
		else if (pNSkillProperty->GetTotalNum()>0)
		{
			//叠加
			if (pOldState->GetID()==dwID)
			{				
				if (pOldState->GetLv()<dwLv)
				{
					//删除原有低等级的同ID状态（可叠加）
					RemoveState(it);
					continue;
				}
				else if(pOldState->GetLv()>dwLv)
				{	
					//新状态等级小于原有状态（同ID）就直接返回
					CNewSkillFactory::RealeaseBaseMoudule(pModuParam);
					return;
				}

				if (GetStateCount(pModuParam)>=pNSkillProperty->GetTotalNum())
				{
					//状态数量超过设定的叠加数量就直接返回
					ProduceEvent(ET_AddState,dwID);						
					CNewSkillFactory::RealeaseBaseMoudule(pModuParam);
					return;
				}						
			}
			++it;
			continue;			
		}
		else
		{
			//高等级替代低等级
			if (pOldState->GetID()==dwID)
			{	
				if (dwLv>pOldState->GetLv())				
				{
					//删除原有低等级的同ID状态
					RemoveState(it);				
					continue;
				}
				else if (dwLv<pOldState->GetLv())				
				{	
					//新状态等级小于原有状态（同ID）就直接返回
					CNewSkillFactory::RealeaseBaseMoudule(pModuParam);
					return;
				}
			}
			++it;
		}		
	}

	CBaseModule *pBaseModule = CNewSkillFactory::GetBaseModule(pModuParam);
	if (pBaseModule->Begin()==MR_Step_End)
	{
		//状态启动成功

		AddStateCount(pBaseModule->GetParam());
		pBaseModule->GetParam()->SetbRun(true);

		ProduceEvent(ET_AddState,dwID);
		OnAddBuff(dwID, dwLv);
		if (1 == pNSkillProperty->GetBreak())
		{
			//状态中断
			ProduceEvent(ET_StateBreak);
		}

		itModu it = mapState.find(dwGlobleID);
		if (it==mapState.end())
		{
			mapState[dwGlobleID] = pModuParam;
		}
	}
	else
	{	
		//状态启动失败释放
		CNewSkillFactory::RealeaseBaseMoudule(pModuParam);
	}
}

//添加Buff
void CMoveShape::AddNewState(CMoveShape *pUser,DWORD dwStateID, DWORD dwLevel,long lType)
{
#ifdef _RUNSTACKINFO3_
	char pszStatckInfo[256]="";
	_snprintf(pszStatckInfo,256,"CMoveShape::AddNewState(ID:%d,Name:%s,Type:%d) Start",dwStateID,GetName(),lType);
	CMessage::AsyWriteFile(GetGame()->GetStatckFileName(),pszStatckInfo);
#endif
	if (NULL == pUser)
	{
		return;
	}

	CNewSkill *pSkillProperty = CNewSkillFactory::QueryNewSkillProperty(dwStateID);

	if (IsDied())
	{
		return;
	}

	if (pSkillProperty && (pSkillProperty->GetType()==ModuType_Buff || pSkillProperty->GetType()==ModuType_DeBuff))
	{
		++m_ActiveID;
		stModuParam *pModuParam = InitModuParam(dwStateID,dwLevel,m_ActiveID);

		if (pModuParam)
		{
			pModuParam->Reset(this,pUser);				
			CBaseModule* pState = CNewSkillFactory::CreateModule(dwStateID,dwLevel,pModuParam,m_ActiveID,0,lType);

			if (pState)
			{
				if(pModuParam->GetModType()==ModuType_Buff)
				{
					//Buff
					OnBeenAddAppendSkill(pUser);
					StartUpState(pUser,pModuParam,m_BuffParam);
					if (this->GetState()==STATE_FIGHT)
					{
						pUser->GetAI()->ChangeStateTo(STATE_FIGHT);
					}

				}
				else if(pModuParam->GetModType()==ModuType_DeBuff)
				{
					//DeBuff
					if( TYPE_PET == pUser->GetType() &&
						PET_TYPE_ELF == ((CPet*)pUser)->GetPetType() )
					{
						GetAI()->WhenBeenHurted(pUser->GetType(),pUser->GetExID(),0);
					}
					else
					{
						GetAI()->WhenBeenHurted(pUser->GetType(),pUser->GetExID(),0);
					}
					StartUpState(pUser,pModuParam,m_DeBuffParam);
				}
			}
			else
			{
				//创建对象实例失败
				CNewSkillFactory::RealeaseBaseMoudule(pModuParam);
			}
		}
	}
}

long CMoveShape::OnTimeOut(DWORD timerid,DWORD curtime,const void* var)
{
	if(var)
	{
		stModuParam *pModuParam = NULL;
		eModuleRet ret = MR_No;

		tagTimerVar* timervar = (tagTimerVar*)(var);
		switch(timervar->TimerType)
		{
		case ChildObjType_Buff:
			{
				itModu itBuff = m_BuffParam.find(timervar->lvar);
				if (itBuff!=m_BuffParam.end())
				{
					pModuParam = itBuff->second;				
				}			
			}
			break;
		case ChildObjType_DeBuff:
			{
				itModu itBuff = m_DeBuffParam.find(timervar->lvar);
				if (itBuff!=m_DeBuffParam.end())
				{
					pModuParam = itBuff->second;					
				}			
			}
			break;	

		case ChildObjType_ActiveSkill:
			{
				itModu it = m_ActiveSkillParam.find(timervar->lvar);
				if (it!=m_ActiveSkillParam.end())
				{
					pModuParam = it->second;
					if (pModuParam && pModuParam->GetProcNum()==0 && pModuParam->FindTimerParam(timerid)->nCalledCount==0)
					{
						ProduceEvent(ET_SuccessSkill);
					}
				}			
			}
			break;

		case ChildObjType_AI:
			{
				int b=0;
				if(this->GetType()==400)
				{
					b=1;
				}
				m_pAI->Run(timerid,(int)timervar->lvar);
			}
			break;
		case ChildObjType_RunScript:
			{
				OnScripttTimeOut(timerid);
			}
			break;
		case ChildObjType_Beneficiary:
			{
				OnLoseBeneficiary();
			}
			break;
		case ChildObjType_CollectionDied:
			{
				this->OnDiedEvent();
			}
			break;
		case ChildObjType_UpdateAttr:
			{
				OnUpdateAttrTimeOut();
			}
			break;
		}
		if (pModuParam)
		{
			CBaseModule *pBaseModule = CNewSkillFactory::GetBaseModule(pModuParam);
			if(pBaseModule)
			{
				ret = pBaseModule->OnTimeOut(timerid,curtime,var);
				if (ret == MR_Modu_End)
				{
					RemoveBaseModule(pModuParam);
				}
			}

		}

	}
	return 0;
}

//! 得到所BUFF的ID
void CMoveShape::GetAllBuffID(list<pair<DWORD, DWORD>>& listBuffID)	
{
	listBuffID.clear();

	mapModules::iterator ite = m_BuffParam.begin();
	for (; ite != m_BuffParam.end(); ++ite)
	{
		listBuffID.push_back(pair<DWORD, DWORD>(ite->second->GetID(), ite->second->GetLv()));
	}

	//debuff
	ite = m_DeBuffParam.begin();
	for (; ite != m_DeBuffParam.end(); ++ite)
	{
		listBuffID.push_back(pair<DWORD, DWORD>(ite->second->GetID(), ite->second->GetLv()));
	}
}

void CMoveShape::OnAddBuff(DWORD dwStateID, DWORD dwBuffLevel)
{

}

void CMoveShape::OnDelBuff(DWORD dwStateID, DWORD dwBuffLevel)
{

}

void CMoveShape::EndActiveSkill(long lID,long lKeyID)
{
	if (lKeyID!=-1)
	{
		itModu it = m_ActiveSkillParam.find(lKeyID);
		if (it!=m_ActiveSkillParam.end())
		{
			CBaseModule *pActiveSkill = CNewSkillFactory::GetBaseModule(it->second);
			if (pActiveSkill)
			{
				pActiveSkill->End();				
				RemoveActive(pActiveSkill);	
				it=m_ActiveSkillParam.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	else
	{
		if (m_ActiveSkillParam.size()>0)
		{
			itModu it = m_ActiveSkillParam.begin();
			for(;it!=m_ActiveSkillParam.end();)
			{
				CBaseModule *pActiveSkill =  CNewSkillFactory::GetBaseModule(it->second);
				if (pActiveSkill && pActiveSkill->GetParam()->GetID()==lID)
				{
					pActiveSkill->End();			
					RemoveActive(pActiveSkill);
					it = m_ActiveSkillParam.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

	}

}



long CMoveShape::Proc(eEventType type,const tagProcVar *pVar)
{
	stModuParam *pModuParam=NULL;	

	eModuleRet ret = MR_No;

	switch(pVar->lType)
	{
	case ChildObjType_Buff:
		{
			itModu itBuff = m_BuffParam.find(pVar->lID);
			if (itBuff!=m_BuffParam.end())
			{
				pModuParam = itBuff->second;
			}
		}
		break;
	case ChildObjType_DeBuff:
		{
			itModu itBuff = m_DeBuffParam.find(pVar->lID);
			if (itBuff!=m_DeBuffParam.end())
			{
				pModuParam = itBuff->second;
			}
		}
		break;
	case ChildObjType_ActiveSkill:
		{

			itModu it = m_ActiveSkillParam.find(pVar->lID);
			if (it!=m_ActiveSkillParam.end())
			{
				pModuParam = it->second;		
			}			
		}
		break;
	case ChildObjType_PassiveSkill:
		{
			itModu it = m_PassiveSkillParam.find(pVar->lID);
			if (it!=m_PassiveSkillParam.end())
			{
				pModuParam = it->second;			
			}
		}
		break;
	};
	if (pModuParam)
	{
		CBaseModule *pBaseModule = CNewSkillFactory::GetBaseModule(pModuParam);
		if (pBaseModule)
		{
			ret = pBaseModule->Proc(type,pVar);
			if (ret==MR_Modu_End)
			{	
				if (pVar->lType==ChildObjType_ActiveSkill)
				{
					this->GetAI()->OnSkillEnd(pBaseModule->GetParam()->GetAddSub());
				}
				RemoveBaseModule(pModuParam);

			}
		}

	}

	CShape::Proc(type,pVar);
	return true;
}



long CMoveShape::SkillReinforceBegin(bool isMagic, bool isAttack)
{
	return 0;
}
//!									技能加成结束（清空效果缓存）
void CMoveShape::SkillReinforceEnd(long lKey)
{
}
//! 获得技能加成值（first：吟唱时间变化比例；second：效果值变化比例）
const pair<float, float>&	CMoveShape::GetSkillReinforceAmend(long lKey)
{
	return m_FixedReinforce;
}


VOID	CMoveShape::SetCollectionID(DWORD l)
{
	m_dwCollectionID=l;
	AddAttributeToDirty("dwCollectionID");
	UpdateAttribute(true);
}

VOID	CMoveShape::SetCollectionTimes(DWORD l)
{
	m_dwCollectTimes=l;
	AddAttributeToDirty("dwCollectionTimes");
	UpdateAttribute(true);
}

VOID	CMoveShape::SetIsBeenCollect(bool l)
{
	m_bIsBeenCollect=l;
	AddAttributeToDirty("bIsBeenCollect");
	UpdateAttribute(true);
}

VOID	CMoveShape::SetCanBeenCollect(bool l)
{
	m_bCanBeenCollect=l;
	AddAttributeToDirty("bCanBeenCollect");
	UpdateAttribute(true);
}

BOOL	CMoveShape::BeenCollected()
{
	SetCollectionTimes(GetCollectionTimes()-1);
	if (GetCollectionTimes()<=0)
	{
		BeenCollectedAll();
	}
	return TRUE;
}
// 重载接口
int CMoveShape::SetScriptVarValue(CScript* scriptPtr, const char* varName, const char* value)
{
	return -1;
}
int CMoveShape::SetScriptVarValue(CScript* scriptPtr, const char* varName, long lArray, double value)
{
	return -1;
}
const char* CMoveShape::GetScriptVarValue(CScript* scriptPtr, const char* varName)
{
	return NULL;
}
double CMoveShape::GetScriptVarValue(CScript* scriptPtr, const char* varName, long lArray)
{
	return -1.0f;
}

int CMoveShape::SetScriptGuidValue(CScript* scriptPtr, const char* varName, const CGUID& guid)
{
	return -1;
}
const CGUID& CMoveShape::GetScriptGuidValue(CScript* scriptPtr, const char* varName)
{
	return NULL_GUID;
}

CVariableList* CMoveShape::GetVariableList(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//	受益者
//////////////////////////////////////////////////////////////////////////
void CMoveShape::SetBeneficiary(long lType,CGUID guidFirst)
{
	if (m_BeneficiaryID== guidFirst)
	{
		return;
	}
	m_BeneficiaryID= guidFirst;
	m_BeneficiaryType= (OBJECT_TYPE)lType;

	CMessage mes(MSG_S2C_SHAPE_CHANGE_FIRSTATTACKER);
	mes.Add(GetExID());					//被设置受益者guid
	mes.Add(GetType());					//被设置受益者type
	mes.Add(m_BeneficiaryID);			//受益者guid	
	mes.Add((long)m_BeneficiaryType);	//受益者type
	mes.SendToAround(this);
}

void CMoveShape::CleanBeneficiary()			//清空受益者
{
	if (m_lBeneficiaryTimerID!= -1)
	{
		GameManager::GetInstance()->Cancel(m_lBeneficiaryTimerID,0);
	}
	SetBeneficiary(0,NULL_GUID);
	m_lBeneficiaryTimerID=-1;

}
long CMoveShape::AddBeneficiaryEvent(long lTime)		//添加受益者事件
{
	if (m_lBeneficiaryTimerID!= -1) return 0;
	if (lTime<=0) return 0;

	tagTimerVar* pTimvar =  CreateTimerVar(4);
	pTimvar->TimerType = ChildObjType_Beneficiary;
	pTimvar->lvar = 0;
	m_lBeneficiaryTimerID= GameManager::GetInstance()->Schedule(this,(void*)pTimvar,timeGetTime()+lTime,0);
	return 0;
}

void CMoveShape::AddDelEvent(DWORD dwTime)					// 注册删除事件
{
	if (m_lDelEventTimerID!=-1)	return ;

	tagTimerVar* pTimvar =  CreateTimerVar(5);
	pTimvar->TimerType = ChildObjType_DeleteObj;
	//pTimvar->lvar= (long)this;
	pTimvar->pvar = M_ALLOC(sizeof(stDestObj));
	((stDestObj*)pTimvar->pvar)->DestID= GetExID();
	((stDestObj*)pTimvar->pvar)->nDestType= GetType();
	pTimvar->pvar_size = sizeof(stDestObj);
	m_lDelEventTimerID=GameManager::GetInstance()->Schedule(GetFather(),(void*)pTimvar,timeGetTime()+dwTime);
	return ;
}

void CMoveShape::LogoutDelEvent()				// 注销删除事件
{
	if (m_lDelEventTimerID==-1)
	{
		return ;
	}
	GameManager::GetInstance()->Cancel(m_lDelEventTimerID,0);
	m_lDelEventTimerID=-1;

	return ;
}
void CMoveShape::CleanDelEvent()				// 清空删除事件
{
	m_lDelEventTimerID=-1;
}


//注册更新属性的定时器
void CMoveShape::RegisterUpdateAttrTimer()
{
	if(m_lUpdateAttrTimerID != -1 )
		return;
	tagTimerVar* pTimvar =  CreateTimerVar(6);
	pTimvar->TimerType = ChildObjType_UpdateAttr;
	pTimvar->pvar = NULL;
	m_lUpdateAttrTimerID = GameManager::GetInstance()->Schedule(this,(void*)pTimvar,timeGetTime()+CGlobeSetup::GetSetup()->lUpdateOtherPropertyDelayTime);
}

//注销更新属性定时器
void CMoveShape::CancelUpdateAttrTimer()
{
	GameManager::GetInstance()->Cancel(m_lUpdateAttrTimerID);
	m_lUpdateAttrTimerID=-1;
}

//触发属性定时器
void CMoveShape::OnUpdateAttrTimeOut()
{
	OnUpdateAttributeTimeOut();
	m_lUpdateAttrTimerID=-1;
}
//设置对象参数
void CMoveShape:: SetTarget(CGUID guid,long lType)
{
	m_guidTarget = guid;
	m_lTargetType = lType;
}
//获取对象参数
void CMoveShape::GetTarget(CGUID &guidTarget,long &lTargetType)
{
	guidTarget =  m_guidTarget;
	lTargetType = m_lTargetType;
}

//设置刺杀者参数
void	CMoveShape::SetKillerInfo(CGUID guid,long lType)
{
	m_guidKiller = guid;
	m_lKillerType = lType;
}

//获取刺杀者参数		
void	CMoveShape::GetKillerInfo(CGUID &guidKiller,long &lKillerType)
{
	guidKiller = m_guidKiller;
	lKillerType = m_lKillerType;
}
//设置攻击目标参数
void	CMoveShape::SetAttackTarget(CGUID guid,long lType)
{
	m_guidAttackTarget  = guid;
	m_lAttackTargetType = lType;
}
//获取攻击目标参数
void	CMoveShape::GetAttacktarget(CGUID &guid,long &lType)
{
	guid = m_guidAttackTarget;
	lType = m_lAttackTargetType;
}




void CMoveShape::AddSelectSelfPlayer(const CGUID& PlayerID)
{
	if(PlayerID != NULL_GUID)
		m_SelectSelfPlayers.insert(PlayerID);
}

void CMoveShape::RemoveSelectSelfPlayer(const CGUID& PlayerID)
{
	m_SelectSelfPlayers.erase(PlayerID);
}


//重置发送消息的时间,如果超时则返回true
bool CMoveShape::ResetSendMoveMsgTime()
{
	DWORD dwCurTime = timeGetTime();
	if( (dwCurTime - m_dwLastSendMoveSendTime) > 4000)
	{
		m_dwLastSendMoveSendTime= dwCurTime;
		return true;
	}
	return false;
}
/*
<!--================================================================-->
<!--玩家PK日志-->
<Table name="t130_player_attack_log" log_no="130" log_pos="1">
<item type="client_head"     head_name=""></item>
<item type="pos"      head_name=""></item>
<item type="player"   head_name="sponsor"></item>
<item type="player"   head_name="participator"></item>
</Table>
*/
bool LogicLogInterface::logT130_player_attack_log(CPlayer *pSponsor, CPlayer *pParticipator)
{
	if(0 == GetGame()->GetSetup()->lUseLogServer) 
		return true;
	if(NULL == m_pLogClient)
		return false;
	if(NULL == pSponsor || NULL == pParticipator)
		return false;
	CRegion *pRegion = dynamic_cast<CRegion*>(GetGame()->FindRegion(pParticipator->GetRegionGUID()));
	if(!pRegion)
		return false;

	string strTime;
	GetCurrTimeString(strTime);

	float posX = pParticipator->GetTileX();
	float posY = pParticipator->GetTileY();

	return m_pLogClient->SendLogToServer(
		130, strTime.c_str(), 
		pRegion->GetID(), pRegion->GetName(), &posX, &posY,
		&(pSponsor->GetExID()), pSponsor->GetName(),
		&(pParticipator->GetExID()), pParticipator->GetName()
		);
}
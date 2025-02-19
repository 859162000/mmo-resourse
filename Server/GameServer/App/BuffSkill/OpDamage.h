//=============================================================================
/**
 *  file: OpDamage.h
 *
 *  Brief:包装对伤害值的操作
 *
 *  Authtor:wangqiao
 *	
 *	Datae:2007-9-7
 */
//=============================================================================

#pragma once

#include "Operator.h"
class CMoveShape;

const  float  lDamMulti = 100.0;
class COpDamage :
	public COperator
{
public:
	COpDamage(stModuParam *pParam);
	COpDamage(void *pParam);
	virtual ~COpDamage(void);
	//操作类型
	enum eDamageOper
	{
		//加操作
		DAMAOP_Add,
		//减操作
		DAMAOP_Sub,
        //乘法操作
        DAMAOP_Multi,
		//设置操作
		DAMAOP_Set,
		//清除
		DAMAOP_Clear,
		//作用于对象
		DAMAOP_Do,
		//加法加血
		MODOP_Inc,
        //乘法加血
        MODOP_MultiInc,
	};

private:
	//伤害类型
	tagDamageType m_lType;
	//值
	long m_lValue;
	//变量名
	string m_strVariName;
    //@by:expter 伤害变量
    string m_strDamParamname;

	//操作函数
	typedef void (COpDamage::*OPFUN)(CMoveShape* pShape);
	OPFUN m_pFun;

	void AddDamage(CMoveShape* pShape);
	void SubDamge(CMoveShape* pShape);
    void MultiDamge(CMoveShape* pShape);
	void SetDamge(CMoveShape* pShape);
	void ClearDamge(CMoveShape* pShape);
	void DoDamage(CMoveShape* pShape);
	void IncHP(CMoveShape* pShape);
    void MultiInc(CMoveShape* pShape);


public:
	OPFUN GetFun(string Oper)
	{
		OPFUN op;

		if (Oper=="Add")
		{
			op = &COpDamage::AddDamage;//OPFUN *
		} 
		else if(Oper=="Sub")
		{
			op = &COpDamage::SubDamge;
		}
        else if (Oper=="Multi")
        {
            op = &COpDamage::MultiDamge;
        }
		else if (Oper=="Set")
		{
			op = &COpDamage::SetDamge;
		}
		else if (Oper=="Clear")
		{
			op = &COpDamage::ClearDamge;
		}	
		else if (Oper=="Inc")
		{
			op = &COpDamage::IncHP;
		}
        else if (Oper=="MultiInc")
        {
            op = &COpDamage::MultiInc;
        }
		else
		{
			op = &COpDamage::AddDamage;
		}
		return op;
	}

	//初始化
	void InitBaseData(eObj obj,string strOper,tagDamageType dType,long value,string strVname,string Damagename)
	{
		//m_eOp = Oper;
		m_lType = dType;
		m_lValue = value;
		m_strVariName = strVname;		
		m_pFun = GetFun(strOper);
		m_Obj = obj;
        m_strDamParamname = Damagename;
	}
	
	
	//执行
    void Execute(CMoveShape* pShape);
};


//实现对目标的伤害
class COpDoDamage :
	public COperator
{
public:
	COpDoDamage(stModuParam *pParam);
	COpDoDamage(void *pParam);
	virtual ~COpDoDamage(void);



private:
	//值
	long m_lDamageReslut;
	//变量名
	string m_strDamaResVariName;
	//是否暴击
	long m_nBlastAttack;
	//脚本变量名
	string m_strBlasAttVariName;
    //@by:expter 最终计算伤害变量名
    string m_strDamParamname;       /// 最终伤害变量
    string m_strBlastParamname;     /// 最终暴击伤害变量

	void DoDestDamage(CMoveShape* pShape);
	void DoSelfDamage(CMoveShape* pShape);
	void DoDestIncHp(CMoveShape* pShape);
	void DoSelfIncHp(CMoveShape* pShape);
	//void DoEnergy(CMoveShape *pShape);	//能量值计算


	//普通减血消息
	void SendSelfDamageMsg(CMoveShape* pShape);
	//减血消息
	void SendDestDamageMsg(CMoveShape* pShape,hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//创建闪避消息
	void SendDestDodgeMsg(CMoveShape* pShape,hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//创建格挡消息
	void SendDestBlockMsg(CMoveShape* pShape,hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//招架消息
	void SendDestParryMsg(CMoveShape* pShape,hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//抵抗消息
	void SendDestResistMsg(CMoveShape* pShape,hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//吸收消息
	void SendDestSuckMsg(CMoveShape* pShape,hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//反弹消息
	void SendDestReboundMsg(CMoveShape* pShape,hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//加血
	void SendDestIncHpMsg(CMoveShape *pShape,hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//丢失消息
	void SendSelfLoseMsg(hash_set<CGUID,hash_guid_compare> &hsPlayers);
	//死亡消息
	void SendDiedMsg(CMoveShape* pShape);
	////特效消息
	//void SendEffectMsg(CMoveShape* pShape);
public:
	void InitBaseData(eObj Obj,long lRes,string strResVName,long lBlast,string strBlastVName,string Damname,string Blastname)
	{
		m_Obj = Obj;
		m_lDamageReslut = lRes;
		m_strDamaResVariName = strResVName;
		m_nBlastAttack = lBlast;
		m_strBlasAttVariName = strBlastVName;
        m_strDamParamname = Damname;
        m_strBlastParamname=Blastname;
	}	
	//执行
    void Execute(CMoveShape* pShape);
};

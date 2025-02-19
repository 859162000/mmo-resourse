//=============================================================================
/**
 *  file: Condition.h
 *
 *  Brief:条件判断类，包装了技能操作中需要的条件，例如距离、属性值等条件
 *
 *  Authtor:wangqiao
 *	
 *	Datae:2007-8-31
 */
//=============================================================================

#ifndef __CONDITION_H__
#define __CONDITION_H__

#pragma once

#include "BuffSkillDef.h"
#include "../../../../Public/Common/CStrConverter.h"

class CExpressions;
class CMoveShape;
class CScript;
struct stModuParam;

//定义得比较函数
typedef bool CMPFUN(long,long);

class CCondition
{
public:
	CCondition(stModuParam *pParam);
	virtual ~CCondition(void);
protected:
	//技能参数
	stModuParam *m_pParam;
public:
	virtual bool Result(CMoveShape* pDest) = 0;
	void		 SetParam(stModuParam *pParam) {m_pParam=pParam;}
};


class CConProperty:public CCondition
{
public:
	CConProperty(stModuParam *pParam);
	CConProperty(void *pParam);
	virtual ~CConProperty(void);
	struct tagCheckes
	{
		//检测对象
		eObj Obj;
		//属性名字
		string strName;
		//比较函数
		CMPFUN *pCmpFun;
		//值
		int nValue;
        //技能属性名
        string strParamam;

		void Init(eObj object,string &strname,CMPFUN *pCmp,int value,string Paramam)
		{
			Obj = object;
			strName = strname;
			pCmpFun = pCmp;
			nValue = value;
            strParamam = Paramam;
		}
	};
private:
	typedef list<tagCheckes>::iterator itCheck;
	list<tagCheckes> m_Checks;
public:
	void InitBaseData(const list<tagCheckes> &lChecks)
	{
		m_Checks = lChecks;
	}
	bool Result(CMoveShape* pDest);
};



class CConGoods:public CCondition
{
public:
	CConGoods(stModuParam *pParam);
	CConGoods(void *pParam);
	virtual ~CConGoods(void);

	struct tagAddProperty
	{
		DWORD dwAddProType;	//附加属性类型
		DWORD dwValue1;		//值1
		DWORD dwValue2;		//值2	
	};

	struct tagCheck
	{
		//检测对象
		eObj Obj;
		//物品所在的地方
		int nPlace;
		//物品所在的位置
		int nPos;
		//物品原始名
		string strOrigName;
		//物品类别
		int nType;
		//比较函数
		CMPFUN *pCmpFun;
		//值
		int nValue;
		//附加属性
		list<tagAddProperty*> lAddProperty;

		//脚本变量名
		string strVariaName;
		void Init(eObj object,int place,int pos,int type, const  string &strName,CMPFUN *op,int value,const string &strVname, list<tagAddProperty*> &lProperty)
		{
			Obj = object;
			nPlace = place;
			nPos = pos;
			strOrigName = strName;
			nType = type;
			pCmpFun = op;
			nValue = value;
			strVariaName = strVname;
			lAddProperty = lProperty;
		}
	};

private:
	typedef list<tagCheck>::iterator itCheck;
	list<tagCheck> m_Checkes;

public:
	void InitBaseData(const list<tagCheck> &ltgCheck)
	{
		m_Checkes = ltgCheck;
	}
	//得到条件结果
	bool Result(CMoveShape* pDest);
	//比较物品附加属性
	bool GetGoodAddProperty(CGoods* pEquip,list<tagAddProperty*> &lAddProperty);
};



class CConBuff:public CCondition
{
public:
	CConBuff(stModuParam *pParam);
	CConBuff(void *pParam);
	virtual ~CConBuff(void);

	struct tagCheck
	{
		//检测对象
		eObj Obj;
        //检测对象附带buf对象id
        string BufObj;
		//buff id
		DWORD nBuffID;
		//buff level
		int nLevel;
		//数量
		int nNum;
        //等级字符串
        string LevelName;
        //比较函数
        CMPFUN*  CmpFun;
		void Init(eObj object,string Bobj,DWORD id,int lv,int num,string name,CMPFUN* Cmp)
		{
			Obj = object;
            BufObj=Bobj;
			nBuffID = id;
			nLevel = lv;
			nNum = num;
            LevelName = name;
            CmpFun=Cmp;
		}
	};
private:
	typedef list<tagCheck>::iterator itCheck;
	list<tagCheck> m_Checkes;
public:
	void InitBaseData(const list<tagCheck> &ltgCheck)
	{
		m_Checkes = ltgCheck;
	}
	//得到条件结果
	bool Result(CMoveShape* pDest);
};



class CConDistance:public CCondition
{
public:
	CConDistance(stModuParam *pParam);
	CConDistance(void *pParam);
	virtual ~CConDistance(void);

private:
	//比较函数
	CMPFUN *m_CmpFun;
	//值
	/*int m_nValue;*/
    long m_value;
	//参照对象
	eObj m_refObj;
	//脚本变量名
	string m_strValueName;
public:
	void InitBaseData(CMPFUN *op,float value,const string &strVaName,const string &strRefObj)
	{
		m_CmpFun = op;
		m_value = (long)value;
		m_strValueName = strVaName;
		m_refObj = GetRefObj(strRefObj);
	}

	//得到条件结果
	bool Result(CMoveShape* pDest);
	eObj GetRefObj(string strRefObj)
	{
		if (strRefObj=="Obj_Self")
		{
			return Obj_Self;
		} 
		else if(strRefObj=="Obj_Dest")
		{
			return Obj_Dest;
		}
		else if (strRefObj=="Obj_Mouse")
		{
			return Obj_Mouse;
		}
		else if (strRefObj=="Obj_Firer")
		{
			return Obj_Firer;
		}
		else 
		{
			return Obj_Dest;
		}
	}
};

class CConScript:public CCondition
{
public:
	CConScript(stModuParam *pParam);
	CConScript(void *pParam);
	virtual ~CConScript(void);

private:
	/// 改用LUA后不再保存脚本内容，而是文件名
	string m_scriptFile;
public:
	void InitBaseData(const string &file)
	{
		m_scriptFile = file;	
	}
	//得到条件结果
	bool Result(CMoveShape* pDest);
};


class CConValidObj:public CCondition
{
public:
	CConValidObj(stModuParam *pParam);
	CConValidObj(void *pParam);
	virtual ~CConValidObj(void);
	//有效对象类型
	enum eValidObjType
	{
		SelfPlayer_Type,	 //本国玩家			
		EnemyPlayer_Type,	 //表示敌国玩家
		NormalPlayer_Type,	 //表示本国普通玩家	
		RedPlayer_Type,		 //表示本国红名玩家
		PurplePlayer_Type,	 //表示本国紫名玩家
		TeamPlayer_Type,	 //队伍成员
		TongPlayer_Type,	 //帮会成员
		LeaguePlayer_Type,	 //同盟成员
		Monster_Type,		 //表示普通怪物
		Npc_Type,			 //表示NPC
		Guard_Type,			 //门卫
		SMonster_Type,		 //表示特殊怪物		
	};
	enum eType
	{
		ePlayer_Type,		//玩家类型
		eMonster_Type,		//怪物类型
		eGate_Type,			//门类型
		ePet_Type,			//宠物类型
	};

public:
	void InitBaseData(eType type,list<eValidObjType> &lValidObjType)
	{
		m_lValidObjType = lValidObjType;// = type;
		m_lType			= type;
	}
	
	//得到条件结果
	bool Result(CMoveShape* pDest);
private:
	list<eValidObjType> m_lValidObjType;
	eType m_lType;
};

class CConTimer:public CCondition
{
public:
	CConTimer(stModuParam *pParam);
	CConTimer(void *pParam);
	virtual ~CConTimer(void);
private:
	long m_lFlag;
public:
	//
	void InitBaseData(long flag)
	{
		m_lFlag = flag;
	}
	//得到条件结果
	bool Result(CMoveShape* pDest);
};

class CConEvent:public CCondition
{
public:
	CConEvent(stModuParam *pParam);
	CConEvent(void *pParam);
	virtual ~CConEvent(void);
private:
	long m_lFlag;
	//事件类型
	long m_lEventType;
	long m_lVar1;
	long m_lVar2;
	long m_lVar3;
	long m_lVar4;
	string m_strVar3;

    list<long> m_skilllist;

public:
	void InitBaseData(long flag,long type,long lvar1,long lvar2,long lvar3,long lvar4,string skillid,string strName)
	{
		m_lFlag = flag;
		m_lEventType = type;
		m_lVar1 = lvar1;
		m_lVar2 = lvar2;
		m_lVar3 = lvar3;
		m_lVar4 = lvar4;
		m_strVar3 = strName;
        
        char  strKill[128];
        strcpy( strKill , skillid.c_str() );
        CStrConverter::parseList(strKill,m_skilllist);
	}
	//得到条件结果
	bool Result(CMoveShape* pDest);
};


class CConCooldown:public CCondition
{
public:
	CConCooldown(stModuParam *pParam);
	virtual ~CConCooldown(void);
public:
	//得到条件结果
	bool Result(CMoveShape* pDest);
};
//和目标的面向是否相同
class CConDir: public CCondition
{
public:
	CConDir(stModuParam *pParam);
	CConDir(void *pParam);
	virtual ~CConDir(void);
private:
	int m_nFaceType;
	enum eFace
	{
		//正面
		eFace_Front=1,
		//背边
		eFace_Back,		
	};
public:
	bool Result(CMoveShape* pDest);
	int  GetFace(string strType)
	{
		if (strType=="Face_Front")
		{
			return eFace_Front;
		} 
		else
		{
			return eFace_Back;
		}
	}
	void InitBaseData(string strType)
	{
		m_nFaceType = GetFace(strType);
	}
};
//场景条件
class CConRegion:public CCondition
{
public:
	CConRegion(stModuParam *pParam);
	CConRegion(void *pParam);
	virtual ~CConRegion(void);
private:
	string  m_strName;		//属性名称
	int		m_nRegionValue; //值
public:
	//初始化
	void InitBaseData(string strName, int nType)
	{
		m_strName = strName;
		m_nRegionValue = nType;
	}
	bool Result(CMoveShape* pDest);

};

//和目标之间的阻挡条件
class CConBlock:public CCondition
{
public:
	CConBlock(stModuParam *pParam);
	CConBlock(void *pParam);
	virtual ~CConBlock(void);
	
	enum eBlockType
	{
		BT_NoAnyBlock=0x1,//没有任何阻挡
		BT_NoFullBlock=0x2,//没有全阻挡(飞行阻挡)
		BT_NoHalfBlock=0x3,//没有半阻挡(地面阻挡)
		BT_NoShapeBlock=0x4,//对象阻挡
	};
	enum eType
	{
		e_Line=0,			//线型
		e_Point,			//点
	};
	enum eDirec
	{
		//前边
		DIREC_Front,
		//后边
		DIRECT_Back,
		//鼠标点
		Direct_MousePos,
	};
private:
	//阻挡条件类型,可以是eBlockType组合类型
	int  m_NoBlockType;
	//当有BT_NoShapeBlock值得时候，用来判断shape类型，如果是0表示所有类型
	int m_nShapeType;
	//计算阻挡方式
	int m_nType;
	//方向
	int m_nDir;

	bool NoAnyBlock(BYTE bBlock);
	bool NoFullBlock(BYTE bBlock);
	bool NoHalfBlock(BYTE bBlock);
	bool NoShapeBlock(BYTE bBlock,long lType);
public:
	eDirec GetDir(string strDir)
	{
		if (strDir=="DIREC_Front")
		{
			return DIREC_Front;
		}
		else if (strDir=="DIRECT_Back")
		{
			return DIRECT_Back;
		}
		else if (strDir=="Direct_MousePos")
		{
			return Direct_MousePos;
		}		
		return DIREC_Front;
	}

	eType GetType(string strType)
	{
		if (strType=="Point")
		{
			return e_Point;
		} 
		else if(strType=="Line")
		{
			return e_Line;
		}
		return e_Line;
	}
	//得到条件结果
	bool Result(CMoveShape* pDest);
	//初始化
	void InitBaseData(DWORD dwBlockType,DWORD dwShapeType,string strType,string strDir)
	{
		m_NoBlockType = dwBlockType;
		m_nShapeType = dwShapeType;
		m_nType		= GetType(strType);
		m_nDir		= GetDir(strDir);
	}
};

//几率事件判断
class CConOdds:public CCondition
{
private:
   long    m_nValue;	//几率
   string   valuename;  //几率配置
   CMPFUN*  cmpfun;     //比较函数

public:
    CConOdds(stModuParam *pParam);

    CConOdds(void *pParam);

    virtual ~CConOdds(void);

    bool Result(CMoveShape* pDest);

    void InitBaseData(short va,string vname,CMPFUN* cmp)
    {
         m_nValue = va;
         valuename = vname;
         cmpfun=cmp;
    }
};

//驱散条件
class CConDispel:public CCondition
{
private:
    long            m_nValue;         //驱散数量    
    eSkillAttackType DispelType;      //驱散类型
    string           strDispelNum;    //驱散字符串

public:
    CConDispel(stModuParam *pParam);

    CConDispel(void *pParam);

    virtual ~CConDispel();

    bool  Result(CMoveShape* pDest);

    void  InitBaseData(short va,eSkillAttackType Type,string DispelNum)
    {
        m_nValue = va;
        DispelType=Type;
        strDispelNum=DispelNum;
    }

};

//比较函数
bool Less(long val1,long val2);
bool Equal(long val1,long val2);
bool LessEqual(long val1,long val2);
bool Greater(long val1,long val2);
bool GreaterEqual(long val1,long val2);
bool NotEqual(long val1,long val2);

#endif


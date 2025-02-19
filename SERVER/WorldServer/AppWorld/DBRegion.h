#ifndef DB_REGION_H
#define DB_REGION_H
#include "BaseDBEntity.h"
#include "..\..\..\public\Date.h"
#include "../public/DBDefine.h"
#include <list>

using namespace std;


class CDBRegion : public CBaseDBEntity
{
public:
	//税
	struct tagRegionParam
	{
		long  lID;							//场景的ID
		long  lMaxTaxRate;					//最大税率
		long  lCurrentTaxRate;				//当前税率
		long  lYuanbaoTaxRate;				//元宝税率
		long  lBusinessTaxRate;				//跑商税率
		DWORD dwTotalTax;					//税金总合
		DWORD dwBusinessTotalTax;			//跑商税金总和
		DWORD dwTodayTotalTax;				//当天税金累计值
		long  lSupRegionID;					//供奉税金的场景ID(0表示不上缴税金)
		long  lTurnInTaxRate;				//上缴到其他场景的税率
		CGUID  OwnedFactionID;				//拥有这个场景的帮会
		CGUID  OwnedUnionID;				//拥有这个场景的同盟ID 
	};

	CDBRegion();
	CDBRegion(const CGUID& id);
	~CDBRegion();

	long GetID(void) { return m_ID; }
	void SetID(long id) { m_ID = id; }
	// virtual interfaces
	virtual void ProcessMsg(DB_OPERATION_TYPE op, long msgid, BYTE* msgBuf, long& msgBufPos, long bufSize=-1);

	void Load(long msgid, BYTE* msgBuf, long& msgBufPos, long bufSize=-1);
	void Save(long msgid, BYTE* msgBuf, long& msgBufPos);


	tagRegionParam& GetRegionParam(void) { return m_RegionParam; }

	void SetRegionParam(tagRegionParam& value) { memcpy_s(&m_RegionParam, sizeof(tagRegionParam), &value, sizeof(tagRegionParam)); }

	// Other interfaces
	// 准备发送的属性
	//vector<DB_CARD_ATTR_TYPE>& GetSendAttrEnumVector(void) { return m_SendAttrEnumVector; }
	// 打开准备发送的属性在Vector中的相应位置的标志
	void OpenAttrFlagInVector(DB_REGION_ATTR_TYPE attrType);
	// 关闭准备发送的属性在Vector中的相应位置的标志
	void CloseAttrFlagInVector(DB_REGION_ATTR_TYPE attrType);
	// 组装消息
	void AddEntityDataToMsg(DB_OPERATION_TYPE opType, vector<BYTE>& pBA);

private:
	// 等待被发送给DBS的属性链表
	vector<long> m_SendAttrEnumVector;
	tagRegionParam m_RegionParam;
	long m_ID;
};

#endif//DB_REGION_H
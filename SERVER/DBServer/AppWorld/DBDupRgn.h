#ifndef DB_DUP_RGN_H
#define DB_DUP_RGN_H

#include <map>
#include "../public/GUID.h"
#include "../public/msgtype.h"
#include "DBEntityDef.h"
#include "BaseDBEntity.h"
#include <list>
#include "../public/DBDefine.h"

class CVariableList;
class CDBRgnGoodsGroup;

class CDBDupRgn : public CBaseDBEntity
{
private:
	CDBDupRgn() {};
public:
	CDBDupRgn(const CGUID& guid);
	virtual ~CDBDupRgn();

public:
	// virtual interfaces
	virtual void ProcessMsg(DB_OPERATION_TYPE op, long msgid, BYTE* msgBuf, long& msgBufPos, long bufSize=-1);

	void Save(long msgid, BYTE* msgBuf, long& msgBufPos, long bufSize=-1);

	void AddEntityDataToMsg(DB_OPERATION_TYPE opType, vector<BYTE>& pBA);

	virtual CDBDupRgn& operator=(CDBDupRgn& right)
	{		
		CBaseDBEntity::operator=(right);
		if(this != &right)
		{
		}
		return *this;
	}

	// 初始化属性配置
	void				InitProperty();

	//// 检查更新属性里是否有指定表名的属性名
	bool				FindTableUpdateFlag(const string& tableName);
	CDBRgnGoodsGroup* GetGoodsGroupPtr(void) { return m_GetGoodsGroupPtr; }
private:
	CDBRgnGoodsGroup* m_GetGoodsGroupPtr;
};

#endif//DB_DUP_RGN_H
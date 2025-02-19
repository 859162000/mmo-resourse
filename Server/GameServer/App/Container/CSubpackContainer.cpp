// Copyright (C) 1991 - 1999 Rational Software Corporation

#include "StdAfx.h"
#include "CSubpackContainer.h"
#include "CVolumeLimitGoodsContainer.h"
#include "CContainer.h"
#include "../Player.h"
#include "../Goods/CGoods.h"
#include "../Goods/CGoodsFactory.h"

#include "../../../../Public/Common/GoodsDef.h"
#include "../../../../Public/Setup/GoodsSetup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSubpackContainer::CSubpackContainer()
{
	// TODO: Add your specialized code here.
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		m_Subpack[i].pGoods=NULL;
		m_Subpack[i].pSubpackContainer=OBJ_CREATE(CVolumeLimitGoodsContainer);
		m_Subpack[i].pSubpackContainer->SetContainerVolume(0);
	}

	AddListener( this );
}


CSubpackContainer::~CSubpackContainer()
{
	// TODO: Add your specialized code here.
	Release();
}

bool CSubpackContainer::Add(CBaseObject* pObject, tagPreviousContainer* pPreviousContainer, LPVOID pVoid)
{
	// TODO: Add your specialized code here.
	// NOTE: Requires a correct return value to compile.
	bool bResult = false;
	CGoods* pGoods = dynamic_cast<CGoods*>( pObject );
	
	return bResult;
}

void CSubpackContainer::Clear(LPVOID pVoid)
{
	for(int j=0;j<SUBPACK_NUM;j++)
	{
		if(m_Subpack[j].pGoods==NULL)
		{
			continue;
		}
		for( size_t i = 0; i < m_vListeners.size(); i ++ )
		{
			if( m_vListeners[i] )
			{
				m_vListeners[i] -> OnObjectRemoved( this, m_Subpack[j].pGoods, 
					m_Subpack[j].pGoods-> GetAmount(),pVoid );
			}
			//##用完之后先把这个置为NULL。但是不删除该物品。也许这个容器的克隆容器会使用。
			
		}
		m_Subpack[j].pGoods= NULL;
	}
}

void CSubpackContainer::Release()
{	
	if( GetContainerMode() == CGoodsContainer::GCM_NORMAL )
	{		
		for(int i=0;i<SUBPACK_NUM;i++)
		{
			if(m_Subpack[i].pGoods)
			{
#ifdef _GOODSLOG1_
				
					char pszGoodsLog[1024]="";			
					char srcGoodsGUID[50]="";
					m_Subpack[i].pGoods->GetExID().tostring(srcGoodsGUID);					
					_snprintf(pszGoodsLog,1024,"[物品回收:CSubpackContainer析构][物品GUID:%s][物品数量:%d]",
						srcGoodsGUID,m_Subpack[i].pGoods->GetAmount());
					CMessage::AsyWriteFile(GetGame()->GetGoodsLogFile(),pszGoodsLog);	
											
#endif
				CGoodsFactory::GarbageCollect(&(m_Subpack[i].pGoods),8);
			}
		}
	}
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pSubpackContainer)
		{
			OBJ_RELEASE(CVolumeLimitGoodsContainer, m_Subpack[i].pSubpackContainer);
			m_Subpack[i].pSubpackContainer=NULL;
		}
	}
	
	CGoodsContainer::Release();
	AddListener( this );
}

//! 清空装载的所有物品
void CSubpackContainer::ReleaseAllGoods()
{
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(NULL != m_Subpack[i].pGoods)
		{
#ifdef _GOODSLOG1_

			char pszGoodsLog[1024]="";			
			char srcGoodsGUID[50]="";
			m_Subpack[i].pGoods->GetExID().tostring(srcGoodsGUID);					
			_snprintf(pszGoodsLog,1024,"[物品回收:CSubpackContainer::ReleaseAllGoods][物品GUID:%s][物品数量:%d]",
				srcGoodsGUID,m_Subpack[i].pGoods->GetAmount());
			CMessage::AsyWriteFile(GetGame()->GetGoodsLogFile(),pszGoodsLog);	

#endif
			CGoodsFactory::GarbageCollect( &(m_Subpack[i].pGoods) ,9);
		}
		if(NULL != m_Subpack[i].pSubpackContainer)
		{
			m_Subpack[i].pSubpackContainer->Release();
		}
	}
}

DWORD CSubpackContainer::GetGoodsAmountByName(const char* strName)
{
	DWORD dwAmount=0;

	return dwAmount;
}

DWORD CSubpackContainer::GetGoodsAmountByType(int nType)
{
	DWORD dwAmount=0;

	return dwAmount;
}


void CSubpackContainer::TraversingContainer(CContainerListener* pListener)
{
	if( pListener == NULL ) 
	{
		return;
	}	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if( pListener -> OnTraversingContainer(this, m_Subpack[i].pGoods) == false )
		{
			//break;
		}
	}
}

bool CSubpackContainer::IsGoodsExisted(DWORD dwGoodsBasePropertiesIndex)
{	
	bool bResult = false;	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if( m_Subpack[i].pGoods && m_Subpack[i].pGoods-> GetBasePropertiesIndex() == 
			dwGoodsBasePropertiesIndex )
		{
			bResult	= true;
			break;
		}
	}
	return bResult;
}

CSubpackContainer::tagSubpack* CSubpackContainer::GetSubpack(int i)
{
	if(i>=0 && i<SUBPACK_NUM)
	{
		return &m_Subpack[i];
	}
	return NULL;
}

CSubpackContainer::tagSubpack* CSubpackContainer::GetBusinessPackTag(DWORD& dwPos)
{
	dwPos=0;
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods)
		{
			DWORD dwPackType=m_Subpack[i].pGoods->GetAddonPropertyValues(GAP_PACK_TYPE,1);
			if(dwPackType==SPT_BUSINESS)
			{
				dwPos=CS2CContainerObjectMove::PEI_PACK1+i;
				return &m_Subpack[i];
			}
		}
	}
	return NULL;
}

bool CSubpackContainer::CanSwap(SUBPACK_COLUMN ecColumn,CGoods* pIn)
{
	if(!pIn)
		return false;
	CGoodsBaseProperties* pProperties = GoodsSetup::QueryGoodsBaseProperties( pIn -> GetBasePropertiesIndex() );
	if(!pProperties)
		return false;
	//必须是子背包类型
	if(pProperties->GetGoodsBaseType()!=GBT_SUBPACK)
		return false;
	//范围检查
	if(ecColumn<CS2CContainerObjectMove::PEI_PACK1 || ecColumn>CS2CContainerObjectMove::PEI_PACK5)
		return false;
	//取得目标位置放置的子背包
	CGoods* pDesGoods = GetGoods( ecColumn );
	if(pDesGoods)
	{
		//目标位置容器
		CVolumeLimitGoodsContainer* pDesContainer=m_Subpack[ecColumn-CS2CContainerObjectMove::PEI_PACK1].pSubpackContainer;
		if (!pDesContainer)
			return false;
		//目标容器放置了多少个物品		
		DWORD dwGoodsNum=pDesContainer->GetGoodsAmount();
		//源容器的容积
		DWORD dwSrcVolume=pIn->GetAddonPropertyValues(GAP_PACK_CAPACITY,1);
		if(dwSrcVolume<dwGoodsNum)
		{
			//装不下
			return false;
		}
		return true;
	}
	return false;
}

bool CSubpackContainer::CanPutIn(CGoods* pGoods,CPlayer* pPlayer)
{
	if(pGoods && pPlayer)
	{
		CGoodsBaseProperties* pProperty=GoodsSetup::QueryGoodsBaseProperties(pGoods->GetBasePropertiesIndex());
		if(!pProperty)
			return false;
		if(pProperty->GetGoodsBaseType()!=GBT_SUBPACK)
			return false;
		return pPlayer->CanMountSubpack(pGoods);			
	}		
	return false;
}

bool CSubpackContainer::Swap(SUBPACK_COLUMN ecColumn, CGoods* pIn, CGoods** ppOut, LPVOID pVoid)
{
	bool bResult = false;
	if( pIn && ppOut )
	{
		if(!CanSwap(ecColumn,pIn))
			return false;							
		CGoods* pOriginalGoods = GetGoods( ecColumn );
		if( pOriginalGoods )
		{
			CVolumeLimitGoodsContainer* pContainer=m_Subpack[ecColumn-10001].pSubpackContainer;
			if (NULL == pContainer)
				return false;
			DWORD dwGoodsNum=pContainer->GetGoodsAmount();
			CS2CContainerObjectMove* pMsg = reinterpret_cast<CS2CContainerObjectMove*>( pVoid );
			bool bSwitchAndFill=false;
			if(dwGoodsNum==0)
			{
				if(pMsg)
				{
					pMsg->SetOperation(CS2CContainerObjectMove::OT_SWITCH_OBJECT);
				}
			}
			else if(dwGoodsNum>0)
			{
				if(pMsg)
				{
					pMsg->SetOperation(CS2CContainerObjectMove::OT_SWITCH_FILL);
				}
				bSwitchAndFill=true;
			}
			//移除目标位置物品
			Remove(pOriginalGoods);		

			//源物品添加到目标位置
			Add(ecColumn, pIn, NULL);										
			if( pMsg )
			{
				pMsg -> SetSourceObject( pIn -> GetType(),pIn->GetExID() );
				pMsg -> SetDestinationObject( pOriginalGoods -> GetType(),pOriginalGoods->GetExID());
			}
			*ppOut = pOriginalGoods;

			if(bSwitchAndFill)
			{
				//物品交换
				//原来ecColumn位置所对应的container需要排列物品位置
				pContainer->ReFillCell();
			}
			bResult = true;
		}
	}
	return bResult;
}

CBaseObject* CSubpackContainer::Find(const CGUID& guid)
{	
	CGoods* pResult = NULL;	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if( m_Subpack[i].pGoods)
		{			
			if(m_Subpack[i].pGoods->GetExID() == guid )
			{
				pResult = m_Subpack[i].pGoods;
				break;
			}
		}
	}
	return pResult;
}

//##ModelId=41B3D0B80395
CBaseObject* CSubpackContainer::Remove(const CGUID& guid, LPVOID pVoid)
{	
	CGoods* pResult = NULL;	
	int index=-1;
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods)
		{			
			if(m_Subpack[i].pGoods->GetExID() == guid )
			{
				pResult =m_Subpack[i].pGoods;
				index=i;
				break;
			}
		}
	}
	if( pResult )
	{
		for( size_t i = 0; i < m_vListeners.size(); i ++ )
		{
			if( m_vListeners[i] )
			{
				m_vListeners[i] -> OnObjectRemoved( this, pResult, pResult -> GetAmount(), pVoid );
			}
		}		
		m_Subpack[index].pGoods=NULL;
		
	}
	return pResult;
}

bool CSubpackContainer::Add(DWORD dwPosition, CGoods* pObject, tagPreviousContainer* pPreviousContainer, LPVOID pVoid)
{
	// TODO: Add your specialized code here.
	// NOTE: Requires a correct return value to compile.
	if( pObject == NULL )
	{
		return false;
	}
	
	SUBPACK_COLUMN ecColumn = static_cast<SUBPACK_COLUMN>( dwPosition );
	int index=ecColumn-10001;
	if(m_Subpack[index].pGoods)
		return false;	
	DWORD dwPackType=pObject->GetAddonPropertyValues(GAP_PACK_TYPE,1);
	CPlayer* pPlayer=GetGame()->FindPlayer(GetOwnerID());
	if(dwPackType==SPT_BUSINESS)
	{		
		if(pPlayer && pPlayer->IsEquipBusinessPack())
			return false;
	}

	bool bResult = false;
	CGoodsBaseProperties* pProperties = 
		GoodsSetup::QueryGoodsBaseProperties( pObject -> GetBasePropertiesIndex() );
	if( pProperties )
	{		
		if( pProperties -> GetGoodsBaseType() == GBT_SUBPACK )
		{		
			m_Subpack[index].pGoods=pObject;
			if(dwPosition>=CS2CContainerObjectMove::PEI_PACK1 && dwPosition<=CS2CContainerObjectMove::PEI_PACK5)
			{				
				//CPlayer* pOwner = GetGame() -> FindPlayer( GetOwnerID() );
				//if( pOwner )
				//{
				LONG lVal=pObject->GetAddonPropertyValues(GAP_PACK_CAPACITY,1);
				m_Subpack[index].pSubpackContainer->ChangeContainerVolume(lVal);
					//pOwner->getSubPack(index)->ChangeContainerVolume(5,lVal/5);	
				//}		
			}	
			bResult=true;
		}
	}
	if( bResult )
	{
		//如果是跑商背包				
		if(dwPackType==SPT_BUSINESS && pPlayer)
		{
			//状态检查
			long lRet=pPlayer->GetStateParamByID(5101,"Num");
			if(lRet==0)
			{
				pPlayer->AddNewState(pPlayer,5101,1,12);
				GetGame()->UpdateSavePlayer(pPlayer, SAVE_DETAIL_ATTRSTATELIST);
			}
		}
		for( size_t i = 0; i < m_vListeners.size(); i ++ )
		{
			if( m_vListeners[i] )
			{
				m_vListeners[i] -> OnObjectAdded( this, pObject, pObject -> GetAmount(), pVoid );
			}
		}
	}
	return bResult;
}

void CSubpackContainer::AI()
{	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods)
		{
			m_Subpack[i].pGoods-> AI();
		}
	}
}

bool CSubpackContainer::IsFull()
{	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods==NULL)
			return false;
	}
	return true;
}

bool CSubpackContainer::QueryGoodsPosition(CGoods* pGoods, DWORD& dwPosition)
{
	if( pGoods == NULL )
	{
		return false;
	}
	bool bResult = false;	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods== pGoods )
		{
			dwPosition	=10001+i;
			bResult		= true;
			break;
		}
	}
	return bResult;
}

bool CSubpackContainer::QueryGoodsPosition(const CGUID& guid, DWORD& dwPosition)
{
	bool bResult = false;	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods && m_Subpack[i].pGoods->GetExID() == guid)
		{
			dwPosition	=10001+i;
			bResult		= true;
			break;
		}
	}
	return bResult;
}

CGoods* CSubpackContainer::GetTheFirstGoods(DWORD dwGoodsBasePropertiesIndex)
{
	CGoods* pResult = NULL;	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods && m_Subpack[i].pGoods-> GetBasePropertiesIndex() == 
			dwGoodsBasePropertiesIndex )
		{
			pResult	= m_Subpack[i].pGoods;
			break;
		}
	}
	return pResult;
}

//##ModelId=41BFA1AC0242
CGoods* CSubpackContainer::GetGoods(DWORD dwPosition)
{	
	CGoods* pResult				= NULL;
	if(dwPosition<EC_PACK1 || dwPosition>EC_PACK5)
		return pResult;
	SUBPACK_COLUMN ecColumn	= static_cast<SUBPACK_COLUMN>( dwPosition );
	int index=ecColumn-10001;
	if( m_Subpack[index].pGoods)
	{
		pResult =m_Subpack[index].pGoods;
	}	
	return pResult;
}

//##ModelId=41BFA1B8007D
void CSubpackContainer::GetGoods(DWORD dwGoodsBasePropertiesIndex, vector<CGoods*> vOut)
{	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods && m_Subpack[i].pGoods-> GetBasePropertiesIndex() == 
			dwGoodsBasePropertiesIndex )
		{
			vOut.push_back( m_Subpack[i].pGoods);
		}
	}
}


bool CSubpackContainer::OnObjectAdded(CContainer* pContainer, CBaseObject* pObj, DWORD dwAmount, LPVOID pVoid)
{
	CGoodsContainer* pGoodsContainer	= dynamic_cast<CGoodsContainer*>( pContainer );
	CGoods* pGoods						= dynamic_cast<CGoods*>( pObj );
	CS2CContainerObjectMove* pMsg		= reinterpret_cast<CS2CContainerObjectMove*>( pVoid );
	
	if( pGoodsContainer && pGoods && pMsg )
	{
		DWORD dwPosition = 0;
		if( pGoodsContainer -> QueryGoodsPosition(pGoods, dwPosition) )
		{
			pMsg -> SetDestinationContainer( GetOwnerType(), GetOwnerID(), dwPosition );
			pMsg -> SetDestinationObject( pGoods -> GetType(),pGoods->GetExID() );
			pMsg -> SetDestinationObjectAmount( pGoods -> GetAmount() );
		}
	}
	return true;
}

bool CSubpackContainer::OnObjectRemoved(CContainer* pContainer, CBaseObject* pObj, DWORD dwAmount, LPVOID pVoid)
{
	CGoodsContainer* pGoodsContainer	= dynamic_cast<CGoodsContainer*>( pContainer );
	CGoods* pGoods						= dynamic_cast<CGoods*>( pObj );
	CS2CContainerObjectMove* pMsg		= reinterpret_cast<CS2CContainerObjectMove*>( pVoid );
	
	if( pGoodsContainer && pGoods && pMsg )
	{
		DWORD dwPosition = 0;
		if( pGoodsContainer -> QueryGoodsPosition(pGoods, dwPosition) )
		{
			pMsg -> SetSourceContainer( GetOwnerType(), GetOwnerID(), dwPosition );
			pMsg -> SetSourceObject( pGoods -> GetType(),pGoods->GetExID() );
			pMsg -> SetSourceObjectAmount( dwAmount );
		}
	}
	return true;
}

//##ModelId=41BFEAD70138
DWORD CSubpackContainer::GetGoodsAmount()
{	
	DWORD dwAmount = 0;	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods)
		{
			if( GoodsSetup::QueryGoodsBaseProperties( 
				m_Subpack[i].pGoods-> GetBasePropertiesIndex() ))
			{
				++ dwAmount;
			}
		}
	}
	return dwAmount;
}


DWORD CSubpackContainer::GetEquipAmount()
{
	DWORD dwAmount = 0;	
	return dwAmount;
}

CBaseObject* CSubpackContainer::Remove( CBaseObject* pObject, LPVOID pVoid )
{
	return CGoodsContainer::Remove( pObject, pVoid );
}

CBaseObject* CSubpackContainer::Remove( LONG oType, const CGUID& guid, LPVOID pVoid )
{
	return CGoodsContainer::Remove( oType, guid, pVoid );
}

CBaseObject* CSubpackContainer::Find( CBaseObject* pObj )
{
	return CGoodsContainer::Find( pObj );
}

CBaseObject* CSubpackContainer::Find( LONG oType, const CGUID& guid )
{
	return CGoodsContainer::Find( oType, guid );
}

bool CSubpackContainer::Serialize( DBWriteSet& setWriteDB, bool b  )
{
	setWriteDB.AddToByteArray( GetGoodsAmount() );
	
	for(int i=0;i<SUBPACK_NUM;i++)
	{
		if(m_Subpack[i].pGoods)
		{
			if( GoodsSetup::QueryGoodsBaseProperties( 
				m_Subpack[i].pGoods-> GetBasePropertiesIndex() ) )
			{
				setWriteDB.AddToByteArray( static_cast<DWORD>(10001+i) );
				m_Subpack[i].pGoods-> Serialize( setWriteDB, b );				
				m_Subpack[i].pSubpackContainer->Serialize(setWriteDB,b);
			}
		}
	}
	return true;
}
bool CSubpackContainer::Unserialize( DBReadSet& setReadDB, bool b )
{
	CSubpackContainer::Clear();
	DWORD dwGoodsAmount = setReadDB.GetDwordFromByteArray( );
	
	for( DWORD i = 0; i < dwGoodsAmount; i ++ )
	{
		SUBPACK_COLUMN ecColumn = static_cast<SUBPACK_COLUMN>( 
			setReadDB.GetDwordFromByteArray( ) );
		CGoods* pGoods = CGoodsFactory::UnserializeGoods( setReadDB,17);
		if( pGoods )
		{
			Add( ecColumn, pGoods, NULL );
		}
		m_Subpack[ecColumn-10001].pSubpackContainer->Unserialize(setReadDB);
	}
	return true;
}

//##ModelId=41C0E6DB0119
CBaseObject* CSubpackContainer::Remove(DWORD dwPosition, DWORD dwAmount, LPVOID pVoid)
{
	// TODO: Add your specialized code here.
	CBaseObject* pObj= CGoodsContainer::Remove( dwPosition, dwAmount, pVoid );
	//如果拿起背包装备，要更新容器容积
	if(dwPosition>=CS2CContainerObjectMove::PEI_PACK1 && dwPosition<=CS2CContainerObjectMove::PEI_PACK5)
	{
		int pos=dwPosition-10001;
		m_Subpack[pos].pSubpackContainer->ChangeContainerVolume(0);
	}	
	return pObj;
}

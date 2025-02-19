// Copyright (C) 1991 - 1999 Rational Software Corporation

#include "stdafx.h"
#include "..\Player.h"
#include "..\BaseObject.h"
#include "..\Goods\CGoods.h"
#include "..\Goods\CGoodsFactory.h"
#include "..\Message Packaging\CS2CContainerObjectMove.h"
#include "CShadowWallet.h"
#include "CContainer.h"



//##ModelId=41F0713100D0
CShadowWallet::CShadowWallet()
{
	// TODO: Add your specialized code here.
	SetGoodsAmountLimit( 1 );
}

//##ModelId=41F0713A0293
//CShadowWallet::CShadowWallet(LONG lID)
//{
//	// TODO: Add your specialized code here.
//}

//##ModelId=41F0713F035D
CShadowWallet::~CShadowWallet()
{
	// TODO: Add your specialized code here.
	Release();
}

BOOL CShadowWallet::Add(CBaseObject* pObject, tagPreviousContainer* pPreviousContainer, LPVOID pVoid)
{
	return CShadowWallet::Add( 0, dynamic_cast<CGoods*>(pObject), pPreviousContainer, pVoid );
}

BOOL CShadowWallet::Add(DWORD dwPosition, CGoods* pObject, tagPreviousContainer* pPreviousContainer, LPVOID pVoid)
{
	BOOL bResult = FALSE;

	//##首先清除自己的影子。
	goodsshadowIt it = m_mGoodsShadow.begin();
	if( it != m_mGoodsShadow.end() )
	{
		RemoveShadow( it -> first );
	}
	Clear();

	//##然后添加新的物品
	CGoods* pGoods = dynamic_cast<CGoods*>( pObject );
	if( pPreviousContainer && pGoods )
	{
		if( pGoods -> GetBasePropertiesIndex() == CGoodsFactory::GetGoldCoinIndex() )
		{
			//##首先,添加到原容器内.
			CGoodsContainer* pContainer = NULL;
			if( pPreviousContainer -> lContainerType == TYPE_PLAYER )
			{
				CPlayer* pPlayer = GetGame() -> FindPlayer( pPreviousContainer -> ContainerID );
				if( pPlayer )
				{
					if( pPreviousContainer -> lContainerExtendID == CS2CContainerObjectMove::PEI_WALLET )
					{
						pContainer = &( pPlayer -> m_cWallet );
					}
				}
			}

			if( pContainer )
			{				
				CGUID	guGoodID	= pGoods ->GetExID();
				DWORD	dwAmount	= pGoods -> GetAmount();
				DWORD	dwBaseIndex	= pGoods -> GetBasePropertiesIndex();
				if( pPreviousContainer -> dwGoodsPosition != 0xFFFFFFFF )
				{					
					CGoods* pOriginalGoods = pContainer -> GetGoods( pPreviousContainer -> dwGoodsPosition );
					if( pOriginalGoods )
					{							
						guGoodID	= pOriginalGoods ->GetExID();
					}					
				}
				else 
				{
					pContainer -> QueryGoodsPosition( pGoods, pPreviousContainer -> dwGoodsPosition );

				}

				//##然后再自身容器内添加影子.
				tagGoodsShadow tShadow;
				tShadow.lOriginalContainerType		= pPreviousContainer -> lContainerType;
				tShadow.OriginalContainerID			= pPreviousContainer -> ContainerID;
				tShadow.lOriginalContainerExtendID	= pPreviousContainer -> lContainerExtendID;
				tShadow.dwOriginalGoodsPosition		= pPreviousContainer -> dwGoodsPosition;
				tShadow.dwGoodsAmount				= dwAmount;				
				tShadow.guGoodsID					= guGoodID;
				tShadow.dwGoodsBasePropertiesIndex	= dwBaseIndex;
				
				m_mGoodsShadow[guGoodID]			= tShadow;

				CGoodsShadowContainer::AddShadow( guGoodID  );

				bResult								= TRUE;				
			}
		}
	}
	return bResult;
}



//##ModelId=41F071F30047
BOOL CShadowWallet::OnObjectAdded(CContainer* pContainer, CBaseObject* pObj, DWORD dwAmount, LPVOID pVoid)
{
	// TODO: Add your specialized code here.
	// NOTE: Requires a correct return value to compile.
	return FALSE;
}

//##ModelId=41F071FB01EC
BOOL CShadowWallet::OnObjectRemoved(CContainer* pContainer, CBaseObject* pObj, DWORD dwAmount, LPVOID pVoid)
{
	// TODO: Add your specialized code here.
	// NOTE: Requires a correct return value to compile.
	if( pObj )
	{
		RemoveShadow( pObj->GetExID());
	}
	return TRUE;
}


//##ModelId=41F5EBAD0188
DWORD CShadowWallet::GetGoldCoinsAmount()
{
	// TODO: Add your specialized code here.
	// NOTE: Requires a correct return value to compile.
	DWORD dwResult = 0;
	if( m_mGoodsShadow.size() > 0 )
	{
		goodsshadowIt it = 	m_mGoodsShadow.begin();
		if( it != m_mGoodsShadow.end() )
		{
			dwResult = it -> second.dwGoodsAmount;
		}
	}
	return dwResult;
}

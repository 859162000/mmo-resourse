// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// ("Fluent UI")，该示例仅作为参考资料提供， 
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有 (C) Microsoft Corporation
// 保留所有权利。

#pragma once

#include "ViewTree.h"

class CClassToolBar : public CMFCToolBar
{
    virtual VOID OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
    {
        CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
    }

    virtual BOOL AllowShowOnList() CONST { return FALSE; }
};

class CClassView : public CDockablePane
{
public:
    CClassView();
    virtual ~CClassView();

    VOID AdjustLayout();
    VOID OnChangeVisualStyle();

protected:
    CClassToolBar m_wndToolBar;
    CViewTree m_wndClassView;
    CImageList m_ClassViewImages;
    UINT m_nCurrSort;

    VOID FillClassView();

// 重写
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg VOID OnSize(UINT nType, INT cx, INT cy);
    afx_msg VOID OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg VOID OnClassAddMemberFunction();
    afx_msg VOID OnClassAddMemberVariable();
    afx_msg VOID OnClassDefinition();
    afx_msg VOID OnClassProperties();
    afx_msg VOID OnNewFolder();
    afx_msg VOID OnPaint();
    afx_msg VOID OnSetFocus(CWnd* pOldWnd);
    afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);
    afx_msg VOID OnSort(UINT id);
    afx_msg VOID OnUpdateSort(CCmdUI* pCmdUI);

    DECLARE_MESSAGE_MAP()
};


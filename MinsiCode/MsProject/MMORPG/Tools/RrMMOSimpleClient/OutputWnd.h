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

/////////////////////////////////////////////////////////////////////////////
// COutputList 窗口

class COutputList : public CListBox
{
// 构造
public:
    COutputList();

// 实现
public:
    virtual ~COutputList();

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnEditCopy();
    afx_msg void OnEditClear();
    afx_msg void OnViewOutput();

    DECLARE_MESSAGE_MAP()
};

class COutputWnd : public CDockablePane
{
// 构造
public:
    COutputWnd();

// 属性
protected:
    CFont m_Font;

    CMFCTabCtrl    m_wndTabs;

    COutputList m_wndOutputBuild;
    COutputList m_wndOutputDebug;
    COutputList m_wndOutputFind;

protected:
    void FillBuildWindow();
    void FillDebugWindow();
    void FillFindWindow();

    void AdjustHorzScroll(CListBox& wndListBox);

// 实现
public:
    virtual ~COutputWnd();

protected:
    afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, INT cx, INT cy);

    DECLARE_MESSAGE_MAP()
};


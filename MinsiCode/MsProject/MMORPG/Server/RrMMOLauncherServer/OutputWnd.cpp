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

#include "Precompiled.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

INT COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_Font.CreateStockObject(DEFAULT_GUI_FONT);

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // 创建选项卡窗口:
    if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
    {
        TRACE0("未能创建输出选项卡窗口\n");
        return -1;      // 未能创建
    }

    // 创建输出窗格:
    CONST DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

    if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
        !m_wndOutputDebug.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
        !m_wndOutputFind.Create(dwStyle, rectDummy, &m_wndTabs, 4))
    {
        TRACE0("未能创建输出窗口\n");
        return -1;      // 未能创建
    }

    m_wndOutputBuild.SetFont(&m_Font);
    m_wndOutputDebug.SetFont(&m_Font);
    m_wndOutputFind.SetFont(&m_Font);

    CString strTabName;
    BOOL bNameValid;

    // 将列表窗口附加到选项卡:
    bNameValid = strTabName.LoadString(IDS_BUILD_TAB);
    ASSERT(bNameValid);
    m_wndTabs.AddTab(&m_wndOutputBuild, strTabName, (UINT)0);
    bNameValid = strTabName.LoadString(IDS_DEBUG_TAB);
    ASSERT(bNameValid);
    m_wndTabs.AddTab(&m_wndOutputDebug, strTabName, (UINT)1);
    bNameValid = strTabName.LoadString(IDS_FIND_TAB);
    ASSERT(bNameValid);
    m_wndTabs.AddTab(&m_wndOutputFind, strTabName, (UINT)2);

    // 使用一些虚拟文本填写输出选项卡(无需复杂数据)
    FillBuildWindow();
    FillDebugWindow();
    FillFindWindow();

    return 0;
}

VOID COutputWnd::OnSize(UINT nType, INT cx, INT cy)
{
    CDockablePane::OnSize(nType, cx, cy);

    // 选项卡控件应覆盖整个工作区:
    m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

VOID COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(&m_Font);

    INT cxExtentMax = 0;

    for (INT i = 0; i < wndListBox.GetCount(); i ++)
    {
        CString strItem;
        wndListBox.GetText(i, strItem);

        cxExtentMax = max(cxExtentMax, dc.GetTextExtent(strItem).cx);
    }

    wndListBox.SetHorizontalExtent(cxExtentMax);
    dc.SelectObject(pOldFont);
}

VOID COutputWnd::FillBuildWindow()
{
    m_wndOutputBuild.AddString(_T("生成输出正显示在此处。"));
    m_wndOutputBuild.AddString(_T("输出正显示在列表视图的行中"));
    m_wndOutputBuild.AddString(_T("但您可以根据需要更改其显示方式..."));
}

VOID COutputWnd::FillDebugWindow()
{
    m_wndOutputDebug.AddString(_T("调试输出正显示在此处。"));
    m_wndOutputDebug.AddString(_T("输出正显示在列表视图的行中"));
    m_wndOutputDebug.AddString(_T("但您可以根据需要更改其显示方式..."));
}

VOID COutputWnd::FillFindWindow()
{
    m_wndOutputFind.AddString(_T("查找输出正显示在此处。"));
    m_wndOutputFind.AddString(_T("输出正显示在列表视图的行中"));
    m_wndOutputFind.AddString(_T("但您可以根据需要更改其显示方式..."));
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList()
{
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
    ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList 消息处理程序

VOID COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CMenu menu;
    menu.LoadMenu(IDR_OUTPUT_POPUP);

    CMenu* pSumMenu = menu.GetSubMenu(0);

    if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
    {
        CMFCPopupMenu* pPopupMenu = NEW CMFCPopupMenu;

        if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
            return;

        ((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
        UpdateDialogControls(this, FALSE);
    }

    SetFocus();
}

VOID COutputList::OnEditCopy()
{
    MessageBox(_T("复制输出"));
}

VOID COutputList::OnEditClear()
{
    MessageBox(_T("清除输出"));
}

VOID COutputList::OnViewOutput()
{
    CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
    CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

    if (pMainFrame != NULL && pParentBar != NULL)
    {
        pMainFrame->SetFocus();
        pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
        pMainFrame->RecalcLayout();

    }
}



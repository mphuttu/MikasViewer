
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "MikasViewer.h"
#include "FolderTreeView.h"
#include "MikasViewerView.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;
const UINT_PTR slideshowTimer = 0x4D56;
const UINT slideshowIntervals[] = { 2, 5, 10, 30 };

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_MESSAGE(WM_FOLDER_SELECTED, &CMainFrame::OnFolderSelected)
	ON_MESSAGE(WM_FILE_SELECTED,   &CMainFrame::OnFileSelected)
	ON_MESSAGE(WM_FILE_LIST_SELECTED, &CMainFrame::OnFileListSelected)
	ON_COMMAND(ID_MODE_MANAGE, &CMainFrame::OnModeManage)
	ON_COMMAND(ID_MODE_VIEW, &CMainFrame::OnModeView)
	ON_UPDATE_COMMAND_UI(ID_MODE_MANAGE, &CMainFrame::OnUpdateModeManage)
	ON_UPDATE_COMMAND_UI(ID_MODE_VIEW, &CMainFrame::OnUpdateModeView)
	ON_COMMAND(ID_SLIDESHOW_START, &CMainFrame::OnSlideshowStart)
	ON_COMMAND(ID_SLIDESHOW_STOP, &CMainFrame::OnSlideshowStop)
	ON_COMMAND(ID_SLIDESHOW_LOOP, &CMainFrame::OnSlideshowLoop)
	ON_COMMAND_RANGE(ID_SLIDESHOW_INTERVAL_2, ID_SLIDESHOW_INTERVAL_30, &CMainFrame::OnSlideshowInterval)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_START, &CMainFrame::OnUpdateSlideshowStart)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_STOP, &CMainFrame::OnUpdateSlideshowStop)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_LOOP, &CMainFrame::OnUpdateSlideshowLoop)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SLIDESHOW_INTERVAL_2, ID_SLIDESHOW_INTERVAL_30, &CMainFrame::OnUpdateSlideshowInterval)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	if (!m_wndModeBar.CreateEx(this, TBSTYLE_FLAT,
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY,
		CRect(1, 1, 1, 1), AFX_IDW_CONTROLBAR_FIRST + 32))
	{
		TRACE0("Failed to create mode toolbar\n");
		return -1;
	}
	m_wndModeBar.SetWindowText(_T("Modes"));
	CMFCToolBarButton manageButton(ID_MODE_MANAGE, -1, _T("Manage"), FALSE);
	manageButton.m_bText = TRUE;
	manageButton.m_bImage = FALSE;
	CMFCToolBarButton viewButton(ID_MODE_VIEW, -1, _T("View"), FALSE);
	viewButton.m_bText = TRUE;
	viewButton.m_bImage = FALSE;
	m_wndModeBar.InsertButton(manageButton);
	m_wndModeBar.InsertButton(viewButton);

	if (!CreateSlideshowBar())
	{
		TRACE0("Failed to create slideshow toolbar\n");
		return -1;
	}

	// Allow user-defined toolbars operations:
	InitUserToolbars(nullptr, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndModeBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndSlideshowBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndModeBar);
	DockPane(&m_wndSlideshowBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == nullptr)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_FILE_PRINT);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_SLIDESHOW_START);
	lstBasicCommands.AddTail(ID_SLIDESHOW_STOP);
	lstBasicCommands.AddTail(ID_SLIDESHOW_LOOP);
	lstBasicCommands.AddTail(ID_SLIDESHOW_INTERVAL_2);
	lstBasicCommands.AddTail(ID_SLIDESHOW_INTERVAL_5);
	lstBasicCommands.AddTail(ID_SLIDESHOW_INTERVAL_10);
	lstBasicCommands.AddTail(ID_SLIDESHOW_INTERVAL_30);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);

	return 0;
}

bool CMainFrame::CreateSlideshowBar()
{
	if (!m_wndSlideshowBar.CreateEx(this, TBSTYLE_FLAT,
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY,
		CRect(1, 1, 1, 1), AFX_IDW_CONTROLBAR_FIRST + 33))
		return false;
	m_wndSlideshowBar.SetWindowText(_T("Slideshow"));

	CMenu menu;
	CMenu intervals;
	if (!menu.CreatePopupMenu() || !intervals.CreatePopupMenu() ||
		!intervals.AppendMenu(MF_STRING, ID_SLIDESHOW_INTERVAL_2, _T("2 seconds")) ||
		!intervals.AppendMenu(MF_STRING, ID_SLIDESHOW_INTERVAL_5, _T("5 seconds")) ||
		!intervals.AppendMenu(MF_STRING, ID_SLIDESHOW_INTERVAL_10, _T("10 seconds")) ||
		!intervals.AppendMenu(MF_STRING, ID_SLIDESHOW_INTERVAL_30, _T("30 seconds")) ||
		!menu.AppendMenu(MF_STRING, ID_SLIDESHOW_START, _T("Start")) ||
		!menu.AppendMenu(MF_STRING, ID_SLIDESHOW_STOP, _T("Stop\tEsc")) ||
		!menu.AppendMenu(MF_SEPARATOR) ||
		!menu.AppendMenu(MF_POPUP, reinterpret_cast<UINT_PTR>(intervals.GetSafeHmenu()), _T("Interval")))
		return false;
	intervals.Detach();
	if (!menu.AppendMenu(MF_STRING, ID_SLIDESHOW_LOOP, _T("Repeat")))
		return false;

	CMFCToolBarMenuButton button(static_cast<UINT>(-1), menu.GetSafeHmenu(), -1, _T("Slideshow"));
	button.m_bText = TRUE;
	button.m_bImage = FALSE;
	return m_wndSlideshowBar.InsertButton(button) >= 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && m_wndContentSplitter.GetSafeHwnd() &&
		!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) &&
		!(GetKeyState(VK_MENU) & 0x8000))
	{
		CWnd* imageView = m_wndContentSplitter.GetDlgItem(m_wndContentSplitter.IdFromRowCol(1, 0));
		CWnd* fileList = m_wndContentSplitter.GetDlgItem(m_wndContentSplitter.IdFromRowCol(0, 0));
		if ((imageView && pMsg->hwnd == imageView->GetSafeHwnd()) ||
			(fileList && pMsg->hwnd == fileList->GetSafeHwnd()))
		{
			switch (pMsg->wParam)
			{
			case VK_ESCAPE:
				if (m_slideshowRunning) { StopSlideshow(); return TRUE; }
				break;
			case VK_LEFT: NavigateImage(ImageNavigation::Previous); return TRUE;
			case VK_RIGHT: NavigateImage(ImageNavigation::Next); return TRUE;
			case VK_HOME: NavigateImage(ImageNavigation::First); return TRUE;
			case VK_END: NavigateImage(ImageNavigation::Last); return TRUE;
			}
		}
	}
	return CFrameWndEx::PreTranslateMessage(pMsg);
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}


BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}


	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != nullptr)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// create a splitter with 1 rows and 2 column
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to create splitter window\n");
		return FALSE;
	}

	// Vasen paneeli: hakemistopuu
	if (!m_wndSplitter.CreateView(
		0,0,
		RUNTIME_CLASS(CFolderTreeView),
		CSize(250,0),
		pContext
	)) {
		return FALSE;
	}

	if (!m_wndContentSplitter.CreateStatic(&m_wndSplitter, 2, 1,
		WS_CHILD | WS_VISIBLE, m_wndSplitter.IdFromRowCol(0, 1)) ||
		!m_wndContentSplitter.CreateView(0, 0, RUNTIME_CLASS(CFileListView), CSize(0, 250), pContext) ||
		!m_wndContentSplitter.CreateView(1, 0, RUNTIME_CLASS(CMikasViewerView), CSize(0, 0), pContext))
	{
		TRACE0("Failed to create content views\n");
		return FALSE;
	}

	return TRUE;
	// return m_wndSplitter.Create(this, 2, 1, CSize(10, 10), pContext);
}

LRESULT CMainFrame::OnFolderSelected(WPARAM wParam, LPARAM lParam)
{
	StopSlideshow();
	CString* pPath = (CString*)lParam;
	GetImageView()->ClearImage();
	GetFileListView()->ShowFolder(*pPath);
	delete pPath;
	return 0;
}

// Käyttäjä klikkasi kuvatiedostoa puunäkymässä → lataa suoraan
LRESULT CMainFrame::OnFileSelected(WPARAM wParam, LPARAM lParam)
{
	StopSlideshow();
	CString* pPath = (CString*)lParam;
	if (pPath)
	{
		const int slash = pPath->ReverseFind(_T('\\'));
		if (slash >= 0)
		{
			GetImageView()->ClearImage();
			GetFileListView()->ShowFolder(pPath->Left(slash + 1), pPath->Mid(slash + 1));
		}
	}
	delete pPath;
	return 0;
}

LRESULT CMainFrame::OnFileListSelected(WPARAM, LPARAM lParam)
{
	CString* path = reinterpret_cast<CString*>(lParam);
	if (path)
	{
		if (m_slideshowRunning)
			KillTimer(slideshowTimer);
		GetImageView()->ClearImage();
		const bool loaded = GetImageView()->LoadImage(*path);
		delete path;
		if (m_slideshowRunning)
		{
			if (loaded)
				RestartSlideshowTimer();
			else
				StopSlideshow();
		}
	}
	return 0;
}

CMikasViewerView* CMainFrame::GetImageView()
{
	return static_cast<CMikasViewerView*>(m_wndContentSplitter.GetPane(1, 0));
}

CFileListView* CMainFrame::GetFileListView()
{
	return static_cast<CFileListView*>(m_wndContentSplitter.GetPane(0, 0));
}

bool CMainFrame::CanNavigateImage(ImageNavigation navigation)
{
	return GetFileListView()->CanNavigateImage(navigation);
}

void CMainFrame::NavigateImage(ImageNavigation navigation)
{
	GetFileListView()->NavigateImage(navigation);
}

void CMainFrame::SetManageMode(bool manage)
{
	if (m_manageMode == manage)
		return;
	int minimum = 0;
	if (!manage)
	{
		m_wndSplitter.GetColumnInfo(0, m_folderWidth, minimum);
		m_wndContentSplitter.GetRowInfo(0, m_listHeight, minimum);
		SetActiveView(GetImageView());
	}
	m_manageMode = manage;
	m_wndSplitter.SetColumnInfo(0, manage ? m_folderWidth : 0, 0);
	m_wndContentSplitter.SetRowInfo(0, manage ? m_listHeight : 0, 0);
	m_wndSplitter.GetPane(0, 0)->ShowWindow(manage ? SW_SHOW : SW_HIDE);
	GetFileListView()->ShowWindow(manage ? SW_SHOW : SW_HIDE);
	m_wndSplitter.RecalcLayout();
	m_wndContentSplitter.RecalcLayout();
	if (!manage)
		GetImageView()->SetFocus();
}

void CMainFrame::OnModeManage() { StopSlideshow(); SetManageMode(true); }
void CMainFrame::OnModeView() { SetManageMode(false); }
void CMainFrame::OnUpdateModeManage(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_manageMode); }
void CMainFrame::OnUpdateModeView(CCmdUI* pCmdUI) { pCmdUI->SetCheck(!m_manageMode); }

void CMainFrame::OnSlideshowStart()
{
	if (m_slideshowRunning || !GetFileListView()->HasSelectedImage() || !GetImageView()->HasImage())
		return;
	SetManageMode(false);
	m_slideshowRunning = true;
	RestartSlideshowTimer();
}

void CMainFrame::StopSlideshow()
{
	if (!m_slideshowRunning)
		return;
	m_slideshowRunning = false;
	KillTimer(slideshowTimer);
}

bool CMainFrame::RestartSlideshowTimer()
{
	KillTimer(slideshowTimer);
	if (SetTimer(slideshowTimer, m_slideshowIntervalSeconds * 1000, nullptr))
		return true;
	m_slideshowRunning = false;
	AfxMessageBox(_T("Could not start the slideshow timer."), MB_OK | MB_ICONERROR);
	return false;
}

void CMainFrame::OnSlideshowStop() { StopSlideshow(); }

void CMainFrame::OnSlideshowLoop()
{
	m_slideshowLoop = !m_slideshowLoop;
}

void CMainFrame::OnSlideshowInterval(UINT nID)
{
	m_slideshowIntervalSeconds = slideshowIntervals[nID - ID_SLIDESHOW_INTERVAL_2];
	if (m_slideshowRunning)
		RestartSlideshowTimer();
}

void CMainFrame::OnUpdateSlideshowStart(CCmdUI* pCmdUI)
{
	CWnd* list = m_wndContentSplitter.GetSafeHwnd()
		? m_wndContentSplitter.GetDlgItem(m_wndContentSplitter.IdFromRowCol(0, 0)) : nullptr;
	CWnd* image = m_wndContentSplitter.GetSafeHwnd()
		? m_wndContentSplitter.GetDlgItem(m_wndContentSplitter.IdFromRowCol(1, 0)) : nullptr;
	CFileListView* fileList = DYNAMIC_DOWNCAST(CFileListView, list);
	CMikasViewerView* imageView = DYNAMIC_DOWNCAST(CMikasViewerView, image);
	pCmdUI->Enable(!m_slideshowRunning && fileList && fileList->HasSelectedImage() &&
		imageView && imageView->HasImage());
}

void CMainFrame::OnUpdateSlideshowStop(CCmdUI* pCmdUI) { pCmdUI->Enable(m_slideshowRunning); }
void CMainFrame::OnUpdateSlideshowLoop(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_slideshowLoop); }

void CMainFrame::OnUpdateSlideshowInterval(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_slideshowIntervalSeconds == slideshowIntervals[pCmdUI->m_nID - ID_SLIDESHOW_INTERVAL_2]);
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != slideshowTimer)
	{
		CFrameWndEx::OnTimer(nIDEvent);
		return;
	}
	if (!m_slideshowRunning)
		return;
	KillTimer(slideshowTimer);
	if (!GetFileListView()->AdvanceSlideshow(m_slideshowLoop))
		StopSlideshow();
	if (m_slideshowRunning)
		RestartSlideshowTimer();
}

void CMainFrame::OnDestroy()
{
	StopSlideshow();
	CFrameWndEx::OnDestroy();
}

// Tulostuskomennot ohjataan aina kuvanäkymään riippumatta aktiivisesta paneelista
BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (nID == ID_FILE_PRINT || nID == ID_FILE_PRINT_DIRECT || nID == ID_FILE_PRINT_PREVIEW)
	{
		if (nCode == CN_COMMAND && pHandlerInfo == nullptr)
			StopSlideshow();
		CMikasViewerView* pView = GetImageView();
		if (pView && pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
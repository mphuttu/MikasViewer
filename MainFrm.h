
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "FolderTreeView.h"
#include "MikasViewerView.h"
#include "FileListView.h"

#define WM_FOLDER_SELECTED (WM_USER + 1)
#define WM_FILE_SELECTED   (WM_USER + 2)
#define WM_FILE_LIST_SELECTED (WM_USER + 3)
#define ID_MODE_MANAGE 32772
#define ID_MODE_VIEW 32773


class CMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMainFrame() noexcept;
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CMainFrame();
	bool CanNavigateImage(ImageNavigation navigation);
	void NavigateImage(ImageNavigation navigation);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCToolBar       m_wndModeBar;
	CMFCToolBar       m_wndSlideshowBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;

	CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndContentSplitter;
	bool m_manageMode = true;
	int m_folderWidth = 250;
	int m_listHeight = 250;
	bool m_slideshowRunning = false;
	bool m_slideshowLoop = true;
	UINT m_slideshowIntervalSeconds = 5;

	CMikasViewerView* GetImageView();
	CFileListView* GetFileListView();
	void SetManageMode(bool manage);
	bool CreateSlideshowBar();
	void StopSlideshow();
	bool RestartSlideshowTimer();

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnFolderSelected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFileSelected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFileListSelected(WPARAM wParam, LPARAM lParam);
	afx_msg void OnModeManage();
	afx_msg void OnModeView();
	afx_msg void OnUpdateModeManage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateModeView(CCmdUI* pCmdUI);
	afx_msg void OnSlideshowStart();
	afx_msg void OnSlideshowStop();
	afx_msg void OnSlideshowLoop();
	afx_msg void OnSlideshowInterval(UINT nID);
	afx_msg void OnUpdateSlideshowStart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSlideshowStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSlideshowLoop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSlideshowInterval(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

};

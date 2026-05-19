
// MikasViewer.h : main header file for the MikasViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include <gdiplus.h>


// CMikasViewerApp:
// See MikasViewer.cpp for the implementation of this class
//

class CMikasViewerApp : public CWinAppEx
{
public:
	CMikasViewerApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;
	ULONG_PTR m_gdiplusToken;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnHelpTopics();
	DECLARE_MESSAGE_MAP()
};

extern CMikasViewerApp theApp;

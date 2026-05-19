
// MikasViewerView.h : interface of the CMikasViewerView class
//

#pragma once
#include "MikasViewerDoc.h"
#include <afxwin.h>
#include <memory>
#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")


class CMikasViewerView : public CView
{
protected:
	CMikasViewerView() noexcept;
	DECLARE_DYNCREATE(CMikasViewerView)

public:
	CMikasViewerDoc* GetDocument() const;

public:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate();
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);

public:
	virtual ~CMikasViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void LoadImage(const CString& strPath);

private:
	void UpdateScrollBars();

protected:
	std::unique_ptr<Gdiplus::Image> m_pImage;
	double  m_dZoom         = 1.0;
	CPoint  m_scrollPos     = CPoint(0, 0);
	bool    m_bDragging     = false;
	CPoint  m_ptDragStart   = CPoint(0, 0);
	CPoint  m_ptScrollStart = CPoint(0, 0);

protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnUpdatePrint(CCmdUI* pCmdUI);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MikasViewerView.cpp
inline CMikasViewerDoc* CMikasViewerView::GetDocument() const
   { return reinterpret_cast<CMikasViewerDoc*>(m_pDocument); }
#endif


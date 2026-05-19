
// MikasViewerView.cpp : implementation of the CMikasViewerView class
//

#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "MikasViewer.h"
#endif

#include "MikasViewerDoc.h"
#include "MikasViewerView.h"
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMikasViewerView

IMPLEMENT_DYNCREATE(CMikasViewerView, CView)

BEGIN_MESSAGE_MAP(CMikasViewerView, CView)
	ON_COMMAND(ID_FILE_PRINT,         &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT,  &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMikasViewerView::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT,         &CMikasViewerView::OnUpdatePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_DIRECT,  &CMikasViewerView::OnUpdatePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CMikasViewerView::OnUpdatePrint)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// Construction/Destruction

CMikasViewerView::CMikasViewerView() noexcept
{
}

CMikasViewerView::~CMikasViewerView()
{
}

BOOL CMikasViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_HSCROLL | WS_VSCROLL;
	return CView::PreCreateWindow(cs);
}

void CMikasViewerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	UpdateScrollBars();
}

void CMikasViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	UpdateScrollBars();
	Invalidate(FALSE);
}


// ---------- Scroll bar helpers ----------

void CMikasViewerView::UpdateScrollBars()
{
	CRect rc;
	GetClientRect(&rc);
	int W = rc.Width(), H = rc.Height();
	int sw = m_pImage ? (int)(m_pImage->GetWidth()  * m_dZoom) : 0;
	int sh = m_pImage ? (int)(m_pImage->GetHeight() * m_dZoom) : 0;

	// Rajoita scroll kelvolliselle alueelle
	m_scrollPos.x = max(0, min(m_scrollPos.x, max(0, sw - W)));
	m_scrollPos.y = max(0, min(m_scrollPos.y, max(0, sh - H)));

	SCROLLINFO si = {};
	si.cbSize = sizeof(si);
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin   = 0;

	si.nMax  = max(0, sw - 1);   // kun sw <= W, nPage > nMax → scrollbar disabloituu
	si.nPage = W;
	si.nPos  = m_scrollPos.x;
	SetScrollInfo(SB_HORZ, &si, TRUE);

	si.nMax  = max(0, sh - 1);
	si.nPage = H;
	si.nPos  = m_scrollPos.y;
	SetScrollInfo(SB_VERT, &si, TRUE);
}

void CMikasViewerView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	SCROLLINFO si = {};
	si.cbSize = sizeof(si);
	si.fMask  = SIF_ALL;
	GetScrollInfo(SB_HORZ, &si);

	int oldPos = si.nPos;
	switch (nSBCode)
	{
	case SB_LINELEFT:          si.nPos -= 30; break;
	case SB_LINERIGHT:         si.nPos += 30; break;
	case SB_PAGELEFT:          si.nPos -= (int)si.nPage; break;
	case SB_PAGERIGHT:         si.nPos += (int)si.nPage; break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:     si.nPos  = (int)nPos; break;
	}
	si.nPos = max(0, min(si.nPos, max(0, si.nMax - (int)si.nPage + 1)));

	if (si.nPos != oldPos)
	{
		m_scrollPos.x = si.nPos;
		SetScrollInfo(SB_HORZ, &si, TRUE);
		Invalidate(FALSE);
	}
}

void CMikasViewerView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	SCROLLINFO si = {};
	si.cbSize = sizeof(si);
	si.fMask  = SIF_ALL;
	GetScrollInfo(SB_VERT, &si);

	int oldPos = si.nPos;
	switch (nSBCode)
	{
	case SB_LINEUP:            si.nPos -= 30; break;
	case SB_LINEDOWN:          si.nPos += 30; break;
	case SB_PAGEUP:            si.nPos -= (int)si.nPage; break;
	case SB_PAGEDOWN:          si.nPos += (int)si.nPage; break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:     si.nPos  = (int)nPos; break;
	}
	si.nPos = max(0, min(si.nPos, max(0, si.nMax - (int)si.nPage + 1)));

	if (si.nPos != oldPos)
	{
		m_scrollPos.y = si.nPos;
		SetScrollInfo(SB_VERT, &si, TRUE);
		Invalidate(FALSE);
	}
}


// ---------- Drawing ----------

BOOL CMikasViewerView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;  // Tausta piirretään OnDraw:ssa double-bufferilla → ei värinää
}

void CMikasViewerView::OnDraw(CDC* pDC)
{
	CMikasViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	CRect rc;
	GetClientRect(&rc);

	// Double-buffer: piirrä muistiin ensin, kopioi sitten ruudulle
	CDC     memDC;
	CBitmap bmp;
	memDC.CreateCompatibleDC(pDC);
	bmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);

	// Tumma tausta kuten kuvankatselimissa
	memDC.FillSolidRect(rc, RGB(40, 40, 40));

	if (m_pImage)
	{
		int sw = (int)(m_pImage->GetWidth()  * m_dZoom);
		int sh = (int)(m_pImage->GetHeight() * m_dZoom);

		// Kuva on keskitetty kun se mahtuu ikkunaan; muuten scroll määrää sijainnin
		int ox = max(0, (rc.Width()  - sw) / 2) - m_scrollPos.x;
		int oy = max(0, (rc.Height() - sh) / 2) - m_scrollPos.y;

		Graphics g(memDC.GetSafeHdc());
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
		g.DrawImage(m_pImage.get(), ox, oy, sw, sh);
	}

	pDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}


// ---------- Pan (hiiren raahaus) ----------

void CMikasViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_pImage) return;
	m_bDragging     = true;
	m_ptDragStart   = point;
	m_ptScrollStart = m_scrollPos;
	SetCapture();
	CView::OnLButtonDown(nFlags, point);
}

void CMikasViewerView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging && m_pImage)
	{
		CRect rc;
		GetClientRect(&rc);
		int sw = (int)(m_pImage->GetWidth()  * m_dZoom);
		int sh = (int)(m_pImage->GetHeight() * m_dZoom);

		CPoint ns;
		ns.x = m_ptScrollStart.x - (point.x - m_ptDragStart.x);
		ns.y = m_ptScrollStart.y - (point.y - m_ptDragStart.y);
		ns.x = max(0, min(ns.x, max(0, sw - rc.Width())));
		ns.y = max(0, min(ns.y, max(0, sh - rc.Height())));

		if (ns != m_scrollPos)
		{
			m_scrollPos = ns;
			SetScrollPos(SB_HORZ, m_scrollPos.x, TRUE);
			SetScrollPos(SB_VERT, m_scrollPos.y, TRUE);
			Invalidate(FALSE);
		}
	}
	CView::OnMouseMove(nFlags, point);
}

void CMikasViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		m_bDragging = false;
		ReleaseCapture();
	}
	CView::OnLButtonUp(nFlags, point);
}

BOOL CMikasViewerView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_pImage && nHitTest == HTCLIENT)
	{
		SetCursor(LoadCursor(NULL, m_bDragging ? IDC_SIZEALL : IDC_HAND));
		return TRUE;
	}
	return CView::OnSetCursor(pWnd, nHitTest, message);
}


// ---------- Zoom (hiiren rullaus) ----------

BOOL CMikasViewerView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint pt)
{
	if (!m_pImage) return TRUE;

	ScreenToClient(&pt);
	CRect rc;
	GetClientRect(&rc);

	int sw = (int)(m_pImage->GetWidth()  * m_dZoom);
	int sh = (int)(m_pImage->GetHeight() * m_dZoom);

	// Kuvan koordinaatti hiiren alla ennen zoomausta
	// Kaava: screen_x = max(0,(W-sw)/2) - scroll.x + imgX*zoom  →  imgX = ...
	double imgX = (pt.x - max(0, (rc.Width()  - sw) / 2) + m_scrollPos.x) / m_dZoom;
	double imgY = (pt.y - max(0, (rc.Height() - sh) / 2) + m_scrollPos.y) / m_dZoom;

	// Sovella zoom (±15 % per naksaus)
	double factor = (zDelta > 0) ? 1.15 : (1.0 / 1.15);
	m_dZoom = max(0.05, min(20.0, m_dZoom * factor));

	int sw2 = (int)(m_pImage->GetWidth()  * m_dZoom);
	int sh2 = (int)(m_pImage->GetHeight() * m_dZoom);

	// Laske uusi scroll niin että sama kuvan piste pysyy hiiren alla
	// Kaava: scroll_new = max(0,(W-sw2)/2) + imgX*zoom_new - pt.x
	double cx = max(0.0, (double)(rc.Width()  - sw2) / 2.0);
	double cy = max(0.0, (double)(rc.Height() - sh2) / 2.0);
	m_scrollPos.x = max(0, (int)(cx + imgX * m_dZoom - pt.x));
	m_scrollPos.y = max(0, (int)(cy + imgY * m_dZoom - pt.y));

	UpdateScrollBars();
	Invalidate(FALSE);
	return TRUE;
}


// ---------- Kuvan lataus ----------

// WIC-pohjainen lataus formaateille joita GDI+ ei tue (esim. WebP)
static std::unique_ptr<Gdiplus::Bitmap> LoadViaWIC(const CString& strPath)
{
	IWICImagingFactory* pFactory = nullptr;
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory))))
		return nullptr;

	IWICBitmapDecoder* pDecoder = nullptr;
	HRESULT hr = pFactory->CreateDecoderFromFilename(
		(LPCWSTR)strPath, nullptr, GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr)) { pFactory->Release(); return nullptr; }

	IWICBitmapFrameDecode* pFrame = nullptr;
	hr = pDecoder->GetFrame(0, &pFrame);
	if (FAILED(hr)) { pDecoder->Release(); pFactory->Release(); return nullptr; }

	IWICFormatConverter* pConverter = nullptr;
	hr = pFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr)) { pFrame->Release(); pDecoder->Release(); pFactory->Release(); return nullptr; }

	hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppBGRA,
		WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom);

	std::unique_ptr<Gdiplus::Bitmap> pBmp;
	if (SUCCEEDED(hr))
	{
		UINT w = 0, h = 0;
		pConverter->GetSize(&w, &h);
		pBmp = std::make_unique<Gdiplus::Bitmap>(w, h, PixelFormat32bppARGB);
		if (pBmp->GetLastStatus() == Gdiplus::Ok)
		{
			Gdiplus::BitmapData data{};
			Gdiplus::Rect rect(0, 0, (INT)w, (INT)h);
			pBmp->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &data);
			pConverter->CopyPixels(nullptr, data.Stride,
				(UINT)(data.Height * abs(data.Stride)), (BYTE*)data.Scan0);
			pBmp->UnlockBits(&data);
		}
		else
		{
			pBmp.reset();
		}
	}

	pConverter->Release();
	pFrame->Release();
	pDecoder->Release();
	pFactory->Release();
	return pBmp;
}

void CMikasViewerView::LoadImage(const CString& strPath)
{
	// Kokeile ensin GDI+ (make_unique käyttää ::new → ohittaa DEBUG_NEW-makron)
	auto pNew = std::make_unique<Gdiplus::Image>((LPCWSTR)strPath);
	if (pNew->GetLastStatus() != Gdiplus::Ok)
	{
		// GDI+ ei tukenut formaattia → kokeile WIC (esim. WebP)
		auto pBmp = LoadViaWIC(strPath);
		if (!pBmp)
		{
			AfxMessageBox(_T("Kuvan lataus epäonnistui."));
			return;
		}
		// Bitmap periytyy Image:sta → release+siirto on turvallista
		m_pImage = std::unique_ptr<Gdiplus::Image>(pBmp.release());
	}
	else
	{
		m_pImage = std::move(pNew);
	}

	// Fit-to-window oletuksena (max 100 %)
	CRect rc;
	GetClientRect(&rc);
	if (rc.Width() > 0 && rc.Height() > 0)
	{
		double sx = (double)rc.Width()  / m_pImage->GetWidth();
		double sy = (double)rc.Height() / m_pImage->GetHeight();
		m_dZoom = min(1.0, min(sx, sy));
	}
	else
	{
		m_dZoom = 1.0;
	}

	m_scrollPos = CPoint(0, 0);
	UpdateScrollBars();
	Invalidate(FALSE);

	// Päivitä ikkunan otsikko: "tiedostonimi - MikasViewer"
	int nSlash = strPath.ReverseFind(L'\\');
	CString fileName = (nSlash >= 0) ? strPath.Mid(nSlash + 1) : strPath;
	if (AfxGetMainWnd())
		AfxGetMainWnd()->SetWindowText(fileName + _T(" - MikasViewer"));
}


// ---------- Tulostus ----------

void CMikasViewerView::OnUpdatePrint(CCmdUI* pCmdUI)
{
	// Tulostus käytössä vain kun kuva on ladattu
	pCmdUI->Enable(m_pImage != nullptr);
}

void CMikasViewerView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMikasViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Yksi sivu
	pInfo->SetMaxPage(1);
	return DoPreparePrinting(pInfo);
}

void CMikasViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CMikasViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

// OnPrint kutsutaan tulostuksessa ja esikatseltaessa (korvaa OnDraw:n tulostustilanteessa)
void CMikasViewerView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	if (!m_pImage) return;

	// Tulostusalueen koko laitepikseleissä (ei sisällä fyysisiä marginaaleja)
	int pageW = pDC->GetDeviceCaps(HORZRES);
	int pageH = pDC->GetDeviceCaps(VERTRES);

	// Pieni esteettinen marginaali (5 % per reuna)
	int marginX = pageW / 20;
	int marginY = pageH / 20;
	int areaW   = pageW - 2 * marginX;
	int areaH   = pageH - 2 * marginY;

	UINT imgW = m_pImage->GetWidth();
	UINT imgH = m_pImage->GetHeight();

	// Skaalaa kuva täyttämään alue kuvasuhdetta rikkomatta
	double sx    = (double)areaW / imgW;
	double sy    = (double)areaH / imgH;
	double scale = min(sx, sy);

	int drawW = (int)(imgW * scale);
	int drawH = (int)(imgH * scale);

	// Keskitä alueeseen
	int ox = marginX + (areaW - drawW) / 2;
	int oy = marginY + (areaH - drawH) / 2;

	Graphics g(pDC->GetSafeHdc());
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.DrawImage(m_pImage.get(), ox, oy, drawW, drawH);
}

void CMikasViewerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMikasViewerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// ---------- Diagnostics ----------

#ifdef _DEBUG
void CMikasViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CMikasViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMikasViewerDoc* CMikasViewerView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMikasViewerDoc)));
	return (CMikasViewerDoc*)m_pDocument;
}
#endif //_DEBUG


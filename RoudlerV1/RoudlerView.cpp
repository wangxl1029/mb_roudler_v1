
// RoudlerView.cpp : implementation of the CRoudlerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Roudler.h"
#endif

#include "RoudlerDoc.h"
#include "RoudlerView.h"
#include "SetOrginPos.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HWND g_hViewWnd = NULL;
// CRoudlerView

IMPLEMENT_DYNCREATE(CRoudlerView, CView)

BEGIN_MESSAGE_MAP(CRoudlerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_GEO_SETORGIN, &CRoudlerView::OnGeoSetorgin)
	ON_COMMAND(ID_FILE_IMPORT_JSON, &CRoudlerView::OnFileImportJson)
	ON_COMMAND(ID_FILE_NEW, &CRoudlerView::OnFileNew)
	ON_MESSAGE(WM_MYNOTIFY, &CRoudlerView::OnMyNotification)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CRoudlerView construction/destruction

CRoudlerView::CRoudlerView()
{
	// TODO: add construction code here

}

CRoudlerView::~CRoudlerView()
{
}

BOOL CRoudlerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CRoudlerView drawing

void CRoudlerView::OnDraw(CDC* pDC)
{
	g_hViewWnd = m_hWnd;
	CRoudlerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	if (pDoc->IsDispReady())
	{
		CRect rectClient;
		GetClientRect(rectClient);
		rectClient.OffsetRect(m_orgin);
		pDoc->TestOnDraw(pDC, rectClient);
	}
}


// CRoudlerView printing

BOOL CRoudlerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CRoudlerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CRoudlerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CRoudlerView diagnostics

#ifdef _DEBUG
void CRoudlerView::AssertValid() const
{
	CView::AssertValid();
}

void CRoudlerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CRoudlerDoc* CRoudlerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRoudlerDoc)));
	return (CRoudlerDoc*)m_pDocument;
}
#endif //_DEBUG


// CRoudlerView message handlers


void CRoudlerView::OnGeoSetorgin()
{
	// TODO: Add your command handler code here
	CSetOrginPos dlg;
	dlg.m_nOrginX = m_orgin.x;
	dlg.m_nOrginY = m_orgin.y;

	auto ret = dlg.DoModal();
	switch (ret)
	{
	case IDOK:
		TRACE0("Set Orgin postion OK.\n");
		m_orgin.SetPoint(dlg.m_nOrginX, dlg.m_nOrginY);
		Invalidate();
		break;
	case IDCANCEL:
		TRACE0("Orgin postion setting cancelled!\n");
		break;
	default:
		TRACE0("Unknown Value!\n");
		break;
	}
}


void CRoudlerView::OnFileImportJson()
{
	// TODO: Add your command handler code here
	static TCHAR BASED_CODE szFilter[] = _T("JSON FILE(*.json)|*.json||");
	// TODO: Add your command handler code here
	CFileDialog dlg(TRUE,
		_T("json"),
		_T("SomeRoute"),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	auto retcode = dlg.DoModal();
	if (IDOK == retcode)
	{
		CString logPath = dlg.GetPathName();
		auto pDoc = GetDocument();

		//pDoc->LoadJson(logPath);
		pDoc->asynLoadRouteSearchData(logPath);
	}
}


void CRoudlerView::OnFileNew()
{
	// TODO: Add your command handler code here
}

afx_msg LRESULT CRoudlerView::OnMyNotification(WPARAM wParam, LPARAM lParam)
{
	CRoudlerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	return (pDoc ? pDoc->ProcMyNotification(wParam, lParam) : 0);
}


void CRoudlerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();

	CRoudlerDoc* pDoc = GetDocument();
	ASSERT(pDoc);

	if (pDoc->IsDispReady())
	{
		CClientDC dc(this);
		OnPrepareDC(&dc);
		dc.DPtoLP(&point);

		m_oldPos = point;
	}


	//CView::OnLButtonDown(nFlags, point);
}


void CRoudlerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (GetCapture() != this)
	{
		return;
	}

	CRoudlerDoc* pDoc = GetDocument();
	ASSERT(pDoc);

	if (pDoc->IsDispReady())
	{
		CClientDC dc(this);
		OnPrepareDC(&dc);
		dc.DPtoLP(&point);

		m_oldPos = point;
	}

	ReleaseCapture();
	//CView::OnLButtonUp(nFlags, point);
}


void CRoudlerView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (GetCapture() != this)
	{
		return;
	}


	CRoudlerDoc* pDoc = GetDocument();
	if (pDoc->IsDispReady())
	{
		CClientDC dc(this);
		OnPrepareDC(&dc);
		dc.DPtoLP(&point);

		//pDoc->ViewPortOffset(m_orgin - point);
		m_orgin.Offset(m_oldPos - point);
		m_oldPos = point;

		Invalidate();
	}


	//CView::OnMouseMove(nFlags, point);
}


// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Roudler.h"

#include "MainFrm.h"
#include "RoudlerView.h"
#include "SendView.h"
#include "InfoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);


	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers



BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class

	BOOL result = FALSE;
	if (m_wndSpliteTop.CreateStatic(this, 2, 1))
	{
		CRect rect;
		GetClientRect(&rect);
		CSize size = rect.Size();
		size.cy -= 260;
		if (m_wndSpliteTop.CreateView(0, 0, RUNTIME_CLASS(CRoudlerView), size, pContext))
		{
			if (m_wndSpliteMsg.CreateStatic(&m_wndSpliteTop, 1, 2, WS_CHILD | WS_VISIBLE, m_wndSpliteTop.IdFromRowCol(1, 0)))
			{
				if (m_wndSpliteMsg.CreateView(0, 0, RUNTIME_CLASS(CSendView), CSize(size.cx / 2, 0), pContext))
				{
					if (m_wndSpliteMsg.CreateView(0, 1, RUNTIME_CLASS(CInfoView), CSize(0, 0), pContext))
					{
						SetActiveView((CView*)m_wndSpliteMsg.GetPane(0, 0));
						result = TRUE;
					}
				}
			}
		}
	}

	return result;
}

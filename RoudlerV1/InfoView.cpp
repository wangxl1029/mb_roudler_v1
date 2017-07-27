// InfoView.cpp : implementation file
//

#include "stdafx.h"
#include "InfoView.h"


// CInfoView

IMPLEMENT_DYNCREATE(CInfoView, CEditView)

CInfoView::CInfoView()
{

}

CInfoView::~CInfoView()
{
}

BEGIN_MESSAGE_MAP(CInfoView, CEditView)
END_MESSAGE_MAP()


// CInfoView diagnostics

#ifdef _DEBUG
void CInfoView::AssertValid() const
{
	CEditView::AssertValid();
}

#ifndef _WIN32_WCE
void CInfoView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif
#endif //_DEBUG


// CInfoView message handlers


BOOL CInfoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= ES_READONLY | ES_NOHIDESEL | ES_AUTOVSCROLL | WS_VSCROLL | ES_MULTILINE;

	return CEditView::PreCreateWindow(cs);
}


// show message info
void CInfoView::Message(LPCTSTR msg)
{
	if (msg)
	{
		CString line = msg;
		line += _T("\r\n");
		auto len = GetWindowTextLength();
		GetEditCtrl().SetSel(len, len);
		GetEditCtrl().ReplaceSel(line);
	}
}

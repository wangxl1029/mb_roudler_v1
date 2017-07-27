// SendView.cpp : implementation file
//

#include "stdafx.h"
#include "SendView.h"
#include "RoudlerDoc.h"

// CSendView

IMPLEMENT_DYNCREATE(CSendView, CEditView)

CSendView::CSendView()
{

}

CSendView::~CSendView()
{
}

BEGIN_MESSAGE_MAP(CSendView, CEditView)
	ON_WM_CHAR()
END_MESSAGE_MAP()


// CSendView diagnostics

#ifdef _DEBUG
void CSendView::AssertValid() const
{
	CEditView::AssertValid();
}

#ifndef _WIN32_WCE
void CSendView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSendView message handlers


void CSendView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if (VK_RETURN ==  nChar && 1 == nRepCnt)
	{
		CRoudlerDoc* pDoc = DYNAMIC_DOWNCAST(CRoudlerDoc,GetDocument());
		ASSERT_VALID(pDoc);

		CString strText;
		GetEditCtrl().GetWindowText(strText);
		pDoc->SendInfo(_T("user"), strText.LockBuffer());
		strText.ReleaseBuffer();

		strText = _T("");
		GetEditCtrl().SetWindowText(strText);
	}
	else
	{
		CEditView::OnChar(nChar, nRepCnt, nFlags);
	}
}


BOOL CSendView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class

	return CEditView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

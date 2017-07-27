
// RoudlerView.h : interface of the CRoudlerView class
//

#pragma once
class CRoudlerDoc;

class CRoudlerView : public CView
{
protected: // create from serialization only
	CRoudlerView();
	DECLARE_DYNCREATE(CRoudlerView)

// Attributes
public:
	CRoudlerDoc* GetDocument() const;
// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CRoudlerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// data
private:
	CPoint m_orgin;
	CPoint m_oldPos;
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileNew();
	afx_msg void OnGeoSetorgin();
	afx_msg void OnFileImportJson();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
protected:
	afx_msg LRESULT OnMyJsonNotified(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMyNotification(WPARAM wParam, LPARAM lParam);
};

#ifndef _DEBUG  // debug version in RoudlerView.cpp
inline CRoudlerDoc* CRoudlerView::GetDocument() const
   { return reinterpret_cast<CRoudlerDoc*>(m_pDocument); }
#endif


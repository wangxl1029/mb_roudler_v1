#pragma once


// CInfoView view

class CInfoView : public CEditView
{
	DECLARE_DYNCREATE(CInfoView)

protected:
	CInfoView();           // protected constructor used by dynamic creation
	virtual ~CInfoView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	// show message info
	void Message(LPCTSTR msg);
};



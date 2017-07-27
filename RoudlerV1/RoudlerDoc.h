
// RoudlerDoc.h : interface of the CRoudlerDoc class
//


#pragma once

#define WM_MYNOTIFY (WM_APP + 1)

class CFromDSegment : public CObject
{
protected:
	DECLARE_SERIAL(CFromDSegment);
	// Overrides
	CFromDSegment();
public: // attribute
	CArray<CPoint> m_aryPoint;
public:
	virtual void Serialize(CArchive& ar);
	// Implementation
public:
//	virtual ~CFromDSegment();
#ifdef _DEBUG
	//virtual void AssertValid() const;
	//virtual void Dump(CDumpContext& dc) const;
#endif
};

class CRoudlerDoc : public CDocument
{
	class CPrivate;
protected: // create from serialization only
	CRoudlerDoc();
	DECLARE_DYNCREATE(CRoudlerDoc)

// Attributes
public:
	CTypedPtrList<CObList, CFromDSegment*> m_fromDSegList;

// Operations
public:
	int asynLoadRouteSearchData(LPCTSTR lpszPathName);
	void TestOnDraw(CDC*, LPRECT);
	// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CRoudlerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// helper
private:
	CPoint convertFromGeoPnt2viewPnt(CPoint const&, CRect const&);
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

private:
	CPrivate* mp;

public:
	afx_msg void OnFileImportJson();
	LRESULT ProcMyNotification(WPARAM wParam, LPARAM lParam);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual void OnFinalRelease();
	// send message
	void SendInfo(LPTSTR nick, LPTSTR msg);
	void SendInfo(LPTSTR msg);
	// check ready to display
	BOOL IsDispReady() const;
	// asynichronical import json
	BOOL AsynImportJson(LPCTSTR);
};

enum {
	E_NID_NONE,
	E_NID_JSON_COMPLETE,
	// doc cmd parser notification
	E_TID_DISP_PREP_,
	E_TID_DISP_PREP_RUTLNK_SHPPNT_NUM,
	E_NID_DOC_PARSER_PRINT_PARENT_COST_INFO,
	E_NID_DOC_PARSER_PRINT_BRANCH_INFO,
	E_NID_DOC_PARSER_PRINT_JSONINFO,
	// Worker Thread
	E_WID_LIFE_CYCLE,
	E_WID_PROC_START,
	E_WID_PROC_END,
	E_NID_TEST,
	E_NID_MAX
};

enum class THD_WK_LC_E : LPARAM {
	// thread worker lifecycle
	UNKNOWN = 0,
	PROC_START,
	PROC_DISP_PREPARED,
	PROC_DISP_PREPARE_FAILURE,
	PROC_END
};


extern LPTSTR g_szDocNick;
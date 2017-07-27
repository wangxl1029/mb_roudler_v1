#pragma once

#include "resource.h"       // main symbols

// CSetOrginPos dialog

class CSetOrginPos : public CDialogEx
{
	DECLARE_DYNAMIC(CSetOrginPos)

public:
	CSetOrginPos(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetOrginPos();

// Dialog Data
	enum { IDD = IDD_SETORGINXY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// orginal X
	long m_nOrginX;
	// Orginal Y
	long m_nOrginY;
};

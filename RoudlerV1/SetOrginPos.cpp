// SetOrginPos.cpp : implementation file
//

#include "stdafx.h"
#include "SetOrginPos.h"
#include "afxdialogex.h"


// CSetOrginPos dialog

IMPLEMENT_DYNAMIC(CSetOrginPos, CDialogEx)

CSetOrginPos::CSetOrginPos(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetOrginPos::IDD, pParent)
	, m_nOrginX(0)
	, m_nOrginY(0)
{

}

CSetOrginPos::~CSetOrginPos()
{
}

void CSetOrginPos::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_nOrginX);
	DDX_Text(pDX, IDC_EDIT2, m_nOrginY);
}


BEGIN_MESSAGE_MAP(CSetOrginPos, CDialogEx)
END_MESSAGE_MAP()


// CSetOrginPos message handlers

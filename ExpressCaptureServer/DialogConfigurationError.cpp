// ConfigurationError.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogConfigurationError.h"
#include "afxdialogex.h"
#include "Dvb.h"
#include "error_codes.h"

// CConfigurationError dialog

IMPLEMENT_DYNAMIC(CDialogConfigurationError, CDialogEx)

CDialogConfigurationError::CDialogConfigurationError(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_CONFIGURATION_ERROR, pParent)
{

}

CDialogConfigurationError::~CDialogConfigurationError()
{
}

void CDialogConfigurationError::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CONFIGURATION_ERROR, m_error_text);
}


BEGIN_MESSAGE_MAP(CDialogConfigurationError, CDialogEx)
END_MESSAGE_MAP()


// CConfigurationError message handlers


BOOL CDialogConfigurationError::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CString error;
	error.Format("%s %d", get_error_text(), get_error());
	m_error_text.SetWindowTextA(error);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

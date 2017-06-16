#pragma once
#include "afxwin.h"


// CConfigurationError dialog

class CDialogConfigurationError : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogConfigurationError)

public:
	CDialogConfigurationError(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogConfigurationError();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONFIGURATION_ERROR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_error_text;
	virtual BOOL OnInitDialog();
};

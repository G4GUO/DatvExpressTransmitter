#pragma once
#include "afxwin.h"


// CDialogDVBS2Settings dialog

class CDialogDVBS2Settings : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogDVBS2Settings)

public:
	CDialogDVBS2Settings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogDVBS2Settings();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DVB_S2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_restart_required;
	virtual BOOL OnInitDialog();
	void validate_fec(int mod);

	afx_msg void OnClickedDvbs2Qpsk();
	afx_msg void OnDvbs28psk();
	afx_msg void OnDvbs216apsk();
	afx_msg void OnDvbs232apsk();
	afx_msg void OnBnClickedOk();
};

#pragma once
#include "afxwin.h"


// CDialogDVBTSettings dialog

class CDialogDVBTSettings : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogDVBTSettings)

public:
	CDialogDVBTSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogDVBTSettings();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DVB_T };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CButton m_mode;
	CButton m_channel;
	CButton m_constellation;
	CButton m_fec;
	CButton m_guard;
public:
	BOOL m_restart_required;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};

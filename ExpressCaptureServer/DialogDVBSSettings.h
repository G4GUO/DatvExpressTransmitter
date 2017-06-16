#pragma once


// CDialogDVBSSettings dialog

class CDialogDVBSSettings : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogDVBSSettings)

public:
	CDialogDVBSSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogDVBSSettings();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DVB_S };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_restart_required;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CDialogExpressSettings dialog

class CDialogExpressSettings : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogExpressSettings)

public:
	CDialogExpressSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogExpressSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG_EXPRESS_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CEdit m_express_frequency;
	CEdit m_express_symbol_rate;
	CEdit m_express_level;
public:
	BOOL m_restart_required;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
private:
	CButton m_ptt_port_a;
	CButton m_ptt_port_b;
	CButton m_ptt_port_c;
	CButton m_ptt_port_d;
public:
	afx_msg void OnClickedButtonApply();
private:
	CIPAddressCtrl m_sdr_ip;
};

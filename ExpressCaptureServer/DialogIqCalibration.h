#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CDialogIqCalibration dialog

class CDialogIqCalibration : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogIqCalibration)

public:
	CDialogIqCalibration(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogIqCalibration();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_IQ_CALIBRATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDeltaposSpinICalibraion(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinQCalibraion(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickedCheckEnableIqCalibration();
private:
	CButton m_iq_enable;
	CSpinButtonCtrl m_i_offset;
	CSpinButtonCtrl m_q_offset;
	CStatic m_i_offset_text;
	CStatic m_q_offset_text;
	int m_i_value;
	int m_q_value;
public:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
};

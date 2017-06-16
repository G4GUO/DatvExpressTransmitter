#pragma once
#include "afxwin.h"


// CDialogNoiseTool dialog

class CDialogNoiseTool : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogNoiseTool)

public:
	CDialogNoiseTool(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogNoiseTool();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NOISE_TOOL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CEdit m_sn_ratio;
	CButton m_enable_noise;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedCheckEnableNoise();
	afx_msg void OnClickedButtonApplyNoise();
};

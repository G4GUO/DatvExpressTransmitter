#pragma once
#include "afxwin.h"


// CDialogOutputFormat dialog

class CDialogOutputFormat : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogOutputFormat)

public:
	CDialogOutputFormat(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogOutputFormat();

// Dialog Data
	enum { IDD = IDD_DIALOG_OUTPUT_FORMAT };

protected:
	HICON m_hIcon;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CComboBox m_onair_video_format;
	CButton m_aspect_ratio;
public:
	BOOL m_restart_required;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};

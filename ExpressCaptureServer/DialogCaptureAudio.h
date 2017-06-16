#pragma once
#include "afxwin.h"


// CDialogCaptureAudio dialog

class CDialogCaptureAudio : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogCaptureAudio)

public:
	CDialogCaptureAudio(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogCaptureAudio();

// Dialog Data
	enum { IDD = IDD_DIALOG_CAPTURE_AUDIO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_restart_required;
public:
	CComboBox m_audio_capture_device;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};

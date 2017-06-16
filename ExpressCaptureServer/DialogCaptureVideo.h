#pragma once
#include "afxwin.h"


// CDialogCaptureVideo dialog

class CDialogCaptureVideo : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogCaptureVideo)

public:
	CDialogCaptureVideo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogCaptureVideo();

// Dialog Data
	enum { IDD = IDD_DIALOG_CAPTURE_VIDEO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CComboBox m_video_capture_device;
	void CDialogCaptureVideo::UpdateFormatCombo(void);
public:
	virtual BOOL OnInitDialog();
private:
	CComboBox m_video_formats;
public:
	BOOL m_restart_required;
public:
	afx_msg void OnSetfocusComboVideoFormats();
	afx_msg void OnBnClickedOk();

private:
	CButton m_interlaced;
};

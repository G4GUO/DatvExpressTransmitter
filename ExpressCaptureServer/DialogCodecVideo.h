#pragma once
#include "afxwin.h"


// CDialogCodecVideo dialog

class CDialogCodecVideo : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogCodecVideo)

public:
	CDialogCodecVideo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogCodecVideo();

// Dialog Data
	enum { IDD = IDD_DIALOG_CODEC_VIDEO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CStatic m_video_bitrate_display;
private:
	CEdit m_video_gop;
	CEdit m_video_bframes;
	CComboBox m_performance;
	CEdit m_video_bitrate_twiddle;
public:
	afx_msg void OnClickedRadioH262();
	afx_msg void OnRadioH264();
	afx_msg void OnRadioH265();
private:
	CStatic m_performance_text;
public:
	BOOL m_restart_required;
};

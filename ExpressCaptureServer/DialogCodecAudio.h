#pragma once
#include "afxwin.h"


// CDialogCodecAudio dialog

class CDialogCodecAudio : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogCodecAudio)

public:
	CDialogCodecAudio(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogCodecAudio();

// Dialog Data
	enum { IDD = IDD_DIALOG_CODEC_AUDIO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CButton m_audio_codec;
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
private:
	CButton m_enable_audio;
	CButton m_audio_bitrate;
public:
	BOOL m_restart_required;
	afx_msg void OnBnClickedRadioBitrate33();
	afx_msg void OnBnClickedRadioBitrate65();
};

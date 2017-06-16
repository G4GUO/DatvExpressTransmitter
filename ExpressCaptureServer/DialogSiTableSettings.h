#pragma once
#include "afxwin.h"


// CDialogSiTableSettings dialog

class CDialogSiTableSettings : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogSiTableSettings)

public:
	CDialogSiTableSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogSiTableSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG_SI_TABLE_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CEdit m_video_pid;
	CEdit m_audio_pid;
	CEdit m_pcr_pid;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
private:
	CEdit m_pmt_pid;
	CEdit m_provider_name;
	CEdit m_service_name;
	CEdit m_event_title;
	CStatic m_event_duration;
	CEdit m_event_text;
	CEdit m_network_id;
	CEdit m_stream_id;
	CEdit m_program_nr;
	CEdit m_service_id;
};

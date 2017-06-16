
// ExpressCaptureServerDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CExpressCaptureServerDlg dialog
class CExpressCaptureServerDlg : public CDialogEx
{
// Construction
public:
	CExpressCaptureServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EXPRESSCAPTURESERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	BOOL m_fully_configured;
	CStatic m_ptt_status;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileExit();
	afx_msg void OnFileSave();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSaveAs();
	afx_msg void OnClickedButtonPtt();
	afx_msg void OnHelp();
	afx_msg void OnHelpAbout();
	afx_msg void OnCaptureVideo();
	afx_msg void OnCaptureAudio();
	afx_msg void OnCodecVideo();
	afx_msg void OnCodecAudio();
	afx_msg void OnSitablesSettings();
	afx_msg void OnClickedCheckTsLogging();
	CButton m_check_ts_logging;
	afx_msg void OnFileRestart();
	CProgressCtrl m_tx_queue;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClickedCheckCarrier();
private:
	BOOL m_display_refresh;
	int  m_last_txq;
	int  m_last_tx_level;
	double m_last_tx_freq;
	CButton m_carrier;
	CProgressCtrl m_tx_level;
	CStatic m_tx_freq;
	CStatic m_video_bitrate;
	CStatic m_audio_bitrate;
	CStatic m_tx_symbolrate;
	CStatic m_tx_fec;
	CStatic m_tx_bitrate;
	CStatic m_tx_mode;
	CStatic m_tx_constellation;

	void display_fixed_params(void);
	void display_adjustable_params(void);
	void DisplayConfigFilename();
	void RestartRequired(BOOL restart);

public:
	afx_msg void OnOptionsOutputformat();
	afx_msg void OnClickedCheckVideoIdent();
private:
	CButton m_video_ident;
	CStatic m_video_codec;
	CButton m_ptt_button;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	CStatic m_tx_level_text;
	CStatic m_tx_queue_text;
public:
	afx_msg void OnModulatorCommon();
	afx_msg void OnModulatorDvbS();
	afx_msg void OnModulatorDvbS2();

	afx_msg void OnReleasedcaptureSliderTxLevel(NMHDR *pNMHDR, LRESULT *pResult);
private:
	CSliderCtrl m_tx_level_slider;
public:
	afx_msg void OnCustomdrawSliderTxLevel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnModulatorDvbT();
	afx_msg void OnOptionsIqcalibration();
	afx_msg void OnOptionsNoisetool();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
};

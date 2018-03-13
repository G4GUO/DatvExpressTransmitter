
// ExpressCaptureServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "ExpressCaptureServerDlg.h"
#include "afxdialogex.h"
#include "DialogCaptureVideo.h"
#include "DialogCaptureAudio.h"
#include "DialogCodecVideo.h"
#include "DialogCodecAudio.h"
#include "DialogSiTableSettings.h"
#include "DialogExpressSettings.h"
#include "DialogDVBSSettings.h"
#include "DialogDVBS2Settings.h"
#include "DialogDVBTSettings.h"
#include "DialogOutputFormat.h"
#include "DialogConfigurationError.h"
#include "DialogIqCalibration.h"
#include "DialogNoiseTool.h"
#include "Dvb.h"
#include "DVB-T\dvb_t.h"
#include "hardware.h"
#include <WinSock.h>
#pragma warning(disable : 4996)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CExpressCaptureServerDlg dialog



CExpressCaptureServerDlg::CExpressCaptureServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CExpressCaptureServerDlg::IDD, pParent)
	
	, m_TsinPort(0)
	, m_TsInMode(FALSE)
	, m_TsInAddress(0)
	, m_sLocalNic(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CExpressCaptureServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PTT, m_ptt_status);
	DDX_Control(pDX, IDC_CHECK_TS_LOGGING, m_check_ts_logging);
	DDX_Control(pDX, IDC_PROGRESS_TXQUEUE, m_tx_queue);
	DDX_Control(pDX, IDC_CHECK_CARRIER, m_carrier);
	DDX_Control(pDX, IDC_PROGRESS_RF_LEVEL, m_tx_level);
	DDX_Control(pDX, IDC_STATIC_TX_FREQ, m_tx_freq);
	DDX_Control(pDX, IDC_STATIC_VIDEO_BITRATE, m_video_bitrate);
	DDX_Control(pDX, IDC_STATIC_AUDIO_BITRATE, m_audio_bitrate);
	DDX_Control(pDX, IDC_STATIC_TX_SYMBOLRATE, m_tx_symbolrate);
	DDX_Control(pDX, IDC_STATIC_TX_FEC, m_tx_fec);
	DDX_Control(pDX, IDC_STATIC_TX_BITRATE, m_tx_bitrate);
	DDX_Control(pDX, IDC_CHECK_VIDEO_IDENT, m_video_ident);
	DDX_Control(pDX, IDC_STATIC_VIDEO_CODEC, m_video_codec);
	DDX_Control(pDX, IDC_BUTTON_PTT, m_ptt_button);
	DDX_Control(pDX, IDC_STATIC_TX_LEVEL, m_tx_level_text);
	DDX_Control(pDX, IDC_STATIC_TX_QUEUE, m_tx_queue_text);
	DDX_Control(pDX, IDC_STATIC_TX_MODE, m_tx_mode);
	DDX_Control(pDX, IDC_STATIC_TX_CONSTELLATION, m_tx_constellation);
	DDX_Control(pDX, IDC_SLIDER_TX_LEVEL, m_tx_level_slider);
	DDX_Control(pDX, IDC_IPADDRESS_TSIN, m_IpTsin);

	DDX_Text(pDX, IDC_IPTSINPORT, m_TsinPort);
	DDX_Check(pDX, IDC_TSINMODE, m_TsInMode);
	DDX_IPAddress(pDX, IDC_IPADDRESS_TSIN, m_TsInAddress);
	DDX_Control(pDX, IDC_COMBO1, m_ListNic);
	DDX_CBString(pDX, IDC_COMBO1, m_sLocalNic);
}

BEGIN_MESSAGE_MAP(CExpressCaptureServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_EXIT, &CExpressCaptureServerDlg::OnFileExit)
	ON_COMMAND(ID_FILE_SAVE, &CExpressCaptureServerDlg::OnFileSave)
	ON_BN_CLICKED(IDC_BUTTON_PTT, &CExpressCaptureServerDlg::OnClickedButtonPtt)
	ON_COMMAND(ID_HELP_ABOUT, &CExpressCaptureServerDlg::OnHelpAbout)
	ON_COMMAND(ID_CAPTURE_VIDEO, &CExpressCaptureServerDlg::OnCaptureVideo)
	ON_COMMAND(ID_CAPTURE_AUDIO, &CExpressCaptureServerDlg::OnCaptureAudio)
	ON_COMMAND(ID_CODEC_VIDEO, &CExpressCaptureServerDlg::OnCodecVideo)
	ON_COMMAND(ID_CODEC_AUDIO, &CExpressCaptureServerDlg::OnCodecAudio)
	ON_COMMAND(ID_SITABLES_SETTINGS, &CExpressCaptureServerDlg::OnSitablesSettings)
	ON_BN_CLICKED(IDC_CHECK_TS_LOGGING, &CExpressCaptureServerDlg::OnClickedCheckTsLogging)
	ON_COMMAND(ID_FILE_RESTART, &CExpressCaptureServerDlg::OnFileRestart)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK_CARRIER, &CExpressCaptureServerDlg::OnClickedCheckCarrier)
	ON_COMMAND(ID_OPTIONS_OUTPUTFORMAT, &CExpressCaptureServerDlg::OnOptionsOutputformat)
	ON_BN_CLICKED(IDC_CHECK_VIDEO_IDENT, &CExpressCaptureServerDlg::OnClickedCheckVideoIdent)
	ON_COMMAND(ID_FILE_OPEN, &CExpressCaptureServerDlg::OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE_AS, &CExpressCaptureServerDlg::OnFileSaveAs)
	ON_WM_CTLCOLOR()
ON_COMMAND(ID_MODULATOR_COMMON, &CExpressCaptureServerDlg::OnModulatorCommon)
ON_COMMAND(ID_MODULATOR_DVB_S, &CExpressCaptureServerDlg::OnModulatorDvbS)
ON_COMMAND(ID_MODULATOR_DVB_S2, &CExpressCaptureServerDlg::OnModulatorDvbS2)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_TX_LEVEL, &CExpressCaptureServerDlg::OnReleasedcaptureSliderTxLevel)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TX_LEVEL, &CExpressCaptureServerDlg::OnCustomdrawSliderTxLevel)
ON_COMMAND(ID_MODULATOR_DVB_T, &CExpressCaptureServerDlg::OnModulatorDvbT)
ON_COMMAND(ID_OPTIONS_IQCALIBRATION, &CExpressCaptureServerDlg::OnOptionsIqcalibration)
ON_COMMAND(ID_OPTIONS_NOISETOOL, &CExpressCaptureServerDlg::OnOptionsNoisetool)
ON_WM_MENUSELECT()
ON_BN_CLICKED(IDC_TSINMODE, &CExpressCaptureServerDlg::OnBnClickedTsinmode)
ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_TSIN, &CExpressCaptureServerDlg::OnIpnFieldchangedIpaddressTsin)
ON_CBN_SELCHANGE(IDC_COMBO1, &CExpressCaptureServerDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CExpressCaptureServerDlg message handlers

BOOL CExpressCaptureServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	if (system_start(theApp.m_sConfigFile) == S_OK) {
		m_ptt_button.EnableWindow(TRUE);
		m_fully_configured = TRUE;
	}
	else {
		CDialogConfigurationError dlg;
		dlg.DoModal();
		m_ptt_button.EnableWindow(FALSE);
		m_fully_configured = FALSE;
	}

	if (m_fully_configured == TRUE) {
		SetTimer(1, 200, NULL);
	}
	m_display_refresh = TRUE;
	m_last_txq = -1;

	m_tx_queue.SetRange(0, 100);
	m_tx_queue.SetPos(0);
	m_tx_level.SetRange(0, 47);
	m_tx_level.SetPos(0);
	m_video_ident.SetCheck(get_video_ident());
	display_fixed_params();
	DisplayConfigFilename();
	m_tx_level_slider.SetRange(0, 47);
	m_tx_level_slider.SetPos(47-get_current_tx_level());
	m_carrier.SetCheck(FALSE);
	//m_IpTsin.SetAddress(get_TSIn_ip_addr());
	m_TsInAddress = get_TSIn_ip_addr();
	m_TsinPort = get_TSIn_port();
	UpdateData(false);

	// List NIC
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		char name[255];
		PHOSTENT hostinfo;
		wVersionRequested = MAKEWORD(1, 1);
		char *ip;

		if (WSAStartup(wVersionRequested, &wsaData) == 0)
			if (gethostname(name, sizeof(name)) == 0)
			{
				printf("Host name: %s\n", name);

				if ((hostinfo = gethostbyname(name)) != NULL)
				{
					int nCount = 0;
					while (hostinfo->h_addr_list[nCount])
					{
						ip = inet_ntoa(*(
							struct in_addr *)hostinfo->h_addr_list[nCount]);
						m_ListNic.AddString(ip);
						printf("IP #%d: %s\n", ++nCount, ip);
					}
				}
			}
		m_ListNic.SetCurSel(0);
		UpdateData(true);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CExpressCaptureServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CExpressCaptureServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CExpressCaptureServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//
// Added Stuff
//
void CExpressCaptureServerDlg::display_fixed_params(void) {
	CString text;
	double param;
	if (m_display_refresh == TRUE) {
		switch (get_txmode()) {
		case M_DVBS:
			text.Format("Tx Mode\t DVB-S");
			break;
		case M_DVBT:
			if(get_dvbt_mode() == TM_2K) text.Format("Tx Mode\t DVB-T 2K");
			if(get_dvbt_mode() == TM_8K) text.Format("Tx Mode\t DVB-T 8K");
			break;
		case M_DVBS2:
			text.Format("Tx Mode\t DVB-S2");
			if (get_dvbs2_rolloff() == RO_20) text += " 0.20";
			if (get_dvbs2_rolloff() == RO_25) text += " 0.25";
			if (get_dvbs2_rolloff() == RO_35) text += " 0.35";
			if (get_dvbs2_pilots()) text += " Pilots";
			break;
		}
		m_tx_mode.SetWindowTextA(text);

		text.Format("Tx Constellation\t");

		if (get_txmode() == M_DVBS2) {
			switch (get_dvbs2_constellation()) {
			case QPSK:
				text += " QPSK";
				break;
			case PSK8:
				text += " 8PSK";
				break;
			case APSK16:
				text += " 16APSK";
				break;
			case APSK32:
				text += " 32APSK";
				break;
			}
		}
		
		if(get_txmode() == M_DVBS )	text += " QPSK";
		
		if (get_txmode() == M_DVBT) {
			switch (get_dvbt_constellation()) {
			case CO_QPSK:
				text += " QPSK";
				break;
			case CO_16QAM:
				text += " 16QAM";
				break;
			case CO_64QAM:
				text += " 64QAM";
				break;
			}
		}

		m_tx_constellation.SetWindowTextA(text);

		//
		if (get_txmode() == M_DVBT) {
			int br;
			switch (get_dvbt_channel()) {
			case CH_8M: 
				br = 8;
				break;
			case CH_7M: 
				br = 7;
				break;
			case CH_6M: 
				br = 6;
				break;
			case CH_4M: 
				br = 4;
				break;
			case CH_3M: 
				br = 3;
				break;
			case CH_2M: 
				br = 2;
				break;
			case CH_1M: 
				br = 1;
				break;
			default:
				br = 4;
				break;
			}
			text.Format("Tx Channel\t %d MHz", br);
		}
		else
		{
			param = get_current_tx_symbol_rate();
			if (param >= 1000000) {
				param = param / 1000000;
				text.Format("Tx Symbolrate\t %.3f MSps", param);
			}
			else {
				if (param >= 1000) {
					param = param / 1000;
					text.Format("Tx Symbolrate\t %.3f KSps", param);
				}
				else {
					text.Format("Tx Symbolrate\t %.3f Sps", param);
				}
			}
		}
		m_tx_symbolrate.SetWindowTextA(text);

		text.Format("Tx FEC\t\t %s", get_current_fec_string());
		m_tx_fec.SetWindowTextA(text);

		//
		param = get_tx_net_bitrate();
		if (param >= 1000000) {
			param = param / 1000000;
			text.Format("Tx Bitrate\t %.3f MBps", param);
		}
		else {
			if (param >= 1000) {
				param = param / 1000;
				text.Format("Tx Bitrate\t %.3f KBps", param);
			}
			else {
				text.Format("Tx Bitrate\t %.3f Bps", param);
			}
		}
		m_tx_bitrate.SetWindowTextA(text);

		//
		param = get_video_bitrate();
		if (param >= 1000000) {
			param = param / 1000000;
			text.Format("Video Bitrate\t %.3f MBps", param);
		}
		else {
			if (param >= 1000) {
				param = param / 1000;
				text.Format("Video Bitrate\t %.3f KBps", param);
			}
			else {
				text.Format("Video Bitrate\t %.3f Bps", param);
			}
		}
		m_video_bitrate.SetWindowTextA(text);

		text.Format("Video Codec\t %s", get_video_codec_name());
		m_video_codec.SetWindowTextA(text);

		//
		if (get_audio_status() == TRUE) {
			param = get_audio_bitrate();
			if (param >= 1000000) {
				param = param / 1000000;
				text.Format("Audio Bitrate\t %.3f MBps", param);
			}
			else {
				if (param >= 1000) {
					param = param / 1000;
					text.Format("Audio Bitrate\t %.3f KBps", param);
				}
				else {
					text.Format("Audio Bitrate\t %.3f Bps", param);
				}
			}
		}
		else {
			text.Format("Audio Bitrate\t %d Bps", 0);
		}
		m_audio_bitrate.SetWindowTextA(text);
		m_display_refresh = FALSE;
	}
	//m_sdr_ip.SetAddress
	
}

void CExpressCaptureServerDlg::display_adjustable_params(void) {
	int val;
	CString text;

	val = get_tx_buf_percent();
	if (val != m_last_txq) {
		text.Format("Tx Queue %d%c", val, '%');
		m_tx_queue_text.SetWindowTextA(text);
		m_tx_queue.SetPos(val);
		m_last_txq = val;
	}
	val = get_current_tx_level();
	if (val != m_last_tx_level) {
		text.Format("Tx Level %d", val);
		m_tx_level_text.SetWindowTextA(text);
		m_tx_level.SetPos(val);
		m_last_tx_level = val;
	}
	double param = (double)get_current_tx_frequency();
	if (param != m_last_tx_freq) {
		if (param >= 1000000000) {
			param = param / 1000000000;
			text.Format("Tx Frequency\t%.4f GHz", param);
		}
		else {
			param = param / 1000000;
			text.Format("Tx Frequency\t%.4f MHz", param);
		}
		m_tx_freq.SetWindowTextA(text);
		m_last_tx_freq = param;
	}
	
}



void CExpressCaptureServerDlg::OnFileExit()
{
	// TODO: Add your command handler code here
	OnCancel();
}
void CExpressCaptureServerDlg::OnFileSave()
{
	// TODO: Add your command handler code here
	SaveConfigToDisk(theApp.m_sConfigFile);
}
void CExpressCaptureServerDlg::DisplayConfigFilename()
{
	CString text;
	text = "DATV-Express  - ";
	text += get_config_filename();
	text.TrimRight(".cfg");
	SetWindowText(text);
}
void CExpressCaptureServerDlg::OnFileOpen()
{
	// TODO: Add your command handler code here
	char szFilter[] = "Config Files (*.cfg)|*.cfg";
	CFileDialog dlg(TRUE,"cfg","*.cfg",OFN_NOCHANGEDIR,szFilter);
	TCHAR szFolder[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szFolder);

	if (dlg.DoModal() == IDOK) {
		theApp.m_sConfigFile = dlg.GetFileName();
		LoadConfigFromDisk(theApp.m_sConfigFile);
		if(m_fully_configured == TRUE)	OnFileRestart();
		DisplayConfigFilename();
		m_IpTsin.SetAddress(get_TSIn_ip_addr());

		m_TsinPort = (int)get_TSIn_port();
	}
}

void CExpressCaptureServerDlg::OnFileSaveAs()
{
	// TODO: Add your command handler code here
	char szFilter[] = "Config Files (*.cfg)|*.cfg";
	CFileDialog dlg(FALSE,"cfg","*.cfg", OFN_NOCHANGEDIR,szFilter);
	TCHAR szFolder[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szFolder);
	dlg.m_ofn.lpstrInitialDir = szFolder;

	if (dlg.DoModal() == IDOK) {
		theApp.m_sConfigFile = dlg.GetFileName();
		cmd_set_config_filename(theApp.m_sConfigFile);
		SaveConfigToDisk(theApp.m_sConfigFile);
		DisplayConfigFilename();
	}
}


void CExpressCaptureServerDlg::OnClickedButtonPtt()
{
	UpdateData(true);
	cmd_set_tx_level(47 - m_tx_level_slider.GetPos());
	DWORD IPLocal=inet_addr(m_sLocalNic);
	
	cmd_set_TsIn_ip_addr(m_TsInAddress, m_TsinPort, IPLocal);
	if (!m_TsInMode)
	{
		if (get_tx_status() == TRUE) {
			cmd_standby();
			m_ptt_status.SetWindowTextA("STANDBY");
		}
		else {
			cmd_transmit();
			m_ptt_status.SetWindowTextA("TRANSMIT");
		}
	}
	else
	{
		if (get_tx_status() == TRUE) {
			cmd_ip_standby();
			m_ptt_status.SetWindowTextA("STANDBY");
		}
		else {
			cmd_ip_transmit(m_TsInAddress,m_TsinPort);
			m_ptt_status.SetWindowTextA("TRANSMIT");
		}
	}

}

void CExpressCaptureServerDlg::OnHelpAbout()
{
	// TODO: Add your command handler code here
	CAboutDlg dlg;
	dlg.DoModal();
}

void CExpressCaptureServerDlg::OnCaptureVideo()
{
	// TODO: Add your command handler code here
	CDialogCaptureVideo dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}

void CExpressCaptureServerDlg::OnCaptureAudio()
{
	// TODO: Add your command handler code here
	CDialogCaptureAudio dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}

void CExpressCaptureServerDlg::OnCodecVideo()
{
	// TODO: Add your command handler code here
	CDialogCodecVideo dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}

void CExpressCaptureServerDlg::OnCodecAudio()
{
	// TODO: Add your command handler code here
	CDialogCodecAudio dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}

void CExpressCaptureServerDlg::OnSitablesSettings()
{
	// TODO: Add your command handler code here
	CDialogSiTableSettings dlg;
	dlg.DoModal();
}

void CExpressCaptureServerDlg::OnOptionsOutputformat()
{
	// TODO: Add your command handler code here
	CDialogOutputFormat dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}

void CExpressCaptureServerDlg::OnClickedCheckTsLogging()
{
	// TODO: Add your control notification handler code here
	if(m_check_ts_logging.GetCheck())
		start_log();
	else
		stop_log();
}


void CExpressCaptureServerDlg::OnFileRestart()
{
	// Restart the system so new parameters come into operation
	m_display_refresh = TRUE;
	if (m_fully_configured == FALSE) {
		CString msg;
		msg.Format("Program must be restarted");
		::MessageBox(NULL, msg, "Fatal", 0);
		exit(0);
		return;
	}
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	cmd_standby();
	m_ptt_status.SetWindowTextA("STANDBY");
	system_stop();
	tx_buf_empty();
	hw_deinit();

	if (system_restart() == S_OK) {
		m_fully_configured = TRUE;
		m_ptt_button.EnableWindow(TRUE);
		display_fixed_params();
	} else {
		m_fully_configured = FALSE;
		m_ptt_button.EnableWindow(FALSE);
		CDialogConfigurationError dlg;
		dlg.DoModal();
	}
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	// TODO: Add your command handler code here
}



void CExpressCaptureServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	display_adjustable_params();
	display_fixed_params();
	CDialogEx::OnTimer(nIDEvent);
}


void CExpressCaptureServerDlg::OnClickedCheckCarrier()
{
	// TODO: Add your control notification handler code here
	cmd_set_carrier(m_carrier.GetCheck());
}

void CExpressCaptureServerDlg::OnClickedCheckVideoIdent()
{
	// TODO: Add your control notification handler code here
	codec_video_ident(m_video_ident.GetCheck());
}


HBRUSH CExpressCaptureServerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_PTT)
	{
		if (get_tx_status() == TRUE) {
			pDC->SetBkColor(RGB(200, 0, 0)); // Red color for background
			pDC->SetTextColor(RGB(255, 255, 255)); // RED color for text
		}
	}
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void CExpressCaptureServerDlg::OnModulatorCommon()
{
	// TODO: Add your command handler code here
	CDialogExpressSettings dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}
void CExpressCaptureServerDlg::RestartRequired(BOOL restart) {
	if (restart == TRUE) {
		// We must restart the program, probably as the mode has changed
		OnFileSave();
		OnFileRestart();
		m_carrier.SetCheck(FALSE);
		m_ptt_status.SetWindowTextA("STANDBY");
	}
	else {
		m_display_refresh = TRUE;
	}
}

void CExpressCaptureServerDlg::OnModulatorDvbS()
{
	// TODO: Add your command handler code here
	CDialogDVBSSettings dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}

void CExpressCaptureServerDlg::OnModulatorDvbS2()
{
	// TODO: Add your command handler code here
	CDialogDVBS2Settings dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}

void CExpressCaptureServerDlg::OnModulatorDvbT()
{
	// TODO: Add your command handler code here
	CDialogDVBTSettings dlg;
	dlg.DoModal();
	RestartRequired(dlg.m_restart_required);
}

void CExpressCaptureServerDlg::OnReleasedcaptureSliderTxLevel(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	cmd_set_tx_level(47 - m_tx_level_slider.GetPos());
	//display_adjustable_params();

	*pResult = 0;
}

void CExpressCaptureServerDlg::OnCustomdrawSliderTxLevel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	//cmd_set_tx_level(47 - m_tx_level_slider.GetPos());
	display_adjustable_params();
	*pResult = 0;
}



void CExpressCaptureServerDlg::OnOptionsIqcalibration()
{
	// TODO: Add your command handler code here
	CDialogIqCalibration dlg;
	if(dlg.DoModal()==IDOK)	SaveConfigToDisk(theApp.m_sConfigFile);

}


void CExpressCaptureServerDlg::OnOptionsNoisetool()
{
	// TODO: Add your command handler code here
	CDialogNoiseTool dlg;
	dlg.DoModal();
}


void CExpressCaptureServerDlg::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	CDialogEx::OnMenuSelect(nItemID, nFlags, hSysMenu);

	// TODO: Add your message handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(5);
	switch (get_txmode()) {
	case M_DVBS:
		submenu->EnableMenuItem(ID_MODULATOR_DVB_S, MF_BYCOMMAND | MF_ENABLED );
		submenu->EnableMenuItem(ID_MODULATOR_DVB_S2, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		submenu->EnableMenuItem(ID_MODULATOR_DVB_T, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		break;
	case M_DVBT:
		submenu->EnableMenuItem(ID_MODULATOR_DVB_S, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		submenu->EnableMenuItem(ID_MODULATOR_DVB_S2, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		submenu->EnableMenuItem(ID_MODULATOR_DVB_T, MF_BYCOMMAND | MF_ENABLED );
		break;
	case M_DVBS2:
		submenu->EnableMenuItem(ID_MODULATOR_DVB_S, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		submenu->EnableMenuItem(ID_MODULATOR_DVB_S2, MF_BYCOMMAND | MF_ENABLED );
		submenu->EnableMenuItem(ID_MODULATOR_DVB_T, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		break;
	}
}



void CExpressCaptureServerDlg::OnBnClickedTsinmode()
{
	
}


void CExpressCaptureServerDlg::OnIpnFieldchangedIpaddressTsin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
	
	*pResult = 0;
}


void CExpressCaptureServerDlg::OnCbnSelchangeCombo1()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
}

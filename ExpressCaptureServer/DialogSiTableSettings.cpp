// DialogSiTableSettings.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogSiTableSettings.h"
#include "afxdialogex.h"
#include "Dvb.h"

// CDialogSiTableSettings dialog

IMPLEMENT_DYNAMIC(CDialogSiTableSettings, CDialogEx)

CDialogSiTableSettings::CDialogSiTableSettings(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogSiTableSettings::IDD, pParent)
{

}

CDialogSiTableSettings::~CDialogSiTableSettings()
{
}

void CDialogSiTableSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_VIDEO_PID, m_video_pid);
	DDX_Control(pDX, IDC_EDIT_AUDIO_PID, m_audio_pid);
	DDX_Control(pDX, IDC_EDIT_PCR_PID, m_pcr_pid);
	DDX_Control(pDX, IDC_EDIT_PMT_PID, m_pmt_pid);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_NAME, m_provider_name);
	DDX_Control(pDX, IDC_EDIT_SERVICE_NAME, m_service_name);
	DDX_Control(pDX, IDC_EDIT_EVENT_TITLE, m_event_title);
	DDX_Control(pDX, IDC_EDIT_EVENT_DURATION, m_event_duration);
	DDX_Control(pDX, IDC_EDIT_EVENT_TEXT, m_event_text);
	DDX_Control(pDX, IDC_EDIT_NETWORK_ID, m_network_id);
	DDX_Control(pDX, IDC_EDIT_STREAM_ID, m_stream_id);
	DDX_Control(pDX, IDC_EDIT_PROGRAM_NUMBER, m_program_nr);
	DDX_Control(pDX, IDC_EDIT_SERVICE_ID, m_service_id);
}


BEGIN_MESSAGE_MAP(CDialogSiTableSettings, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDialogSiTableSettings::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogSiTableSettings message handlers


BOOL CDialogSiTableSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CString text;
	text.Format("%d",get_video_pid());
	m_video_pid.SetWindowTextA(text);
	text.Format("%d",get_audio_pid());
	m_audio_pid.SetWindowTextA(text);
	text.Format("%d",get_pcr_pid());
	m_pcr_pid.SetWindowTextA(text);
	text.Format("%d",get_pmt_pid());
	m_pmt_pid.SetWindowTextA(text);
	text.Format("%s",get_provider_name());
	m_provider_name.SetWindowTextA(text);
	text.Format("%s",get_service_name());
	m_service_name.SetWindowTextA(text);
	text.Format("%s",get_event_title());
	m_event_title.SetWindowTextA(text);
	text.Format("%s",get_event_text());
	m_event_text.SetWindowTextA(text);
	text.Format("%d",get_event_duration());
	m_event_duration.SetWindowTextA(text);
	text.Format("%d",get_network_id());
	m_network_id.SetWindowTextA(text);
	text.Format("%d",get_stream_id());
	m_stream_id.SetWindowTextA(text);
	text.Format("%d",get_service_id());
	m_service_id.SetWindowTextA(text);
	text.Format("%d",get_program_nr());
	m_program_nr.SetWindowTextA(text);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDialogSiTableSettings::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString text;
	m_video_pid.GetWindowTextA(text);
	cmd_set_video_pid(text);
	m_audio_pid.GetWindowTextA(text);
	cmd_set_audio_pid(text);
	m_pcr_pid.GetWindowTextA(text);
	cmd_set_pcr_pid(text);
	m_pmt_pid.GetWindowTextA(text);
	cmd_set_pmt_pid(text);
	m_provider_name.GetWindowTextA(text);
	cmd_set_service_provider_name(text);
	m_service_name.GetWindowTextA(text);
	cmd_set_service_name(text);
	m_event_title.GetWindowTextA(text);
	cmd_set_event_title(text);
	m_event_duration.GetWindowTextA(text);
	cmd_set_event_duration(text);
	m_event_text.GetWindowTextA(text);
	cmd_set_event_text(text);

	m_network_id.GetWindowTextA(text);
	cmd_set_network_id(text);
	m_stream_id.GetWindowTextA(text);
	cmd_set_stream_id(text);
	m_service_id.GetWindowTextA(text);
	cmd_set_service_id(text);
	m_program_nr.GetWindowTextA(text);
	cmd_set_program_nr(text);
	configure_si();
	overlay_init(get_provider_name());

	CDialogEx::OnOK();
}

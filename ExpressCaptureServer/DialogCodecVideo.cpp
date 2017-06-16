// DialogCodecVideo.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogCodecVideo.h"
#include "afxdialogex.h"
#include "Dvb.h"

// CDialogCodecVideo dialog

IMPLEMENT_DYNAMIC(CDialogCodecVideo, CDialogEx)

CDialogCodecVideo::CDialogCodecVideo(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogCodecVideo::IDD, pParent)
{

}

CDialogCodecVideo::~CDialogCodecVideo()
{
}

void CDialogCodecVideo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIDEO_BITRATE, m_video_bitrate_display);
	DDX_Control(pDX, IDC_EDIT_VIDEO_GOP, m_video_gop);
	DDX_Control(pDX, IDC_EDIT_VIDEO_B_FRAMES, m_video_bframes);
	DDX_Control(pDX, IDC_COMBO_CODEC_PERFORMANCE, m_performance);
	DDX_Control(pDX, IDC_EDIT_VIDEO_BITRATE_TWIDDLE, m_video_bitrate_twiddle);
	DDX_Control(pDX, IDC_STATIC_PERFORMANCE, m_performance_text);
}


BEGIN_MESSAGE_MAP(CDialogCodecVideo, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDialogCodecVideo::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_H262, &CDialogCodecVideo::OnClickedRadioH262)
	ON_COMMAND(IDC_RADIO_H264, &CDialogCodecVideo::OnRadioH264)
	ON_COMMAND(IDC_RADIO_H265, &CDialogCodecVideo::OnRadioH265)
END_MESSAGE_MAP()


// CDialogCodecVideo message handlers




BOOL CDialogCodecVideo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	switch(get_video_codec()){
	case AV_CODEC_ID_MPEG2VIDEO:
		CheckRadioButton(IDC_RADIO_H262,IDC_RADIO_H265,IDC_RADIO_H262);
		m_performance.ShowWindow(SW_HIDE);
		m_performance_text.ShowWindow(SW_HIDE);
		break;
	case AV_CODEC_ID_H264:
		CheckRadioButton(IDC_RADIO_H262,IDC_RADIO_H265,IDC_RADIO_H264);
		m_performance.ShowWindow(SW_SHOW);
		m_performance_text.ShowWindow(SW_SHOW);
		break;
	case AV_CODEC_ID_HEVC:
		CheckRadioButton(IDC_RADIO_H262,IDC_RADIO_H265,IDC_RADIO_H265);
		m_performance.ShowWindow(SW_SHOW);
		m_performance_text.ShowWindow(SW_SHOW);
		break;
	}
	CString text;
	text.Format("Video Bitrate %d",get_video_bitrate());
	m_video_bitrate_display.SetWindowText(text);
	text.Format("%d",get_video_gop());
	m_video_gop.SetWindowText(text);
	text.Format("%d",get_video_b_frames());
	m_video_bframes.SetWindowText(text);
	// Add performance settings
	m_performance.AddString("ultrafast");
	m_performance.AddString("superfast");
	m_performance.AddString("veryfast");
	m_performance.AddString("fast");
	m_performance.AddString("faster");
	m_performance.AddString("medium");
	m_performance.AddString("slow");
	//m_performance.AddString("slower");
	//m_performance.AddString("veryslow");
	//m_performance.AddString("placebo");
	m_performance.SelectString(0,get_video_codec_performance());
	
	text.Format("%f", get_vbr_twiddle());
	m_video_bitrate_twiddle.SetWindowTextA(text);
	m_restart_required = FALSE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void CDialogCodecVideo::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	int id = GetCheckedRadioButton(IDC_RADIO_H262,IDC_RADIO_H265);
    if( id == IDC_RADIO_H262 ) cmd_set_video_codec("h262"); 
    if( id == IDC_RADIO_H264 ) cmd_set_video_codec("h264"); 
    if( id == IDC_RADIO_H265 ) cmd_set_video_codec("h265"); 
	CString text;
	m_video_gop.GetWindowText(text);
	cmd_set_video_gop(text);
	m_video_bframes.GetWindowText(text);
	cmd_set_video_b_frames(text);
	m_performance.GetWindowText(text);
	cmd_set_video_codec_performance(text);
	m_video_bitrate_twiddle.GetWindowText(text);
	cmd_set_vbr_twiddle(text);
	m_restart_required = TRUE;
	CDialogEx::OnOK();
}


void CDialogCodecVideo::OnClickedRadioH262()
{
	// TODO: Add your control notification handler code here
	m_performance.ShowWindow(SW_HIDE);
	m_performance_text.ShowWindow(SW_HIDE);
}


void CDialogCodecVideo::OnRadioH264()
{
	// TODO: Add your command handler code here
	m_performance.ShowWindow(SW_SHOW);
	m_performance_text.ShowWindow(SW_SHOW);
}


void CDialogCodecVideo::OnRadioH265()
{
	// TODO: Add your command handler code here
	m_performance.ShowWindow(SW_SHOW);
	m_performance_text.ShowWindow(SW_SHOW);
}

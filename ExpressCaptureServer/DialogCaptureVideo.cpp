// DialogCaptureVideo.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogCaptureVideo.h"
#include "afxdialogex.h"
#include "Dvb.h"
// CDialogCaptureVideo dialog

IMPLEMENT_DYNAMIC(CDialogCaptureVideo, CDialogEx)

CDialogCaptureVideo::CDialogCaptureVideo(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogCaptureVideo::IDD, pParent)
{

}

CDialogCaptureVideo::~CDialogCaptureVideo()
{
}

void CDialogCaptureVideo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_VIDEO_CAPTURE_DEVICE, m_video_capture_device);
	DDX_Control(pDX, IDC_COMBO_VIDEO_FORMATS, m_video_formats);
	DDX_Control(pDX, IDC_CHECK_INTERLACED, m_interlaced);
}


BEGIN_MESSAGE_MAP(CDialogCaptureVideo, CDialogEx)
	ON_CBN_SETFOCUS(IDC_COMBO_VIDEO_FORMATS, &CDialogCaptureVideo::OnSetfocusComboVideoFormats)
	ON_BN_CLICKED(IDOK, &CDialogCaptureVideo::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogCaptureVideo message handlers


BOOL CDialogCaptureVideo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CString items[200];
	int n = GetVideoInputCaptureList( items, 200 );
	m_restart_required = FALSE;

	for( int i = 0; i < n; i++){
		m_video_capture_device.AddString(items[i]);
	}
	m_video_capture_device.SelectString(-1,get_video_capture_device());
	UpdateFormatCombo();
		
	m_interlaced.SetCheck(get_cap_interlaced());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CDialogCaptureVideo::UpdateFormatCombo(void)
{
	CString device;
	m_video_capture_device.GetWindowText(device);
	CString items[100];
	m_video_formats.ResetContent();
	int n = GetVideoInputCaptureFormatList(&device, items, 100);
	for( int i = 0; i < n; i++){
		m_video_formats.AddString(items[i]);
	}
	m_video_formats.SelectString(-1,get_video_capture_format());
}

void CDialogCaptureVideo::OnSetfocusComboVideoFormats()
{
	// TODO: Add your control notification handler code here
	UpdateFormatCombo();
}


void CDialogCaptureVideo::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString device;
	m_video_capture_device.GetWindowText(device);
    cmd_set_video_capture_device(device);
	CString format;
	m_video_formats.GetWindowText(format);
	cmd_set_video_capture_format(format);
	if (m_interlaced.GetCheck())
		cmd_set_cap_interlaced("yes");
	else
		cmd_set_cap_interlaced("no");
	m_restart_required = TRUE;
	CDialogEx::OnOK();
}

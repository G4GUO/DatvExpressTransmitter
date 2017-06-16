// DialogCaptureAudio.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogCaptureAudio.h"
#include "afxdialogex.h"
#include "Dvb.h"


// CDialogCaptureAudio dialog

IMPLEMENT_DYNAMIC(CDialogCaptureAudio, CDialogEx)

CDialogCaptureAudio::CDialogCaptureAudio(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogCaptureAudio::IDD, pParent)
{

}

CDialogCaptureAudio::~CDialogCaptureAudio()
{
}

void CDialogCaptureAudio::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_AUDIO_CAPURE_DEVICE, m_audio_capture_device);
}


BEGIN_MESSAGE_MAP(CDialogCaptureAudio, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDialogCaptureAudio::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogCaptureAudio message handlers


BOOL CDialogCaptureAudio::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CString items[100];
	int n = GetAudioInputCaptureList( items, 100 );
	m_restart_required = FALSE;

	for( int i = 0; i < n; i++){
		m_audio_capture_device.AddString(items[i]);
	}
	m_audio_capture_device.AddString(S_USE_VIDEO_DEVICE);
	m_audio_capture_device.SelectString(-1,get_audio_capture_device());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDialogCaptureAudio::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString device;
	m_audio_capture_device.GetWindowText(device);
    cmd_set_audio_capture_device(device);
	m_restart_required = TRUE;
	CDialogEx::OnOK();
}

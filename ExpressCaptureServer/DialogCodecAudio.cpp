// DialogCodecAudio.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogCodecAudio.h"
#include "afxdialogex.h"
#include "Dvb.h"

// CDialogCodecAudio dialog

IMPLEMENT_DYNAMIC(CDialogCodecAudio, CDialogEx)

CDialogCodecAudio::CDialogCodecAudio(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogCodecAudio::IDD, pParent)
{

}

CDialogCodecAudio::~CDialogCodecAudio()
{
}

void CDialogCodecAudio::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_MP1_L2, m_audio_codec);
	DDX_Control(pDX, IDC_CHECK_ENABLE_AUDIO, m_enable_audio);
	DDX_Control(pDX, IDC_RADIO_BITRATE_32, m_audio_bitrate);
}


BEGIN_MESSAGE_MAP(CDialogCodecAudio, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDialogCodecAudio::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogCodecAudio message handlers


void CDialogCodecAudio::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	int id = GetCheckedRadioButton(IDC_RADIO_MP1_L2,IDC_RADIO_AAC);
    if( id == IDC_RADIO_MP1_L2 ) cmd_set_audio_codec("mp1"); 
    if( id == IDC_RADIO_AAC )    cmd_set_audio_codec("aac"); 

	id = GetCheckedRadioButton(IDC_RADIO_BITRATE_32, IDC_RADIO_BITRATE_192);
	if (id == IDC_RADIO_BITRATE_32)  cmd_set_audio_codec_bitrate("32000");
	if (id == IDC_RADIO_BITRATE_64)  cmd_set_audio_codec_bitrate("64000");
	if (id == IDC_RADIO_BITRATE_192) cmd_set_audio_codec_bitrate("192000");

	if(m_enable_audio.GetCheck() == TRUE)
		cmd_set_audio_status("true");
	else
		cmd_set_audio_status("false");
	m_restart_required = TRUE;
	CDialogEx::OnOK();
}


BOOL CDialogCodecAudio::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	switch(get_audio_codec()){
	case AV_CODEC_ID_MP2:
		CheckRadioButton(IDC_RADIO_MP1_L2,IDC_RADIO_AAC,IDC_RADIO_MP1_L2);
		break;
	case AV_CODEC_ID_AAC:
		CheckRadioButton(IDC_RADIO_MP1_L2,IDC_RADIO_AAC,IDC_RADIO_AAC);
		break;
	}
	switch (get_audio_codec_bitrate()) {
		case 32000:
			CheckRadioButton(IDC_RADIO_BITRATE_32, IDC_RADIO_BITRATE_192, IDC_RADIO_BITRATE_32);
			break;
		case 64000:
			CheckRadioButton(IDC_RADIO_BITRATE_32, IDC_RADIO_BITRATE_192, IDC_RADIO_BITRATE_64);
			break;
		case 192000:
			CheckRadioButton(IDC_RADIO_BITRATE_32, IDC_RADIO_BITRATE_192, IDC_RADIO_BITRATE_192);
			break;
	}
	m_restart_required = FALSE;
	m_enable_audio.SetCheck(get_audio_status());
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// DialogOutputFormat.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogOutputFormat.h"
#include "afxdialogex.h"
#include "Dvb.h"

// CDialogOutputFormat dialog

IMPLEMENT_DYNAMIC(CDialogOutputFormat, CDialogEx)

CDialogOutputFormat::CDialogOutputFormat(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogOutputFormat::IDD, pParent)
{

}

CDialogOutputFormat::~CDialogOutputFormat()
{
}

void CDialogOutputFormat::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_ONAIR_FORMAT, m_onair_video_format);
	DDX_Control(pDX, IDC_RADIO_ASPECT_1_1, m_aspect_ratio);
}


BEGIN_MESSAGE_MAP(CDialogOutputFormat, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDialogOutputFormat::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogOutputFormat message handlers

BOOL CDialogOutputFormat::OnInitDialog()
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
	m_onair_video_format.AddString("1920 1080 30");
	m_onair_video_format.AddString("720 480 30");
	m_onair_video_format.AddString("1920 1080 25");
	m_onair_video_format.AddString("720 576 25");

	CString fmts[50];

	int n = LoadOnAirFormatsFromDisk(50, fmts);
	for (int i = 0; i < n; i++ ) m_onair_video_format.AddString(fmts[i]);
	m_onair_video_format.AddString("Input Format");
	m_onair_video_format.SelectString(-1,get_onair_video_format());

	int ratio = get_video_aspect_ratio();
	if (ratio == 11) 	CheckRadioButton(IDC_RADIO_ASPECT_1_1, IDC_RADIO_ASPECT_16_9, IDC_RADIO_ASPECT_1_1);
	if (ratio == 43)		CheckRadioButton(IDC_RADIO_ASPECT_1_1, IDC_RADIO_ASPECT_16_9, IDC_RADIO_ASPECT_4_3);
	if (ratio == 169)	CheckRadioButton(IDC_RADIO_ASPECT_1_1, IDC_RADIO_ASPECT_16_9, IDC_RADIO_ASPECT_16_9);
	m_restart_required = FALSE;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDialogOutputFormat::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString text;
	m_onair_video_format.GetWindowText(text);	
	cmd_set_onair_video_format(text);
	int id = GetCheckedRadioButton(IDC_RADIO_ASPECT_1_1, IDC_RADIO_ASPECT_16_9);
	if (id == IDC_RADIO_ASPECT_1_1) cmd_set_video_aspect_ratio("1:1");
	if (id == IDC_RADIO_ASPECT_4_3) cmd_set_video_aspect_ratio("4:3");
	if (id == IDC_RADIO_ASPECT_16_9) cmd_set_video_aspect_ratio("16:9");
	m_restart_required = TRUE;
	CDialogEx::OnOK();
}

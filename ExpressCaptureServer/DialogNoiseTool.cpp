// DialogNoiseTool.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogNoiseTool.h"
#include "afxdialogex.h"

#include "noise.h"

// CDialogNoiseTool dialog

IMPLEMENT_DYNAMIC(CDialogNoiseTool, CDialogEx)

CDialogNoiseTool::CDialogNoiseTool(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_NOISE_TOOL, pParent)
{

}

CDialogNoiseTool::~CDialogNoiseTool()
{
}

void CDialogNoiseTool::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SN_RATIO, m_sn_ratio);
	DDX_Control(pDX, IDC_CHECK_ENABLE_NOISE, m_enable_noise);
}


BEGIN_MESSAGE_MAP(CDialogNoiseTool, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_NOISE, &CDialogNoiseTool::OnClickedCheckEnableNoise)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_NOISE, &CDialogNoiseTool::OnClickedButtonApplyNoise)
END_MESSAGE_MAP()


// CDialogNoiseTool message handlers


BOOL CDialogNoiseTool::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_enable_noise.SetCheck(noise_is_enabled());
	CString text;
	text.Format("%2.3f", noise_get_sn());
	m_sn_ratio.SetWindowTextA(text);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDialogNoiseTool::OnClickedCheckEnableNoise()
{
	// TODO: Add your control notification handler code here
	if (m_enable_noise.GetCheck())
		noise_on();
	else
		noise_off();
}


void CDialogNoiseTool::OnClickedButtonApplyNoise()
{
	// TODO: Add your control notification handler code here
	CString text;
	m_sn_ratio.GetWindowTextA(text);
	double sn = _ttof(text);
	noise_set_sn(sn);
}

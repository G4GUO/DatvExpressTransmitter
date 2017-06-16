// DialogDVBSSettings.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogDVBSSettings.h"
#include "afxdialogex.h"
#include "Dvb.h"


// CDialogDVBSSettings dialog

IMPLEMENT_DYNAMIC(CDialogDVBSSettings, CDialogEx)

CDialogDVBSSettings::CDialogDVBSSettings(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_DVB_S, pParent)
{

}

CDialogDVBSSettings::~CDialogDVBSSettings()
{
}

void CDialogDVBSSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogDVBSSettings, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDialogDVBSSettings::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogDVBSSettings message handlers


void CDialogDVBSSettings::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	int id;

	id = GetCheckedRadioButton(IDC_DVBS_FEC12, IDC_DVBS_FEC78);
	if (id == IDC_DVBS_FEC12) cmd_set_dvbs_fec("1/2");
	if (id == IDC_DVBS_FEC23) cmd_set_dvbs_fec("2/3");
	if (id == IDC_DVBS_FEC34) cmd_set_dvbs_fec("3/4");
	if (id == IDC_DVBS_FEC56) cmd_set_dvbs_fec("5/6");
	if (id == IDC_DVBS_FEC78) cmd_set_dvbs_fec("7/8");
	m_restart_required = TRUE;

	CDialogEx::OnOK();
}


BOOL CDialogDVBSSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CString fec = get_current_fec_string();
	if (fec.Compare("1/2")==0) CheckRadioButton(IDC_DVBS_FEC12, IDC_DVBS_FEC78, IDC_DVBS_FEC12);
	if (fec.Compare("2/3")==0) CheckRadioButton(IDC_DVBS_FEC12, IDC_DVBS_FEC78, IDC_DVBS_FEC23);
	if (fec.Compare("3/4")==0) CheckRadioButton(IDC_DVBS_FEC12, IDC_DVBS_FEC78, IDC_DVBS_FEC34);
	if (fec.Compare("5/6")==0) CheckRadioButton(IDC_DVBS_FEC12, IDC_DVBS_FEC78, IDC_DVBS_FEC56);
	if (fec.Compare("7/8")==0) CheckRadioButton(IDC_DVBS_FEC12, IDC_DVBS_FEC78, IDC_DVBS_FEC78);
	m_restart_required = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

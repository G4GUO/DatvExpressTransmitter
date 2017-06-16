// DialogDVBTSettings.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogDVBTSettings.h"
#include "afxdialogex.h"
#include "DVB-T\dvb_t.h"
#include "Dvb.h"

// CDialogDVBTSettings dialog

IMPLEMENT_DYNAMIC(CDialogDVBTSettings, CDialogEx)

CDialogDVBTSettings::CDialogDVBTSettings(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_DVB_T, pParent)
{

}

CDialogDVBTSettings::~CDialogDVBTSettings()
{
}

void CDialogDVBTSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_DVBT_2K_MODE, m_mode);
	DDX_Control(pDX, IDC_RADIO_DVBT_CHAN8, m_channel);
	DDX_Control(pDX, IDC_RADIO_DVBT_CON_QPSK, m_constellation);
	DDX_Control(pDX, IDC_RADIO_DVBT_FEC12, m_fec);
	DDX_Control(pDX, IDC_RADIO_DVBT_GI14, m_guard);
}


BEGIN_MESSAGE_MAP(CDialogDVBTSettings, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDialogDVBTSettings::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogDVBTSettings message handlers


BOOL CDialogDVBTSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	switch (get_dvbt_mode())
	{
	case TM_2K:
		CheckRadioButton(IDC_RADIO_DVBT_2K_MODE, IDC_RADIO_DVBT_8K_MODE, IDC_RADIO_DVBT_2K_MODE);
		break;
	case TM_8K:
		CheckRadioButton(IDC_RADIO_DVBT_2K_MODE, IDC_RADIO_DVBT_8K_MODE, IDC_RADIO_DVBT_8K_MODE);
		break;
	}
	switch (get_dvbt_channel())
	{
	case CH_8M:
		CheckRadioButton(IDC_RADIO_DVBT_CHAN8, IDC_RADIO_DVBT_CHAN1, IDC_RADIO_DVBT_CHAN8);
		break;
	case CH_7M:
		CheckRadioButton(IDC_RADIO_DVBT_CHAN8, IDC_RADIO_DVBT_CHAN1, IDC_RADIO_DVBT_CHAN7);
		break;
	case CH_6M:
		CheckRadioButton(IDC_RADIO_DVBT_CHAN8, IDC_RADIO_DVBT_CHAN1, IDC_RADIO_DVBT_CHAN6);
		break;
	case CH_4M:
		CheckRadioButton(IDC_RADIO_DVBT_CHAN8, IDC_RADIO_DVBT_CHAN1, IDC_RADIO_DVBT_CHAN4);
		break;
	case CH_3M:
		CheckRadioButton(IDC_RADIO_DVBT_CHAN8, IDC_RADIO_DVBT_CHAN1, IDC_RADIO_DVBT_CHAN3);
		break;
	case CH_2M:
		CheckRadioButton(IDC_RADIO_DVBT_CHAN8, IDC_RADIO_DVBT_CHAN1, IDC_RADIO_DVBT_CHAN2);
		break;
	case CH_1M:
		CheckRadioButton(IDC_RADIO_DVBT_CHAN8, IDC_RADIO_DVBT_CHAN1, IDC_RADIO_DVBT_CHAN1);
		break;
	}
	switch (get_dvbt_constellation())
	{
	case CO_QPSK:
		CheckRadioButton(IDC_RADIO_DVBT_CON_QPSK, IDC_RADIO_DVBT_CON_64QAM, IDC_RADIO_DVBT_CON_QPSK);
		break;
	case CO_16QAM:
		CheckRadioButton(IDC_RADIO_DVBT_CON_QPSK, IDC_RADIO_DVBT_CON_64QAM, IDC_RADIO_DVBT_CON_16QAM);
		break;
	case CO_64QAM:
		CheckRadioButton(IDC_RADIO_DVBT_CON_QPSK, IDC_RADIO_DVBT_CON_64QAM, IDC_RADIO_DVBT_CON_64QAM);
		break;
	}
	switch (get_dvbt_fec())
	{
	case CR_12:
		CheckRadioButton(IDC_RADIO_DVBT_FEC12, IDC_RADIO_DVBT_FEC78, IDC_RADIO_DVBT_FEC12);
		break;
	case CR_23:
		CheckRadioButton(IDC_RADIO_DVBT_FEC12, IDC_RADIO_DVBT_FEC78, IDC_RADIO_DVBT_FEC23);
		break;
	case CR_34:
		CheckRadioButton(IDC_RADIO_DVBT_FEC12, IDC_RADIO_DVBT_FEC78, IDC_RADIO_DVBT_FEC34);
		break;
	case CR_56:
		CheckRadioButton(IDC_RADIO_DVBT_FEC12, IDC_RADIO_DVBT_FEC78, IDC_RADIO_DVBT_FEC56);
		break;
	case CR_78:
		CheckRadioButton(IDC_RADIO_DVBT_FEC12, IDC_RADIO_DVBT_FEC78, IDC_RADIO_DVBT_FEC78);
		break;
	}
	switch (get_dvbt_guard())
	{
	case GI_14:
		CheckRadioButton(IDC_RADIO_DVBT_GI14, IDC_RADIO_DVBT_GI132, IDC_RADIO_DVBT_GI14);
		break;
	case GI_18:
		CheckRadioButton(IDC_RADIO_DVBT_GI14, IDC_RADIO_DVBT_GI132, IDC_RADIO_DVBT_GI18);
		break;
	case GI_116:
		CheckRadioButton(IDC_RADIO_DVBT_GI14, IDC_RADIO_DVBT_GI132, IDC_RADIO_DVBT_GI116);
		break;
	case GI_132:
		CheckRadioButton(IDC_RADIO_DVBT_GI14, IDC_RADIO_DVBT_GI132, IDC_RADIO_DVBT_GI132);
		break;
	}
	m_restart_required = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDialogDVBTSettings::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	int id;

	id = GetCheckedRadioButton(IDC_RADIO_DVBT_2K_MODE, IDC_RADIO_DVBT_8K_MODE);
	if (id == IDC_RADIO_DVBT_2K_MODE) cmd_set_dvbt_mode("2K");
	if (id == IDC_RADIO_DVBT_8K_MODE) cmd_set_dvbt_mode("8K");

	id = GetCheckedRadioButton(IDC_RADIO_DVBT_CHAN8, IDC_RADIO_DVBT_CHAN1);
	if (id == IDC_RADIO_DVBT_CHAN8) cmd_set_dvbt_channel("8MHz");
	if (id == IDC_RADIO_DVBT_CHAN7) cmd_set_dvbt_channel("7MHz");
	if (id == IDC_RADIO_DVBT_CHAN6) cmd_set_dvbt_channel("6MHz");
	if (id == IDC_RADIO_DVBT_CHAN4) cmd_set_dvbt_channel("4MHz");
	if (id == IDC_RADIO_DVBT_CHAN3) cmd_set_dvbt_channel("3MHz");
	if (id == IDC_RADIO_DVBT_CHAN2) cmd_set_dvbt_channel("2MHz");
	if (id == IDC_RADIO_DVBT_CHAN1) cmd_set_dvbt_channel("1MHz");

	id = GetCheckedRadioButton(IDC_RADIO_DVBT_CON_QPSK, IDC_RADIO_DVBT_CON_64QAM);
	if (id == IDC_RADIO_DVBT_CON_QPSK) cmd_set_dvbt_constellation("QPSK");
	if (id == IDC_RADIO_DVBT_CON_16QAM) cmd_set_dvbt_constellation("16QAM");
	if (id == IDC_RADIO_DVBT_CON_64QAM) cmd_set_dvbt_constellation("64QAM");

	id = GetCheckedRadioButton(IDC_RADIO_DVBT_FEC12, IDC_RADIO_DVBT_FEC78);
	if (id == IDC_RADIO_DVBT_FEC12) cmd_set_dvbt_fec("1/2");
	if (id == IDC_RADIO_DVBT_FEC23) cmd_set_dvbt_fec("2/3");
	if (id == IDC_RADIO_DVBT_FEC34) cmd_set_dvbt_fec("3/4");
	if (id == IDC_RADIO_DVBT_FEC56) cmd_set_dvbt_fec("5/6");
	if (id == IDC_RADIO_DVBT_FEC78) cmd_set_dvbt_fec("7/8");

	id = GetCheckedRadioButton(IDC_RADIO_DVBT_GI14, IDC_RADIO_DVBT_GI132);
	if (id == IDC_RADIO_DVBT_GI14) cmd_set_dvbt_guard("1/4");
	if (id == IDC_RADIO_DVBT_GI18) cmd_set_dvbt_guard("1/8");
	if (id == IDC_RADIO_DVBT_GI116) cmd_set_dvbt_guard("1/16");
	if (id == IDC_RADIO_DVBT_GI132) cmd_set_dvbt_guard("1/32");

	m_restart_required = TRUE;

	CDialogEx::OnOK();
}

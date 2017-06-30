// DialogDVBS2Settings.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogDVBS2Settings.h"
#include "afxdialogex.h"
#include "Dvb.h"


// CDialogDVBS2Settings dialog

IMPLEMENT_DYNAMIC(CDialogDVBS2Settings, CDialogEx)

CDialogDVBS2Settings::CDialogDVBS2Settings(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_DVB_S2, pParent)
{

}

CDialogDVBS2Settings::~CDialogDVBS2Settings()
{
}

void CDialogDVBS2Settings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogDVBS2Settings, CDialogEx)
	ON_BN_CLICKED(IDC_DVBS2_QPSK, &CDialogDVBS2Settings::OnClickedDvbs2Qpsk)
//	ON_COMMAND(IDC_DVBS2_8PSK, &CDialogDVBS2Settings::OnDvbs28psk)
ON_COMMAND(IDC_DVBS2_8PSK, &CDialogDVBS2Settings::OnDvbs28psk)
ON_COMMAND(IDC_DVBS2_16APSK, &CDialogDVBS2Settings::OnDvbs216apsk)
ON_COMMAND(IDC_DVBS2_32APSK, &CDialogDVBS2Settings::OnDvbs232apsk)
ON_BN_CLICKED(IDOK, &CDialogDVBS2Settings::OnBnClickedOk)
END_MESSAGE_MAP()

// Validate buttons on mode
void CDialogDVBS2Settings::validate_fec(int mod) {

	CWnd *w;
	
	switch (mod) {
	case QPSK:
		mod = M_QPSK;
		break;
	case PSK8:
		mod = M_8PSK;
		break;
	case APSK16:
		mod = M_16APSK;
		break;
	case APSK32:
		mod = M_32APSK;
		break;
	default:
		break;
	}
	w = GetDlgItem(IDC_DVBS2_FEC14);
	if(theDvbS2.is_valid(mod, CR_1_4)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC14,FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC13);
	if (theDvbS2.is_valid(mod, CR_1_3)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC13, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC25);
	if (theDvbS2.is_valid(mod, CR_2_5)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC25, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC12);
	if (theDvbS2.is_valid(mod, CR_1_2)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC12, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC35);
	if (theDvbS2.is_valid(mod, CR_3_5)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC35, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC23);

	if (theDvbS2.is_valid(mod, CR_2_3)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC23, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC34);
	if (theDvbS2.is_valid(mod, CR_3_4)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC34, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC45);
	if (theDvbS2.is_valid(mod, CR_4_5)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC45, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC56);
	if (theDvbS2.is_valid(mod, CR_5_6)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC56, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC89);
	if (theDvbS2.is_valid(mod, CR_8_9)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC89, FALSE);
		w->EnableWindow(FALSE);
	}

	w = GetDlgItem(IDC_DVBS2_FEC910);
	if (theDvbS2.is_valid(mod, CR_9_10)==0)
		w->EnableWindow(TRUE);
	else {
		CheckDlgButton(IDC_DVBS2_FEC910, FALSE);
		w->EnableWindow(FALSE);
	}
}

// CDialogDVBS2Settings message handlers


BOOL CDialogDVBS2Settings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	DVB2FrameFormat fmt;
	uint32_t val;

	theDvbS2.s2_get_configure(&fmt);

	val = get_dvbs2_constellation();

	if (val == QPSK)   CheckRadioButton(IDC_DVBS2_QPSK, IDC_DVBS2_32APSK, IDC_DVBS2_QPSK);
	if (val == PSK8)   CheckRadioButton(IDC_DVBS2_QPSK, IDC_DVBS2_32APSK, IDC_DVBS2_8PSK);
	if (val == APSK16) CheckRadioButton(IDC_DVBS2_QPSK, IDC_DVBS2_32APSK, IDC_DVBS2_16APSK);
	if (val == APSK32) CheckRadioButton(IDC_DVBS2_QPSK, IDC_DVBS2_32APSK, IDC_DVBS2_32APSK);
	validate_fec(val);

	val = get_dvbs2_fec();

	if (val == FEC_14) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC14);
	if (val == FEC_13) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC13);
	if (val == FEC_25) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC25);
	if (val == FEC_12) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC12);
	if (val == FEC_35) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC35);
	if (val == FEC_23) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC23);
	if (val == FEC_34) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC34);
	if (val == FEC_45) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC45);
	if (val == FEC_56) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC56);
	if (val == FEC_89) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC89);
	if (val == FEC_910) CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC910);

	val = get_dvbs2_rolloff();

	if (val == RO_35) CheckRadioButton(IDC_DVBS2_RO_35, IDC_DVBS2_RO_20, IDC_DVBS2_RO_35);
	if (val == RO_25) CheckRadioButton(IDC_DVBS2_RO_35, IDC_DVBS2_RO_20, IDC_DVBS2_RO_25);
	if (val == RO_20) CheckRadioButton(IDC_DVBS2_RO_35, IDC_DVBS2_RO_20, IDC_DVBS2_RO_20);

	val = get_dvbs2_pilots();

	if (val == 0) CheckRadioButton(IDC_DVBS2_PILOT_ON, IDC_DVBS2_PILOT_OFF, IDC_DVBS2_PILOT_OFF);
	if (val == 1 ) CheckRadioButton(IDC_DVBS2_PILOT_ON, IDC_DVBS2_PILOT_OFF, IDC_DVBS2_PILOT_ON);

	m_restart_required = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogDVBS2Settings::OnClickedDvbs2Qpsk()
{
	// TODO: Add your control notification handler code here
	validate_fec(M_QPSK);
	CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC23);
}

void CDialogDVBS2Settings::OnDvbs28psk()
{
	// TODO: Add your command handler code here
	validate_fec(M_8PSK);
	CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC23);
}

void CDialogDVBS2Settings::OnDvbs216apsk()
{
	// TODO: Add your command handler code here
	validate_fec(M_16APSK);
	CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC23);
}

void CDialogDVBS2Settings::OnDvbs232apsk()
{
	// TODO: Add your command handler code here
	validate_fec(M_32APSK);
	CheckRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910, IDC_DVBS2_FEC23);
}


void CDialogDVBS2Settings::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	DVB2FrameFormat fmt;

	theDvbS2.s2_get_configure(&fmt);

	int id;

	id = GetCheckedRadioButton(IDC_DVBS2_QPSK, IDC_DVBS2_32APSK);
	switch (id) {
	case IDC_DVBS2_QPSK:   
		fmt.constellation = M_QPSK;
		cmd_set_dvbs2_constellation("QPSK");
		break;
	case IDC_DVBS2_8PSK:   
		fmt.constellation = M_8PSK;
		cmd_set_dvbs2_constellation("8PSK");
		break;
	case IDC_DVBS2_16APSK: 
		fmt.constellation = M_16APSK;
		cmd_set_dvbs2_constellation("16APSK");
		break;
	case IDC_DVBS2_32APSK: 
		fmt.constellation = M_32APSK;
		cmd_set_dvbs2_constellation("32APSK");
		break;
	}

	id = GetCheckedRadioButton(IDC_DVBS2_RO_35, IDC_DVBS2_RO_20);
    // Only 0.35 rolloff supported due to FPGA size
	fmt.roll_off = RO_0_35;
	cmd_set_dvbs2_rolloff("0.35");
	express_set_filter(RO_35);
	/*
	switch (id) {
	case IDC_DVBS2_RO_35:  
		fmt.roll_off = RO_0_35;
		cmd_set_dvbs2_rolloff("0.35");
		express_set_filter(RO_35);
		break;
	case IDC_DVBS2_RO_25:  
		fmt.roll_off = RO_0_25;
		cmd_set_dvbs2_rolloff("0.25");
		express_set_filter(RO_35);
		break;
	case IDC_DVBS2_RO_20:  
		fmt.roll_off = RO_0_20;
		cmd_set_dvbs2_rolloff("0.20");
		express_set_filter(RO_20);
		break;
	}
	*/
	id = GetCheckedRadioButton(IDC_DVBS2_FEC14, IDC_DVBS2_FEC910);
	switch(id) {
	case IDC_DVBS2_FEC14:    
		fmt.code_rate = CR_1_4;
		cmd_set_dvbs2_fec("1/4");
		break;
	case IDC_DVBS2_FEC13:    
		fmt.code_rate = CR_1_3;
		cmd_set_dvbs2_fec("1/3");
		break;
	case IDC_DVBS2_FEC25:
		fmt.code_rate = CR_2_5;
		cmd_set_dvbs2_fec("2/5");
		break;
	case IDC_DVBS2_FEC12:
		fmt.code_rate = CR_1_2;
		cmd_set_dvbs2_fec("1/2");
		break;
	case IDC_DVBS2_FEC35:
		fmt.code_rate = CR_3_5;
		cmd_set_dvbs2_fec("3/5");
		break;
	case IDC_DVBS2_FEC23:
		fmt.code_rate = CR_2_3;
		cmd_set_dvbs2_fec("2/3");
		break;
	case IDC_DVBS2_FEC34:
		fmt.code_rate = CR_3_4;
		cmd_set_dvbs2_fec("3/4");
		break;
	case IDC_DVBS2_FEC45:
		fmt.code_rate = CR_4_5;
		cmd_set_dvbs2_fec("4/5");
		break;
	case IDC_DVBS2_FEC56:
		fmt.code_rate = CR_5_6;
		cmd_set_dvbs2_fec("5/6");
		break;
	case IDC_DVBS2_FEC89:
		fmt.code_rate = CR_8_9;
		cmd_set_dvbs2_fec("8/9");
		break;
	case IDC_DVBS2_FEC910:
		fmt.code_rate = CR_9_10;
		cmd_set_dvbs2_fec("9/10");
		break;
	default:
		fmt.code_rate = CR_3_4;
		cmd_set_dvbs2_fec("3/4");
		break;
	}

	id = GetCheckedRadioButton(IDC_DVBS2_PILOT_ON, IDC_DVBS2_PILOT_OFF);
	switch (id) {
	case IDC_DVBS2_PILOT_ON:  
		fmt.pilots = PILOTS_ON;
		cmd_set_dvbs2_pilots("ON");
		break;
	case IDC_DVBS2_PILOT_OFF: 
		fmt.pilots = PILOTS_OFF;
		cmd_set_dvbs2_pilots("OFF");
		break;
	}

	if (theDvbS2.s2_set_configure(&fmt) == 0) {
		set_modem_params();
	}
	m_restart_required = TRUE;

	CDialogEx::OnOK();
}

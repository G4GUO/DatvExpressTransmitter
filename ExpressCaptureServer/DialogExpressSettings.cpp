// DialogExpressSettings.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogExpressSettings.h"
#include "afxdialogex.h"
#include "Dvb.h"
#include "hardware.h"


// CDialogExpressSettings dialog

IMPLEMENT_DYNAMIC(CDialogExpressSettings, CDialogEx)

CDialogExpressSettings::CDialogExpressSettings(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogExpressSettings::IDD, pParent)
{

}

CDialogExpressSettings::~CDialogExpressSettings()
{
}

void CDialogExpressSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_EXPRESS_FREQUENCY, m_express_frequency);
	DDX_Control(pDX, IDC_EDIT_EXPRESS_SYMBOL_RATE, m_express_symbol_rate);
	DDX_Control(pDX, IDC_EDIT_EXPRESS_LEVEL, m_express_level);
	DDX_Control(pDX, IDC_CHECK_TX_PORTA, m_ptt_port_a);
	DDX_Control(pDX, IDC_CHECK_TX_PORTB, m_ptt_port_b);
	DDX_Control(pDX, IDC_CHECK_TX_PORTC, m_ptt_port_c);
	DDX_Control(pDX, IDC_CHECK_TX_PORTD, m_ptt_port_d);
	DDX_Control(pDX, IDC_IPADDRESS_SDR_HW, m_sdr_ip);
}


BEGIN_MESSAGE_MAP(CDialogExpressSettings, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDialogExpressSettings::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDialogExpressSettings::OnClickedButtonApply)
END_MESSAGE_MAP()


// CDialogExpressSettings message handlers


BOOL CDialogExpressSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CString text;
	text.Format("%I64d",get_current_tx_frequency());
	m_express_frequency.SetWindowTextA(text);
	text.Format("%d",get_current_tx_level());
	m_express_level.SetWindowTextA(text);
	text.Format("%d",get_current_tx_symbol_rate());
	m_express_symbol_rate.SetWindowTextA(text);
	uint8_t ports = get_tx_ports();
	if (ports & 0x10) m_ptt_port_a.SetCheck(1);
	if (ports & 0x20) m_ptt_port_b.SetCheck(1);
	if (ports & 0x40) m_ptt_port_c.SetCheck(1);
	if (ports & 0x80) m_ptt_port_d.SetCheck(1);

	int mode = get_txmode();
	if (mode == M_DVBS) CheckRadioButton(IDC_TXMODE_DVBS, IDC_TXMODE_DVBT, IDC_TXMODE_DVBS);
	if (mode == M_DVBS2) CheckRadioButton(IDC_TXMODE_DVBS, IDC_TXMODE_DVBT, IDC_TXMODE_DVBS2);
	if (mode == M_DVBT) CheckRadioButton(IDC_TXMODE_DVBS, IDC_TXMODE_DVBT, IDC_TXMODE_DVBT);

	SdrHwType type = get_sdrhw_type();
	if (type == HW_DATV_EXPRESS)  CheckRadioButton(IDC_RADIO_LIME, IDC_RADIO_FMCOMMS, IDC_RADIO_EXPRESS);
	if (type == HW_LIME_SDR)      CheckRadioButton(IDC_RADIO_LIME, IDC_RADIO_FMCOMMS, IDC_RADIO_LIME);
	if (type == HW_ADALM_PLUTO)   CheckRadioButton(IDC_RADIO_LIME, IDC_RADIO_FMCOMMS, IDC_RADIO_PLUTO);
	if (type == HW_FMCOMMSx)      CheckRadioButton(IDC_RADIO_LIME, IDC_RADIO_FMCOMMS, IDC_RADIO_FMCOMMS);

	m_sdr_ip.SetAddress(get_sdr_ip_addr());

	
	m_restart_required = FALSE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDialogExpressSettings::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString text;
	int id, old_mode, old_sr;
	m_restart_required = FALSE;
	
	old_mode = get_txmode();
	old_sr = get_tx_symbolrate();

	m_express_frequency.GetWindowTextA(text);
	cmd_set_dvbs_freq(text);

	m_express_level.GetWindowTextA(text);
	cmd_set_dvbs_level(text);

	m_express_symbol_rate.GetWindowTextA(text);
	cmd_set_dvbs_srate(text);
	if(old_sr != atoi(text)) m_restart_required = TRUE;


	uint8_t ports = 0;
	if (m_ptt_port_a.GetCheck()) ports += 1;
	if (m_ptt_port_b.GetCheck()) ports += 2;
	if (m_ptt_port_c.GetCheck()) ports += 4;
	if (m_ptt_port_d.GetCheck()) ports += 8;
	text.Format("%d",ports);
	cmd_set_tx_ports(text);


	id = GetCheckedRadioButton(IDC_TXMODE_DVBS, IDC_TXMODE_DVBT);

	if (id == IDC_TXMODE_DVBS) {
		if (old_mode != M_DVBS)	m_restart_required = TRUE;
		cmd_set_txmode("DVB-S");
	}

	if (id == IDC_TXMODE_DVBS2) {
		if (old_mode != M_DVBS2) m_restart_required = TRUE;
		cmd_set_txmode("DVB-S2");
	}
	if (id == IDC_TXMODE_DVBT) {
		if (old_mode != M_DVBT) m_restart_required = TRUE;
		cmd_set_txmode("DVB-T");
	}
	id = GetCheckedRadioButton(IDC_RADIO_LIME, IDC_RADIO_FMCOMMS);
	if (id == IDC_RADIO_EXPRESS) {
		m_restart_required = TRUE;
		cmd_set_sdrhw_type("EXPRESS");
	}
	if (id == IDC_RADIO_LIME) {
		m_restart_required = TRUE;
		cmd_set_sdrhw_type("LIME");
	}
	if (id == IDC_RADIO_PLUTO) {
		m_restart_required = TRUE;
		cmd_set_sdrhw_type("PLUTO");
	}
	if (id == IDC_RADIO_FMCOMMS) {
		m_restart_required = TRUE;
		cmd_set_sdrhw_type("FMCOMMS");
	}
	DWORD a;
	m_sdr_ip.GetAddress(a);
	cmd_set_sdr_ip_addr(a);

	hw_receive();

	CDialogEx::OnOK();
}
//
// Applies those parameters that do not need a restart
//
void CDialogExpressSettings::OnClickedButtonApply()
{
	// TODO: Add your control notification handler code here
	CString text;
	m_express_frequency.GetWindowTextA(text);
	cmd_set_dvbs_freq(text);

	m_express_level.GetWindowTextA(text);
	cmd_set_dvbs_level(text);

	uint8_t ports = 0;
	if (m_ptt_port_a.GetCheck()) ports += 1;
	if (m_ptt_port_b.GetCheck()) ports += 2;
	if (m_ptt_port_c.GetCheck()) ports += 4;
	if (m_ptt_port_d.GetCheck()) ports += 8;
	text.Format("%d", ports);
	cmd_set_tx_ports(text);

}

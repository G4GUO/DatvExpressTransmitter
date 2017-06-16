// DialogIqCalibration.cpp : implementation file
//

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "DialogIqCalibration.h"
#include "afxdialogex.h"
#include "Dvb.h"
#include "express.h"

// CDialogIqCalibration dialog

IMPLEMENT_DYNAMIC(CDialogIqCalibration, CDialogEx)

CDialogIqCalibration::CDialogIqCalibration(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_IQ_CALIBRATION, pParent)
{

}

CDialogIqCalibration::~CDialogIqCalibration()
{
}

void CDialogIqCalibration::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_ENABLE_IQ_CALIBRATION, m_iq_enable);
	DDX_Control(pDX, IDC_SPIN_I_CALIBRATION, m_i_offset);
	DDX_Control(pDX, IDC_SPIN_Q_CALIBRATION, m_q_offset);
	DDX_Control(pDX, IDC_STATIC_I_OFFSET, m_i_offset_text);
	DDX_Control(pDX, IDC_STATIC_Q_OFFSET, m_q_offset_text);
}


BEGIN_MESSAGE_MAP(CDialogIqCalibration, CDialogEx)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_I_CALIBRATION, &CDialogIqCalibration::OnDeltaposSpinICalibraion)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Q_CALIBRATION, &CDialogIqCalibration::OnDeltaposSpinQCalibraion)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_IQ_CALIBRATION, &CDialogIqCalibration::OnClickedCheckEnableIqCalibration)
END_MESSAGE_MAP()


// CDialogIqCalibration message handlers

BOOL CDialogIqCalibration::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CString text;
	m_i_value = get_i_dc_offset();
	m_q_value = get_q_dc_offset();
	text.Format("%d", m_i_value);
	m_i_offset_text.SetWindowTextA(text);
	text.Format("%d", m_q_value);
	m_q_offset_text.SetWindowTextA(text);
	express_receive();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDialogIqCalibration::OnDeltaposSpinICalibraion(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (m_iq_enable.GetCheck() == TRUE) {
		if (pNMUpDown->iDelta < 0) {
			m_i_value++;
		}
		else {
			m_i_value--;
		}
		CString text;
		text.Format("%d", m_i_value);
		m_i_offset_text.SetWindowTextA(text);
		express_set_ical(m_i_value);
	}

	*pResult = 0;
}


void CDialogIqCalibration::OnDeltaposSpinQCalibraion(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (m_iq_enable.GetCheck() == TRUE) {
		if (pNMUpDown->iDelta < 0) {
			m_q_value++;
		}
		else {
			m_q_value--;
		}
		CString text;
		text.Format("%d", m_q_value);
		m_q_offset_text.SetWindowTextA(text);
		express_set_qcal(m_q_value);
	}

	*pResult = 0;
}


void CDialogIqCalibration::OnClickedCheckEnableIqCalibration()
{
	// TODO: Add your control notification handler code here
	if (m_iq_enable.GetCheck() == TRUE) {
		express_set_iqcalibrate(TRUE);
		express_transmit();
	}
	else {
		express_set_iqcalibrate(FALSE);
		express_receive();
	}
}




void CDialogIqCalibration::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	// Go back to the old values
	m_i_value = get_i_dc_offset();
	m_q_value = get_q_dc_offset();
	express_set_iqcalibrate(FALSE);
	express_set_ical(m_i_value);
	express_set_qcal(m_q_value);
	express_receive();
	CDialogEx::OnCancel();
}


void CDialogIqCalibration::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	// Save the new vaklues
	cmd_set_i_dc_offset(m_i_value);
	cmd_set_q_dc_offset(m_q_value);
	express_set_iqcalibrate(FALSE);
	express_set_ical(m_i_value);
	express_set_qcal(m_q_value);
	express_receive();
	CDialogEx::OnOK();
}

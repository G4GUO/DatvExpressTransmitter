#include "stdafx.h"
#include "extender.h"
#include "hardware.h"
#include "error_codes.h"
#include "express.h"
#include "Dvb.h"
#include "DVB-T\dvb_t.h"

static SdrHwType m_hw_type;
static bool m_running = false;
// Mutexes
static CRITICAL_SECTION mutex_hw;

int hw_init(void) {
	int res = -1;
	m_hw_type = HW_DATV_EXPRESS;
	m_hw_type = get_sdrhw_type();

	m_running = false;

	InitializeCriticalSection(&mutex_hw);
	EnterCriticalSection(&mutex_hw);

	if (m_hw_type == HW_DATV_EXPRESS) {
		FILE *fpga, *fx2;
		char directory[250];
		char rbfname[80];
		char ihxname[80];

		res = 0;
		switch (get_txmode()) {
		/*case M_DVBS:
			sprintf_s(ihxname, "\\datvexpress8.ihx");
			sprintf_s(rbfname, "\\datvexpressdvbs.rbf");
			break;*/
		case M_DVBS:
		case M_DVBS2:
			sprintf_s(ihxname, "\\datvexpress16.ihx");
			sprintf_s(rbfname, "\\datvexpressraw16.rbf");
			break;
		case M_DVBT:
			sprintf_s(ihxname, "\\datvexpress16.ihx");
			sprintf_s(rbfname, "\\datvexpressraw16.rbf");
			break;
		}
		if (GetCurrentDirectory(250, directory)) {
			strncat_s(directory, ihxname, 40);
			if (fopen_s(&fx2, directory, "rb") == 0)
			{
				if (GetCurrentDirectory(250, directory))
				{
					strncat_s(directory, rbfname, 40);
					if (fopen_s(&fpga, directory, "rb") == 0)
					{
						res = express_init(fx2, fpga);
						fclose(fpga);
						fclose(fx2);
					}
					else
					{
						report_error(ERROR_FPGA_NOT_FOUND);
						cmd_set_error_text("FPGA .rbf File not found");
						res = -2;
					}
				}
			}
			else
			{
				report_error(ERROR_FIRMWARE_NOT_FOUND);
				cmd_set_error_text("FX2 firmware .ihx File not found");
				res = -3;
			}
		}
	}

	if (m_hw_type == HW_LIME_SDR) {
		rrc_init();
		res = limesdr_init();
	}

	if (m_hw_type == HW_ADALM_PLUTO) {
		rrc_init();
		res = fmc_init();
		if (res == 0) fmc_set_tx_level(-70);
	}
	if (m_hw_type == HW_FMCOMMSx) {
		rrc_init();
		res = fmc_init();
		if (res == 0) fmc_set_tx_level(-70);
	}
	LeaveCriticalSection(&mutex_hw);
	m_running = true;
	return res;
}
void hw_deinit(void) {
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_deinit();
	}
	if (m_hw_type == HW_FMCOMMSx) {
		fmc_deinit();
	}
	if (m_hw_type == HW_LIME_SDR) {
		limesdr_deinit();
	}
	LeaveCriticalSection(&mutex_hw);
//	DeleteCriticalSection(&mutex_hw);
	m_running = false;
}
void hw_enter_critical(void) {
	EnterCriticalSection(&mutex_hw);
}
void hw_leave_critical(void) {
	LeaveCriticalSection(&mutex_hw);
}
void hw_set_freq(double f) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_freq(ext_set_express_freq(f));
	}
	if (m_hw_type == HW_ADALM_PLUTO) {
		if((f >= 48000000)&&(f <= 6000000000)) fmc_set_tx_frequency(f);
	}
	if (m_hw_type == HW_FMCOMMSx) {
		if ((f >= 48000000) && (f <= 6000000000)) fmc_set_tx_frequency(f);
	}
	if (m_hw_type == HW_LIME_SDR) {
		limesdr_set_freq(f);
	}
	LeaveCriticalSection(&mutex_hw);
}

void hw_set_sr(double sr) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_sr(sr);
	}

	if (m_hw_type == HW_FMCOMMSx) {
		long long int i_sr = (long long int)sr;
		if (i_sr > 521000) {
			fmc_set_tx_sr(i_sr);
		}
	}

	if (m_hw_type == HW_ADALM_PLUTO) {
		long long int i_sr = (long long int)sr;
		if (i_sr > 65000) {
			if (i_sr > 521000) {
				fmc_set_tx_sr(i_sr);
			}
			else {
				// Low symbol rate
				fmc_set_tx_sr(i_sr * 8);
			}
			fmc_configure_x8_int_dec(i_sr);
		}
	}
	if (m_hw_type == HW_LIME_SDR) {
		limesdr_set_sr(sr,16);
	}
	LeaveCriticalSection(&mutex_hw);
}
void hw_set_level(double lvl) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_level((int)lvl);
	}
	if ((m_hw_type == HW_FMCOMMSx) || (m_hw_type == HW_ADALM_PLUTO)) {
		fmc_set_tx_level(lvl-47.0);
	}
	if (m_hw_type == HW_LIME_SDR) {
		limesdr_set_level(lvl);
	}
	LeaveCriticalSection(&mutex_hw);
}
void hw_transmit(void) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_transmit();
	}
	if ((m_hw_type == HW_FMCOMMSx) || (m_hw_type == HW_ADALM_PLUTO)) {
		fmc_set_transmit();
	}
	if (m_hw_type == HW_LIME_SDR)
	{
		limesdr_transmit();
	}
	LeaveCriticalSection(&mutex_hw);
}
void hw_receive(void) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_receive();
	}
	if ((m_hw_type == HW_FMCOMMSx) || (m_hw_type == HW_ADALM_PLUTO)) {
		fmc_set_receive();
	}
	if (m_hw_type == HW_LIME_SDR)
	{
		limesdr_receive();
	}
	LeaveCriticalSection(&mutex_hw);
}
void hw_set_carrier(int chk) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		if (chk)
			express_set_carrier(true);
		else
			express_set_carrier(false);
	}
	if ((m_hw_type == HW_FMCOMMSx) || (m_hw_type == HW_ADALM_PLUTO) || (m_hw_type == HW_LIME_SDR)) {
		if (chk)
			tx_carrier(true);
		else
			tx_carrier(false);
	}
	LeaveCriticalSection(&mutex_hw);
}
void hw_set_fec(int fec) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_fec(fec);
	}
	LeaveCriticalSection(&mutex_hw);
}
void hw_set_iqcalibrate(int status) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_iqcalibrate(status);
	}
	if(m_hw_type == HW_LIME_SDR)
	{
		hw_set_carrier(status);
	}
	LeaveCriticalSection(&mutex_hw);
}
void hw_set_ical(int offset) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_ical(offset);
	}
	if (m_hw_type == HW_LIME_SDR)
	{
		limesdr_set_ical(offset);
	}
	LeaveCriticalSection(&mutex_hw);
}

void hw_set_qcal(int offset) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_qcal(offset);
	}
	if (m_hw_type == HW_LIME_SDR)
	{
		limesdr_set_qcal(offset);
	}
	LeaveCriticalSection(&mutex_hw);
}

void hw_set_filter(int rolloff){
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_filter(rolloff);
	}
	if ((m_hw_type == HW_ADALM_PLUTO) || (m_hw_type == HW_FMCOMMSx)) {

		// Select the default filter

		if (get_txmode() == M_DVBT) {
			fmc_load_tx_lpf_filter(0.9f/dvb_t_get_interpolater());
		}

		if (get_txmode() == M_DVBS) {
			fmc_load_tx_rrc_filter(0.35f);
		}

		if (get_txmode() == M_DVBS2) {
			switch (get_dvbs2_rolloff()) {
			case RO_35:
				fmc_load_tx_rrc_filter(0.35f);
				break;
			case RO_25:
				fmc_load_tx_rrc_filter(0.25f);
				break;
			case RO_20:
				fmc_load_tx_rrc_filter(0.20f);
				break;
			default:
				break;
			}
		}
	}
	if (m_hw_type == HW_LIME_SDR)
	{
		if (get_txmode() == M_DVBT) {
			//fmc_load_tx_lpf_filter(0.9f / dvb_t_get_interpolater());
		}

		if (get_txmode() == M_DVBS) {
			limesdr_tx_rrc_filter(0.35f);
		}

		if (get_txmode() == M_DVBS2) {
			switch (get_dvbs2_rolloff()) {
			case RO_35:
				limesdr_tx_rrc_filter(0.35f);
				break;
			case RO_25:
				limesdr_tx_rrc_filter(0.25f);
				break;
			case RO_20:
				limesdr_tx_rrc_filter(0.20f);
				break;
			default:
				break;
			}
		}
	}
	LeaveCriticalSection(&mutex_hw);
}
void hw_set_analogue_lpf(double bw) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
	}
	if ((m_hw_type == HW_ADALM_PLUTO) || (m_hw_type == HW_FMCOMMSx)) {
		fmc_set_analog_lpf(bw);
	}
	LeaveCriticalSection(&mutex_hw);
}

void hw_set_ports(int ports) {
	if (m_running == false) return;
	EnterCriticalSection(&mutex_hw);
	if (m_hw_type == HW_DATV_EXPRESS) {
		express_set_ports(ports);
	}
	LeaveCriticalSection(&mutex_hw);
}

#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "Dvb.h"
#include "error_codes.h"
#include "DVB-T\dvb_t.h"

static int g_sr;
static int g_fec[2];
static int64_t g_gross_bitrate;
static int64_t g_net_bitrate;
static int64_t g_tp_tick; // 27 MHz

void calculate_bitrate(void){
	if (get_txmode() == M_DVBS) {
		g_gross_bitrate = (g_sr * 2 * g_fec[0]) / g_fec[1];
		g_net_bitrate = (g_gross_bitrate * 188) / 204;//RS overhead
	}
	if (get_txmode() == M_DVBT) {
		g_gross_bitrate = dvb_t_raw_bitrate();
		g_net_bitrate = (g_gross_bitrate * 188) / 204;//RS overhead
	}
}
void set_modem_params(void){
	g_sr = get_tx_symbolrate();
	if (get_txmode() == M_DVBS) {
		switch (get_dvbs_fec()) {
		case FEC_12:
			g_fec[0] = 1;
			g_fec[1] = 2;
			break;
		case FEC_23:
			g_fec[0] = 2;
			g_fec[1] = 3;
			break;
		case FEC_34:
			g_fec[0] = 3;
			g_fec[1] = 4;
			break;
		case FEC_56:
			g_fec[0] = 5;
			g_fec[1] = 6;
			break;
		case FEC_78:
			g_fec[0] = 7;
			g_fec[1] = 8;
			break;
		}
		calculate_bitrate();
		
	}

	if (get_txmode() == M_DVBT) {
		DVBTFormat fmt;
		fmt.chan = get_dvbt_channel();
		fmt.tm   = get_dvbt_mode();
		fmt.gi   = get_dvbt_guard();
		fmt.co   = get_dvbt_constellation();
		fmt.fec  = get_dvbt_fec();
		fmt.ir   = 1;
		fmt.sf   = SF_NH;
		dvb_t_configure(&fmt);
		express_set_sr(fmt.sr);
		calculate_bitrate();
	}

	if (get_txmode() == M_DVBS2) {
		// Configure the DVB-S2 modem stack
		DVB2FrameFormat fmt;
		fmt.frame_type = FRAME_NORMAL;
		switch (get_dvbs2_fec()) {
		case FEC_14:
			fmt.code_rate = CR_1_4;
			break;
		case FEC_13:
			fmt.code_rate = CR_1_3;
			break;
		case FEC_25:
			fmt.code_rate = CR_2_5;
			break;
		case FEC_12:
			fmt.code_rate = CR_1_2;
			break;
		case FEC_35:
			fmt.code_rate = CR_3_5;
			break;
		case FEC_23:
			fmt.code_rate = CR_2_3;
			break;
		case FEC_34:
			fmt.code_rate = CR_3_4;
			break;
		case FEC_45:
			fmt.code_rate = CR_4_5;
			break;
		case FEC_56:
			fmt.code_rate = CR_5_6;
			break;
		case FEC_89:
			fmt.code_rate = CR_8_9;
			break;
		case FEC_910:
			fmt.code_rate = CR_9_10;
			break;
		}
		switch (get_dvbs2_constellation()) {
		case QPSK:
			fmt.constellation = M_QPSK;
			break;
		case PSK8:
			fmt.constellation = M_8PSK;
			break;
		case APSK16:
			fmt.constellation = M_16APSK;
			break;
		case APSK32:
			fmt.constellation = M_32APSK;
			break;
		}
		switch (get_dvbs2_rolloff()) {
		case RO_35:
			fmt.roll_off = RO_0_35;
			break;
		case RO_25:
			fmt.roll_off = RO_0_25;
			break;
		case RO_20:
			fmt.roll_off = RO_0_20;
			break;
		}
		switch (get_dvbs2_pilots()) {
		case 0:
			fmt.pilots = PILOTS_OFF;
			break;
		case 1:
			fmt.pilots = PILOTS_ON;
			break;
		}
		fmt.dummy_frame = 0;
		fmt.null_deletion = 0;

		if (theDvbS2.s2_set_configure(&fmt) == 0) {
			// Get the channel bitrate
			g_gross_bitrate = (int)(theDvbS2.s2_get_efficiency()*g_sr);
		}
		else {
			// Error has occured, set the bitrate to a value so as to prevent a crash
			g_gross_bitrate = 4000000;
			report_error(ERROR_ILLEGAL_S2_VALUES);
			cmd_set_error_text("Illegal S2 configuration attempted");
		}
		g_net_bitrate = g_gross_bitrate;
	}

	g_tp_tick = (27000000*8)/g_net_bitrate;// 27 MHz, tick is the time to send one TP byte
	g_tp_tick = g_tp_tick*188;// 27 MHz 
}
int64_t get_tp_tick(void){return g_tp_tick;};

uint32_t get_audio_bitrate(void){
	if (get_audio_status() == TRUE)
		return get_audio_codec_bitrate();
	else
		return 0;
}
uint32_t get_tx_bitrate(void){
	return (uint32_t)g_net_bitrate;
}

//
// Calculate the bitrate available for video
//
uint32_t get_video_bitrate(void){
	// Allow for TP packet overhead
	uint32_t br = (uint32_t)g_net_bitrate;
	// remove audio overhead
	br -= get_audio_bitrate();
	// Remove the SI overhead
	br -= si_overhead();
	br = (uint32_t)(br*get_vbr_twiddle());
	return br;
}
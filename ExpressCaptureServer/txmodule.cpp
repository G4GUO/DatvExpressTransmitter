#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "Dvb.h"
#include "express.h"
#include "DVB-T\dvb_t.h"
#include "noise.h"
#include "hardware.h"

DWORD lpThreadId;
int g_running;
static bool g_carrier;
static uint8_t m_dibits[DVBS_T_CODED_FRAME_LEN * 16];
static scmplx m_samples[DVBS_T_CODED_FRAME_LEN * 16];

#define SMAG 0x3FFF

const scmplx tx_lookup[4] = {
	{  SMAG,  SMAG },
	{ -SMAG,  SMAG },
	{  SMAG, -SMAG },
	{ -SMAG, -SMAG }
};

void set_carrier_symbols(scmplx *in, int len) {
	for (int i = 0; i < len; i++) {
		in[i] = tx_lookup[0];
	}
}
void set_calibration_symbols(scmplx *in, int len, float delta) {
	static float acc;

	for (int i = 0; i < len; i++) {
		in[i].re = (short)(cosf(acc)*SMAG);
		in[i].im = (short)(sinf(acc)*SMAG);
		acc += delta;
	}
	double ival;
	float fval = (float)modf(acc /(2.0 * M_PI), &ival);
	acc = (float)(fval * 2.0 * M_PI);
}

int qpsk_encode(uint8_t *dibits, scmplx *s, int len) {

	for (int i = 0; i < len; i++) {
		s[i] = tx_lookup[dibits[i]];
	}
	return len;
}
//
// For DATV-Express
//
UINT express_tx_thread(LPVOID pParam)
{
	uint8_t *b;
	int len;
	while( g_running )
	{
		//Sleep(0);
		int Queue = get_tx_buf_qsize();
			if(Queue<100)
			{
				TRACE("Buff is %d\n", Queue);
				for (int i = 0; i < 100; i++)
				{
					UCHAR* b = alloc_copy_tx_buff(null_pkt());
					post_tx_buff(b);
				}
			
			}
		
		switch(get_txmode()){
		case M_DVBS:
			//
			// The mode is DVB-S
			//
			if ((b = get_tx_buff()) != NULL) 
			{
					tp_log(b);
					len = dvb_encode_frame(b, m_dibits);
					len = qpsk_encode(m_dibits, m_samples, len);
					hw_enter_critical();
					express_write_16_bit_samples(m_samples, len);
					hw_leave_critical();
					rel_tx_buff(b);
					//rel_tx_buff(b);
			}
			else
			{
				//Sleep(10);
				//send_tp(null_pkt());
			}
			break;
		case M_DVBS2:
			//
			// The mode is DVB-S2
			//
			if (((b = get_tx_buff()) != NULL)) {
				tp_log(b);
				if ((len = theDvbS2.s2_add_ts_frame(b))>0) {
					// Time to send new frame
					hw_enter_critical();
					express_write_16_bit_samples(noise_add(theDvbS2.pl_get_frame(), len),len);
					hw_leave_critical();
					//express_write_16_bit_samples(theDvbS2.pl_get_frame(), len);
				}
				rel_tx_buff(b);
			}
			else
			{
				//send_tp(null_pkt());
			}
			break;
		case M_DVBT:
			if ((b = get_tx_buff()) != NULL) {
				if ((len = dvb_t_encode_and_modulate(b)) > 0) {
					// Time to send new frame
					hw_enter_critical();
					express_write_16_bit_samples((scmplx*)dvb_t_get_frame(), len);
					hw_leave_critical();
				}
				rel_tx_buff(b);
			}
			else
			{
				send_tp(null_pkt());
			}
			break;
		}
	}
	return 0;
}
//
// For LimeSDR
//
UINT lime_tx_thread(LPVOID pParam)
{
	uint8_t *b;
	int len;
	while (g_running)
	{
		if (g_carrier == true) {
			tx_buf_empty();
			set_carrier_symbols(m_samples, (DVBS_T_CODED_FRAME_LEN * 16));
			//set_calibration_symbols(m_samples, (DVBS_T_CODED_FRAME_LEN * 16), 0.013);
			hw_enter_critical();
			lime_tx_samples(m_samples, (DVBS_T_CODED_FRAME_LEN * 16));
			hw_leave_critical();
		}
		else
		{
			int Queue = get_tx_buf_qsize();
			if (Queue < 200)
			{
				TRACE("Buff is %d\n", Queue);
				for (int i = 0; i < 100; i++)
				{
					UCHAR* b = alloc_copy_tx_buff(null_pkt());
					post_tx_buff(b);
				}

			}
			switch (get_txmode()) {
			case M_DVBS:
				//
				// The mode is DVB-S
				//
				if ((b = get_tx_buff()) != NULL) {
					tp_log(b);
					len = dvb_encode_frame(b, m_dibits);
					len = qpsk_encode(m_dibits, m_samples, len);
					hw_enter_critical();
					lime_tx_samples(m_samples, len);
					
					hw_leave_critical();
					rel_tx_buff(b);
				}
				else
				{
					//send_tp(null_pkt());
				}
				break;
			case M_DVBS2:
				//
				// The mode is DVB-S2
				//
				if ((b = get_tx_buff()) != NULL) {
					tp_log(b);
					if ((len = theDvbS2.s2_add_ts_frame(b)) > 0) {
						
						// Time to send new frame
						hw_enter_critical();
						lime_tx_samples(theDvbS2.pl_get_frame(), len);
						hw_leave_critical();
					}
					rel_tx_buff(b);
				}
				else
				{
					//send_tp(null_pkt());
				}
				break;
			case M_DVBT:
				if ((b = get_tx_buff()) != NULL) {
					if ((len = dvb_t_encode_and_modulate(b)) > 0) {
						// Time to send new frame
						hw_enter_critical();
						lime_tx_samples((scmplx*)dvb_t_get_frame(), len);
						hw_leave_critical();
					}
					rel_tx_buff(b);
				}
				else
				{
					//send_tp(null_pkt());
				}
				break;
			}
		}
	}
	return 0;
}
//
// For PLUTO and FMCOMMS
//
UINT fmcomms_tx_thread(LPVOID pParam)
{
	uint8_t *b;
	int len;
	int count = 0;
	while (g_running)
	{
		if (g_carrier == true) {
			tx_buf_empty();
			set_carrier_symbols(m_samples, (DVBS_T_CODED_FRAME_LEN * 16));
			//set_calibration_symbols(m_samples, (DVBS_T_CODED_FRAME_LEN * 16), 0.013);
			hw_enter_critical();
			fmc_tx_samples(m_samples, (DVBS_T_CODED_FRAME_LEN * 16));
			hw_leave_critical();
		}
		else {
			//Sleep(0);
			int Queue = get_tx_buf_qsize();
			if (Queue<100)
			{
				TRACE("Buff is %d\n", Queue);
				for (int i = 0; i < 100; i++)
				{
					UCHAR* b = alloc_copy_tx_buff(null_pkt());
					post_tx_buff(b);
				}

			}

			switch (get_txmode()) {
			case M_DVBS:
				//
				// The mode is DVB-S
				//
				if ((b = get_tx_buff()) != NULL) {
					tp_log(b);
					len = dvb_encode_frame(b, m_dibits);
					len = qpsk_encode(m_dibits, m_samples, len);
					/*for (int i = 0; i < len; i++)
					{
						m_samples[i].im = m_samples[i].im << 4;
						m_samples[i].re = m_samples[i].re << 4;
					}*/
					hw_enter_critical();
					fmc_tx_samples(m_samples, len);
					hw_leave_critical();
					rel_tx_buff(b);
				}
				else
				{
					//send_tp(null_pkt());
				}
				break;
			case M_DVBS2:
				//
				// The mode is DVB-S2
				//
				if ((b = get_tx_buff()) != NULL) {
					tp_log(b);
					if ((len = theDvbS2.s2_add_ts_frame(b)) > 0) {
						// Time to send new frame
						hw_enter_critical();
						fmc_tx_samples(theDvbS2.pl_get_frame(), len);
						hw_leave_critical();
					}
					rel_tx_buff(b);
				}
				else
				{
					//send_tp(null_pkt());
				}
				break;
			case M_DVBT:
				if ((b = get_tx_buff()) != NULL) {
					if ((len = dvb_t_encode_and_modulate(b)) > 0) {
						// Time to send new frame
						hw_enter_critical();
						//	memset(dvb_t_get_frame(), 0, sizeof(scmplx)*len);
						fmc_tx_samples((scmplx*)dvb_t_get_frame(), len);
						hw_leave_critical();
					}
					rel_tx_buff(b);
				}
				else
				{
					//send_tp(null_pkt());
				}
				break;
			}
		}
	}
	return 0;
}
void tx_carrier(bool val) {
	g_carrier = val;
}

void tx_start(void){
	int n;

	g_running = 1;
	g_carrier = false;
	//
	// Create the thread processes
	//
	if (get_sdrhw_type() == HW_DATV_EXPRESS) {
		CreateThread(NULL, //Choose default security
			0, //Default stack size
			(LPTHREAD_START_ROUTINE)&express_tx_thread, //Routine to execute
			(LPVOID)&n, //Thread parameter
			0, //Immediately run the thread
			&lpThreadId //Thread Id
			);
	}
	if (get_sdrhw_type() == HW_LIME_SDR) {
		CreateThread(NULL, //Choose default security
			0, //Default stack size
			(LPTHREAD_START_ROUTINE)&lime_tx_thread, //Routine to execute
			(LPVOID)&n, //Thread parameter
			0, //Immediately run the thread
			&lpThreadId //Thread Id
			);
	}
	if (get_sdrhw_type() == HW_ADALM_PLUTO) {
		CreateThread(NULL, //Choose default security
			0, //Default stack size
			(LPTHREAD_START_ROUTINE)&fmcomms_tx_thread, //Routine to execute
			(LPVOID)&n, //Thread parameter
			0, //Immediately run the thread
			&lpThreadId //Thread Id
			);
	}
	if (get_sdrhw_type() == HW_FMCOMMSx) {
		CreateThread(NULL, //Choose default security
			0, //Default stack size
			(LPTHREAD_START_ROUTINE)&fmcomms_tx_thread, //Routine to execute
			(LPVOID)&n, //Thread parameter
			0, //Immediately run the thread
			&lpThreadId //Thread Id
			);
	}

}

void tx_stop(void){
	g_running = 0;
}

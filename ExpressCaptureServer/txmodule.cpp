#include "stdafx.h"
#include "ExpressCaptureServer.h"
#include "Dvb.h"
#include "express.h"
#include "DVB-T\dvb_t.h"
#include "noise.h"

DWORD lpThreadId;
int g_running;

UINT tx_thread(LPVOID pParam)
{
	uint8_t *b;
	int len;
	while( g_running )
	{
		switch(get_txmode()){
		case M_DVBS:
			//
			// The mode is DVB-S
			//
			if ((b = get_tx_buff()) != NULL) {
				express_write_transport_stream(b, TP_SIZE);
			}
			else
			{
				send_tp(null_pkt());
			}
			break;
		case M_DVBS2:
			//
			// The mode is DVB-S2
			//
			if ((b = get_tx_buff()) != NULL) {
				if ((len = theDvbS2.s2_add_ts_frame(b))>0) {
					// Time to send new frame
					express_write_16_bit_samples(noise_add(theDvbS2.pl_get_frame(), len),len);
					//express_write_16_bit_samples(theDvbS2.pl_get_frame(), len);
				}
				rel_tx_buff(b);
			}
			else
			{
				send_tp(null_pkt());
			}
			break;
		case M_DVBT:
			if ((b = get_tx_buff()) != NULL) {
				if ((len = dvb_t_encode_and_modulate(b)) > 0) {
					// Time to send new frame
					express_write_16_bit_samples((scmplx*)dvb_t_get_frame(), len);
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

void tx_start(void){
	int n;

	g_running = 1;
	//
	// Create the thread processes
	//
	CreateThread(NULL, //Choose default security
                 0, //Default stack size
                 (LPTHREAD_START_ROUTINE)&tx_thread, //Routine to execute
                 (LPVOID) &n, //Thread parameter
                 0, //Immediately run the thread
                 &lpThreadId //Thread Id
     );

}

void tx_stop(void){
	g_running = 0;
}

#include "stdafx.h"
#include <stdint.h>
#include "Dvb.h"
#include "si.h"

#define TMR_30_MS    (int64_t)(0.03*27000000)
#define TMR_50_MS    (int64_t)(0.05*27000000)
#define TMR_100_MS   (int64_t)(0.10*27000000)
#define TMR_1000_MS  (int64_t)(1*27000000)
#define TMR_2000_MS  (int64_t)(2*27000000)
#define TMR_9000_MS  (int64_t)(9*27000000)
#define TMR_30000_MS (int64_t)(30*27000000)

bool g_pat_flag;
bool g_tdt_flag;
bool g_pmt_flag;
bool g_eit_flag;
bool g_sdt_flag;
bool g_nit_flag;

static int64_t g_50ms_clock_ticks;
static int64_t g_100ms_clock_ticks;
static int64_t g_1s_clock_ticks;
static int64_t g_2s_clock_ticks;
static int64_t g_9s_clock_ticks;
static int64_t g_30s_clock_ticks;

void timer_tick( void )
{
	g_50ms_clock_ticks-= get_tp_tick();
    if( g_50ms_clock_ticks <= 0 )
    {
        g_50ms_clock_ticks = TMR_50_MS;
    }

    g_100ms_clock_ticks-= get_tp_tick();
    if( g_100ms_clock_ticks <= 0 )
    {
        g_pat_flag = true;
        g_pmt_flag = true;
        g_100ms_clock_ticks = TMR_100_MS;
    }

    g_1s_clock_ticks-= get_tp_tick();
    if( g_1s_clock_ticks <= 0 )
    {
        g_sdt_flag = true;
        g_eit_flag = true;
        g_nit_flag = true;
        g_1s_clock_ticks = TMR_1000_MS;
     }

     g_2s_clock_ticks-= get_tp_tick();
     if( g_2s_clock_ticks <= 0 )
     {
         g_2s_clock_ticks = TMR_2000_MS;
     }

     g_9s_clock_ticks-= get_tp_tick();;
     if( g_9s_clock_ticks <= 0 )
     {
         g_tdt_flag        = true;
         g_9s_clock_ticks = TMR_9000_MS;
     }

     g_30s_clock_ticks-= get_tp_tick();
     if( g_30s_clock_ticks <= 0 )
     {
         g_30s_clock_ticks = TMR_30000_MS;
     }
}
// Estimate the number of bps for sending the SI tables 
uint32_t si_overhead(void) {
	double oh = 0;
	oh += 2 * 10    * 188 * 8;// PAT PMT
	oh += 3 * 1     * 188 * 8;// SDT EIT NIT
	oh += 1 * 0.111 * 188 * 8;// TDT
	return (uint32_t)oh;
}
//
// Called at the end of a sequence of video or audio frames.
// It sends any outstanding SI frames frequired
//
void timer_work(void){

	if(g_pat_flag == true){
		g_pat_flag = false;
		pat_dvb();
	}
    if(g_tdt_flag==true){
		g_tdt_flag = false;
		tdt_dvb();
	}
	
    if(g_pmt_flag==true){
		g_pmt_flag = false;
		pmt_dvb();
	}
    if(g_eit_flag==true){
		g_eit_flag = false;
		eit_dvb();
	}
    if(g_sdt_flag==true){
		g_sdt_flag = false;
		sdt_dvb();
	}
    if(g_nit_flag==true){
		g_nit_flag = false;
		nit_dvb();
	}
}
#pragma once
#include "stdafx.h"
#include <stdint.h>
#include "dvb.h"
#include "tp.h"

int64_t g_pcr_clock;
int64_t g_pcr_clk_dwncnt;

void pcr_increment_clock(void){
	g_pcr_clock += get_tp_tick();
    g_pcr_clk_dwncnt -= get_tp_tick();
	timer_tick();
}

bool is_pcr_update(void){
	bool res = false;
	if(g_pcr_clk_dwncnt < 0 ){
        res = true;
		g_pcr_clk_dwncnt += (int64_t)(0.035*27000000);// 35 ms
	}
	return res;
}
int64_t get_pcr_clock(void){
	return g_pcr_clock;
}
void set_pcr_clock(int64_t clk){
	g_pcr_clock = clk;
}

//
// Spec 13818-1 page 22
//
int add_pcr_field( uint8_t *b )
{
    int64_t m,l;

    m = g_pcr_clock/300;
    l = g_pcr_clock%300;

    b[0] = (uint8_t)((m>>25)&0xFF);
    b[1] = (uint8_t)((m>>17)&0xFF);
    b[2] = (uint8_t)((m>>9)&0xFF);
    b[3] = (uint8_t)((m>>1)&0xFF);
    if(m&1)
        b[4] = 0x80 | 0x7E;
    else
        b[4] = 0x00 | 0x7E;
    if(l&0x100) b[4] |= 1;// MSB of extension
    b[5] = (uint8_t)(l&0xFF);
    return 6;
}

//
// This module formats and sends PCR packets
// The PCR field is sent in the adaption field and can be added to any
// transport frame.

int adaption_fmt( uint8_t *b, tp_adaption *a )
{
    b[0] = 0;

    if(a->discontinuity_ind)     b[0] |= 0x80;
    if(a->random_access_ind)     b[0] |= 0x40;
    if(a->elem_stream_pr_ind)    b[0] |= 0x20;
    if(a->PCR_flag)              b[0] |= 0x10;
    if(a->OPCR_flag)             b[0] |= 0x08;
    if(a->splicing_point_flag)   b[0] |= 0x04;
    if(a->trans_priv_data_flag)  b[0] |= 0x02;
    if(a->adapt_field_extn_flag) b[0] |= 0x01;

    return 1;
}
int pcr_fmt( uint8_t *b, tp_hdr *h )
{
    int len;
    tp_adaption apt;

    len = 1;//length field

    apt.discontinuity_ind     = 0;
    apt.random_access_ind     = 0;
    apt.elem_stream_pr_ind    = 0;
    apt.PCR_flag              = 1;
    apt.OPCR_flag             = 0;
    apt.splicing_point_flag   = 0;
    apt.trans_priv_data_flag  = 0;
    apt.adapt_field_extn_flag = 0;

    // First byte will contain the adaption field length
    len += adaption_fmt( &b[len], &apt );

    // Now add the PCR field
    len += add_pcr_field( &b[len] );
	if(h->adaption_field_control == ADAPTION_FIELD_ONLY )
		b[0] = 183;//Fxed value
	else
		b[0] = len-1;// Encode the length of the adaption field
    return len;
}
//
// This inspects a transport packet to see whether a PCR field
// is present and if it is re-timestamps it
//
void pcr_timestamp( uint8_t *b )
{
    if((b[3]&0xC0)==0xC0)
    {
        if(b[5]&0x10)
        {
            // PCR field present
            add_pcr_field( &b[6] );
        }
    }
}
//
// Send a PCR packet with no payload if required
//
void pcr_dvb( uint8_t cont){
    int    len;
    tp_hdr hdr;
	uint8_t pkt[TP_LEN];

	if(is_pcr_update()==true){
		// Header
		hdr.transport_error_indicator    = 0;
		hdr.payload_unit_start_indicator = 0;
		hdr.transport_priority           = 0 ;
		hdr.pid                          = get_pcr_pid();
		hdr.transport_scrambling_control = 0;
		hdr.continuity_counter           = cont;

        // Adaption only (PCR)
        hdr.adaption_field_control  = ADAPTION_FIELD_ONLY;
        len = tp_fmt( pkt, &hdr );
        // Add PCR field
        len += pcr_fmt( &pkt[len], &hdr );
		// Stuff the body
		for( int i = len; i < TP_LEN;i++){
			pkt[i] = 0xFF;
		}
		// The continuity counter is only update if there is a payload
	    ts_write_transport_queue( pkt );
	}
}

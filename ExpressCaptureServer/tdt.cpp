#include "stdafx.h"
#include <stdint.h>
#include <time.h>
#include "Dvb.h"
#include "tp.h"

uint8_t tdt_pkt[188];
uint8_t m_seq;

int tdt_time_fmt( uint8_t *b )
{
    dvb_si_time st;
    dvb_si_system_time( &st );
    return dvb_si_add_time( b, &st);
}
int f_tdt( uint8_t *b )
{
    b[0]  = 0x70;
    b[1]  = 0x00;
    b[1] |= 0x40;
    b[1] |= 0x30;
    b[2]  = 5;
    tdt_time_fmt( &b[3] );
    return 8;
}
//
// Format the header and payload of the TDT packet
//
void tdt_fmt( uint8_t *b )
{
    int i,len;
    tp_hdr hdr;

    hdr.transport_error_indicator    = TRANSPORT_ERROR_FALSE;
    hdr.payload_unit_start_indicator = PAYLOAD_START_TRUE;
    hdr.transport_priority           = TRANSPORT_PRIORITY_LOW;
    hdr.pid                          = TDT_PID;//
    hdr.transport_scrambling_control = SCRAMBLING_OFF;
    hdr.adaption_field_control       = ADAPTION_PAYLOAD_ONLY;
    hdr.continuity_counter           = m_seq&0x0F;
    len = tp_fmt( b, &hdr );
    // Payload start immediately
    b[len++] = 0;
    // Add the payload
    len += f_tdt( &b[len] );
    // PAD out the unused bytes
    for( i = len; i < TP_LEN; i++ )
    {
        b[i] = 0xFF;
    }
    // Update the continuity counter
    m_seq = (m_seq+1)&0x0F;
}
void tdt_dvb( void )
{
    // Place marker in the buffer queue
    //tdt_pkt[0] = TDT_FLAG;
    tdt_fmt( tdt_pkt );// The time will be behind by a few seconds
    send_tp( tdt_pkt );
}
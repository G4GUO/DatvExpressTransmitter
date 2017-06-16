#include "stdafx.h"
#include <stdint.h>
#include "Dvb.h"
#include "tp.h"

uint8_t m_null_pkt[188];

void null_fmt( void )
{
    int len,i;
    tp_hdr hdr;

    hdr.transport_error_indicator    = 0;
    hdr.payload_unit_start_indicator = 0;
    hdr.transport_priority           = 0;
    hdr.pid                          = NULL_PID;
    hdr.transport_scrambling_control = 0;
    hdr.adaption_field_control       = ADAPTION_PAYLOAD_ONLY;
    hdr.continuity_counter           = 0;
    len = tp_fmt( m_null_pkt, &hdr );
    // PAD out the unused bytes
    for( i = len; i < TP_LEN; i++ )
    {
        m_null_pkt[i] = 0xFF;
    }
}
uint8_t *null_pkt( void )
{
    update_cont_counter( m_null_pkt );
    return m_null_pkt;
}
//
// This module formats and sends PAT tables
//
#include "stdafx.h"
#include <stdint.h>
#include "Dvb.h"
#include "tp.h"

static uint8_t m_pat_pkt[188];

int tp_pat_fmt( uint8_t *b, tp_pat *p )
{
    int i,len;

    b[0] = 0x00;
    b[1] = 0;
    if( p->section_syntax_indicator ) b[1] |= 0x80;
    b[1] |= 0x30;

    b[3]  = (p->transport_stream_id>>8);
    b[4]  = (p->transport_stream_id&0xFF);
    b[5]  = 0xC0;
    b[5] |= (p->version_number<<1);
    if(p->current_next_indicator) b[5] |= 0x01;
    b[6]  = p->section_number;
    b[7]  = p->last_section_number;
    len = 8;
    for( i = 0; i < p->nr_table_entries; i++ )
    {
        b[len++] = p->entry[i].program_number>>8;
        b[len++] = p->entry[i].program_number&0xFF;
        b[len] = 0xE0;
        b[len++] |= p->entry[i].pid>>8;
        b[len++]  = p->entry[i].pid&0xFF;
    }
    // length from the field after the length and including the CRC
    b[1]  |= ((len+1)>>8);
    b[2]   = (len+1)&0xFF;
    len = crc32_add( b, len );

    return len;
}
void pat_fmt( int stream_id, int nit_pid, int pmt_pid, int program_nr )
{
    int i,len;
    tp_hdr hdr;
    tp_pat pat;

    hdr.transport_error_indicator    = 0;
    hdr.payload_unit_start_indicator = 1;
    hdr.transport_priority           = 0;
    hdr.pid                          = PAT_PID;//PAT table pid
    hdr.transport_scrambling_control = 0;
    hdr.adaption_field_control       = ADAPTION_PAYLOAD_ONLY;
    hdr.continuity_counter           = 0;
    len = tp_fmt( m_pat_pkt, &hdr );

    // Tables follow immediately
    m_pat_pkt[len++]                   = 0;

    // Add the pat table
    pat.section_syntax_indicator = 1;
    pat.transport_stream_id      = stream_id;
    pat.version_number           = 2;
    pat.current_next_indicator   = 1;
    pat.section_number           = 0;
    pat.last_section_number      = 0;
    pat.nr_table_entries         = 2;
    // Entries
    pat.entry[0].program_number  = 0;
    pat.entry[0].pid             = nit_pid;//default value

    pat.entry[1].program_number  = program_nr;// Only one program
    pat.entry[1].pid             = pmt_pid;


    // format
    len += tp_pat_fmt( &m_pat_pkt[len], &pat );
    // PAD out the unused bytes
    for( i = len; i < TP_LEN; i++ )
    {
        m_pat_pkt[i] = 0xFF;
    }
}
//
// Send a PAT transport packet via dvb encoder
//
void pat_dvb( void )
{
    send_tp( m_pat_pkt );
    update_cont_counter( m_pat_pkt );
}
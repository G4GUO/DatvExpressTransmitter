#include "stdafx.h"
#include <stdint.h>
#include "Dvb.h"
#include "tp.h"

uint8_t m_sdt_seq;
uint8_t sdt_pkt[188];

int f_sdts( uint8_t *b, sdt_section *p )
{
    int len,i;
    b[0] = p->service_id>>8;
    b[1] = p->service_id&0xFF;
    b[2] = 0xFC;
    if( p->eit_schedule_flag ) b[2] |= 0x02;
    if( p->eit_present_follow_flag ) b[2] |= 0x01;
    b[3] = p->running_status<<5;
    if( p->free_ca_mode ) b[3] |= 0x10;
    len = 5;
    for( i = 0; i < p->nr_descriptors; i++ )
    {
        len += add_si_descriptor( &b[len], &p->descriptor[i] );
    }
    b[3] |= (len-5)>>8;
    b[4]  = (len-5)&0xFF;
    return len;
}
int f_sdt( uint8_t *b, service_description_section *p )
{	
    int i,len;
    b[0] = 0x42;
    b[1] = 0xF0;
    if( p->section_syntax_indicator ) b[1] |= 0x80;
    b[1] |= 0x40;
    b[1] |= 0x30;
    b[3] = p->transport_stream_id>>8;
    b[4] = p->transport_stream_id&0xFF;
    b[5] = 0xC0;
    b[5] |= p->version_number<<1;
    if( p->current_next_indicator ) b[5] |= 0x01;
    b[6]  = p->section_number;
    b[7]  = p->last_section_number;
    b[8]  = p->original_network_id>>8;
    b[9] = p->original_network_id&0xFF;
    b[10] = 0xFF;
    len = 11;
    for( i = 0; i <= p->last_section_number; i++ )
    {
        len += f_sdts( &b[len], &p->section[i]);
    }
    // length from the field after the length and including the CRC
    b[1]  |= ((len-3+CRC_32_LEN)>>8);
    b[2]   = (len-3+CRC_32_LEN)&0xFF;
    len = crc32_add( b, len );

    return len;
}
//
// Format the header and payload of the SDT packet
//
void sdt_fmt( int stream_id, int network_id, int service_id, char *service_provider_name, char *service_name )
{
    int i,len;
    tp_hdr hdr;
    service_description_section sds;

	hdr.transport_error_indicator    = TRANSPORT_ERROR_FALSE;
    hdr.payload_unit_start_indicator = PAYLOAD_START_TRUE;
    hdr.transport_priority           = TRANSPORT_PRIORITY_LOW;
    hdr.pid                          = SDT_PID;//PAT table pid
    hdr.transport_scrambling_control = SCRAMBLING_OFF;
    hdr.adaption_field_control       = ADAPTION_PAYLOAD_ONLY;
    hdr.continuity_counter           = m_sdt_seq = 0;
    len = tp_fmt( sdt_pkt, &hdr );

    // Payload start immediately
    sdt_pkt[len++] = 0;

    // Add the payload
    sds.transport_stream_id    = stream_id;
    sds.version_number         = 2;
    sds.current_next_indicator = 1;
    sds.original_network_id    = network_id;
    sds.section_number         = 0;
    sds.last_section_number    = 0;

    // First section
    sds.section[0].service_id              = service_id;
    sds.section[0].eit_schedule_flag       = 1;
    sds.section[0].eit_present_follow_flag = 1;
    sds.section[0].running_status          = 4;
    sds.section[0].free_ca_mode            = 0;
    sds.section[0].nr_descriptors          = 1;
    // Add the descriptor to the section
    sds.section[0].descriptor[0].tag      = SI_DESC_SERVICE;
    sds.section[0].descriptor[0].sd.type  = SVC_DIGITAL_TV;

    memcpy( sds.section[0].descriptor[0].sd.provider, service_provider_name, strlen(service_provider_name));
    sds.section[0].descriptor[0].sd.provider_length = strlen(service_provider_name);

    memcpy( sds.section[0].descriptor[0].sd.name, service_name, strlen(service_name));
    sds.section[0].descriptor[0].sd.name_length = strlen(service_name);

    len += f_sdt( &sdt_pkt[len], &sds );
    // PAD out the unused bytes
    for( i = len; i < TP_LEN; i++ )
    {
        sdt_pkt[i] = 0xFF;
    }
}
void sdt_dvb( void )
{
    send_tp( sdt_pkt );
    update_cont_counter( sdt_pkt );
}
#include "stdafx.h"
#include <stdint.h>
#include <time.h>
#include "Dvb.h"
#include "tp.h"

uint32_t dvb_crc32_calc( uint8_t *b, int len )
{
    int i,n,bit;
    unsigned long crc = 0xFFFFFFFF;

    for( n = 0; n < len; n++ )
    {
        for( i=0x80; i; i>>=1 )
        {
            bit = ( ( ( crc&0x80000000 ) ? 1 : 0 ) ^ ( (b[n]&i) ? 1 : 0 ) );
            crc <<= 1;
            if(bit) crc ^= 0x04C11DB7;
        }
    }
    return crc;
}

int crc32_add( uint8_t *b, int len )
{
    unsigned long crc = dvb_crc32_calc( b, len );

    b[len++] = (crc>>24)&0xFF;
    b[len++] = (crc>>16)&0xFF;
    b[len++] = (crc>>8)&0xFF;
    b[len++] = (crc&0xFF);
    return len;
}
void update_cont_counter( uint8_t *b )
{
        uint8_t c;

        c = b[3]&0x0F;
        c = (c+1)&0x0F;
        b[3] = (b[3]&0xF0) | c;
}
void set_cont_counter( uint8_t *b, uint8_t c )
{
        b[3] = (b[3]&0xF0) | (c&0x0F);
}
void to_bcd( int v, uint8_t *b )
{
        b[0]  = ((v/10)<<4)&0xF0;
        b[0] |= (v%10)&0x0F;
}

int  dvb_si_add_time( uint8_t *b, dvb_si_time *t)
{
    int l,d,m,y;
    int mjd;

    d = t->day;
    m = t->month;
    y = t->year;

    if((m == 1)||(m == 2 ))
            l = 1;
    else
            l = 0;

    mjd = 14956 + d + (int)((y - l)*365.25f) + (int)((m+1+(l*12))*30.6001);
    b[0] = (mjd>>8)&0xFF;
    b[1] = mjd&0xFF;

    to_bcd( t->hour,    &b[2] );
    to_bcd( t->minute,  &b[3] );
    to_bcd( t->second,  &b[4] );
    return 5;
}

int  dvb_si_add_duration( uint8_t *b, dvb_si_duration *d )
{
    to_bcd( d->hour,    &b[0] );
    to_bcd( d->minute,  &b[1] );
    to_bcd( d->second,  &b[2] );
    return 3;
}

void dvb_si_system_time( dvb_si_time *st )
{
    time_t t;
    struct tm    gt;

    t  = time(NULL);
    gmtime_s( &gt, &t );

    // modify to correct format
    st->day    = gt.tm_mday;
    st->month  = gt.tm_mon + 1;
    st->year   = gt.tm_year;
    st->hour   = gt.tm_hour;
    st->minute = gt.tm_min;
    st->second = gt.tm_sec;
}
int si_service_desc_fmt( uint8_t *b, si_service_descriptor *sd )
{
	int len = 0;
	b[len++] = SI_DESC_SERVICE;
	len++;
	b[len++] = sd->type;

	b[len++] = sd->provider_length;
	memcpy(&b[len],sd->provider, sd->provider_length );
	len += sd->provider_length;

	b[len++] = sd->name_length;
	memcpy(&b[len],sd->name, sd->name_length );
	len += sd->name_length;
	b[1] = len-2;

	return len;
}
int si_network_name_desc_fmt( uint8_t *b, si_net_name_desc *nd )
{
	int len = 0;
	b[len++] = SI_DESC_NET_NAME;
	b[len++] = nd->name_length;
	memcpy(&b[len],nd->name, nd->name_length );
	len += nd->name_length;
	return len;
}

int si_svc_lst_desc_fmt( uint8_t *b, si_svc_list_desc *ld )
{
	int i;
	int len = 0;
	b[len++] = SI_DESC_SVC_LST;
	len++;
	for( i = 0; i < ld->table_length; i++ )
	{
		b[len++] = ld->entry[i].service_id>>8;
		b[len++] = ld->entry[i].service_id&0xFF;
		b[len++] = ld->entry[i].service_type;
	}
	b[1] = (len-2);
	return len;
}
//
// Description of the event
//
int si_extended_event_desc_fmt( uint8_t *b, si_ee_descriptor *ed )
{
	int i,n;
	int len = 0;
	b[len++] = SI_DESC_EXT_EVNT;
	b[len++] = ed->length;
	b[len++] = ed->number;
	b[len++] = ed->last_number;
	b[len++] = (ed->iso_639_language_code>>16)&0xFF;
	b[len++] = (ed->iso_639_language_code>>8)&0xFF;
	b[len++] = (ed->iso_639_language_code)&0xFF;
	b[len++] = ed->length_of_items;
	for( i = 0; i < ed->length_of_items; i++ )
	{
		b[len++] = ed->items[i].item_description_length;
		for( n = 0; n < ed->items[i].item_description_length; n++ )
		{
			b[len++] = ed->items[i].item_desc_text[n];
		}
		b[len++] = ed->items[i].item_description_length;
		for( n = 0; n < ed->items[i].item_length; n++ )
		{
			b[len++] = ed->items[i].item_text[n];
		}
	}
	b[len++] = ed->text_length;
	for( n = 0; n < ed->text_length; n++ )
	{
		b[len++] = ed->text[n];
	}
	return len;
}
//
// Description of the event
//
int si_short_event_desc_fmt( uint8_t *b, si_se_descriptor *ed )
{
    int i,n;
    int len = 0;
    b[len++] = SI_DESC_SE;
    len++;
    b[len++] = (ed->iso_639_language_code>>16)&0xFF;
    b[len++] = (ed->iso_639_language_code>>8)&0xFF;
    b[len++] = (ed->iso_639_language_code)&0xFF;
    // Get the length of the event name
    n = strlen(ed->event_name);
    b[len++] = n;
    // Add the name
    for( i = 0; i < n; i++ )
    {
        b[len++] = ed->event_name[i];
    }
    n = strlen(ed->event_text);
    b[len++] = n;
    for( i = 0; i < n; i++ )
    {
       b[len++] = ed->event_text[i];
    }
    // Set up the length field
    b[1] = len - 2;
    return len;
}

int si_content_event_desc_fmt( uint8_t *b, si_ct_descriptor *ct )
{
    int len = 0;
    b[len++] = SI_DESC_CONTENT;
    b[len++] = ct->nr_items*2;// length
    for( int i = 0; i < ct->nr_items; i++ )
    {
        b[len++] = (ct->item[i].l1_nibble<<4)|(ct->item[i].l2_nibble);
        b[len++] = (ct->item[i].u1_nibble<<4)|(ct->item[i].u2_nibble);
    }
    return len;
}

int si_teletext_desc_fmt( uint8_t *b, si_tt_descriptor *tt )
{
    int len = 0;
    b[len++] = SI_DESC_TELETEXT;
    b[len++] = tt->nr_items*5;// length
    for( int i = 0; i < tt->nr_items; i++ )
    {
        b[len++] = tt->item[i].iso_language_code[0];
        b[len++] = tt->item[i].iso_language_code[1];
        b[len++] = tt->item[i].iso_language_code[2];
        b[len]   = tt->item[i].teletext_type<<3;
        b[len++] |= tt->item[i].teletext_magazine_number&0x07;
        b[len++] = tt->item[i].teletext_page_number;
    }
    return len;
}

int add_si_descriptor( uint8_t *b, si_desc *d )
{
    int len = 0;

    switch( d->tag )
    {
        case SI_DESC_SERVICE:
            d->sd.tag = d->tag;
            len = si_service_desc_fmt( b, &d->sd );
            break;
        case SI_DESC_NET_NAME:
            d->nnd.tag = d->tag;
            len = si_network_name_desc_fmt( b, &d->nnd );
            break;
        case SI_DESC_SVC_LST:
            d->sld.tag = d->tag;
            len = si_svc_lst_desc_fmt( b, &d->sld );
            break;
        case SI_DESC_EXT_EVNT:
            d->eed.tag = d->tag;
            len = si_extended_event_desc_fmt( b, &d->eed );
            break;
        case SI_DESC_SE:
            d->sed.tag = d->tag;
            len = si_short_event_desc_fmt( b, &d->sed );
            break;
        case SI_DESC_CONTENT:
            d->sctd.tag = d->tag;
            len = si_content_event_desc_fmt( b, &d->sctd );
            break;
        case SI_DESC_TELETEXT:
            d->ttd.tag = d->tag;
            len = si_teletext_desc_fmt( b, &d->ttd );
            break;
        default:
            break;
    }
    return len;
}
//
// Format and send a Transport packet 188.
// Pass the buffer to send, the length of the buffer
//
// This is for SI packets
//

void f_create_si_first( uint8_t *pkt, uint8_t *b, int pid, int len )
{
    int  l;
    tp_hdr hdr;

    // Header
    hdr.transport_error_indicator    = TRANSPORT_ERROR_FALSE;
    hdr.payload_unit_start_indicator = PAYLOAD_START_TRUE;
    hdr.transport_priority           = TRANSPORT_PRIORITY_LOW;
    hdr.pid                          = pid;
    hdr.transport_scrambling_control = SCRAMBLING_OFF;
    hdr.adaption_field_control       = ADAPTION_PAYLOAD_ONLY;
    hdr.continuity_counter           = 0;
    l = tp_fmt( pkt, &hdr );
    pkt[l++] = 0;
    if( len > SI_PAYLOAD_LENGTH-1 ) len = SI_PAYLOAD_LENGTH-1;
    memcpy( &pkt[l], b, len );
    l += len;
    for( int i = l; i < TP_LEN; i++ )
    {
        pkt[i] = 0xFF;
    }
}
void f_create_si_next( uint8_t *pkt, uint8_t *b, int pid )
{
    int  l;
    tp_hdr hdr;

    // Header
    hdr.transport_error_indicator    = TRANSPORT_ERROR_FALSE;
    hdr.payload_unit_start_indicator = PAYLOAD_START_FALSE;
    hdr.transport_priority           = TRANSPORT_PRIORITY_LOW;
    hdr.pid                          = pid;
    hdr.transport_scrambling_control = SCRAMBLING_OFF;
    hdr.adaption_field_control       = ADAPTION_PAYLOAD_ONLY;
    hdr.continuity_counter           = 0;
    l   = tp_fmt( pkt, &hdr );
    //pkt[l++] = 0;
    memcpy( &pkt[l], b, SI_PAYLOAD_LENGTH );
}

void f_create_si_last( uint8_t *pkt, uint8_t *b, int pid, int len )
{
    int  l;
    tp_hdr hdr;

    // Header
    hdr.transport_error_indicator    = TRANSPORT_ERROR_FALSE;
    hdr.payload_unit_start_indicator = PAYLOAD_START_FALSE;
    hdr.transport_priority           = TRANSPORT_PRIORITY_LOW;
    hdr.pid                          = pid;
    hdr.transport_scrambling_control = SCRAMBLING_OFF;
    hdr.adaption_field_control       = ADAPTION_PAYLOAD_ONLY;
    hdr.continuity_counter           = 0;
    l = tp_fmt( pkt, &hdr );
    //pkt[l++] = 0;
    memcpy( &pkt[l], b, len );
    l += len;
    for( int i = l; i < TP_LEN; i++ )
    {
        pkt[i] = 0xFF;
    }
}
//
// Send a sequence of SI packets.
// This allows larger tables i.e for EPG
// The sequence number will be set at transmission time
//
void f_create_si_seq( tp_si_seq *cblk )
{
    int payload_remaining;
    int offset;
    // First packet in sequence
    payload_remaining = cblk->ibuff_len;
    cblk->nr_frames   = 0;
    offset            = 0;
    f_create_si_first( cblk->frames[cblk->nr_frames], &cblk->inbuff[offset], cblk->pid, payload_remaining );
    //cblk->seq_count = 0;
    cblk->nr_frames++;
    payload_remaining -= (SI_PAYLOAD_LENGTH-1);
    offset            += (SI_PAYLOAD_LENGTH-1);

    while( payload_remaining >= SI_PAYLOAD_LENGTH )
    {
        f_create_si_next( cblk->frames[cblk->nr_frames], &cblk->inbuff[offset], cblk->pid );
        cblk->nr_frames++;
        payload_remaining -= SI_PAYLOAD_LENGTH;
        offset            += SI_PAYLOAD_LENGTH;
    }

    if( payload_remaining > 0 )
    {
        f_create_si_last( cblk->frames[cblk->nr_frames], &cblk->inbuff[offset], cblk->pid, payload_remaining );
        cblk->nr_frames++;
        payload_remaining -= SI_PAYLOAD_LENGTH;
    }
}
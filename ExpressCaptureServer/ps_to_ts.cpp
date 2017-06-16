//
// used to generate a transport stream from a program stream
//
#include "stdafx.h"
#include <memory.h>
#include <stdint.h>
#include "Dvb.h"
#include "tp.h"

static uint8_t g_video_seq;
static uint8_t g_audio_seq;
static uint8_t g_pcr_seq;

static uint8_t m_tp_pkt[188];
// Mutexes
static CRITICAL_SECTION g_mutex;

void ts_write_transport_queue( uint8_t *tp ){
	uint8_t *b;
	EnterCriticalSection( &g_mutex );
    tp_log( tp );
    b = alloc_copy_tx_buff(tp);// Copy to a buffer
    post_tx_buff(b);// Queue the buffer for transmission
	pcr_increment_clock();// Use the bitstream to driver timers
	LeaveCriticalSection( &g_mutex );
}

void send_tp(uint8_t *tp ){
	ts_write_transport_queue( tp );
	// Only sent if needed
	send_pcr();
}
//
// Format and send a Transport packet 188.
// Pass the buffer to send, the length of the buffer
// and the type (audio, video, system)
//
// This is for PES packets in transport streams
//
//
// Format a transport packet
//
int tp_fmt( uint8_t *b, tp_hdr *hdr )
{
        int len;

        b[0] = 0x47;
        b[1] = 0;
        if( hdr->transport_error_indicator)     b[1] |= 0x80;
        if( hdr->payload_unit_start_indicator ) b[1] |= 0x40;
        if( hdr->transport_priority )           b[1] |= 0x20;
        // Add the 13 bit pid
        b[1] |= (hdr->pid >> 8 );
        b[2]  = (hdr->pid & 0xFF);
        b[3]  = (hdr->transport_scrambling_control << 6);
        b[3] |= (hdr->adaption_field_control << 4);
        b[3] |= (hdr->continuity_counter)&0x0F;
        len = 4;
        return len;
}
//
// Send a PCR with no payload
//
void send_pcr(void){
    if( get_video_pid() == get_pcr_pid())
		pcr_dvb(g_video_seq);
	else
		pcr_dvb(g_pcr_seq);
}

int f_send_pes_first_tp( uint8_t *b, uint32_t pid, uint8_t c, bool pcr )
{
    int  len;
    tp_hdr hdr;

    // Header
    hdr.transport_error_indicator    = 0;
    hdr.payload_unit_start_indicator = 1;
    hdr.transport_priority           = 0 ;
    hdr.pid                          = pid;
    hdr.transport_scrambling_control = 0;
    hdr.continuity_counter           = c;

    if( pcr == true )
    {
        // Adaption and payload (PCR)
        hdr.adaption_field_control  = ADAPTION_BOTH;
        len = tp_fmt( m_tp_pkt, &hdr );
        // Add PCR field
        len += pcr_fmt( &m_tp_pkt[len], &hdr );
    }
    else
    {
        // Payload only
        hdr.adaption_field_control  = ADAPTION_PAYLOAD_ONLY;
        len = tp_fmt( m_tp_pkt, &hdr );
    }
    // Add the payload
    memcpy( &m_tp_pkt[len], b, TP_LEN-len );

	send_tp( m_tp_pkt );
    return (TP_LEN-len);
}
//
// Alter the adaption field flag
// length and add n stuff bytes
void add_pes_stuff_bytes( uint8_t *b, int n )
{
     int i;

     if( n == 1)
     {
         // only one byte required so set the
         // length field to 0
         b[4] = 0;
     }
     else
     {
         b[4] = n-1;// set length field
         b[5] = 0;// Clear adaption flags
         for( i = 0; i < n-2; i++ )
         {
             b[6+i] = 0xFF;
         }
    }
}

int f_send_pes_next_tp( uint8_t *b,  uint32_t pid, uint8_t c, bool pcr )
{
    int  len;
    tp_hdr hdr;

    // Header
    hdr.transport_error_indicator    = 0;
    hdr.payload_unit_start_indicator = 0;
    hdr.transport_priority           = 0 ;
    hdr.pid                          = pid;
    hdr.transport_scrambling_control = 0;
    hdr.continuity_counter           = c;

    if( pcr == true )
    {
        // Adaption and payload (PCR)
        hdr.adaption_field_control  = ADAPTION_BOTH;
        len = tp_fmt( m_tp_pkt, &hdr );
        // Add PCR field
        len += pcr_fmt( &m_tp_pkt[len], &hdr );
    }
    else
    {
        // Payload only
        hdr.adaption_field_control  = ADAPTION_PAYLOAD_ONLY;
        len = tp_fmt( m_tp_pkt, &hdr );
    }
    // Add the payload
    memcpy( &m_tp_pkt[len], b, TP_LEN-len );

	send_tp( m_tp_pkt );
    return (TP_LEN-len);
}

void f_send_pes_last_tp( uint8_t *b, int bytes, uint32_t pid, uint8_t c )
{
    int  len,stuff;
    tp_hdr hdr;

    // Header
    hdr.transport_error_indicator    = 0;
    hdr.payload_unit_start_indicator = 0;
    hdr.transport_priority           = 0 ;
    hdr.pid                          = pid;
    hdr.transport_scrambling_control = 0;
    hdr.continuity_counter           = c;

    hdr.adaption_field_control  = ADAPTION_BOTH;//Adaption and Payload
    len = tp_fmt( m_tp_pkt, &hdr );
    // Add the stuff
    stuff = TP_LEN-(len+bytes);
    add_pes_stuff_bytes( m_tp_pkt, stuff );
    len += stuff;
    // Add the payload
    memcpy( &m_tp_pkt[len], b, bytes );
    send_tp( m_tp_pkt );
}
//
// Create transport packets from the program stream pulled from the video input
// len is the total length of the program packet.
//
void ps_to_ts_video( void )
{
    int payload_remaining;
    uint8_t b[188];
    int len = ps_get_length();
	
    // First packet in sequence
    // The bytes read (from the capture device) is the number of bytes available for a payload
    // in the first transport packet. this will include an adaptation field
    // containing the PCR derived from the last SCR and bitstream updates
    int overhead       = 4;
    bool ph            = false;
    bool pcr_in_video  = false;

    if( get_video_pid() == get_pcr_pid())
    {
        pcr_in_video = true;
        ph = is_pcr_update();
        if(ph == true) overhead = (4 + 8);
    }

    ps_read( b, TP_LEN - overhead);

    g_video_seq = (g_video_seq+1)&0x0F;
    payload_remaining = len - f_send_pes_first_tp( b, get_video_pid(),g_video_seq, ph);// Add adaption field

    // Send the middle
    while( payload_remaining >= PES_PAYLOAD_LENGTH )
    {
        ph       = false;

        if(pcr_in_video  == true)
        {
           if((ph = is_pcr_update())==true)
               overhead = (4 + 8);
		   else
			   overhead = 4;
        }

        ps_read( b, TP_LEN - overhead);
        g_video_seq = (g_video_seq+1)&0x0F;
        payload_remaining -= f_send_pes_next_tp( b, get_video_pid(), g_video_seq, ph );
    }
    // Send the last
    if( payload_remaining > 0 )
    {
        ps_read( b, payload_remaining );
        g_video_seq = (g_video_seq+1)&0x0F;
        f_send_pes_last_tp( b, payload_remaining, get_video_pid(), g_video_seq );
    }
    timer_work();
}
//
// Create transport packets from the program stream pulled from the audio input
//
// Returns the number of stuff bytes that have been added.
//
void ps_to_ts_audio( void )
{

	int payload_remaining;
    uint8_t b[188];
    int len = ps_get_length();
    // First packet in sequence
    int overhead = 4;
    ps_read( b, TP_LEN - overhead);
    payload_remaining = len - (TP_LEN - overhead);

    g_audio_seq = (g_audio_seq+1)&0x0F;
    f_send_pes_first_tp( b, get_audio_pid(), g_audio_seq, false );

	while( payload_remaining >= PES_PAYLOAD_LENGTH )
	{
        ps_read( b,  PES_PAYLOAD_LENGTH );
        g_audio_seq = (g_audio_seq+1)&0x0F;
        f_send_pes_next_tp( b, get_audio_pid(), g_audio_seq, false );
        payload_remaining -= PES_PAYLOAD_LENGTH;
	}

	if( payload_remaining > 0 )
	{
        ps_read( b, payload_remaining );
        g_audio_seq = (g_audio_seq+1)&0x0F;
        f_send_pes_last_tp( b, payload_remaining, get_audio_pid(), g_audio_seq );
	}
    timer_work();
}
void ps_to_ts_init(void){
	InitializeCriticalSection(&g_mutex);
}
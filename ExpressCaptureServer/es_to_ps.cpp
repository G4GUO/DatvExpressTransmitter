//
// used to extract and insert elementary streams
//
#include "stdafx.h"
#include <memory.h>
#include <stdint.h>

#define ESR_FLAG      0x10
#define ORIGINAL_FLAG 0x01
#define PRIORITY_FLAG 0x08
#define ESR_FIELD_LEN 3
#define MB_BUFFER_SIZE 65000

static uint32_t m_video_bitrate;
static uint32_t m_audio_bitrate;
static uint8_t m_nb[MB_BUFFER_SIZE*10];
static int   m_rbp,m_wbp;

//
// See Table 2-17 ISO 13818-1
//
//
// Add TS field
//
void ps_add_pts_dts( uint8_t *b, int64_t ts )
{
    b[0] &= 0xF0;
    // 4 bits 0010 or 0011
    if(ts & 0x100000000) b[0] |= 0x08;
    if(ts & 0x080000000) b[0] |= 0x04;
    if(ts & 0x040000000) b[0] |= 0x02;
    b[0] |= 0x01; // Marker bit
    b[1] = 0;
    if(ts & 0x020000000) b[1] |= 0x80;
    if(ts & 0x010000000) b[1] |= 0x40;
    if(ts & 0x008000000) b[1] |= 0x20;
    if(ts & 0x004000000) b[1] |= 0x10;
    if(ts & 0x002000000) b[1] |= 0x08;
    if(ts & 0x001000000) b[1] |= 0x04;
    if(ts & 0x000800000) b[1] |= 0x02;
    if(ts & 0x000400000) b[1] |= 0x01;
    b[2] = 0;
    if(ts & 0x000200000) b[2] |= 0x80;
    if(ts & 0x000100000) b[2] |= 0x40;
    if(ts & 0x000080000) b[2] |= 0x20;
    if(ts & 0x080040000) b[2] |= 0x10;
    if(ts & 0x000020000) b[2] |= 0x08;
    if(ts & 0x000010000) b[2] |= 0x04;
    if(ts & 0x000008000) b[2] |= 0x02;
    b[2] |= 0x01;// Marker bit
    b[3] = 0;
    if(ts & 0x000004000) b[3] |= 0x80;
    if(ts & 0x000002000) b[3] |= 0x40;
    if(ts & 0x000001000) b[3] |= 0x20;
    if(ts & 0x000000800) b[3] |= 0x10;
    if(ts & 0x000000400) b[3] |= 0x08;
    if(ts & 0x000000200) b[3] |= 0x04;
    if(ts & 0x000000100) b[3] |= 0x02;
    if(ts & 0x000000080) b[3] |= 0x01;
    b[4] = 0;
    if(ts & 0x000000040) b[4] |= 0x80;
    if(ts & 0x000000020) b[4] |= 0x40;
    if(ts & 0x000000010) b[4] |= 0x20;
    if(ts & 0x000000008) b[4] |= 0x10;
    if(ts & 0x000000004) b[4] |= 0x08;
    if(ts & 0x000000002) b[4] |= 0x04;
    if(ts & 0x000000001) b[4] |= 0x02;
    b[4] |= 0x01;// Marker bit
}
//
// Add PTS and DTS fields, returns offset into array after the fields
//
int ps_add_pts_dts( uint8_t *b, int64_t pts, int64_t dts )
{
    if((pts > 0)&&(dts > 0))
    {
        // Both PTS and DTS required
        b[7] |= 0xC0;
        b[8] += 10; // Extra header
        b[9] = 0x30;
        ps_add_pts_dts( &b[9],  pts );
        b[14] = 0x10;
        ps_add_pts_dts( &b[14], dts );
        return 19;
    }
    else
    {
        if( pts > 0 )
        {
            // PTS only required
            b[7] |= 0x80;
            b[8] += 5; // extra header length
            b[9] = 0x20;
            ps_add_pts_dts( &b[9], pts );
            return 14;
        }
        else
        {
            // Neither required
            b[7] |= 0x00;
            b[8] += 0; // payload starts straight away, no extra header
            return 9;
        }
    }
}
//
// Add the Elementary Stream rate field
//
int ps_add_esr_field( uint8_t *b, uint32_t rate  )
{
    // Convert to nr of 50 bytes / sec
	uint32_t ra = rate/400;
	if (rate % 400) ra++;
	ra = (ra << 1);
	ra = ra  | 0x801;
	b[0] = (uint8_t)(ra>>16);
	b[1] = (uint8_t)((ra>>8)&0xFF);
    b[2] = (uint8_t)((ra)&0xFF);
    return ESR_FIELD_LEN;
}
//
// Read from buffer into memory
//
void ps_write_from_memory( uint8_t *b, int len )
{
    memcpy( &m_nb[m_wbp], b, len);
    m_wbp += len;
}
void ps_reset( void )
{
    m_wbp   = 0;// reset the write position
    m_rbp   = 0;// reset the read position
}
// Read the buffer in for processing
void ps_read( uint8_t *b, int len )
{
    memcpy( b, &m_nb[m_rbp], len);
    m_rbp += len;
}
int ps_get_length( void )
{
    return m_wbp;
}
uint8_t *ps_get_packet(void)
{
    return m_nb;
}

int ps_process( void )
{
    int len = m_wbp;
    // Return the new length
    return len;
}

//
// form a pes packet from the video elementary stream
//
void ps_video_el_to_ps( uint8_t *b, int length, int32_t bitrate, int64_t pts, int64_t dts )
{
    int len,start;
    uint8_t d[40];
    // Convert to 90 KHz clock
	pts = pts/300;
	dts = dts/300;
//    for( int i = 0; i < length; i++ ) printf("%.2x ",b[i]);
//    printf("\n\n");
    ps_reset();
    d[0] = 0x00;
    d[1] = 0x00;
    d[2] = 0x01;
    d[3] = 0xE0;
    // Length field 2 bytes
    d[6] = 0x84 | ORIGINAL_FLAG | PRIORITY_FLAG;// Data aligned
    d[7] = 0x00;// Clear 2nd byte of flag field
    d[8] = 0;// Zero length
    ps_reset();
    start = ps_add_pts_dts( d, pts, dts );
	// Extract the ES rate
	//int32_t esr = b[8];
	//esr = (esr << 8) | b[9];
	//esr = (esr << 2) | (b[10] >> 6);
	//esr *= 400;
    // Add the ESR field
//    d[7] |= ESR_FLAG;
//    d[8] += ESR_FIELD_LEN;
//    start += ps_add_esr_field( &d[start], bitrate  );
    // Set the length of the packet
    len = length + start - 6;
    d[4] = (len)>>8;
    d[5] = (len)&0xFF;
    ps_write_from_memory( d, start );
    ps_write_from_memory( b, length );
}
//
// form a pes packet from the video elementary stream
//
void ps_audio_el_to_ps( uint8_t *b, int length, int32_t bitrate, int64_t pts, int64_t dts )
{
    int len,start;
    uint8_t d[40];
	// Convert to 90 KHz clock
	if(pts != -1 ) pts = pts/300;
	if(dts != -1 ) dts = dts/300;

//       for( int i = 0; i < length; i++ ) printf("%.2x ",b[i]);
//        printf("\n\n");
    d[0] = 0x00;
    d[1] = 0x00;
    d[2] = 0x01;
    d[3] = 0xC0;
    // Length field
    d[6] = 0x84 | ORIGINAL_FLAG | PRIORITY_FLAG;// Data aligned
    d[7] = 0x00;// Clear 2nd byte of flag field
    d[8] = 0;// Zero length
    ps_reset();
    start = ps_add_pts_dts( d, pts, dts );
    // Add the ESR field
    d[7] |= ESR_FLAG;
    d[8] += ESR_FIELD_LEN;
    start += ps_add_esr_field( &d[start], bitrate  );
    // Set the length of the packet
    len = length + start - 6;
    d[4] = (len>>8);
    d[5] = (len&0xFF);
    ps_write_from_memory( d, start );
    ps_write_from_memory( b, length );
}
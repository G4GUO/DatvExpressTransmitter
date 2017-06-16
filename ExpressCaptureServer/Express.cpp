#include "stdafx.h"
#include <stdio.h>
#include "express.h"
#include "lusb0_usb.h"
#include "Dvb.h"
#include "error_codes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static usb_dev_handle *m_handle;
static struct usb_device *m_usbdev;
static unsigned int  m_xfrs_in_progress;
static int           m_status;
static int           m_initialised;
static unsigned char m_ancillary;
static int           m_express_status;
static bool          m_si570_fitted;
static int           m_tx_hardware;
static int           m_sample_block_size;
static UCHAR         m_express_ports;// Control of the IO
static CString m_status_s;
BOOL m_status_tx;
//
// Amount of buffering for the bulk transfer
//
#define NR_XFER_BUFFS 40
typedef enum{BTYPE_TS,BTYPE_16_BIT_SAMPLE,BTYPE_8_BIT_SAMPLE};

typedef struct{
	void *context;
	void *buffer;
	int type;
	int inuse;
}Ctx;

int m_index;
Ctx m_context[NR_XFER_BUFFS];

//
// Create a set of free contexts i.e NULL ptrs
//
void usb_context_init( void )
{
	for( int i = 0; i < NR_XFER_BUFFS; i++ ){ 
		m_context[i].context = NULL;
        usb_bulk_setup_async(m_handle, &m_context[i].context, EP2OUT);
		m_context[i].buffer = NULL;
		m_context[i].inuse  = 0;
	}
	m_index = 0;
}
//
// Get a context for the new buffer
//
void *usb_context_get( void *b, int type )
{
	void *ctx;
	// Get a new context, this will overwrite old contexts
	ctx = m_context[m_index].context;
    m_context[m_index].buffer = b;
    m_context[m_index].inuse = 1;
	m_context[m_index].type = type;
	// Point to the oldest context 
	m_index = (m_index + 1)%NR_XFER_BUFFS;
    return ctx;
}
//
// releases the oldest context (blocking)
//
void usb_context_release( void )
{
	// See if context is in use
	if( m_context[m_index].inuse )
	{
		// Yes it is, release it if done
	    if(m_context[m_index].type == BTYPE_TS ) rel_tx_buff((UCHAR*)m_context[m_index].buffer);
        usb_reap_async_nocancel( m_context[m_index].context, USB_TIMEOUT );
        // Place context back in pool
		m_context[m_index].inuse = 0;
	}
}
BOOL express_context_available(void)
{
	if(m_context[m_index].context == NULL)
		return TRUE;
	else
		return FALSE;
}
void express_handle_events(int e )
{
}
//
// version
// 
void express_display_version(void)
{
    const struct usb_version *ver;
	char text[120];
	ver = usb_get_version();

	sprintf_s(text,"USB DLL    version %d.%d.%d.%d\n",ver->dll.major,ver->dll.minor,ver->dll.micro,ver->dll.nano);
	sprintf_s(text,"USB Driver version %d.%d.%d.%d\n",ver->driver.major,ver->driver.minor,ver->driver.micro,ver->driver.nano);
}
void express_error(const char *err) {
	cmd_set_error_text(err);
}
//
// Write to EP1 in blocking mode
//
int ep1_block_write( UCHAR *b, int len )
{
    return usb_bulk_write(m_handle,EP1OUT,(char*)b,len,USB_TIMEOUT);
}
//
// Send high speed buffer
//
void ep2_send_ts_buffer( unsigned char *b, int len )
{
	void *ctx = usb_context_get(b,BTYPE_TS);
	usb_submit_async( ctx, (char *)b, len );
	usb_context_release();
}
void ep2_send_s16_buffer(unsigned char *b, int len)
{
	void *ctx = usb_context_get(b,BTYPE_16_BIT_SAMPLE);
	usb_submit_async(ctx, (char *)b, len);
	usb_context_release();
}
void ep2_send_s8_buffer(unsigned char *b, int len)
{
	void *ctx = usb_context_get(b, BTYPE_8_BIT_SAMPLE);
	usb_submit_async(ctx, (char *)b, len);
	usb_context_release();
}
//
// Used to send I2C messages
//
int express_i2c_bulk_transfer(int ep, unsigned char *b, int len )
{
    return usb_bulk_write(m_handle,ep,(char*)b,len,USB_TIMEOUT);
}
//
// Initialise the fx2 device
//
int express_find( void )
{
	usb_init();

	// Find all busses.
    usb_find_busses();

    // Find all connected devices.
    usb_find_devices();

	struct usb_bus *bus;
    struct usb_device *dev;
	char data[40];
	int res;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == USB_VENDOR
                    && dev->descriptor.idProduct == USB_PROD)
            {
				if(( m_handle = usb_open(dev)) == NULL)        m_status_s = "Open failed";
				res = usb_control_msg(m_handle, 0x80, 0x06, 0x0100, 0x0000, data, 40, USB_TIMEOUT);
				if( usb_set_configuration( m_handle, 1 ) < 0 ) m_status_s = "Config failed";
				if( usb_claim_interface( m_handle, 0   ) < 0 ) m_status_s = "Claim failed";
                if( usb_set_altinterface( m_handle, 1) < 0)    m_status_s = "Alt failed";
	            // Prepare the contexts
	            usb_context_init();
	            express_display_version();
				return 0;
            }
        }
    }
	::MessageBox(NULL,"DATV-Express Hardware not found\nPlease check cables and restart the program",NULL,0);
    return -1;
}
//
// Write a 1 to reset the FX2 device
//
void express_reset(void)
{
    char data = 1;
	
    int res = usb_control_msg(m_handle,0x40,0xA0,0xE600,0,&data,1,USB_TIMEOUT);
    if( res < 0) m_status_s = "Reset failed";
}
//
// Write a 0 to start the FX2 running
//
void express_run(void)
{
    char data = 0;
    int res = usb_control_msg(m_handle,0x40,0xA0,0xE600,0,&data,1,USB_TIMEOUT);
	if( res < 0) m_status_s = "Run failed";
 }
//
// Load the FX2 Firmware
//
int express_firmware_load( FILE *fp)
{
	int res = 0;
    int line = 0;
    const size_t buflen = 1024;
    char buffer[buflen];
    char *b;

    while(fgets(buffer,buflen,fp))
    {
        if(feof(fp)) break;
        line++;
        b = buffer;
        unsigned int nbytes=0,addr=0,type=0;
        if(b[0] == ':')
        {
            b++;
            sscanf_s(b,"%02x%04x%02x",&nbytes,&addr,&type);
            b += 8;
            unsigned int d;
            unsigned char data[64000];
            unsigned char chksum = nbytes+addr+(addr>>8)+type;
            for( unsigned int i = 0; i < nbytes; i++ )
            {
                sscanf_s(b,"%02x",&d);
                b += 2;
                data[i] = (unsigned char)d;
                chksum += d;
            }
            unsigned int fchksum  = 0;
            sscanf_s(b,"%02x",&fchksum);
            if((chksum+fchksum)&0xFF)
            {
                //printf("Firmware file CRC error\n");
				res = -1;
            }
            else
            {
               // Write to RAM
               int n = usb_control_msg(m_handle,0x40,0xA0,addr,0,(char *)data,nbytes,USB_TIMEOUT);
               //printf("Firmware file CRC OK %d %d %d\n",addr,nbytes,n);
            }
        }
    }
	return res;
}
//
// Load the FPGA code
//
int express_fpga_load( FILE *fp )
{
	int res = 0;
    const size_t buflen = 64;
    unsigned char buffer[buflen];
    int nbytes;
	int e;

    while((nbytes=fread(buffer,1,buflen,fp))!=0)
    {
		if((e=ep1_block_write(buffer,nbytes))<0){
           m_status_s = "FPGA Fbulk write failed";
	       res = -1;
		}
    }
	return res;
}
//
// These could be done in a single transaction but as speed is not
// of prime importance they are not being done that way.
//
void express_configure_adrf6755( void )
{
    unsigned char b[3];
    b[0]  = ADRF6755_ADD | I2C_WR;//address

     // set attenuator gain to 47db
     b[1] = 30;b[2]=0x3F;
//     if(libusb_bulk_transfer(m_handle,EP1,b,3,&sbytes,USB_TIMEOUT)< 0)printf("Bulk write failed %d\n",__LINE__);

     // power down the modulator
     b[1]=29;b[2]=0x80;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // power up internal VCO, 1.3 GHz
     b[1]=28;b[2]=0x08;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );
     //
     b[1]=27;b[2]=0x10;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // reserved registers
     b[1]=26;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     b[1]=25;b[2]=0x64;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // enable autocalibration
     b[1]=24;b[2]=0x18;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // lock detect
     b[1]=23;b[2]=0x70;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // reserved registers
     b[1]=22;b[2]=0x80;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );
     b[1]=21;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );
     b[1]=20;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );
     b[1]=19;b[2]=0x80;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );
     b[1]=18;b[2]=0x60;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );
     b[1]=17;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );
     b[1]=16;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );
     b[1]=15;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // enable attenuator
     b[1] = 14;b[2]=0x80;//1b
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     //
     b[1]=13;b[2]=0xE8;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // power up pll
     b[1]=12;b[2]=0x18;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // reserved
     b[1]=11;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     //
     b[1] = 10;b[2]=0x21;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // charge pump
     // CR9
     b[1]=9;b[2]=0xE0;//5mA
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // reserved
     b[1]=8;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // Set frequency to 1300 MHz ?
     // CR7
     b[1]=7;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // CR6 ?
     b[1]=6;b[2]=0x20;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // CR5 Disable R divider
     b[1]=5;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // CR4 reserved
     b[1]=4;b[2]=0x01;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // CR3
     b[1]=3;b[2]=0x05;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // CR2
     b[1]=2;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // CR1
     b[1]=1;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // CR0
     b[1]=0;b[2]=0x00;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

         // power lo monitor
     b[1]=27;b[2]=0x10;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     // wait for PLL lock
     // power up the modulator
     b[1]=29;b[2]=0x81;
     express_i2c_bulk_transfer( EP1OUT, b, 3 );

     express_handle_events( 32 );
}
void express_read_adrf6755_version(void)
{
    if( m_express_status != EXP_OK ) return;
    // Format up into an I2C message for the ADRF6755
    unsigned char msg[4];
    msg[0]  = ADRF6755_ADD | I2C_RD;//address
    msg[1]  = 1;//1 byte to read
    msg[2]  = 33;//CR33
    msg[3]  = 0;//Response will go here
    // Set reg address to read from
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );
    // Read the actual register
    express_i2c_bulk_transfer( EP1IN, msg, 4 );

    express_handle_events( 2 );
}
//
// Code to configure the AD7992 ADC
//
void express_configure_ad7992(void)
{
    if( m_express_status != EXP_OK ) return;
    // Format up into an I2C message for the ADRF6755
    unsigned char msg[3];
    msg[0]  = AD7992_ADD | I2C_WR;//address
    msg[1]  = 0x02;//Configuration register
    msg[2]  = 0x30;//2 channel alternate no alert
    // Set address
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    express_handle_events( 1 );
}
//////////////////////////////////////////////////////////////////////////
//
// Called when system is running
//
//////////////////////////////////////////////////////////////////////////

//
// ARDF6755 Set frequency
//
void express_set_freq( double freq )
{
    double lo;
    unsigned char CR27 = 0;
    unsigned char CR28 = 0;

    if( m_express_status != EXP_OK ) return;

    if( freq > 1155000000 )
    {
        CR28 = 0x00;
        CR27 = 0x10;
        lo = freq;
    }
    else
    {
        if( freq > 577500000)
        {
            CR28 = 0x01;
            CR27 = 0x00;
            lo = freq*2;
        }
        else
        {
            if( freq > 288750000 )
            {
                CR28 = 0x02;
                CR27 = 0x00;
                lo   = freq*4;
            }
            else
            {
                if( freq > 144375000 )
                {
                    CR28 = 0x03;
                    CR27 = 0x00;
                    lo = freq*8;
                }
                else
                {
                    CR28 = 0x04;
                    CR27 = 0x00;
                    lo = freq*16;
                }
            }
        }
    }
    CR27 = CR27 | 0x00;
    CR28 = CR28 | 0x08;//b3=1

    // Divide by the reference frequency Xtal * 2
    double dintp,dfpart;
    dfpart = modf(lo/get_pfd(),&dintp);
    int ipart = (int)dintp;
    // multiply the fractional part by 2^25
    long fpart = (long)(dfpart*33554432.0);
    // Format up into an I2C message for the ADRF6755
    unsigned char msg[3];

    msg[0] = ADRF6755_ADD | I2C_WR;//address
    // Now send it over EP1, should add error checking!
    // The FX2 firmware will send it over I2C

    msg[1] = 28;//CR28
    msg[2] = CR28;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1] = 7;//CR7
    msg[2] = (ipart>>8)&0x0F;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1] = 6;//CR6
    msg[2] = ipart&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1] = 3;//CR3
    msg[2] = 0x04 | ((fpart>>24)&0x01);
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1] = 2;//CR2
    msg[2] = (fpart>>16)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1] = 1;//CR1
    msg[2] = (fpart>>8)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1] = 0;//CR0
    msg[2] = fpart&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1] = 27;//CR27
    msg[2] = CR27;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    express_handle_events( 8 );
}
//
// Routines that affect the FPGA registers
//
int express_set_sr( double sr )
{
    long long val;
	double clk;
	int irate;
	long sr_threshold;
    unsigned char msg[3];
//    printcon("Symbol rate %.2f\n",sr);

    if( sr == 0 ) return -1;
    if( m_express_status != EXP_OK ) return -1;

    if(m_si570_fitted == true )
    {
        sr_threshold = SR_THRESHOLD_SI570_HZ;
    }
    else
    {
		if (get_txmode() == M_DVBS2) 
			sr_threshold = SR_S2_THRESHOLD_HZ;
		else
			sr_threshold = SR_S_THRESHOLD_HZ;
    }

    // Maximum default values
	irate = IRATE2;
	clk = sr * 4 * 2;// 4*2 clock rate

    if( sr < (sr_threshold/256) )
	{
		// We can use x64 interpolator
		irate = IRATE64;
		clk = sr * 4 * 64;// 4*64 clock rate
	}
	else
	{
		if( sr < sr_threshold/128 )
		{
			// We can use x32 interpolator
			irate = IRATE32;
			clk = sr * 4 * 32;// 4*32 clock rate
		}
		else
		{
			if( sr < sr_threshold/64 )
			{
				// We can use x16 interpolator
				irate = IRATE16;
				clk = sr * 4 * 16;// 4*16 clock rate
			}
			else
			{
				if( sr < sr_threshold/32 )
				{
					// We can use x8 interpolator
					irate = IRATE8;
					clk = sr * 4 * 8;// 4*8 clock rate
				}
				else
				{
					// We can use x4 interpolator
					if( sr < sr_threshold/16 )
					{
						irate = IRATE4;
						clk = sr * 4 * 4;// 4*4 clock rate
					}
					else
					{
						// We must use the x2 rate interpolator
						irate = IRATE2;
						clk = sr * 4 * 2;// 4*2 clock rate
					}
				}
			}
		}
	}
/*
	if (get_txmode() == M_DVBS2) {
		// Low rate Interpolators not supported for this mode
		if (sr < sr_threshold / 32)
		{
			// We can use x8 interpolator
			irate = IRATE8;
			clk = sr * 4 * 8;// 4*8 clock rate
		}
		else
		{
			// We can use x4 interpolator
			if (sr < sr_threshold / 16)
			{
				irate = IRATE4;
				clk = sr * 4 * 4;// 4*4 clock rate
			}
			else
			{
				// We must use the x2 rate interpolator
				irate = IRATE2;
				clk = sr * 4 * 2;// 4*2 clock rate
			}
		}
	}
*/
    express_set_interp( irate );
//    printf("Irate %d Srate %f\n",irate, sr);
    if(m_si570_fitted == true )
    {
//        si570_set_clock( sr );
        return irate;
    }
    // We need to set the internal SR gen to something
    //sr = sr/400000000.0;//400 MHz;
    // Turn into a 64 bit number
    //val = (long long)(sr*0xFFFFFFFFFFFFFFFF);
	val = (__int64)(clk*46116860184.2738790375);
    // Send it
    msg[0]  = FPGA_ADD | I2C_WR;

    msg[1]  = FPGA_SR_REG;
    msg[2]  = (val>>56)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1]  = FPGA_SR_REG+1;
    msg[2]  = (val>>48)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1]  = FPGA_SR_REG+2;
    msg[2]  = (val>>40)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1]  = FPGA_SR_REG+3;
    msg[2]  = (val>>32)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1]  = FPGA_SR_REG+4;
    msg[2]  = (val>>24)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1]  = FPGA_SR_REG+5;
    msg[2]  = (val>>16)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1]  = FPGA_SR_REG+6;
    msg[2]  = (val>>8)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[1]  = FPGA_SR_REG+7;
    msg[2]  = (val)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    express_handle_events( 8 );

    return irate;
}
//
// Set 1 of 4 filters in the FPGA
// When in TS mode sets the DVB-S FEC rate.
//
void express_set_interp( int interp )
{
    unsigned char msg[3];

    if( m_express_status != EXP_OK ) return;

    // Send it
    msg[0] = FPGA_ADD | I2C_WR;
    // MSB
    msg[1] = FPGA_INT_REG;
    msg[2] = interp;

    express_i2c_bulk_transfer( EP1OUT, msg, 3 );
    express_handle_events( 1 );
}
void express_set_filter( int filter )
{
    unsigned char msg[3];

    if( m_express_status != EXP_OK ) return;

    // Send it
    msg[0] = FPGA_ADD | I2C_WR;
    // MSB
    msg[1] = FPGA_FIL_REG;
	switch (filter) {
	case RO_35:
		msg[2] = 0;
		break;
	case RO_25:
		msg[2] = 1;
		break;
	case RO_20:
		msg[2] = 2;
		break;
	}

    express_i2c_bulk_transfer( EP1OUT, msg, 3 );
    express_handle_events( 1 );
}
void express_set_fec( int fec )
{
    unsigned char msg[3];

    if( m_express_status != EXP_OK ) return;

    // Send it
    msg[0] = FPGA_ADD | I2C_WR;
    // MSB
    msg[1] = FPGA_FEC_REG;
	switch (fec) {
	case FEC_12:
		msg[2] = 0;
		break;
	case FEC_23:
		msg[2] = 1;
		break;
	case FEC_34:
		msg[2] = 2;
		break;
	case FEC_56:
		msg[2] = 3;
		break;
	case FEC_78:
		msg[2] = 4;
		break;
	}

    express_i2c_bulk_transfer( EP1OUT, msg, 3 );
    express_handle_events( 1 );
}
//
// Send calibration information to the FPGA
//
void express_set_ical( int offset )
{
    short val;
    unsigned char msg[3];

    if( m_express_status != EXP_OK ) return;

    // First do the i channel offset
    val = 0x8000 + offset*4;// bits 1:0 not used
    // Send it
    msg[0]  = FPGA_ADD | I2C_WR;
    // MSB
    msg[1]  = FPGA_I_DC_MSB_REG;
    msg[2]  = (val>>8)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[0]  = FPGA_ADD | I2C_WR;
    // LSB
    msg[1]  = FPGA_I_DC_LSB_REG;
    msg[2]  = val&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    express_handle_events( 2 );

}
void express_set_qcal( int offset )
{
    short val;
    unsigned char msg[3];

    if( m_express_status != EXP_OK ) return;

    // Now the q channel offset
    val = 0x8000 + offset*4;// bits 1:0 not used
    // Send it
    msg[0]  = FPGA_ADD | I2C_WR;
    // MSB
    msg[1]  = FPGA_Q_DC_MSB_REG;
    msg[2]  = (val>>8)&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    msg[0]  = FPGA_ADD | I2C_WR;
    // LSB
    msg[1]  = FPGA_Q_DC_LSB_REG;
    msg[2]  = val&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    express_handle_events( 2 );

}
void express_set_config_byte( void )
{
    unsigned char val;
    unsigned char msg[3];

    if( m_express_status != EXP_OK ) return;

    val = 0;
    if(m_tx_hardware == HW_EXPRESS_16 ) val |= FPGA_16BIT_MODE;
    if(m_si570_fitted == true )         val |= FPGA_USE_SI570;
	                                    val |= FPGA_NULL_INSERT;
    // Send it
    msg[0]  = FPGA_ADD | I2C_WR;
    msg[1]  = FPGA_CONFIG_REG;
    msg[2]  = val;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    express_handle_events( 1 );

}
//
// Send predistortion information to the FPGA
//
void express_load_ptab( UCHAR add, USHORT ival, USHORT qval )
{
    unsigned char msg[3];

    if( m_express_status != EXP_OK ) return;

    // First send the table address

    msg[0]  = FPGA_ADD | I2C_WR;
    msg[1]  = FPGA_PRE_REG;
    msg[2]  = add;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    // MSB of I value
    msg[0]  = FPGA_ADD | I2C_WR;
    msg[1]  = FPGA_PRE_REG + 1;
    msg[2]  = (ival>>8);
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    // LSB of I value
    msg[0]  = FPGA_ADD | I2C_WR;
    msg[1]  = FPGA_PRE_REG + 2;
    msg[2]  = ival&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    // MSB of Q value
    msg[0]  = FPGA_ADD | I2C_WR;
    msg[1]  = FPGA_PRE_REG + 3;
    msg[2]  = (qval>>8);
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );

    // LSB of Q value
    msg[0]  = FPGA_ADD | I2C_WR;
    msg[1]  = FPGA_PRE_REG + 4;
    msg[2]  = qval&0xFF;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );


    express_handle_events( 5 );

}

void express_set_attenuator_level( char level )
{
    if( m_express_status != EXP_OK ) return;
    // Format up into an I2C message for the ADRF6755
    unsigned char msg[3];
    msg[0]  = ADRF6755_ADD | I2C_WR;//address
    msg[1]  = 30;//CR30
    msg[2]  = 0x00;
    if(level > 0 )
    {
        if(level >= 32 )
        {
           msg[2] |= 0x30;
           level  -= 32;
        }
        if(level >= 16 )
        {
           msg[2] |= 0x10;
           level  -= 16;
        }
        if(level >= 8 )
        {
            msg[2] |= 0x08;
            level  -= 8;
        }
        if(level >= 4 )
        {
            msg[2] |= 0x04;
            level  -= 4;
        }
        if(level >= 2 )
        {
            msg[2] |= 0x02;
            level  -= 2;
        }
        if(level >= 1 )
        {
            msg[2] |= 0x01;
            level  -= 1;
        }
    }
    // Now send it over EP1, should add error checking!
    // The FX2 firmware will send it over I2C
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );
    express_handle_events( 1 );
}

void express_set_level( int level )
{
    if( m_express_status != EXP_OK ) return;
    
	char a = 0;
	if (level > 47) level = 47;
	a = 47 - level;
    express_set_attenuator_level( a );
}

//
// Read back two samples from the ADC one for each channel
// This needs a seperate callback as it won't complete atomically.
//
void express_read_ad7992_chans( void )
{
    if( m_express_status != EXP_OK ) return;
    unsigned char msg[7];
    // Format up into an I2C message for the ADRF6755
    msg[0]  = AD7992_ADD | I2C_RD;// ADC I2C address
    msg[1]  = 4;//4 bytes response
    msg[2]  = 0x30;// Address data reg and inniate a sample
    // Set address and issue conversion command
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );
    // Read back sample
    express_i2c_bulk_transfer( EP1IN, msg, 7 );

    express_handle_events( 2 );
}
//
// Read back a specific register on the ADC
//
void express_read_ad7992_reg(unsigned char r)
{
    if( m_express_status != EXP_OK ) return;
    // Format up into an I2C message for the AD7992
    unsigned char msg[4];
    msg[0]  = AD7992_ADD | I2C_RD;//address
    msg[1]  = 1;//1 byte response
    msg[2]  = r;// reg to read
    msg[3]  = 0;//Response will go here
    // Set address
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );
    // Read
    express_i2c_bulk_transfer( EP1IN, msg, 4 );

    express_handle_events( 2 );
}
//
// Read and write to the flash memory
//
// Write
// Add is the address to write to
// b is the bytes to be written
// len id the number of bytes to write (max32)
//
void express_write_24c64( int add, unsigned char *b, int len )
{
    if( m_express_status != EXP_OK ) return;
    // Format up into an I2C message for the ADRF6755
    unsigned char msg[40];
    msg[0]  = FLASH_ADD | I2C_WR;//address
    msg[1]  = add>>8;//msb of address
    msg[2]  = add&0xFF;//lsb of address
    // Attach data
    if( len > 32) len = 32;
    for( int i = 0; i < len; i++)
    {
        msg[i+3] = b[i];//data
    }
    // Send
    express_i2c_bulk_transfer( EP1OUT, msg, len + 3 );

    express_handle_events( 1 );
}
// Read
// Add is the address to read from
// b is the bytes to be read
// len id the number of bytes to write (max32 self imposed)
//
void express_read_24c64( int add, int len )
{
    if( m_express_status != EXP_OK ) return;
    // Format up into an I2C message for the ADRF6755
    unsigned char msg[50];
    if(len > 32) len = 32;
    msg[0]  = FLASH_ADD | I2C_RD;//address
    msg[1]  = len;//bytes to receive
    msg[2]  = add>>8;//msb of address
    msg[3]  = add&0xFF;//lsb of address
    // Set read address
    express_i2c_bulk_transfer( EP1OUT, msg, 4 );
    // Read
    express_i2c_bulk_transfer( EP1IN, msg, len + 4 );

    express_handle_events( 2 );
}

void express_ancillary( void )
{
    // Send it
    UCHAR msg[3];
    msg[0]  = FPGA_ADD | I2C_WR;
    msg[1]  = FPGA_ANC_REG;
    msg[2]  = m_ancillary;
    express_i2c_bulk_transfer( EP1OUT, msg, 3 );
    express_handle_events( 1 );
}

// Set clear carrier output
void express_set_carrier( BOOL b)
{
    if(b == TRUE )
        m_ancillary = m_ancillary |   FPGA_CARRIER;
    else
        m_ancillary = m_ancillary & (~FPGA_CARRIER);
    express_ancillary();
}

// Set/clear calibration output
void express_set_iqcalibrate( BOOL b)
{
    if(b == TRUE )
        m_ancillary = m_ancillary |   FPGA_CALIBRA;
    else
        m_ancillary = m_ancillary & (~FPGA_CALIBRA);
    express_ancillary();
}
void express_set_ramp( BOOL b)
{
    if(b == TRUE)
        m_ancillary = m_ancillary |   FPGA_RAMP;
    else
        m_ancillary = m_ancillary & (~FPGA_RAMP);
    express_ancillary();
}
void express_set_predist(BOOL b)
{
    if(b == TRUE)
        m_ancillary = m_ancillary |   FPGA_PREDIST;
    else
        m_ancillary = m_ancillary & (~FPGA_PREDIST);
    express_ancillary();
}
// End of FPGA I2C messages

//
// Causes the board to break out of it's processing loop
//
void express_breakout(void)
{
    if( m_express_status != EXP_OK ) return;
    // Format up into an I2C message for the ADRF6755
    unsigned char msg[2];
    msg[0]  = FX2_ADD;//address
    msg[1]  = BK_CMD;//Command
    // Set break
    express_i2c_bulk_transfer( EP1OUT, msg, 2 );
    express_handle_events( 1 );
}
//
// Reset the FPGA algorithm
//
void express_fpga_reset(void)
{
    if( m_express_status != EXP_OK ) return;
    // Format up into an I2C message for the ADRF6755
    unsigned char msg[2];
    msg[0]  = FX2_ADD;//address
    msg[1]  = RST_CMD;//Command
    // Set break
    express_i2c_bulk_transfer( EP1OUT, msg, 2 );
    express_handle_events( 1 );
}
//
// Externally callable routines.
//
void express_enable_modulator( void )
{
    if( m_express_status != EXP_OK ) return;
    unsigned char b[3];

    // Power up the modulator, this upsets the attenuator
    b[0]  = ADRF6755_ADD | I2C_WR;//address
    b[1]=29;b[2]=0x01;
    express_i2c_bulk_transfer( EP1OUT, b, 3 );

    express_handle_events( 1 );
}
void express_enable_vco( void )
{
    if( m_express_status != EXP_OK ) return;
    unsigned char b[3];

    // power up internal VCO
    b[1]=28;b[2]=0x01;
    express_i2c_bulk_transfer( EP1OUT, b, 3 );

    express_handle_events( 1 );
}
void express_disable_modulator( void )
{
    if( m_express_status != EXP_OK ) return;
    unsigned char b[3];
    // Power down the modulator, this upsets the attenuator
    b[0]  = ADRF6755_ADD | I2C_WR;//address
    b[1]=29;b[2]=0x00;
    express_i2c_bulk_transfer( EP1OUT, b, 3 );

    express_handle_events( 1 );
}
void express_disable_vco( void )
{
    if( m_express_status != EXP_OK ) return;
    unsigned char b[3];
    b[0]  = ADRF6755_ADD | I2C_WR;//address
    // power down internal VCO
    b[1]=28;b[2]=0x00;
    express_i2c_bulk_transfer( EP1OUT, b, 3 );

    express_handle_events( 1 );
}
void express_transmit(void)
{
    if( m_express_status != EXP_OK ) return;
    unsigned char b[2];
    b[0]  = FX2_ADD | I2C_WR;//address
    // Command
    b[1]=TX_CMD;
    express_i2c_bulk_transfer( EP1OUT, b, 2 );
    express_handle_events( 1 );
}
void express_receive(void)
{
    if( m_express_status != EXP_OK ) return;
    unsigned char b[2];
    b[0]  = FX2_ADD | I2C_WR;//address
    // Command
    b[1]=RX_CMD;
    express_i2c_bulk_transfer( EP1OUT, b, 2 );
    express_handle_events( 1 );
}
void express_set_ports(UCHAR ports) {
	m_ancillary = (m_ancillary&0x0F) | ports;
	if (m_express_status != EXP_OK) return;
	express_ancillary();
}
//
// Service all pending transfer buffers, used when system is closing
//
void express_release_transfer_buffers(void)
{
    if( m_express_status != EXP_OK ) return;

	for( int i = 0; i < NR_XFER_BUFFS; i++ ){ 
		usb_free_async(&m_context[m_index].context);
	}
}
//
//
// length is the number of complex samples
// Send each symbol is 2 x 16 bits
//

#define NR_SBLOCKS   200
#define SIZE_SBLOCKS 50000

static scmplx txb[NR_SBLOCKS][SIZE_SBLOCKS];
static int txi;

int express_write_16_bit_samples( scmplx *s, int len )
{
    for( int i = 0; i < len; i++ )
	{
       txb[txi][i].re = s[i].re | 0x0001;// Mark I channel, LSB is always '1'
       txb[txi][i].im = s[i].im & 0xFFFE;// Mark Q channel, LSB is always '0'
    }
	ep2_send_s16_buffer((unsigned char *)txb[txi], len*sizeof(scmplx));
	txi = (txi + 1) % NR_SBLOCKS;

	return 0;
}
/*

#define NR_SBLOCKS   20000
#define SIZE_SBLOCKS 1024
static scmplx txb[NR_SBLOCKS][SIZE_SBLOCKS];
static int txi;
static int m_b_index;

int express_write_16_bit_samples(scmplx *s, int len)
{
	for (int i = 0; i < len; i++)
	{
		txb[txi][m_b_index].re = s[i].re | 0x0001;// Mark I channel, LSB is always '1'
		txb[txi][m_b_index].im = s[i].im & 0xFFFE;// Mark Q channel, LSB is always '0'
		m_b_index++;

		if (m_b_index == SIZE_SBLOCKS) {
			ep2_send_s16_buffer((unsigned char *)txb[txi], SIZE_SBLOCKS*sizeof(scmplx));
			m_b_index = 0;
			txi = (txi + 1) % NR_SBLOCKS;
		}
	}

	return 0;
}
*/
//
// Send each symbol is 2 x 8 bits
//
int express_write_8_bit_samples( scmplx *s, int len )
{
    unsigned short *b = (unsigned short *)s;
    for( int i = 0; i < len; i++ )
    {
        b[i] = (s[i].re&0xFF00) | ((s[i].im>>8)&0xFF);
    }
	ep2_send_s8_buffer((unsigned char*)b, len*sizeof(unsigned short));
	return 0;
}
int express_write_samples( scmplx *s, int len )
{
    int res;

    if( m_express_status != EXP_OK ) return -1;
    //
    // Record the sample block size, this should be current
    // but as it is used for metrics does not have to be exact.
    //
    m_sample_block_size = len;

    // Decide which format we are using
    switch( m_tx_hardware )
    {
    case HW_EXPRESS_16:
        res = express_write_16_bit_samples( s, len );
        break;
    case HW_EXPRESS_8:
        res = express_write_8_bit_samples( s, len );
        break;
    default:
        res = express_write_16_bit_samples( s, len );
         break;
    }
    return res;
}
//
// Write the transport stream directly to the Express hardware board
//
void express_write_transport_stream( UCHAR *tp, INT len )
{
    if( m_express_status != EXP_OK ) return;
	if( tp == NULL ) return;
     //usb_bulk_write(m_handle,EP2OUT,(char*)tp,len,USB_TIMEOUT);
	 ep2_send_ts_buffer( tp, len );
}
//
// A Si570 clock generator chip has been detected on the board
//
void express_si570_fitted(void)
{
    m_si570_fitted = true;
//    loggerf("Si570 present\n");
}

//
// Returns the outstanding number of IQ samples left to send in the hardware
// This is an approxiamate number.
//
double express_outstanding_queue_size(void)
{
    double qsize = 0;

    if(m_tx_hardware == HW_EXPRESS_16 )
    {
        qsize = (m_xfrs_in_progress * m_sample_block_size)/2;
        // Add samples in the FPGA FIFO (hardcoded)
        qsize += 2048/2;
    }
    if(m_tx_hardware == HW_EXPRESS_8 )
    {
        qsize = (m_xfrs_in_progress * m_sample_block_size);
        // Add samples in the FPGA FIFO (hardcoded)
        qsize += 2048;
    }
    return qsize;
}
//
// Returns the number of buffers yet to be sent
//
int express_get_tx_queue_size(void)
{
    return m_xfrs_in_progress;
}

void express_deinit(void)
{
    if( m_express_status != EXP_OK ) return;
    express_receive();
	usb_release_interface( m_handle, 0);
	usb_close( m_handle );
    express_release_transfer_buffers();
    m_express_status = EXP_CONF;
}

int express_init( FILE *fx2, FILE *fpga )
{
    m_express_status = EXP_CONF;
//    sem_init( &usb_sem, 0, 0 );

    m_initialised      = 0;
    m_xfrs_in_progress = 0;
    m_si570_fitted     = false;
	

    // Find the board
    if(( m_express_status = express_find())<0)
    {
		report_error(ERROR_EXPRESS_NOT_FOUND);
		express_error("Express Hardware not found");
		m_status_s = "HW fail";
		m_status_tx = FALSE;
        return EXP_FAIL;
    }
    // Reset it, this gets it into a state ready for programming
    express_reset();
	Sleep(50);

    // Load the FX2 firmware
    if(( m_express_status = express_firmware_load( fx2 ))<0)
    {
		express_error("Express unable to load FX2 firmware");
		report_error(ERROR_FIRMWARE_NOT_FOUND);
		m_status_s = "FX2 fail";
		m_status_tx = FALSE;
        return EXP_IHX;
    }
	Sleep(50);
    // Start the FX2 code running, this should get the FPGA into a state where it is ready
    // to accept a new program
    express_run();
	Sleep(50);
    // Load the FPGA firmware

    if(( m_express_status = express_fpga_load( fpga ))<0)
    {
		express_error("Express unable to load FPGA firmware");
		report_error(ERROR_FPGA_NOT_FOUND);
		m_status_s = "FPGA fail";
		m_status_tx = FALSE;
        return EXP_RBF;
    }
    // FPGA firmwre loaded
	Sleep(50);
    m_express_status = EXP_OK;
    //
    // The DATV-Express board should now be in a usuable state both the FX2 and FPGA
    // should be running
    //

    // Send the initial configuration to the Modulator
    express_configure_adrf6755();
    // Attenuate the ouput
    express_set_attenuator_level( 47 );
    // Set the operating frequency
    express_set_freq( 1249000000 );

    // Load the configuration byte
    express_set_config_byte();
	express_ancillary();
	// Set to a default Interpolation rate
	express_set_interp( IRATEIV );
    // Zero the Calibration values
    express_set_qcal( 0 );
    express_set_ical( 0 );

    // Configure the ad7992 ADC
    express_configure_ad7992();

    // Get the ADRF6755 chip version number
    express_read_adrf6755_version();

    // Get the contents of the ad7992 register
    //express_read_ad7992_reg(2);
    //express_read_ad7992_chans();

//   unsigned char bb[32];

//   bb[0] = 0xfa;
//   express_write_24c64( 46, bb, 1 );
//   bb[0] = 0;

    //express_read_24c64( 32, 16 );

    express_receive();

	m_initialised = 1;

    m_status_s = "ready";
	m_status_tx = FALSE;

    return m_express_status;
}

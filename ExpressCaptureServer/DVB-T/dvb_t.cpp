// 
// This is the interface module in and out of the dvb_t
// encoder
//
#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/types.h>
#include "dvb_t.h"


extern int m_tx_samples;

DVBTFormat m_format;
double m_sample_rate;
int m_dvbt_initialised;

double dvb_t_get_sample_rate( void )
{
    double srate = 8000000.0/7.0;

    switch( m_format.chan )
    {
    case CH_8M:
        srate = srate*8.0*m_format.ir;
        break;
    case CH_7M:
        srate = srate*7.0*m_format.ir;
        break;
    case CH_6M:
        srate = srate*6.0*m_format.ir;
        break;
    case CH_4M:
        srate = srate*4.0*m_format.ir;
        break;
    case CH_3M:
        srate = srate*3.0*m_format.ir;
        break;
    case CH_2M:
        srate = srate*2.0*m_format.ir;
         break;
    case CH_1M:
        srate = srate*1.0*m_format.ir;
         break;
    default:
        break;
    }
    return srate;
}
void dvb_t_set_interpteter(DVBTFormat *fmt)
{
	switch (m_format.chan)
	{
	case CH_8M:
		fmt->ir = 1;
		break;
	case CH_7M:
		fmt->ir = 1;
		break;
	case CH_6M:
		fmt->ir = 1;
		break;
	case CH_4M:
		fmt->ir = 2;
		break;
	case CH_3M:
		fmt->ir = 2;
		break;
	case CH_2M:
		fmt->ir = 4;
		break;
	case CH_1M:
		fmt->ir = 4;
		break;
	default:
		fmt->ir = 1;
		break;
	}
}
double dvb_t_get_symbol_rate( void )
{
    double symbol_len = 1;//default value
    double srate = dvb_t_get_sample_rate();
    if( m_format.tm == TM_2K ) symbol_len = M2KS*m_format.ir;
    if( m_format.tm == TM_8K ) symbol_len = M8KS*m_format.ir;
    switch( m_format.gi )
    {
        case GI_132:
            symbol_len += symbol_len/32;
            break;
        case GI_116:
            symbol_len += symbol_len/16;
            break;
        case GI_18:
            symbol_len += symbol_len/8;
            break;
        case GI_14:
            symbol_len += symbol_len/4;
            break;
        default:
            symbol_len += symbol_len/4;
            break;
    }
    return srate/symbol_len;
}
//
// Returns the samples to send
//
Scmplx *dvb_t_get_frame(void) {
	m_tx_samples = 0;
	return dvb_t_get_samples();
}
//
// Called externally to send the next
// transport frame. It encodes the frame
// then calls the modulator if a complete frame is ready
//
uint8_t dibit[DVBS_T_CODED_FRAME_LEN * 8];

int dvb_t_encode_and_modulate(uint8_t *tp)
{
	int len;
	if (m_dvbt_initialised == 0) return 0;

	len = dvb_encode_frame(tp, dibit);
	dvb_t_enc_dibit(dibit, len);
	return m_tx_samples;
}
//
// Initialise the module
//
//
void dvb_t_init( void )
{
    // Encode the correct parameters
    m_format.chan = CH_2M;
	m_format.co   = CO_QPSK;
	m_format.fec  = CR_12;
	m_format.gi   = GI_18;
	m_format.sf   = SF_NH;
	m_format.tm   = TM_2K;
	dvb_t_set_interpteter(&m_format);

	m_format.sr   = dvb_t_get_sample_rate();
	m_format.br   = dvb_t_raw_bitrate();

	dvb_t_mod_init();
	dvb_interleave_init();
	dvb_rs_init();
	dvb_encode_init();
	dvb_conv_init();

	build_tx_sym_tabs();
    dvb_t_build_p_tables();
	init_dvb_t_fft();
    init_dvb_t_enc();
    build_tp_block();
    init_reference_frames();
	m_dvbt_initialised = 0;

}
void dvb_t_configure(DVBTFormat *fmt)
{
	// Encode the correct parameters
	m_format = *fmt;
	dvb_t_set_interpteter(&m_format);
	fmt->sr = dvb_t_get_sample_rate();
	fmt->br = dvb_t_raw_bitrate();

	dvb_t_mod_init();
	dvb_interleave_init();
	dvb_rs_init();
	dvb_encode_init();
	dvb_conv_init();

	build_tx_sym_tabs();
	dvb_t_build_p_tables();
	init_dvb_t_fft();
	init_dvb_t_enc();
	build_tp_block();
	init_reference_frames();
	m_dvbt_initialised = 1;

}
//
// Called when altering the DVB-T mode form the GUI
//
// This does not work yet.
//
void dvb_t_re_init( void )
{
    dvb_t_init();
}
//
// Returns FFT resources
//
void dvb_t_deinit( void )
{
    deinit_dvb_t_fft();
}

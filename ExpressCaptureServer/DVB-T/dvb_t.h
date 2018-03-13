#ifndef __DVB_T_H__
#define __DVB_T_H__


#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avfft.h>
#include <libavutil/mem.h>
}

#define SYMS_IN_FRAME 68

#ifndef M_PI
#define M_PI 3.14159f
#endif

// Minimum cell nr
#define K2MIN 0
#define K8MIN 0

// Max cell nr
#define K2MAX 1704
#define K8MAX 6816
#define SF_NR 4

// Size of FFT
#define M2KS  2048
#define M4KS  4096
#define M8KS  8192
#define M16KS 16384
#define M32KS 32768

// Number of data cells
#define M2SI 1512
#define M8SI 6048

// FFT bin number to start at
#define M8KSTART 688
#define M2KSTART 172

#define BIL 126
#define M_QPSK  0
#define M_QAM16 1
#define M_QAM64 2

#define BCH_POLY  0x4377
//#define BCH_RPOLY 0x7761
#define BCH_RPOLY 0x3761

// Definition of mode, these correspond to the values in the TPS
#define FN_1_SP 0
#define FN_2_SP 1
#define FN_3_SP 2
#define FN_4_SP 3

#define CO_QPSK  0
#define CO_16QAM 1
#define CO_64QAM 2

#define SF_NH    0
#define SF_A1    1
#define SF_A2    2
#define SF_A4    3

#define GI_132   0
#define GI_116   1
#define GI_18    2
#define GI_14    3

#define TM_2K    0
#define TM_8K    1

#define CH_8M     0
#define CH_7M     1
#define CH_6M     2
#define CH_4M     3
#define CH_3M     4
#define CH_2M     5
#define CH_1M     6

typedef struct{
	uint8_t co;// Constellation
	uint8_t sf;// single frequency network
	uint8_t gi;// guard interval
	uint8_t tm;// Transmission mode 2K or 8K
    uint8_t chan;// Channel bandwidth
    uint8_t fec;// FEC coderate
	uint8_t ir;// Interpolation ratio
	double sr;
	int br;
}DVBTFormat;

#define AVG_E8 (1.0/300.0)
//#define AVG_E2 (1.0/1704.0)
#define AVG_E2 (1.0/150.0)

typedef struct {
	short re;
	short im;
}Scmplx;

#define MP_T_SYNC 0x47
#define DVBS_T_ISYNC 0xB8
#define DVBS_T_PAYLOAD_LEN 187
#define MP_T_FRAME_LEN 188
#define DVBS_RS_BLOCK_DATA 239
#define DVBS_RS_BLOCK_PARITY 16
#define DVBS_RS_BLOCK (DVBS_RS_BLOCK_DATA+DVBS_RS_BLOCK_PARITY)
#define DVBS_PARITY_LEN 16
#define DVBS_T_CODED_FRAME_LEN (MP_T_FRAME_LEN+DVBS_PARITY_LEN)
#define DVBS_T_FRAMES_IN_BLOCK 8
#define DVBS_T_BIT_WIDTH 8
#define DVBS_T_SCRAM_SEQ_LENGTH 1503

// Prototypes

// dvb_t_tp.c
void build_tp_block( void );

// dvb_t_ta.c
void dvb_t_configure( DVBTFormat *p );

// dvb_t_sym.c
void init_reference_frames( void );
void reference_symbol_reset( void );
int reference_symbol_seq_get( void );
int reference_symbol_seq_update( void );

//dvb_t_i.c
void dvb_t_build_p_tables( void );

// dvb_t_enc.c
void init_dvb_t_enc( void );
void dvb_t_enc_dibit( uint8_t *in, int length );
void dvb_t_encode_and_modulate(uint8_t *in, uint8_t *dibit );

// dvb_t_mod.c
void dvb_t_select_constellation_table( void );
void dvb_t_calculate_guard_period( void );
void dvb_t_modulate(uint8_t *syms );
void dvb_t_write_samples( short *s, int len );
void dvb_t_modulate_init( void );

// dvb_t_linux_fft.c
void init_dvb_t_fft( void );
void deinit_dvb_t_fft( void );
void fft_2k_test( FFTComplex *out );
void dvbt_fft_modulate( FFTComplex *in, int guard );

// dvb_t.cpp
void   dvb_t_init( void );
void   dvb_t_deinit( void );
void   dvb_t_re_init( void );
double dvb_t_get_sample_rate( void );
double dvb_t_get_channel_bandwidth( void );
double dvb_t_get_symbol_rate( void );
int dvb_t_get_interpolater(void);

// dvb_t_qam_tab.cpp
void build_tx_sym_tabs( void );

// Video bitrate
int dvb_t_raw_bitrate(void);

// dvb_t_lpf.cpp
FFTComplex *dvbt_filter( FFTComplex *in, int length );

// RS functions
void dvb_rs_init(void);
void dvb_rs_encode(uint8_t *inout);

// Interleave
void dvb_interleave_init(void);
void dvb_convolutional_interleave(uint8_t *inout);

// Convolutional coder
//void dvb_conv_ctl(sys_config *info);
void dvb_conv_init(void);
int  dvb_conv_encode_frame(uint8_t *in, uint8_t *out, int len);

// Modulators
int dvb_t_encode_and_modulate(uint8_t *tp );
void dvb_t_modulate(FFTComplex *in, int length, int guard);
void dvb_t_modulate(FFTComplex *in, float *taper, int length, int guard);
void dvb_t_clip(FFTComplex *in, int length);
void dvb_t_modulate_init(void);
void dvb_t_mod_init(void);
void dvb_t_configure(DVBTFormat *fmt);
Scmplx *dvb_t_get_frame(void);
Scmplx *dvb_t_get_samples(void);

// dvb.c
void dvb_encode_init(void);
int dvb_encode_frame(uint8_t *tp, uint8_t *dibit);
void dvb_reset_scrambler(void);
void dvb_scramble_transport_packet(uint8_t *in, uint8_t *out);

#endif

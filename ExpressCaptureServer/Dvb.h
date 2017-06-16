#pragma once
#include <stdint.h>
#include "tp.h"
#include "Codecs.h"

extern "C"
{
#include <libavutil/mem.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavutil/pixfmt.h>
#include <libavutil/pixdesc.h>
#include <libavfilter/buffersink.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
}

// Buffering of calback data

typedef struct{
	uint64_t time;
	int len;
	uint8_t *b;
}SampleBuffer;

typedef unsigned int u32;
typedef unsigned char u8;

#define TP_LEN 188
#define DISP_MODE_PAL 0
#define DISP_MODE_NTSC 1
#define PES_PAYLOAD_LENGTH 184
// PIDS
#define PAT_PID  0x0000
#define NIT_PID  0x0010
#define SDT_PID  0x0011
#define EIT_PID  0x0012
#define TDT_PID  0x0014
#define NULL_PID 0x1FFF

#define P1_MAP_PID  0xFFF
#define P1_VID_PID  256
#define P1_AUD_PID  257
#define P1_PCR_PID  0x1FFF
#define P1_DATA_PID 258

#define DEFAULT_NETWORK_ID 1
#define DEFAULT_STREAM_ID  P1_MAP_PID
#define DEFAULT_SERVICE_ID P1_MAP_PID
#define DEFAULT_PROGRAM_NR P1_MAP_PID

#define S_USE_VIDEO_DEVICE "Use Video Device"
#define CRC_32_LEN 4

typedef enum{FEC_14,FEC_13,FEC_25,FEC_12,FEC_35,FEC_23,FEC_34,FEC_45,FEC_56,FEC_89,FEC_78,FEC_910}Fec;
typedef enum{RO_35,RO_25,RO_20}Rolloff;
typedef enum{QPSK,PSK8,QAM16,APSK16,APSK32,QAM64}Constellation;
typedef enum{M_DVBS,M_DVBS2,M_DVBT}TxMode;


void ts_write_transport_queue( uint8_t *tp );
void send_tp( uint8_t *tp );
void ps_video_el_to_ps( uint8_t *b, int length, int32_t bitrate, int64_t pts, int64_t dts );
void ps_audio_el_to_ps( uint8_t *b, int length, int32_t bitrate, int64_t pts, int64_t dts );
int ps_get_length( void );
void ps_read( uint8_t *b, int len );
void ps_to_ts_video( void );
void ps_to_ts_audio( void );
void ps_to_ts_init(void);
void send_pcr(void);
int64_t get_pcr_clock(void);
void    set_pcr_clock(int64_t clk);

// pcr 
int pcr_fmt( uint8_t *b, tp_hdr *h );
void pcr_increment_clock(void);
bool is_pcr_update(void);
void pcr_dvb( uint8_t cont);

// General
void set_modem_params(void);
int64_t get_tp_tick(void);
uint32_t get_audio_bitrate(void);
uint32_t get_video_bitrate(void);
uint32_t get_tx_bitrate(void);
uint32_t si_overhead(void);

// Timer
void timer_tick( void );
void timer_work(void);

// SI
int crc32_add( uint8_t *b, int len );

// Configuration
void SaveConfigToDisk(const char *name);
int LoadConfigFromDisk(const char *name);
int LoadOnAirFormatsFromDisk(int nr, CString *fmts);

int system_start(const char *name);
void system_stop(void);
int system_restart(void);
uint32_t get_video_pid(void);
uint32_t get_audio_pid(void);
uint32_t get_pcr_pid(void);
uint32_t get_pmt_pid(void);
int get_video_codec(void);
uint32_t get_video_gop(void);
uint32_t get_video_b_frames(void);
void configure_si(void);

const char *get_video_capture_device(void);
const char *get_video_capture_format(void);
int get_video_capture_format(uint32_t *w, uint32_t *h, int *fps, int *fmt);
int get_audio_codec(void);
const char *get_audio_capture_device(void);
const char *get_provider_name(void);
const char *get_service_name(void);
const char *get_event_title(void);
const char *get_event_text(void);
const char *get_onair_video_format(void);
uint32_t get_event_duration(void);
uint32_t get_network_id(void);
uint32_t get_stream_id(void);
uint32_t get_service_id(void);
uint32_t get_program_nr(void);
double get_vbr_twiddle(void);
int get_video_aspect_ratio(void);
int get_cap_interlaced(void);
const char *get_video_codec_name(void);
BOOL get_system_status(void);
CString get_error_text(void);
const char *get_video_codec_performance(void);
BOOL get_audio_status(void);
uint32_t get_audio_codec_bitrate(void);
CString get_config_filename(void);
uint8_t get_tx_ports(void);
uint32_t get_pfd(void);
int get_txmode(void);
uint32_t get_dvbs2_constellation(void);
uint32_t get_dvbs2_rolloff(void);
uint32_t get_dvbs2_fec(void);
uint32_t get_dvbs2_pilots(void);
uint32_t get_dvbs_fec(void);
uint32_t get_tx_symbolrate(void);
BOOL get_video_ident(void);
BOOL get_tx_status(void);
uint32_t get_dvbt_fec(void);
uint32_t get_dvbt_constellation(void);
uint32_t get_dvbt_guard(void);
uint32_t get_dvbt_channel(void);
uint32_t get_dvbt_mode(void);
int get_i_dc_offset(void);
int get_q_dc_offset(void);

// Set commands
void cmd_transmit(void);
void cmd_standby(void);
void cmd_set_video_codec(const char *codec);
void cmd_set_audio_codec(const char *codec);
void cmd_set_video_gop(const char *gop);
void cmd_set_video_b_frames(const char *bframes);
void cmd_set_video_capture_device(const char *device);
void cmd_set_video_capture_format(const char *format);
void cmd_set_audio_capture_device(const char *device);
void cmd_set_dvbs_fec(const char *fec);
void cmd_set_dvbs_freq(const char *freq);
void cmd_set_dvbs_level(const char *level);
void cmd_set_tx_level(int level);
void cmd_set_dvbs_srate(const char *srate);
void cmd_set_video_pid(const char *pid);
void cmd_set_audio_pid(const char *pid);
void cmd_set_pcr_pid(const char *pid);
void cmd_set_pmt_pid(const char *pid);
void cmd_set_service_provider_name(const char *name);
void cmd_set_service_name(const char *name);
void cmd_set_event_title(const char *title);
void cmd_set_event_duration(const char *duration);
void cmd_set_event_text(const char *text);
void cmd_set_network_id(const char *id);
void cmd_set_stream_id(const char *id);
void cmd_set_service_id(const char *id);
void cmd_set_program_nr(const char *nr);
void cmd_set_vbr_twiddle(const char *twiddle);
void cmd_set_video_aspect_ratio(const char *ratio);
void cmd_set_onair_video_format(const char *text);
void cmd_set_cap_interlaced(const char *val);
void cmd_set_error_text(const char *text);
void cmd_set_video_codec_performance(const char *perform);
void cmd_set_audio_status(const char *status);
void cmd_set_audio_codec_bitrate(const char *rate);
void cmd_set_tx_ports(const char*ports);
void cmd_set_txmode(const char *txmode);
void cmd_set_dvbs2_fec(const char *fec);
void cmd_set_dvbs2_rolloff(const char *ro);
void cmd_set_dvbs2_pilots(const char *pilots);
void cmd_set_dvbs2_constellation(const char *cons);

void cmd_set_dvbt_fec(const char *fec);
void cmd_set_dvbt_constellation(const char *con);
void cmd_set_dvbt_guard(const char *guard);
void cmd_set_dvbt_channel(const char *chan);
void cmd_set_dvbt_mode(const char *mode);

CString get_current_fec_string(void);
uint32_t get_current_tx_frequency(void);
int get_current_tx_level(void);
void cmd_tx_frequency( const char * freq);
void cmd_tx_level( const char * level);
void cmd_tx_symbol_rate( const char *sr);
void cmd_tx_fec( const char *fec);
void cmd_set_carrier(int chk);
uint32_t get_current_tx_symbol_rate(void);
void cmd_set_config_filename(CString name);
void cmd_set_video_ident(BOOL ident);
void cmd_set_i_dc_offset(int i);
void cmd_set_q_dc_offset(int q);

// Buffers
uint8_t *alloc_tx_buff(void);
// Allocate and copy a new buffer
uint8_t *alloc_copy_tx_buff(uint8_t *in);
// Release a buffer
void rel_tx_buff(uint8_t *b);
// Post a buffer to the tx queue
void post_tx_buff( uint8_t *b);
// Get a buffer from the tx queue
uint8_t *get_tx_buff(void);
int get_tx_buf_qsize(void);
// Get the percentage use of the buffer
int get_tx_buf_percent(void);
// Empty the tx buffer queue
void tx_buf_empty(void);
// Initialise the buffers
void tx_buf_init(void);

// txmodule
void tx_start(void);
void tx_stop(void);

// Capture
int  capture_start(CodecParams *params);
void capture_stop(void);
void capture_pause(void);
void capture_run(void);
int GetAudioInputCaptureList(CString *names, int num );
int GetVideoInputCaptureList(CString *names, int num );
int GetVideoInputCaptureFormatList(CString *device, CString *formats, int num);
void capture_time(void);

// Logging
void tp_log( uint8_t *tp );
void start_log(void);
void stop_log(void);

// Overlat
void overlay_frame( AVFrame *frame );
int overlay_init(const char *text);
void overlay_deinit(void);

// Video buffering for slow systems
void vb_video_post(uint8_t *b, int len);
void vb_audio_post(uint8_t *b, int len);
int vb_av_init(void);
void vb_av_deinit(void);

// DVB clock
void init_dvb_clock(void);
int64_t dvb_get_time(void);
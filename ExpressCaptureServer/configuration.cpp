#include "stdafx.h"
#include <stdint.h>
#include "si.h"
#include "Codecs.h"
#include "error_codes.h"
#include "DVB-T\dvb_t.h"
#include "noise.h"
#include "hardware.h"

static CString g_error_text;
static CString g_config_filename;
static int  g_txmode;
static int  g_service_id; 
static int  g_stream_id; 
static int  g_network_id; 
static int  g_event_duration; 
static char g_event_title[40]; 
static char g_event_text[1024];
static int  g_pmt_pid; 
static uint32_t  g_pcr_pid; 
static uint32_t  g_video_pid; 
static uint32_t  g_audio_pid; 
static int  g_program_nr;
static int  g_video_codec; 
static char  g_video_codec_performance[80];
static int  g_audio_codec;
static BOOL g_audio_present;
static char g_service_provider_name[40]; 
static char g_service_name[40];
static uint64_t g_tx_frequency;
static uint32_t g_tx_sr;
static int  g_tx_level;
static int  g_tx_carrier;

static int g_dvbs_fec;
static int g_dvbs2_fec;
static int g_dvbs2_rolloff;
static int g_dvbs2_constellation;
static int g_dvbs2_pilots;

static int g_dvbt_fec;
static int g_dvbt_channel;
static int g_dvbt_constellation;
static int g_dvbt_mode;
static int g_dvbt_guard;

static int g_disp_videomode;
static char g_cap_videodevice[1024];
static char g_cap_videoformat[1024];
static char g_cap_audiodevice[1024];
static char g_cap_onair_videoformat[1024];
static uint32_t g_video_gop;
static uint32_t g_video_b_frames;
static double g_vbr_twiddle;
static int g_video_aspect_ratio;
static int g_cap_interlaced;
static BOOL g_system_status;
static uint32_t g_audio_bitrate;
static uint8_t  g_tx_ports;
static uint32_t g_pfd;
static BOOL g_video_ident;
static BOOL g_tx_status;
static int g_i_dc_offset;
static int g_q_dc_offset;
static SdrHwType g_sdrhw_type;
static DWORD g_sdr_ip_addr;
static DWORD g_TsIn_ip_addr;
static WORD g_TsIn_ip_port ;
static DWORD g_TsIn_LocalNic;

void load_defaults(void) {
	g_config_filename = "datvexpress.cfg";
	g_service_id      = 4095;
	g_stream_id       = 4095;
	g_network_id      = 1;
	g_event_duration  = 60;
	sprintf_s(g_event_title, 40,"HamRadio Now");
	sprintf_s(g_event_text,1024,"Program about Anmateur Radio");
	g_pmt_pid         = 4095;
	g_pcr_pid         = 256;
	g_video_pid       = 256;
	g_audio_pid       = 257;
	g_program_nr      = 4095;
	g_video_codec     = AV_CODEC_ID_MPEG2VIDEO;
	sprintf_s(g_video_codec_performance,80,"ultrafast");
	g_audio_codec     = AV_CODEC_ID_MP2;
	g_audio_present   = TRUE;
	g_audio_bitrate   = 64000;
	sprintf_s(g_service_provider_name,40,"Your Call");
	sprintf_s(g_service_name,40,"Digital TV");
	g_tx_frequency    = 1249000000;
	g_tx_sr           = 4000000;
	g_tx_level        = 20;
	g_dvbs_fec        = FEC_23;
	g_dvbt_fec        = FEC_23;
	g_dvbs2_fec       = FEC_23;
	g_dvbs2_rolloff   = RO_35;
	g_dvbs2_constellation = QPSK;
	g_dvbs2_pilots    = 0;
	static int g_disp_videomode;
	sprintf_s(g_cap_videodevice,1024,"vMix Video");
	sprintf_s(g_cap_videoformat,1024,"720 576 25 UYVY");
	sprintf_s(g_cap_audiodevice,1024,"vMix Audio");
	sprintf_s(g_cap_onair_videoformat,1024,"Input Format");
	g_video_gop       = 10;
	g_video_b_frames  = 0;
	g_vbr_twiddle     = 0.7;
	g_video_aspect_ratio = 43;
	g_cap_interlaced  = 0;
	g_pfd = 40000000;// Standard Express reference
	g_txmode = M_DVBS;
	g_video_ident = FALSE;
	g_i_dc_offset = 0;
	g_q_dc_offset = 0;
	g_TsIn_ip_addr = 0;
	g_TsIn_ip_port = 10000;
}
uint32_t get_video_pid(void){
	return g_video_pid;
}
uint32_t get_audio_pid(void){
	return g_audio_pid;
}
uint32_t get_pcr_pid(void){
	return g_pcr_pid;
}
uint32_t get_pmt_pid(void){
	return g_pmt_pid;
}
const char *get_provider_name(void){
	return g_service_provider_name;
}
const char *get_service_name(void){
	return g_service_name;
}
const char *get_event_title(void){
	return g_event_title;
}
const char *get_event_text(void){
	return g_event_text;
}
uint32_t get_event_duration(void){
	return g_event_duration;
}
uint32_t get_network_id(void){
	return g_network_id;
}
uint32_t get_stream_id(void){
	return g_stream_id;
}
uint32_t get_service_id(void){
	return g_service_id;
}
uint32_t get_program_nr(void){
	return g_program_nr;
}
double get_vbr_twiddle(void){
	return g_vbr_twiddle;
}
int get_video_aspect_ratio(void){
	return g_video_aspect_ratio;
}
int get_cap_interlaced(void) {
	return g_cap_interlaced;
}
const char *get_video_codec_name(void) {
	if (g_video_codec == AV_CODEC_ID_MPEG2VIDEO ) return "H.262";
	if (g_video_codec == AV_CODEC_ID_H264 ) return "H.264";
	if (g_video_codec = AV_CODEC_ID_HEVC) return "H.265";
	return "";
}
BOOL get_system_status(void) {
	return g_system_status;
}
CString get_error_text(void) {
	return g_error_text;
}
const char *get_video_codec_performance(void){
	return g_video_codec_performance;
}
BOOL get_audio_status(void) {
	return g_audio_present;
}
uint32_t get_audio_codec_bitrate(void) {
	return g_audio_bitrate;
}
uint8_t get_tx_ports(void) {
	return g_tx_ports;
}
uint32_t get_pfd(void) {
	return g_pfd;
}
int get_txmode(void) {
	return g_txmode;
}
uint32_t get_dvbs2_constellation(void) {
	return g_dvbs2_constellation;
}
uint32_t get_dvbs2_rolloff(void) {
	return g_dvbs2_rolloff;
}
uint32_t get_dvbs2_fec(void) {
	return g_dvbs2_fec;
}
uint32_t get_dvbs2_pilots(void) {
	return g_dvbs2_pilots;
}
uint32_t get_dvbs_fec(void) {
	return g_dvbs_fec;
}
uint32_t get_tx_symbolrate(void) {
	return g_tx_sr;
}

BOOL get_video_ident(void) {
	return g_video_ident;
}

uint32_t get_dvbt_fec(void) {
	return g_dvbt_fec;
}
uint32_t get_dvbt_constellation(void) {
	return g_dvbt_constellation;
}
uint32_t get_dvbt_guard(void) {
	return g_dvbt_guard;
}
uint32_t get_dvbt_channel(void) {
	return g_dvbt_channel;
}
uint32_t get_dvbt_mode(void) {
	return g_dvbt_mode;
}

BOOL get_tx_status(void) {
	return g_tx_status;
}
int get_i_dc_offset(void) {
	return g_i_dc_offset;
}
int get_q_dc_offset(void) {
	return g_q_dc_offset;
}
SdrHwType get_sdrhw_type(void) {
	return g_sdrhw_type;
}
DWORD get_sdr_ip_addr(void) {
	return g_sdr_ip_addr;
}

DWORD get_TSIn_ip_addr(void) {
	return g_TsIn_ip_addr;
}

WORD get_TSIn_port(void) {
	return g_TsIn_ip_port;
}

DWORD get_TSIn_LocalNic(void) {
	return g_TsIn_LocalNic;
}

const char *get_sdr_ip_addrs(void) {
	static char text[20];
	sprintf_s(text, 20, "%d.%d.%d.%d", (g_sdr_ip_addr >> 24), (g_sdr_ip_addr >> 16) & 0xFF, (g_sdr_ip_addr >> 8) & 0xFF, g_sdr_ip_addr & 0xFF);
	return text;
}
int get_tx_carrier(void) {
	return g_tx_carrier;
}
// Configure the SI tables
void configure_si(void){
	eit_fmt( g_service_id, g_stream_id, g_network_id, g_event_duration, g_event_title, g_event_text );
	nit_fmt( NIT_PID, g_stream_id, g_network_id, g_service_id );
	pat_fmt( g_stream_id, NIT_PID, g_pmt_pid, g_program_nr );
	pmt_fmt( g_video_codec, g_audio_codec, g_pmt_pid, g_pcr_pid, g_video_pid, g_audio_pid, g_program_nr );
	sdt_fmt( g_stream_id, g_network_id, g_service_id, g_service_provider_name, g_service_name );
	null_fmt();


}
void cmd_set_audio_codec(const char *codec){
	g_audio_codec = AV_CODEC_ID_MP2; 
	if(strncmp(codec,"mp1",3) == 0) g_audio_codec = AV_CODEC_ID_MP2;
	if(strncmp(codec,"aac",3) == 0) g_audio_codec = AV_CODEC_ID_AAC;

}
void cmd_set_video_codec(const char *codec){
	g_video_codec = AV_CODEC_ID_MPEG2VIDEO; 
	if(strncmp(codec,"h262",4) == 0) g_video_codec = AV_CODEC_ID_MPEG2VIDEO;
	if(strncmp(codec,"h264",4) == 0) g_video_codec = AV_CODEC_ID_H264;
	if(strncmp(codec,"h265",4) == 0) g_video_codec = AV_CODEC_ID_HEVC;		
}
void cmd_set_video_gop(const char *gop){
	g_video_gop = atoi(gop);
}
void cmd_set_video_b_frames(const char *bframes){
	g_video_b_frames = atoi(bframes);
}

void cmd_set_video_capture_device(const char *device){
	strncpy_s(g_cap_videodevice,device,strlen(device));
}
void cmd_set_video_capture_format(const char *format){
	strncpy_s(g_cap_videoformat,format,strlen(format));
}
void cmd_set_audio_capture_device(const char *device){
	strncpy_s(g_cap_audiodevice,device,strlen(device));
}
void cmd_set_service_provider_name(const char *name){
	strncpy_s(g_service_provider_name,name,strlen(name));
}
void cmd_set_service_name(const char *name){
	strncpy_s(g_service_name,name,strlen(name));
}
void cmd_set_network_id(const char *id){
	g_network_id = atoi(id);
}
void cmd_set_stream_id(const char *id){
	g_stream_id = atoi(id);
}
void cmd_set_service_id(const char *id){
	g_service_id = atoi(id);
}
void cmd_set_program_nr(const char *nr){
	g_program_nr = atoi(nr);
}

void cmd_set_dvbs_fec(const char *fec){
	g_dvbs_fec = FEC_12; 
	if(strncmp(fec,"1/2",3) == 0) g_dvbs_fec = FEC_12;
	if(strncmp(fec,"2/3",3) == 0) g_dvbs_fec = FEC_23;
	if(strncmp(fec,"3/4",3) == 0) g_dvbs_fec = FEC_34;
	if(strncmp(fec,"5/6",3) == 0) g_dvbs_fec = FEC_56;
	if(strncmp(fec,"7/8",3) == 0) g_dvbs_fec = FEC_78;
}

void cmd_set_dvbt_fec(const char *fec) {
	g_dvbt_fec = FEC_12;
	if (strncmp(fec, "1/2", 3) == 0) g_dvbt_fec = FEC_12;
	if (strncmp(fec, "2/3", 3) == 0) g_dvbt_fec = FEC_23;
	if (strncmp(fec, "3/4", 3) == 0) g_dvbt_fec = FEC_34;
	if (strncmp(fec, "5/6", 3) == 0) g_dvbt_fec = FEC_56;
	if (strncmp(fec, "7/8", 3) == 0) g_dvbt_fec = FEC_78;
}
void cmd_set_dvbt_constellation(const char *con) {
	g_dvbt_constellation = CO_QPSK;
	if (strncmp(con, "QPSK", 4) == 0) g_dvbt_constellation = CO_QPSK;
	if (strncmp(con, "16QAM", 5) == 0) g_dvbt_constellation = CO_16QAM;
	if (strncmp(con, "64QAM", 5) == 0) g_dvbt_constellation = CO_64QAM;
}
void cmd_set_dvbt_guard(const char *guard) {
	g_dvbt_guard = GI_14;
	if (strncmp(guard, "1/4", 3) == 0) g_dvbt_guard = GI_14;
	if (strncmp(guard, "1/8", 3) == 0) g_dvbt_guard = GI_18;
	if (strncmp(guard, "1/16", 4) == 0) g_dvbt_guard = GI_116;
	if (strncmp(guard, "1/32", 4) == 0) g_dvbt_guard = GI_132;
}
void cmd_set_dvbt_channel(const char *chan) {
	g_dvbt_channel = CH_8M;
	if (strncmp(chan, "8MHz", 4) == 0) g_dvbt_channel = CH_8M;
	if (strncmp(chan, "7MHz", 4) == 0) g_dvbt_channel = CH_7M;
	if (strncmp(chan, "6MHz", 4) == 0) g_dvbt_channel = CH_6M;
	if (strncmp(chan, "4MHz", 4) == 0) g_dvbt_channel = CH_4M;
	if (strncmp(chan, "3MHz", 4) == 0) g_dvbt_channel = CH_3M;
	if (strncmp(chan, "2MHz", 4) == 0) g_dvbt_channel = CH_2M;
	if (strncmp(chan, "1MHz", 4) == 0) g_dvbt_channel = CH_1M;
}
void cmd_set_dvbt_mode(const char *mode) {
	g_dvbt_mode = TM_2K;
	if (strncmp(mode, "2K", 2) == 0) g_dvbt_mode = TM_2K;
	if (strncmp(mode, "8K", 2) == 0) g_dvbt_mode = TM_8K;
}

void cmd_set_dvbs2_fec(const char *fec) {
	g_dvbs2_fec = FEC_12;
	if (strncmp(fec, "1/4", 3) == 0) g_dvbs2_fec = FEC_14;
	if (strncmp(fec, "1/3", 3) == 0) g_dvbs2_fec = FEC_13;
	if (strncmp(fec, "2/5", 3) == 0) g_dvbs2_fec = FEC_25;
	if (strncmp(fec, "1/2", 3) == 0) g_dvbs2_fec = FEC_12;
	if (strncmp(fec, "3/5", 3) == 0) g_dvbs2_fec = FEC_35;
	if (strncmp(fec, "2/3", 3) == 0) g_dvbs2_fec = FEC_23;
	if (strncmp(fec, "3/4", 3) == 0) g_dvbs2_fec = FEC_34;
	if (strncmp(fec, "4/5", 3) == 0) g_dvbs2_fec = FEC_45;
	if (strncmp(fec, "5/6", 3) == 0) g_dvbs2_fec = FEC_56;
	if (strncmp(fec, "8/9", 3) == 0) g_dvbs2_fec = FEC_89;
	if (strncmp(fec, "9/10", 4) == 0) g_dvbs2_fec = FEC_910;
}
void cmd_set_dvbs2_rolloff(const char *ro) {
	g_dvbs2_rolloff = RO_35;
	if (strncmp(ro, "0.35", 4) == 0) g_dvbs2_rolloff = RO_35;
	if (strncmp(ro, "0.25", 4) == 0) g_dvbs2_rolloff = RO_25;
	if (strncmp(ro, "0.20", 4) == 0) g_dvbs2_rolloff = RO_20;
}
void cmd_set_dvbs2_pilots(const char *pilots) {
	g_dvbs2_pilots = 0;
	if (strncmp(pilots, "OFF", 3) == 0) g_dvbs2_pilots = 0;
	if (strncmp(pilots, "ON",  2) == 0) g_dvbs2_pilots = 1;
}
void cmd_set_dvbs2_constellation(const char *cons) {
	g_dvbs2_constellation = QPSK;
	if (strncmp(cons, "QPSK", 4) == 0) g_dvbs2_constellation   = QPSK;
	if (strncmp(cons, "8PSK", 4) == 0) g_dvbs2_constellation   = PSK8;
	if (strncmp(cons, "16APSK", 5) == 0) g_dvbs2_constellation = APSK16;
	if (strncmp(cons, "32APSK", 5) == 0) g_dvbs2_constellation = APSK32;
}

void cmd_set_dvbs_freq(const char *freq){
	g_tx_frequency = strtoull(freq,NULL,0);
	hw_set_freq((double)g_tx_frequency);
}
void cmd_set_dvbs_level(const char *level){
	g_tx_level = atoi(level);
	if (g_tx_level > 47) g_tx_level = 47;
	if (g_tx_status == TRUE) hw_set_level(g_tx_level);
}
void cmd_set_tx_level(int level) {
	g_tx_level = level;
	if (g_tx_level > 47) g_tx_level = 47;
	if (g_tx_level < 0)  g_tx_level = 0;
	/*if(g_tx_status == TRUE)*/ hw_set_level(g_tx_level);
}
void cmd_set_tx_ports(const char *ports) {
	g_tx_ports = atoi(ports);
	g_tx_ports <<= 4;
	hw_set_ports(g_tx_ports);
}
void cmd_set_dvbs_srate(const char *srate){
	g_tx_sr = atoi(srate);
}
void cmd_set_video_pid(const char *pid){
	g_video_pid = atoi(pid);
}
void cmd_set_audio_pid(const char *pid){
	g_audio_pid = atoi(pid);
}
void cmd_set_pcr_pid(const char *pid){
	g_pcr_pid = atoi(pid);
}
void cmd_set_pmt_pid(const char *pid){
	g_pmt_pid = atoi(pid);
}
void cmd_set_event_title(const char *title){
	strncpy_s(g_event_title,title,strlen(title));
}
void cmd_set_event_duration(const char *duration){
	g_event_duration = atoi(duration);
}
void cmd_set_event_text(const char *text){
	strncpy_s(g_event_text,text,strlen(text));
}
void cmd_set_vbr_twiddle(const char *twiddle){
	g_vbr_twiddle = atof(twiddle);
}
void cmd_set_video_aspect_ratio(const char *fec){
	g_video_aspect_ratio = 43; 
	if(strncmp(fec,"1:1",3) == 0) g_video_aspect_ratio = 11;
	if(strncmp(fec,"4:3",3) == 0) g_video_aspect_ratio = 43;
	if(strncmp(fec,"16:9",4) == 0) g_video_aspect_ratio = 169;
}
void cmd_set_cap_interlaced(const char *val) {
	g_cap_interlaced = 0;
	if (strncmp(val, "yes", 3) == 0) g_cap_interlaced = 1;
	if (strncmp(val, "no", 2) == 0) g_cap_interlaced;
}

void cmd_set_onair_video_format(const char *format){
	strncpy_s(g_cap_onair_videoformat,format,strlen(format));
}

void cmd_set_error_text(const char *text) {
	g_error_text = text;
}

void cmd_set_video_codec_performance(const char *perform) {
	strncpy_s(g_video_codec_performance, perform, strlen(perform));
}

void cmd_set_audio_status(const char *status) {
	g_audio_present = TRUE;
	if (strncmp("false", status,5) == 0) g_audio_present = FALSE;
	if (strncmp("true", status,4) == 0)  g_audio_present = TRUE;
}

void cmd_set_audio_codec_bitrate(const char *rate) {
	g_audio_bitrate = atoi(rate);
}

void cmd_set_pfd(const char *freq) {
	g_pfd = strtoul(freq, NULL, 0);
}
void cmd_set_txmode(const char *mode) {
	g_txmode = M_DVBS;
	if (strncmp(mode, "DVB-S2", 6) == 0) 
		g_txmode = M_DVBS2;
	else
		if (strncmp(mode, "DVB-S", 5) == 0) 
			g_txmode = M_DVBS;
		else
			if (strncmp(mode, "DVB-T", 5) == 0)
				g_txmode = M_DVBT;
}

void cmd_set_video_ident(BOOL ident) {
	g_video_ident = ident;
}
void cmd_set_i_dc_offset(int i) {
	g_i_dc_offset = i;
}
void cmd_set_q_dc_offset(int q) {
	g_q_dc_offset = q;
}
void cmd_set_sdrhw_type(const char *hw) {
	g_sdrhw_type = HW_DATV_EXPRESS;
	if (strncmp(hw, "EXPRESS", 7) == 0) g_sdrhw_type = HW_DATV_EXPRESS;
	if (strncmp(hw, "PLUTO", 5)   == 0) g_sdrhw_type = HW_ADALM_PLUTO;
	if (strncmp(hw, "LIME", 4)    == 0) g_sdrhw_type = HW_LIME_SDR;
	if (strncmp(hw, "FMCOMMS", 7) == 0) g_sdrhw_type = HW_FMCOMMSx;
}

void cmd_set_sdr_ip_addr(DWORD a) {
	g_sdr_ip_addr = a;
}
void cmd_set_sdr_ip_addr(const char *ip) {
	sscanf_s(ip, "%X", &g_sdr_ip_addr);
}

void cmd_set_TsIn_ip_addr(DWORD a,WORD port,DWORD localnic) {
	g_TsIn_ip_addr = a;
	g_TsIn_ip_port = port;
	g_TsIn_LocalNic = localnic;
}

void save_config_text( FILE *fp){
	char text[20];

	strcpy_s(text, "EXPRESS");
	if (g_sdrhw_type == HW_DATV_EXPRESS) strcpy_s(text, "EXPRESS");
	if (g_sdrhw_type == HW_LIME_SDR) strcpy_s(text, "LIME");
	if (g_sdrhw_type == HW_ADALM_PLUTO) strcpy_s(text, "PLUTO");
	if (g_sdrhw_type == HW_FMCOMMSx) strcpy_s(text, "FMCOMMS");
	fprintf(fp, "[SDR HW] %s\n", text);
	
	sprintf_s(text, 20, "%8X", get_sdr_ip_addr());
	fprintf(fp, "[SDR IP] %s\n", text);


	fprintf(fp,"[ID SERVICE] %d\n",g_service_id); 
    fprintf(fp,"[ID STREAM] %d\n",g_stream_id); 
    fprintf(fp,"[ID NETWORK] %d\n",g_network_id); 
    fprintf(fp,"[EVENT DURATION] %d\n", g_event_duration); 
    fprintf(fp,"[EVENT TITLE] \"%s\"\n",g_event_title); 
    fprintf(fp,"[EVENT TEXT] \"%s\"\n",g_event_text);
    fprintf(fp,"[PID PMT] %d\n",g_pmt_pid); 
    fprintf(fp,"[PID PCR] %d\n",g_pcr_pid); 
    fprintf(fp,"[PID VIDEO] %d\n",g_video_pid); 
    fprintf(fp,"[PID AUDIO] %d\n",g_audio_pid); 
    fprintf(fp,"[PROGRAM NR] %d\n",g_program_nr);

	strcpy_s(text,"h262");
	if(g_video_codec == AV_CODEC_ID_MPEG2VIDEO ) strcpy_s(text,"h262");
	if(g_video_codec == AV_CODEC_ID_H264 ) strcpy_s(text,"h264");
	if(g_video_codec == AV_CODEC_ID_HEVC ) strcpy_s(text,"h265");
    fprintf(fp,"[CODEC VIDEO] %s\n",text); 
	fprintf(fp,"[CODEC VIDEO GOP] %d\n",g_video_gop); 
	fprintf(fp,"[CODEC VIDEO B FRAMES] %d\n",g_video_b_frames); 
	fprintf(fp,"[CODEC VIDEO PERFORMANCE] \"%s\"\n", g_video_codec_performance);

	strcpy_s(text,"mp2");
	if(g_audio_codec == AV_CODEC_ID_MP2 ) strcpy_s(text,"mp1");
	if(g_audio_codec == AV_CODEC_ID_AAC ) strcpy_s(text,"aac");
    fprintf(fp,"[CODEC AUDIO] %s\n",text); 
	strcpy_s(text, "true");
    if(g_audio_present == TRUE) strcpy_s(text, "true");
	if(g_audio_present == FALSE) strcpy_s(text, "false");
	fprintf(fp, "[CODEC AUDIOPRESENT] %s\n", text);
	fprintf(fp, "[CODEC AUDIOBITRATE] %d\n", g_audio_bitrate);

    fprintf(fp,"[NAME PROVIDER] \"%s\"\n",g_service_provider_name); 
    fprintf(fp,"[NAME SERVICE] \"%s\"\n",g_service_name);
	if (g_txmode == M_DVBS) strcpy_s(text, "DVB-S");
	if (g_txmode == M_DVBS2) strcpy_s(text, "DVB-S2");
	if (g_txmode == M_DVBT) strcpy_s(text, "DVB-T");
	fprintf(fp, "[TX MODE] %s\n", text);

    fprintf(fp,"[TX FREQ] %llu\n",g_tx_frequency);
    fprintf(fp,"[TX LEVEL] %d\n",g_tx_level);

	strcpy_s(text,"1/2");
	if(g_dvbs_fec == FEC_12 ) strcpy_s(text,"1/2");
	if(g_dvbs_fec == FEC_23 ) strcpy_s(text,"2/3");
	if(g_dvbs_fec == FEC_34 ) strcpy_s(text,"3/4");
	if(g_dvbs_fec == FEC_56 ) strcpy_s(text,"5/6");
	if(g_dvbs_fec == FEC_78 ) strcpy_s(text,"7/8");
    fprintf(fp,"[DVBS FEC] %s\n",text);

	strcpy_s(text, "1/2");
	if (g_dvbt_fec == FEC_12) strcpy_s(text, "1/2");
	if (g_dvbt_fec == FEC_23) strcpy_s(text, "2/3");
	if (g_dvbt_fec == FEC_34) strcpy_s(text, "3/4");
	if (g_dvbt_fec == FEC_56) strcpy_s(text, "5/6");
	if (g_dvbt_fec == FEC_78) strcpy_s(text, "7/8");
	fprintf(fp, "[DVBT FEC] %s\n", text);

	strcpy_s(text, "QPSK");
	if (g_dvbt_constellation == CO_QPSK)  strcpy_s(text, "QPSK");
	if (g_dvbt_constellation == CO_16QAM) strcpy_s(text, "16QAM");
	if (g_dvbt_constellation == CO_64QAM) strcpy_s(text, "64QAM");
	fprintf(fp, "[DVBT CONSTELLATION] %s\n", text);

	strcpy_s(text, "1/4");
	if (g_dvbt_guard == GI_14) strcpy_s(text, "1/4");
	if (g_dvbt_guard == GI_18) strcpy_s(text, "1/8");
	if (g_dvbt_guard == GI_116) strcpy_s(text, "1/16");
	if (g_dvbt_guard == GI_132) strcpy_s(text, "1/32");
	fprintf(fp, "[DVBT GUARD] %s\n", text);

	strcpy_s(text, "8MHZ");
	if (g_dvbt_channel == CH_8M) strcpy_s(text, "8MHz");
	if (g_dvbt_channel == CH_7M) strcpy_s(text, "7MHz");
	if (g_dvbt_channel == CH_6M) strcpy_s(text, "6MHz");
	if (g_dvbt_channel == CH_4M) strcpy_s(text, "4MHz");
	if (g_dvbt_channel == CH_3M) strcpy_s(text, "3MHz");
	if (g_dvbt_channel == CH_2M) strcpy_s(text, "2MHz");
	if (g_dvbt_channel == CH_1M) strcpy_s(text, "1MHz");
	fprintf(fp, "[DVBT CHANNEL] %s\n", text);

	strcpy_s(text, "2K");
	if (g_dvbt_mode == TM_2K) strcpy_s(text, "2K");
	if (g_dvbt_mode == TM_8K) strcpy_s(text, "8K");
	fprintf(fp, "[DVBT MODE] %s\n", text);


	if (g_dvbs2_fec == FEC_14) strcpy_s(text, "1/4");
	if (g_dvbs2_fec == FEC_13) strcpy_s(text, "1/3");
	if (g_dvbs2_fec == FEC_25) strcpy_s(text, "2/5");
	if (g_dvbs2_fec == FEC_12) strcpy_s(text, "1/2");
	if (g_dvbs2_fec == FEC_35) strcpy_s(text, "3/5");
	if (g_dvbs2_fec == FEC_23) strcpy_s(text, "2/3");
	if (g_dvbs2_fec == FEC_34) strcpy_s(text, "3/4");
	if (g_dvbs2_fec == FEC_45) strcpy_s(text, "4/5");
	if (g_dvbs2_fec == FEC_56) strcpy_s(text, "5/6");
	if (g_dvbs2_fec == FEC_89) strcpy_s(text, "8/9");
	if (g_dvbs2_fec == FEC_910) strcpy_s(text, "9/10");
	fprintf(fp, "[DVBS2 FEC] %s\n", text);

	if (g_dvbs2_pilots == 0) strcpy_s(text, "OFF");
	if (g_dvbs2_pilots == 1) strcpy_s(text, "ON");
	fprintf(fp, "[DVBS2 PILOTS] %s\n", text);

	if (g_dvbs2_constellation == QPSK)   strcpy_s(text, "QPSK");
	if (g_dvbs2_constellation == PSK8)   strcpy_s(text, "8PSK");
	if (g_dvbs2_constellation == APSK16) strcpy_s(text, "16APSK");
	if (g_dvbs2_constellation == APSK32) strcpy_s(text, "32APSK");
	fprintf(fp, "[DVBS2 CONSTELLATION] %s\n", text);

	if (g_dvbs2_rolloff == RO_35) strcpy_s(text, "0.35");
	if (g_dvbs2_rolloff == RO_25) strcpy_s(text, "0.25");
	if (g_dvbs2_rolloff == RO_20) strcpy_s(text, "0.20");
	fprintf(fp, "[DVBS2 ROLLOFF] %s\n", text);

	fprintf(fp,"[TX SR] %d\n",g_tx_sr);
	fprintf(fp, "[TX PORTS] %d\n", g_tx_ports>>4);
	fprintf(fp,"[CAPTURE VIDEODEVICE] \"%s\"\n",g_cap_videodevice);
	fprintf(fp,"[CAPTURE VIDEOFORMAT] \"%s\"\n",g_cap_videoformat);
	if (g_cap_interlaced) 
		strcpy_s(text, "yes");
	else
		strcpy_s(text, "no");
	fprintf(fp, "[CAPTURE VIDEOINTERLACED] %s\n", text);
	strcpy_s(text,"4:3");
	if(g_video_aspect_ratio == 11 ) strcpy_s(text,"1:1");
	if(g_video_aspect_ratio == 43 ) strcpy_s(text,"4:3");
	if(g_video_aspect_ratio == 169 ) strcpy_s(text,"16:9");
	fprintf(fp,"[CAPTURE VIDEOASPECT] %s\n",text);

	fprintf(fp,"[CAPTURE AUDIODEVICE] \"%s\"\n",g_cap_audiodevice);
	fprintf(fp,"[TWIDDLE VBITRATE] %f\n",g_vbr_twiddle);
	fprintf(fp,"[ONAIR VIDEOFORMAT] \"%s\"\n",g_cap_onair_videoformat);

	if (g_video_ident == TRUE) strcpy_s(text, "ON");
	if (g_video_ident == FALSE) strcpy_s(text, "OFF");
	fprintf(fp, "[VIDEO IDENT] %s\n", text);

	fprintf(fp, "[I DC OFFSET] %d\n", g_i_dc_offset);
	fprintf(fp, "[Q DC OFFSET] %d\n", g_q_dc_offset);

	fprintf(fp, "[TSIN ADRESS] %d\n", g_TsIn_ip_addr);
	fprintf(fp, "[TSIN PORT] %d\n", g_TsIn_ip_port);


}
void copy_quoted_string( char *in, char *out ){
	uint32_t state = 0;
	uint32_t n = 0;
	for( uint32_t i = 0; i < strlen(in); i++ ){
		if(in[i] == '"'){
			state++;
		}else{
			if (state > 1) {
				out[n] = 0;// Null terminate the string
				return;
			}
			if(state == 1) out[n++] = in[i];
		}
	}
}
void parse_config_line( char *line ){
	char *item,*value;
	uint32_t is,ie;
	// Find the item
	item = NULL;
	for( uint32_t i = 0; i < strlen(line); i++ ){
		if(line[i] == '['){
			item = &line[i];
			is = i;
			break;
		}
	}
	if(item == NULL ) return;
	// Find the value
	value = NULL;
	for( uint32_t i = 0; i < strlen(line)-2; i++ ){
		if(line[i] == ']'){
			value = &line[i+2];
			ie = i;
			break;
		}
	}
	if(value == NULL ) return;
    int len = ie - is;

	if (strncmp("[SDR HW]", item, len) == 0) {
		cmd_set_sdrhw_type(value);
		return;
	}
	if (strncmp("[SDR IP]", item, len) == 0) {
		cmd_set_sdr_ip_addr(value);
		return;
	}

    if(strncmp("[ID SERVICE]",item,len) == 0 ){
		g_service_id = atoi(value);
		return;
	}
    if(strncmp("[ID STREAM]",item,len) == 0 ){
		g_stream_id = atoi(value);
		return;
	}
	if(strncmp("[ID NETWORK]",item,len) == 0 ){
		g_network_id = atoi(value);
		return;
	}
    if(strncmp("[EVENT DURATION]",item,len) == 0 ){
		g_event_duration = atoi(value);
		return;
	}
    if(strncmp("[EVENT TITLE]",item,len) == 0 ){
		copy_quoted_string( value, g_event_title );
		return;
	}
    if(strncmp("[EVENT TEXT]",item,len) == 0 ){
		copy_quoted_string( value, g_event_text );
		return;
	}
    if(strncmp("[PID PMT]",item,len) == 0 ){
		g_pmt_pid = atoi(value);
		return;
	}
    if(strncmp("[PID PCR]",item,len) == 0 ){
		cmd_set_pcr_pid(value);
		return;
	}
    if(strncmp("[PID VIDEO]",item,len) == 0 ){
		cmd_set_video_pid(value);
		return;
	}
    if(strncmp("[PID AUDIO]",item,len) == 0 ){
		cmd_set_audio_pid(value);
		return;
	}
    if(strncmp("[PROGRAM NR]",item,len) == 0 ){
		g_program_nr = atoi(value);
		return;
	}
    if(strncmp("[CODEC VIDEO]",item,len) == 0 ){
		cmd_set_video_codec(value);
		return;
	}
    if(strncmp("[CODEC VIDEO GOP]",item,len) == 0 ){
		cmd_set_video_gop(value);
		return;
	}
    if(strncmp("[CODEC VIDEO B FRAMES]",item,len) == 0 ){
		cmd_set_video_b_frames(value);
		return;
	}
	if (strncmp("[CODEC VIDEO PERFORMANCE]", item, len) == 0) {
		copy_quoted_string(value, g_video_codec_performance);
		return;
	}
	if(strncmp("[CODEC AUDIO]",item,len) == 0 ){
		cmd_set_audio_codec(value);
		return;
	}
	if (strncmp("[CODEC AUDIOPRESENT]", item, len) == 0) {
		cmd_set_audio_status(value);
		return;
	}
	if (strncmp("[CODEC AUDIOBITRATE]", item, len) == 0) {
		cmd_set_audio_codec_bitrate(value);
		return;
	}

	if(strncmp("[NAME PROVIDER]",item,len) == 0 ){
		copy_quoted_string( value, g_service_provider_name );
		return;
	}
    if(strncmp("[NAME SERVICE]",item,len) == 0 ){
		copy_quoted_string( value, g_service_name );
		return;
	}
    if(strncmp("[TX FREQ]",item,len) == 0 ){
		cmd_set_dvbs_freq(value);
		return;
	}
    if(strncmp("[TX LEVEL]",item,len) == 0 ){
		cmd_set_dvbs_level(value);
		return;
	}
    if(strncmp("[DVBS FEC]",item,len) == 0 ){
		cmd_set_dvbs_fec(value);
		return;
	}

	if (strncmp("[DVBT FEC]", item, len) == 0) {
		cmd_set_dvbt_fec(value);
		return;
	}
	if (strncmp("[DVBT CHANNEL]", item, len) == 0) {
		cmd_set_dvbt_channel(value);
		return;
	}
	if (strncmp("[DVBT GUARD]", item, len) == 0) {
		cmd_set_dvbt_guard(value);
		return;
	}
	if (strncmp("[DVBT MODE]", item, len) == 0) {
		cmd_set_dvbt_mode(value);
		return;
	}
	if (strncmp("[DVBT CONSTELLATION]", item, len) == 0) {
		cmd_set_dvbt_constellation(value);
		return;
	}

	if (strncmp("[DVBS2 FEC]", item, len) == 0) {
		cmd_set_dvbs2_fec(value);
		return;
	}
	if (strncmp("[DVBS2 ROLLOFF]", item, len) == 0) {
		cmd_set_dvbs2_rolloff(value);
		return;
	}
	if (strncmp("[DVBS2 PILOTS]", item, len) == 0) {
		cmd_set_dvbs2_pilots(value);
		return;
	}
	if (strncmp("[DVBS2 CONSTELLATION]", item, len) == 0) {
		cmd_set_dvbs2_constellation(value);
		return;
	}

	if(strncmp("[TX SR]",item,len) == 0 ){
		cmd_set_dvbs_srate(value);
		return;
	}
	if (strncmp("[TX PORTS]", item, len) == 0) {
		cmd_set_tx_ports(value);
		return;
	}

	if(strncmp("[CAPTURE VIDEODEVICE]",item,len) == 0 ){
		copy_quoted_string( value, g_cap_videodevice );
		return;
	}
	if(strncmp("[CAPTURE VIDEOFORMAT]",item,len) == 0 ){
		copy_quoted_string( value, g_cap_videoformat );
		return;
	}

	if(strncmp("[CAPTURE AUDIODEVICE]",item,len) == 0 ){
		copy_quoted_string( value, g_cap_audiodevice );
		return;
	}
	if(strncmp("[TWIDDLE VBITRATE]",item,len) == 0 ){
		cmd_set_vbr_twiddle( value );
		return;
	}
	if(strncmp("[CAPTURE VIDEOASPECT]",item,len) == 0 ){
		cmd_set_video_aspect_ratio( value );
		return;
	}
	if (strncmp("[CAPTURE VIDEOINTERLACED]", item, len) == 0) {
		cmd_set_cap_interlaced(value);
		return;
	}
	if(strncmp("[ONAIR VIDEOFORMAT]",item,len) == 0 ){
		copy_quoted_string( value, g_cap_onair_videoformat );
		return;
	}
	if (strncmp("[SYNTH PFD]", item, len) == 0) {
		cmd_set_pfd(value);
		return;
	}
	if (strncmp("[TX MODE]", item, len) == 0) {
		cmd_set_txmode(value);
		return;
	}

	if (strncmp("[VIDEO IDENT]", item, len) == 0) {
		g_video_ident = FALSE;
		if( strncmp("ON", value, 2)  == 0 ) g_video_ident = TRUE;
 		if( strncmp("OFF", value, 3) == 0 ) g_video_ident = FALSE;
		return;
	}
	if (strncmp("[I DC OFFSET]", item, len) == 0) {
		g_i_dc_offset = atoi(value);
		return;
	}
	if (strncmp("[Q DC OFFSET]", item, len) == 0) {
		g_q_dc_offset = atoi(value);
		return;
	}
	if (strncmp("[TSIN ADRESS]]", item, len) == 0) {
		g_TsIn_ip_addr = atoi(value);
		return;
	}
	if (strncmp("[TSIN PORT]", item, len) == 0) {
		g_TsIn_ip_port = atoi(value);
		return;
	}
}
//
// Read in Configuration information from Disk
//
int LoadConfigFromDisk(const char *name)
{
	char directory[250];
	char line[2048];
	g_config_filename = name;
	if(GetCurrentDirectory(250,directory)){
		strncat_s(directory, "\\",2);
		strncat_s(directory,name,strlen(name));
		FILE *fp;
		if(fopen_s(&fp,directory,"r") == 0 ){
			while(fgets(line, sizeof(line),fp)!= NULL){
                parse_config_line(line);
			}
			fclose(fp);
			return 0;
		}
	}
	report_error(ERROR_CONFIG_NOT_FOUND);
	return -1;
}
//
// Save the configuration information to disk
// called after OK exit
//
void SaveConfigToDisk(const char *name)
{
	char filename[2][250];
	if(GetCurrentDirectory(250,filename[0])){
		strncat_s(filename[0], "\\", 2);
		strncat_s(filename[0],name,strlen(name));
  		FILE *fp;
		if(fopen_s(&fp,filename[0],"w") == 0 ){
            save_config_text(fp);
		    fclose(fp);
		}
	}
}//
// Load on air formats from Disk
//
int LoadOnAirFormatsFromDisk(int nr, CString *fmts)
{
	char directory[250];
	char line[2048];
	int n = 0;

	if (GetCurrentDirectory(250, directory)) {
		strncat_s(directory, "\\onairformats.txt", 80);
		FILE *fp;
		if (fopen_s(&fp, directory, "r") == 0) {
			while ((fgets(line, sizeof(line), fp) != NULL)&&(n < nr)) {
				// Remove empty lines
				if (strlen(line) > 5) {
					line[strlen(line)-1] = 0;// Remove CR
					fmts[n] = line;
					n++;
				}
			}
			fclose(fp);
		}
	}
	return n;
}

void SetExpressChannel(void) {
	hw_set_fec(g_dvbs_fec);
	hw_set_freq((double)g_tx_frequency);
	hw_set_ical(g_i_dc_offset);
	hw_set_qcal(g_q_dc_offset);
	if (get_txmode() == M_DVBT) {
		hw_set_sr(dvb_t_get_sample_rate());
		hw_set_analogue_lpf(dvb_t_get_channel_bandwidth()*1.5f);
	}
	else{
		hw_set_sr(g_tx_sr);
		hw_set_analogue_lpf(g_tx_sr*0.75f);
	}
	hw_set_filter(g_dvbs2_rolloff);
}
void DeConfigureExpress(void)
{
	hw_deinit();
}
int get_video_codec(void){
	return g_video_codec;
}
uint32_t get_video_gop(void){
	return g_video_gop;
}
uint32_t get_video_b_frames(void){
	return g_video_b_frames;
}

const char *get_video_capture_device(void){
	return g_cap_videodevice;
}
const char *get_video_capture_format(void){
	return g_cap_videoformat;
}

CString get_config_filename(void) {
	return g_config_filename;
}

int get_video_capture_format(uint32_t *w, uint32_t *h, int *fps, int *fmt){
	char text[100];
	sscanf_s(g_cap_videoformat, "%d%d%d%s", w, h, fps,text,(unsigned)sizeof(text));
	*fmt = -1;
	if(strcmp(text,"UYVY") == 0 )  *fmt = AV_PIX_FMT_UYVY422; 
	if(strcmp(text,"YUY2") == 0 )  *fmt = AV_PIX_FMT_YUYV422; 
	if(strcmp(text,"RGB24") == 0 ) *fmt = AV_PIX_FMT_RGB24; 
	if(strcmp(text,"MJPG") == 0 )  *fmt = AV_PIX_FMT_JPEG; 
	if(strcmp(text,"I420") == 0 )  *fmt = AV_PIX_FMT_YUV420P;
	if(strcmp(text,"HDYC") == 0 )  *fmt = AV_PIX_FMT_UYVY422;
	if (*fmt != -1)
		return S_OK;
	else {
		cmd_set_error_text("No Video Capture format");
		return -1;
	}
}
void get_output_format( uint32_t *w, uint32_t *h, int *fps ){
	if(strcmp(g_cap_onair_videoformat,"Input Format")==0 ) return;
	sscanf_s(g_cap_onair_videoformat, "%d%d%d", w, h, fps);
}

const char *get_onair_video_format(void){
	return g_cap_onair_videoformat;
}

int get_audio_codec(void){
	return g_audio_codec;
}

const char *get_audio_capture_device(void){
	return g_cap_audiodevice;
}

CString get_current_fec_string(void){
	CString fec = "1/2";
	if (g_txmode == M_DVBS) {
		if (g_dvbs_fec == FEC_12) fec = "1/2";
		if (g_dvbs_fec == FEC_23) fec = "2/3";
		if (g_dvbs_fec == FEC_34) fec = "3/4";
		if (g_dvbs_fec == FEC_56) fec = "5/6";
		if (g_dvbs_fec == FEC_78) fec = "7/8";
	}
	if (g_txmode == M_DVBT) {
		if (g_dvbt_fec == FEC_12) fec = "1/2";
		if (g_dvbt_fec == FEC_23) fec = "2/3";
		if (g_dvbt_fec == FEC_34) fec = "3/4";
		if (g_dvbt_fec == FEC_56) fec = "5/6";
		if (g_dvbt_fec == FEC_78) fec = "7/8";
	}
	if (g_txmode == M_DVBS2) {
		if (g_dvbs2_fec == FEC_14) fec = "1/4";
		if (g_dvbs2_fec == FEC_13) fec = "1/3";
		if (g_dvbs2_fec == FEC_25) fec = "2/5";
		if (g_dvbs2_fec == FEC_12) fec = "1/2";
		if (g_dvbs2_fec == FEC_35) fec = "3/5";
		if (g_dvbs2_fec == FEC_23) fec = "2/3";
		if (g_dvbs2_fec == FEC_34) fec = "3/4";
		if (g_dvbs2_fec == FEC_45) fec = "4/5";
		if (g_dvbs2_fec == FEC_56) fec = "5/6";
		if (g_dvbs2_fec == FEC_89) fec = "8/9";
		if (g_dvbs2_fec == FEC_910) fec = "9/10";
	}
	return fec;
}
uint64_t get_current_tx_frequency(void){
	return g_tx_frequency;
}
int get_current_tx_level(void){
	return g_tx_level;
}
uint32_t get_current_tx_symbol_rate(void){
	return g_tx_sr;
}

void cmd_transmit(void){
	if (g_system_status == TRUE) {
		capture_run();
		hw_transmit();
		g_tx_status = TRUE;
	}
}
void cmd_standby(void){
	if (g_system_status == TRUE) {
		tx_buf_empty();
		capture_pause();
		hw_receive();
		g_tx_status = FALSE;
	}
}

void cmd_ip_transmit(DWORD IP,int Port) {
	if (g_system_status == TRUE) {
		TsIn_run();
		hw_transmit();
		g_tx_status = TRUE;
	}
}
void cmd_ip_standby(void) {
	if (g_system_status == TRUE) {
		tx_buf_empty();
		TsIn_pause();
		hw_receive();
		g_tx_status = FALSE;
	}
}

void cmd_tx_frequency( const char * freq){
	g_tx_frequency = atol(freq);
	hw_set_freq((double)g_tx_frequency);
}
void cmd_tx_level( const char * level){
	g_tx_level = atol(level);
	hw_set_level(g_tx_level);
}
void cmd_tx_symbol_rate( const char *sr){
	g_tx_sr = atol(sr);
	hw_set_level(g_tx_sr);
}
void cmd_tx_fec( const char *fec){
	g_dvbs_fec = FEC_12;
	if(strncmp(fec,"1/2",3) == 0) g_dvbs_fec = FEC_12;
	if(strncmp(fec,"2/3",3) == 0) g_dvbs_fec = FEC_23;
	if(strncmp(fec,"3/4",3) == 0) g_dvbs_fec = FEC_34;
	if(strncmp(fec,"5/6",3) == 0) g_dvbs_fec = FEC_56;
	if(strncmp(fec,"7/8",3) == 0) g_dvbs_fec = FEC_78;

	hw_set_fec(g_dvbs_fec);
}
void cmd_set_carrier(int chk ){
	g_tx_carrier = chk;
	hw_set_carrier(chk);
}
void cmd_set_config_filename(CString name) {
	g_config_filename = name;
}

int system_restart(void) {
	if (hw_init() < 0) return -1;
	codec_init();
	SetExpressChannel();
	configure_si();
	set_modem_params();
	init_dvb_clock();
	CodecParams params;
	params.v_br = get_video_bitrate();
	params.a_br = get_audio_bitrate();
	params.v_codec = g_video_codec;
	params.a_codec = g_audio_codec;
	//params.a_codec = AV_CODEC_ID_MP2;// Force it to this as AAC requires Planar floating point
	// Retrieve the capture devices aspect ratio
	params.v_ar[0] = 1;
	params.v_ar[1] = 1;
	if (g_video_aspect_ratio == 11) {
		params.v_ar[0] = 1;
		params.v_ar[1] = 1;
	}
	if (g_video_aspect_ratio == 43) {
		params.v_ar[0] = 4;
		params.v_ar[1] = 3;
	}
	if (g_video_aspect_ratio == 169) {
		params.v_ar[0] = 16;
		params.v_ar[1] = 9;
	}
	params.v_dst_fpf = params.v_src_fpf = 2;

	// Load the format of the selected capture device
	if (get_video_capture_format(&params.v_src_width, &params.v_src_height, &params.v_src_fps, &params.v_src_pixfmt) == S_OK) {
		// Assume the output format is the same as the input format
		params.v_dst_width = params.v_src_width;
		params.v_dst_height = params.v_src_height;
		params.v_dst_fps = params.v_src_fps;
		// Now set the output format if different
		get_output_format(&params.v_dst_width, &params.v_dst_height, &params.v_dst_fps);
		// The destination FPS determins the transmitted FPS
		params.v_src_fps = params.v_dst_fps;
		params.v_dst_i = g_cap_interlaced;//not interlaced

		if (capture_start(&params) == S_OK) {
			codec_start(&params);
			ps_to_ts_init();
			tx_start();
			hw_receive();
			return 0;
		}
	}
	return -1;
}

// So we can find the string to save to the registry if we have to use a default

#include "ExpressCaptureServer.h"
extern CExpressCaptureServerApp theApp;

int system_start(const char *name){
	// Set up some default values here
	g_vbr_twiddle = 0.9f;
	g_system_status = FALSE;
	g_tx_status = FALSE;
	// Continue
	tx_buf_init();
	vb_av_init();
	udp_init();
	noise_init();

	CString msg;
	
	load_defaults();

	if(LoadConfigFromDisk(name)==0){
		set_modem_params();
		if (system_restart() == S_OK) {
			g_system_status = TRUE;
			return S_OK;
		}
		else {
			return -1;
		}
	}

	msg.Format("%s not found, trying datvexpress.cfg", name);
	::MessageBox(NULL,msg,"Warning",0);
	theApp.m_sConfigFile = "datvexpress.cfg";

	if (LoadConfigFromDisk("datvexpress.cfg") == 0) {
		set_modem_params();
		if (system_restart() == S_OK) {
			g_system_status = TRUE;
			return S_OK;
		}
		else {
			return -1;
		}
	}

	msg.Format("datvexpress.cfg not found, defaults being used");
	::MessageBox(NULL, msg, "Warning", 0);
	set_modem_params();
	if (system_restart() == S_OK) {
		g_system_status = TRUE;
		return S_OK;
	}
	else {
		return -1;
	}
	return -1;
}
void system_stop(void) {
	if (g_system_status == TRUE) {
		hw_receive();
		capture_stop();
		tx_stop();
		stop_log();
		hw_deinit();
	}
}

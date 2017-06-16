#pragma once
#include "Dvb.h"
#include "tp.h"

void f_create_si_seq( tp_si_seq *cblk );
void nit_dvb( void );
void padding_null_dvb( void );
void pat_dvb( void );
void pmt_dvb( void );
void sdt_dvb( void );
void tdt_dvb( void );
void eit_dvb( void );
void eit_fmt( int service_id, int stream_id, int network_id, int duration, char *title, char *text );
void nit_fmt( int nit_pid, int stream_id, int network_id, int service_id );
void pat_fmt( int stream_id, int nit_pid, int pmt_pid, int program_nr );
void pmt_fmt( int video_codec, int audio_codec, int pmt_pid, int pcr_pid, int video_pid, int audio_pid, int program_nr );
void sdt_fmt( int stream_id, int network_id, int service_id, char *service_provider_name, char *service_name );

#pragma once
#include <stdint.h>

#define TP_SYNC  0x47
#define TP_CHANS 10
#define TP_LEN 188
#define TP_STREAM_ID 0x001
#define VIDEO_CHAN 0
#define AUDIO_CHAN 1
#define SYSTM_CHAN 2
#define PGM_ID     1
#define TDT_FLAG   0x33
#define PES_PAYLOAD_LENGTH 184
#define SI_PAYLOAD_LENGTH  184

// Transport Error 
#define TRANSPORT_ERROR_FALSE 0
#define TRANSPORT_ERROR_TRUE  1

// Transport priority
#define TRANSPORT_PRIORITY_LOW  0
#define TRANSPORT_PRIORITY_HIGH 1

// Payload start
#define PAYLOAD_START_FALSE 0
#define PAYLOAD_START_TRUE  1


// Adaption field values
#define ADAPTION_RESERVED     0
#define ADAPTION_PAYLOAD_ONLY 1
#define ADAPTION_FIELD_ONLY   2
#define ADAPTION_BOTH         3

// Scrambling field
#define SCRAMBLING_OFF    0
#define SCRAMBLING_USER_1 1
#define SCRAMBLING_USER_2 2
#define SCRAMBLING_USER_3 3
#define SVC_DIGITAL_TV   0x01

#define SI_DESC_SERVICE  0x48
#define SI_DESC_NET_NAME 0x40
#define SI_DESC_SVC_LST  0x41
#define SI_DESC_EXT_EVNT 0x43
#define SI_DESC_SE       0x4D
#define SI_DESC_CONTENT  0x54
#define SI_DESC_TELETEXT 0x56
// Transport stream packet format

typedef struct{
    uint8_t transport_error_indicator;
	uint8_t payload_unit_start_indicator;
	uint8_t transport_priority;
    uint32_t  pid;
	uint8_t transport_scrambling_control;
	uint8_t adaption_field_control;
	uint8_t continuity_counter;
}tp_hdr;

typedef struct{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
}dvb_si_time;

typedef struct{
    int hour;
    int minute;
    int second;
}dvb_si_duration;

int  dvb_si_add_time( uint8_t *b, dvb_si_time *t);
int  dvb_si_add_duration( uint8_t *b, dvb_si_duration *d);
void dvb_si_system_time( dvb_si_time *t );

// Program map section
// Descriptors
// Video
typedef struct{
	uint8_t multiple_frame_rate_flag;
	uint8_t frame_rate_code;
	uint8_t mpeg_1_only_flag;
	uint8_t constrained_parameter_flag;
	uint8_t still_picture_flag;
	uint8_t profile_and_level_indication;
	uint8_t chroma_format;
	uint8_t frame_rate_extension_flag;
}tp_v_desc;

// Audio
typedef struct{
	uint8_t free_format_flag;
	uint8_t id;
	uint8_t layer;
	uint8_t variable_rate_audio_indicator;
}tp_a_desc;

// Hierarchy descriptor
typedef struct{
	uint8_t tag;
	uint8_t length;
	uint8_t type;
	uint8_t layer_index;
	uint8_t embedded_layer_index;
	uint8_t channel;
}tp_h_desc;

// Registration descriptor
typedef struct{
	uint8_t tag;
	uint8_t length;
	uint8_t format_indentifier;
	uint8_t info_length;
	uint8_t info[1024];
}tp_r_desc;

// Descriptor
typedef union{
	uint8_t table_id;
	tp_v_desc video;
	tp_a_desc audio;
	tp_h_desc hier;
	tp_r_desc reg;
}td_descriptor;



typedef struct{
	int service_id;
	uint8_t service_type;
}si_svc_tbl_entry;

typedef struct{
	uint8_t tag;
	int table_length;
	si_svc_tbl_entry entry[10];
}si_svc_list_desc;

typedef struct{
	uint8_t tag;
	uint8_t name_length;
	uint8_t name[1024];
}si_net_name_desc;

typedef struct{
	uint8_t tag;
	uint8_t type;
	uint8_t provider_length;
	uint8_t provider[1024];
	uint8_t name_length;
	uint8_t name[1024];
}si_service_descriptor;

typedef struct{
	uint8_t item_description_length;
	uint8_t item_desc_text[300];
	uint8_t item_length;
	uint8_t item_text[300];
}ee_item;

typedef struct{
	uint8_t tag;
	uint8_t length;
	uint8_t number;
	uint8_t last_number;
	unsigned long iso_639_language_code;
	uint8_t length_of_items;
	ee_item items[10];
	uint8_t text_length;
	uint8_t text[300];
}si_ee_descriptor;

typedef struct{
    uint8_t tag;
    int iso_639_language_code;
    const char *event_name;
    const char *event_text;
}si_se_descriptor;

typedef struct{
    uint8_t l1_nibble;
    uint8_t l2_nibble;
    uint8_t u1_nibble;
    uint8_t u2_nibble;
}content_nibbles;

typedef struct{
    uint8_t tag;
    int nr_items;
    content_nibbles item[5];
}si_ct_descriptor;

typedef struct{
    uint8_t iso_language_code[3];
    uint8_t teletext_type;
    uint8_t teletext_magazine_number;
    uint8_t teletext_page_number;
}si_teletext_item;

typedef struct{
    uint8_t tag;
    int nr_items;
    si_teletext_item item[5];
}si_tt_descriptor;

typedef union{
	uint8_t tag;
	si_service_descriptor sd;
	si_net_name_desc      nnd;
	si_svc_list_desc      sld;
	si_ee_descriptor      eed;
    si_se_descriptor      sed;
    si_ct_descriptor      sctd;
    si_tt_descriptor      ttd;
}si_desc;

// EIS

typedef struct{
	int event_id;
    dvb_si_time     start_time;
    dvb_si_duration duration;
	uint8_t running_status;
	uint8_t free_ca_mode;
	int nr_descriptors;
	si_desc descriptors[10];
}eis_info;

typedef struct{
	uint8_t table_id;
	uint8_t section_syntax_indicator;
	int section_length;
	int service_id;
	uint8_t version_number;
	uint8_t current_next_indicator;
	uint8_t section_number;
	uint8_t last_section_number;
	int transport_stream_id;
	int original_network_id;
	uint8_t segment_last_section_number;
	uint8_t last_table_id;
	eis_info section[10];
}event_information_section;

int add_si_descriptor( uint8_t *b, si_desc *d );
//
// SDT definition
//

typedef struct{
	int service_id;
	uint8_t eit_schedule_flag;
	uint8_t eit_present_follow_flag;
	uint8_t running_status;
	uint8_t free_ca_mode;
	int nr_descriptors;
	si_desc descriptor[10];
}sdt_section;

typedef struct{
	uint8_t table_id;
	uint8_t section_syntax_indicator;
	int section_length;
	int transport_stream_id;
	uint8_t version_number;
	uint8_t current_next_indicator;
	uint8_t section_number;
	uint8_t last_section_number;
	int original_network_id;
	sdt_section section[10];
}service_description_section;

// Network Information section{

typedef struct{
	int transport_stream_id;
	int original_network_id;
	int transport_desriptors_length;
	int nr_descriptors;
	si_desc desc[10];
}transport_stream_info;

typedef struct{
	uint8_t table_id;
	uint8_t section_syntax_indicator;
	int section_length;
	int network_id;
	uint8_t version_number;
	uint8_t current_next_indicator;
	uint8_t section_number;
	uint8_t last_section_number;
	int nr_network_descriptors;
	si_desc n_desc[10];
	int nr_transport_streams;
	transport_stream_info ts_info[10];
}network_information_section;


// Transport stream packet format
typedef struct{
	uint8_t b[20];
	uint8_t seq;
	uint8_t len;
}tp_chan;

// Program associated section
typedef struct{
	int program_number;
	int pid;
}tp_pa_sections;
/*
// Program associated section
typedef struct{
	uint8_t table_id;
	uint8_t section_syntax_indicator;
	int section_length;
	int transport_stream_id;
	uint8_t version_number;
	uint8_t current_next_indicator;
	uint8_t section_number;
	uint8_t last_section_number;
	tp_pa_sections sections(10);
}tp_pass;
*/

// PAT table
typedef struct{
	uint8_t section_syntax_indicator;
	int transport_stream_id;
	uint8_t version_number;
	uint8_t current_next_indicator;
	uint8_t section_number;
	uint8_t last_section_number;
	int   nr_table_entries;
	tp_pa_sections entry[10];
}tp_pat;


typedef struct{
	uint8_t stream_type;
	int elementary_pid;
    int nr_descriptors;
    td_descriptor desc[10];
}tp_pmt_section;

typedef struct{
	uint8_t section_syntax_indicator;
	int program_number;
	uint8_t version_number;
	uint8_t current_next_indicator;
	uint8_t section_number;
	uint8_t last_section_number;
	int pcr_pid;
    int nr_elementary_streams;
    tp_pmt_section stream[5];
}tp_pmt;

//
// Program Clock Reference
//
typedef struct{
    uint8_t pgm_clk_ref_base[5];//33 bits
    uint8_t pgm_clk_ref_extn[2];//9 bits
}tp_pcr;

typedef struct{
    uint8_t pts_val[5];//33 bits
}tp_pts;

typedef struct{
    uint8_t dts_val[5];//33 bits
}tp_dts;

typedef struct{
    uint8_t  discontinuity_ind;
    uint8_t  random_access_ind;
    uint8_t  elem_stream_pr_ind;
    uint8_t  PCR_flag;
    uint8_t  OPCR_flag;
    uint8_t  splicing_point_flag;
    uint8_t  trans_priv_data_flag;
    uint8_t  adapt_field_extn_flag;
}tp_adaption;

// Transport stream descriptor table

typedef struct{
	uint8_t table_id;
	uint8_t section_syntax_indicator;
	int section_length;
	uint8_t version_number;
	uint8_t current_next_indicator;
	uint8_t section_number;
	uint8_t last_section_number;
	uint8_t descriptor[10];
}tp_sdt;

typedef struct{
    uint8_t *inbuff;
    int ibuff_len;
    int seq_count;
    int nr_frames;
    int pid;
    uint8_t frames[100][188];
}tp_si_seq;

// Teletext
typedef struct{
    uint8_t data_unit_id;
    uint8_t field_parity;
    uint8_t line_offset;
    uint8_t framing_code;
    int magazine_address;
    uint8_t data_block[40];
}ebu_teletext_data_field;

typedef struct{
    uint8_t data_identifier;
    int nr_fields;
    ebu_teletext_data_field field[40];
}teletext_pes_data_field;

// Used when extracting program info from a transport stream

typedef struct{
    int pat_detected   : 1;
    int pmt_detected   : 1;
    int pmt_parsed     : 1;
    int video_detected : 1;
    int video_type;
    int audio_detected : 1;
    int audio_type;
    int pmt_id;
    int pcr_id;
    int required_pcr_id;
    int video_id;
    int required_video_id;
    int audio_id;
    int required_audio_id;
}ts_info;

typedef struct{
    int pgm_nr;
    int pgm_id;
}pat_info;

//

int  tp_fmt( uint8_t *b, tp_hdr *hdr );
void f_tp_init( void );
int  tp_pat_fmt( uint8_t *b, tp_pat *p );
void set_cont_counter( uint8_t *b, uint8_t c );

// eit.c
void eit_init(void);
void eit_dvb( void );

// pat.c
void pat_fmt( void );
void pat_init( void );
void pat_dvb( void );

// pmt.c
void pmt_fmt( void );
void pmt_init( void );
void pmt_dvb( void );
void pmt_fmt( int video_stream_type, int audio_stream_type );

// null.c
void null_fmt( void );
uint8_t *null_pkt( void );

void null_init( void );
void padding_null_dvb( void );
uint8_t *get_padding_null_dvb( void );

// sdt.c
void sdt_fmt( void );
void sdt_init( void );
void sdt_dvb( void );

// nit.c
void nit_fmt( void );
void nit_init( void );
void nit_dvb( void);

// eit.c
void eit_fmt( void );
void eit_init( void );
void eit_dvb( void);

// tdt.c
void tdt_init( void );
void tdt_dvb( void );
void tdt_fmt( uint8_t *b );

// f_tp.h
void tp_send( uint8_t *b );

// Update the continuits counter in a packet
void update_cont_counter( uint8_t *b );

int tp_pmt_fmt( uint8_t *b, tp_pmt *p );


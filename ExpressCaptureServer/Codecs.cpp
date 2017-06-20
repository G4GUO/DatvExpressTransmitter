#include "stdafx.h"
#include <stdint.h>
#include "Codecs.h"
#include "Dvb.h"


#define ENVC 0
#define ENAC 1
#define DEVC 2

#define INBUF_SIZE 100000
// 140 ms
#define SOUND_DELAY 27000*120
#define VIDEO_DELAY 27000*240

// Local variables
static struct SwsContext *m_sws;
static AVPacket        m_avpkt[4];
static AVCodecContext *m_pC[4];
static AVFrame *m_pFrameVideoSrc;
static AVFrame *m_pFrameVideo;
static AVFrame *m_pFrameAudio;
static uint8_t *m_p_eb[2];
static int64_t m_video_timestamp_delta;
static int64_t m_audio_timestamp_delta;
static int64_t m_audio_pts;
static int64_t m_video_pts;
static int64_t m_pcr;
static int32_t g_audio_bitrate;
static int32_t g_video_bitrate;
static CRITICAL_SECTION g_mutex;
//1920 samples, 2 channels, 2 bytes per sample, maximum size
static uint8_t *m_paudio_buffer;
static uint8_t *m_paudio_frame;
static int m_sound_capture_buf_size;
static bool m_capturing;
static int m_video_flag;
static int m_audio_flag;
static SampleBuffer m_video_cb;
static int m_audio_channels;

BOOL g_codec_configured;

#define CLEAR(x) memset(&(x), 0, sizeof(x))

void inc_audio_pts(void){
    m_audio_pts++;
}
void inc_video_pts(void){
    m_video_pts++;
}

void inc_pcr(void){
    m_pcr++;
}
void init_clocks(void){
    m_audio_pts = 0;
    m_video_pts = 0;
    m_pcr = 0;
}

int64_t get_video_pts(void){
    return m_video_pts;
}
int64_t get_audio_pts(void){
    return m_audio_pts;
}
void set_audio_pts(int64_t pts){
    m_audio_pts = pts; 
}
int64_t an_get_pcr(void){
    return m_pcr;
}
void rgb24_convert( uint8_t *in, uint8_t *out, int width, int height ){
	int bytes = 3;
	int stride = bytes*width;

	for( int i = 0; i < height; i++ ){
		memcpy(&out[i*stride],&in[(height-i-1)*stride],stride);
	}
}
int video_mjpeg_decode(SampleBuffer *vb, AVFrame *frame){
    int got_picture;
	m_avpkt[DEVC].size = vb->len;
	m_avpkt[DEVC].data = vb->b;
	avcodec_decode_video2( m_pC[DEVC], frame, &got_picture, &m_avpkt[DEVC]);
	return !got_picture;
}
void video_codec_samples( uint8_t *s, long len, uint64_t time){
	if( g_codec_configured == TRUE ){
		// If the memory is not copied it crashes
		// RGB24 (webcams) seem to reverse the byte order?

		if(m_video_cb.len < len ){
			if(m_video_cb.b != NULL ) free(m_video_cb.b); 
			m_video_cb.b   = (uint8_t *)malloc(len);
			m_video_cb.len = len;
		}
		// See if it is a compresses format, if so de-compress it into YUV420P
		if(m_pC[DEVC] != NULL ){
			memcpy(m_video_cb.b,s,len);
			if(video_mjpeg_decode(&m_video_cb,m_pFrameVideoSrc)) return;
		}else{
			// Not compressed format
			if(m_pFrameVideoSrc->format == AV_PIX_FMT_RGB24 ){
				rgb24_convert( s, m_video_cb.b, m_pFrameVideoSrc->width, m_pFrameVideoSrc->height );
			}
			else{
				memcpy(m_video_cb.b,s,len);
			}
			// Fill the source frame with data
			av_image_fill_arrays( m_pFrameVideoSrc->data,m_pFrameVideoSrc->linesize, m_video_cb.b, (AVPixelFormat)m_pFrameVideoSrc->format, m_pFrameVideoSrc->width, m_pFrameVideoSrc->height,16);
			// Do the Scale conversion
		}
		if(m_sws != NULL ) sws_scale( m_sws, m_pFrameVideoSrc->data, m_pFrameVideoSrc->linesize, 0, m_pFrameVideoSrc->height,
                                             m_pFrameVideo->data,    m_pFrameVideo->linesize);
		int got_packet;
		
		inc_video_pts();
		m_avpkt[ENVC].size = INBUF_SIZE;
		m_avpkt[ENVC].data = m_p_eb[ENVC];

		m_pFrameVideo->pts = get_video_pts();

		if(get_video_ident() == TRUE) overlay_frame( m_pFrameVideo );

		if(avcodec_encode_video2( m_pC[ENVC], &m_avpkt[ENVC], m_pFrameVideo, &got_packet ) == 0 )
		{ 
			if(got_packet)
			{
				if (m_avpkt[ENVC].dts > 0) {
					// DTS increments by one at a time
					int64_t dts = time;
					int64_t unit = dts / m_avpkt[ENVC].dts;
					int64_t pts = m_avpkt[ENVC].pts*unit;

					pts = (pts + VIDEO_DELAY);
					dts = (dts + VIDEO_DELAY);
					//if (pts < dts) pts = dts;
					EnterCriticalSection(&g_mutex);
					ps_video_el_to_ps(m_avpkt[ENVC].data, m_avpkt[ENVC].size, g_video_bitrate, pts, dts);
					// Now encode into transport packets
					ps_to_ts_video();
					LeaveCriticalSection(&g_mutex);
				}
			}
		}
	}
}

void process_audio_codec_samples(uint8_t *s, long len, uint64_t time) {
	static int64_t last_pts_time;

	if (get_audio_status() == TRUE) {
		int got_packet;

		avcodec_fill_audio_frame(m_pFrameAudio, m_audio_channels, AV_SAMPLE_FMT_S16, s, len, 0);
		m_avpkt[ENAC].size = INBUF_SIZE;
		m_avpkt[ENAC].data = m_p_eb[ENAC];
		inc_audio_pts();

		if (avcodec_encode_audio2(m_pC[ENAC], &m_avpkt[ENAC], m_pFrameAudio, &got_packet) == 0)
		{
			if (got_packet)
			{
				/*	int64_t pts = get_audio_pts()*m_audio_timestamp_delta;
					int64_t pcr = get_pcr_clock();
					if(pts < pcr){
						set_audio_pts((pcr+CLOCK_DELAY)/m_audio_timestamp_delta);
					}else{
						if(abs(pts - pcr) > (CLOCK_DELAY*3)){
							set_audio_pts((pcr+CLOCK_DELAY)/m_audio_timestamp_delta);
						}
					}*/
				int64_t pts_time = time/(27000 * 24);
				pts_time *= (27000 * 24);
				if (pts_time <= last_pts_time) pts_time += (27000 * 24);
				last_pts_time = pts_time;
				pts_time = (pts_time + SOUND_DELAY);
				EnterCriticalSection(&g_mutex);
				ps_audio_el_to_ps(m_avpkt[ENAC].data, m_avpkt[ENAC].size, g_audio_bitrate, pts_time, -1);
				ps_to_ts_audio();
				LeaveCriticalSection(&g_mutex);
			}
		}
	}
}

void audio_codec_samples(uint8_t *s, long len) {
	if (g_codec_configured == TRUE) {
		static int offset;
		// make sure we have the right block size
		// I am sure this could be improved (sped up)
		for (int i = 0; i < len; i++) {
			m_paudio_buffer[offset++] = s[i];
			if (offset == m_pFrameAudio->nb_samples * m_audio_channels *2) {
				vb_audio_post(m_paudio_buffer, offset);
//				process_audio_codec_samples(m_paudio_buffer, );
				offset = 0;
			}
		}
	}
}

void an_set_image_buffer_sizes(CodecParams *params)
{
	// Video Encoder frame
	m_pFrameVideo = av_frame_alloc();
	av_frame_unref(m_pFrameVideo);
	m_pFrameVideo->width = params->v_dst_width;
	m_pFrameVideo->height = params->v_dst_height;
	m_pFrameVideo->format = AV_PIX_FMT_YUV420P;
	av_frame_get_buffer(m_pFrameVideo, 16);

	// Capture frame
	m_pFrameVideoSrc = av_frame_alloc();
	av_frame_unref(m_pFrameVideoSrc);
	m_pFrameVideoSrc->width = params->v_src_width;
	m_pFrameVideoSrc->height = params->v_src_height;
	// A compressed format, needs to allocate memory for format after de-compression
	if (params->v_src_pixfmt == AV_PIX_FMT_JPEG)
		m_pFrameVideoSrc->format = AV_PIX_FMT_YUVJ422P;
	else
		m_pFrameVideoSrc->format = params->v_src_pixfmt;
	av_frame_get_buffer(m_pFrameVideoSrc, 16);
}

void configure_audio_frame(void)
{
	// Audio
	m_pFrameAudio = av_frame_alloc();
	m_pFrameAudio->nb_samples = m_pC[ENAC]->frame_size;
}
void codec_video_ident(BOOL ident) {
	cmd_set_video_ident(ident);

}
//
// Setting the sample ratio causes the DAR field to be set up correctly
// in the video stream.
//
void cal_ar(CodecParams *params, int *ar) {
	if ((params->v_ar[0] == 4) && (params->v_ar[1] == 3)) {
		if ((params->v_dst_width == 1920) && (params->v_dst_height == 1080)) {
			ar[0] = 3;
			ar[1] = 4;
			return;
		}
		if ((params->v_dst_width == 720) && (params->v_dst_height == 576)) {
			ar[0] = 32;
			ar[1] = 30;
			return;
		}
		if ((params->v_dst_width == 720) && (params->v_dst_height == 480)) {
			ar[0] = 8;
			ar[1] = 9;
			return;
		}
		if ((params->v_dst_width == 640) && (params->v_dst_height == 480)) {
			ar[0] = 1;
			ar[1] = 1;
			return;
		}
	}
	if ((params->v_ar[0] == 16) && (params->v_ar[1] == 9)) {
		if ((params->v_dst_width == 1920) && (params->v_dst_height == 1080)) {
			ar[0] = 1;
			ar[1] = 1;
			return;
		}
	}
	ar[0] = 1;
	ar[1] = 1;
}
int init_codecs(CodecParams *params ){

    av_init_packet(&m_avpkt[ENVC]);
    m_avpkt[ENVC].data = NULL;
    m_avpkt[ENVC].size = 0;

    // 25 frames per sec, every 40 ms
	// 30 frames per sec, every 33.33 ms
    m_video_timestamp_delta = (int64_t)(27000000.0/(params->v_dst_fps));
    // New audio packet sent every 24 ms
    m_audio_timestamp_delta = (int64_t)(0.024*27000000.0);
    //
    // Video
    //
    AVCodec *codec = NULL;
	AVPixelFormat format = (AVPixelFormat)params->v_src_pixfmt;

	m_pC[DEVC] = NULL; // Default no decoder 
	//
	// See if we need to decompress the frame before encoding it again
	//
	if(format == AV_PIX_FMT_JPEG ){
		// Need to decompress the format, so set up a decoder context
        codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
        if(codec != NULL){
            m_pC[DEVC]                     = avcodec_alloc_context3(codec);
			avcodec_get_context_defaults3( m_pC[DEVC], codec);
            m_pC[DEVC]->width              = params->v_src_width;
            m_pC[DEVC]->height             = params->v_src_height;
            m_pC[DEVC]->pix_fmt            = AV_PIX_FMT_YUVJ422P;
			m_pC[DEVC]->thread_count       = 1;
		}else{
            printf("MJPEG Codec not found");
            return -1;
        }
		if(avcodec_open2(m_pC[DEVC], codec, NULL)<0){
            printf("Unable to open Video SW Codec (Decoder), bad params ?");
            return -1;
		}
		format = AV_PIX_FMT_YUVJ422P;
	}
	//
	// Sort out any format conversion
	//
	if(format == AV_PIX_FMT_RGB24) format = AV_PIX_FMT_BGR24;

	// Format conversion will be required
    m_sws = sws_getContext( params->v_src_width, params->v_src_height, format,
                            params->v_dst_width, params->v_dst_height, AV_PIX_FMT_YUV420P,
	 						SWS_BICUBIC, NULL,NULL, NULL);
    // Do the aspect ratio fiddling so FFmpeg selects the righ DAR
	int ar[2];
	cal_ar(params, ar);

	//
	// Allocate the required frame memory
	//
	an_set_image_buffer_sizes( params );
	//
	// Allocate the Encoding Codecs
	//
	if(params->v_codec == AV_CODEC_ID_MPEG2VIDEO){
        codec = avcodec_find_encoder(AV_CODEC_ID_MPEG2VIDEO);
        if(codec != NULL){
            m_pC[ENVC]                     = avcodec_alloc_context3(codec);
			m_pC[ENVC]->qmax = 51;
			//m_pC[ENVC]->rc_strategy = 1;
            m_pC[ENVC]->bit_rate           = params->v_br;// Not used CBR
            m_pC[ENVC]->bit_rate_tolerance = params->v_br/10;// Not used CBR
            m_pC[ENVC]->rc_max_rate        = params->v_br;
            m_pC[ENVC]->rc_min_rate        = params->v_br;
			m_pC[ENVC]->rc_buffer_size = (params->v_br)*4 / params->v_dst_fps; //4 pictures
            m_pC[ENVC]->width              = params->v_dst_width;
			m_pC[ENVC]->height			   = params->v_dst_height;
			m_pC[ENVC]->sample_aspect_ratio.num = ar[0];
			m_pC[ENVC]->sample_aspect_ratio.den = ar[1];

            m_pC[ENVC]->gop_size           = get_video_gop();
            m_pC[ENVC]->max_b_frames       = get_video_b_frames();
            m_pC[ENVC]->pix_fmt            = AV_PIX_FMT_YUV420P;
			if(params->v_dst_i) 
				m_pC[ENVC]->flags          = AV_CODEC_FLAG_INTERLACED_DCT | AV_CODEC_FLAG_INTERLACED_ME;
			m_pC[ENVC]->time_base.num      = 1;
			m_pC[ENVC]->time_base.den      = params->v_dst_fps;
			m_pC[ENVC]->ticks_per_frame = 1;// params->v_dst_fpf == 2 ? 1 : 2;// MPEG2 & 4 (should be 2)
            m_pC[ENVC]->profile            = FF_PROFILE_MPEG2_MAIN;
            m_pC[ENVC]->thread_count       = 1;
		}else{
            printf("MPEG2 Codec not found");
            return -1;
        }
    }
    if(params->v_codec == AV_CODEC_ID_H264){
        codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if(codec != NULL){
            m_pC[ENVC]                     = avcodec_alloc_context3(codec);
			//m_pC[ENVC]->rc_strategy = 2;
            m_pC[ENVC]->bit_rate           = params->v_br;// Not used CBR
			m_pC[ENVC]->bit_rate_tolerance = params->v_br/10;//params->v_br / 10;// Not used CBR
            m_pC[ENVC]->rc_max_rate        = params->v_br;
            m_pC[ENVC]->rc_min_rate        = params->v_br;
            m_pC[ENVC]->rc_buffer_size     = params->v_br/10;
            m_pC[ENVC]->width              = params->v_dst_width;
            m_pC[ENVC]->height             = params->v_dst_height;
			m_pC[ENVC]->sample_aspect_ratio.num = params->v_ar[0];
			m_pC[ENVC]->sample_aspect_ratio.den = params->v_ar[1];

            m_pC[ENVC]->gop_size           = get_video_gop();
            m_pC[ENVC]->max_b_frames       = get_video_b_frames();
            m_pC[ENVC]->pix_fmt            = AV_PIX_FMT_YUV420P;
			if(params->v_dst_i) 
				m_pC[ENVC]->flags              = AV_CODEC_FLAG_INTERLACED_DCT | AV_CODEC_FLAG_INTERLACED_ME;
			m_pC[ENVC]->time_base.num      = 1;
			m_pC[ENVC]->time_base.den      = params->v_dst_fps;
            m_pC[ENVC]->ticks_per_frame    = params->v_dst_fpf == 2 ? 1 : 2;// MPEG2 & 4
            m_pC[ENVC]->profile            = FF_PROFILE_H264_HIGH;
            m_pC[ENVC]->thread_count       = 1;
            av_opt_set(m_pC[ENVC]->priv_data, "preset", get_video_codec_performance(), 0);

        }else{
            printf("MPEG4-AVC Codec not found");
            return -1;
      }
    }

	if(params->v_codec == AV_CODEC_ID_HEVC){
        codec = avcodec_find_encoder(AV_CODEC_ID_HEVC);
        if(codec != NULL){
            m_pC[ENVC]                     = avcodec_alloc_context3(codec);
            m_pC[ENVC]->bit_rate           = params->v_br;// Not used CBR
			m_pC[ENVC]->bit_rate_tolerance = params->v_br / 10;//params->v_br / 10;// Not used CBR
            m_pC[ENVC]->rc_max_rate        = params->v_br;
            m_pC[ENVC]->rc_min_rate        = params->v_br;
            m_pC[ENVC]->rc_buffer_size     = params->v_br/3;
            m_pC[ENVC]->width              = params->v_dst_width;
            m_pC[ENVC]->height             = params->v_dst_height;
			m_pC[ENVC]->sample_aspect_ratio.num = params->v_ar[0];
			m_pC[ENVC]->sample_aspect_ratio.den = params->v_ar[1];
            m_pC[ENVC]->gop_size           = get_video_gop();
            m_pC[ENVC]->max_b_frames       = get_video_b_frames();
            m_pC[ENVC]->pix_fmt            = AV_PIX_FMT_YUV420P;
			if(params->v_dst_i) 
				m_pC[ENVC]->flags          = AV_CODEC_FLAG_INTERLACED_DCT | AV_CODEC_FLAG_INTERLACED_ME;
			m_pC[ENVC]->time_base.num      = 1;
			m_pC[ENVC]->time_base.den      = params->v_dst_fps;
            m_pC[ENVC]->ticks_per_frame    = params->v_dst_fpf == 2 ? 1 : 2;// MPEG2 & 4
            m_pC[ENVC]->profile            = FF_PROFILE_HEVC_MAIN;
            m_pC[ENVC]->thread_count       = 4;
            av_opt_set(m_pC[ENVC]->priv_data, "preset", get_video_codec_performance(), 0);
        }else{
            printf("HEVC Codec not found");
            return -1;
        }
    }
    if(avcodec_open2(m_pC[ENVC], codec, NULL)<0){
            printf("Unable to open Video SW Codec, bad params ?");
            return -1;
    }
    //
    // Audio
    //
    av_init_packet( &m_avpkt[ENAC] );
    //
    // Must be set to 48000, 2 chan
    //
    // Size in bytes 2 channels, 16 bits 1/25 sec
	if(params->a_codec == AV_CODEC_ID_MP2 ){
		codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
		if( codec != NULL )
		{
			m_pC[ENAC] = avcodec_alloc_context3(codec);
			if (params->a_br == 32000) {
				m_audio_channels = 1;
				m_pC[ENAC]->bit_rate = params->a_br;
				m_pC[ENAC]->bit_rate_tolerance = 0;
				m_pC[ENAC]->bits_per_raw_sample = 16;
				m_pC[ENAC]->sample_rate = 48000;
				m_pC[ENAC]->channels = m_audio_channels;
				m_pC[ENAC]->sample_fmt = AV_SAMPLE_FMT_S16;
				m_pC[ENAC]->channel_layout = AV_CH_LAYOUT_MONO;
				m_pC[ENAC]->thread_count = 4;
			}
			else {
				m_audio_channels = 2;
				m_pC[ENAC]->bit_rate = params->a_br;
				m_pC[ENAC]->bit_rate_tolerance = 0;
				m_pC[ENAC]->bits_per_raw_sample = 16;
				m_pC[ENAC]->sample_rate = 48000;
				m_pC[ENAC]->channels = m_audio_channels;
				m_pC[ENAC]->sample_fmt = AV_SAMPLE_FMT_S16;
				m_pC[ENAC]->channel_layout = AV_CH_LAYOUT_STEREO;
				m_pC[ENAC]->thread_count = 4;
			}
		}
		if(avcodec_open2(m_pC[ENAC], codec, NULL)<0 )
		{
			printf("Unable to open Audio codec");
			return -1;
		}
	}
	if(params->a_codec == AV_CODEC_ID_AAC ){
		codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
		if( codec != NULL )
		{
			m_pC[ENAC] = avcodec_alloc_context3(codec);
			m_pC[ENAC]->bit_rate            = params->a_br;
			m_pC[ENAC]->bit_rate_tolerance  = 0;
			m_pC[ENAC]->bits_per_raw_sample = 16;
			m_pC[ENAC]->sample_rate         = 48000;
			m_pC[ENAC]->channels            = 2;
			m_pC[ENAC]->sample_fmt          = codec->sample_fmts[0];//AV_SAMPLE_FMT_S16;
			m_pC[ENAC]->channel_layout      = AV_CH_LAYOUT_STEREO;
			m_pC[ENAC]->thread_count        = 4;
			m_pC[ENAC]->profile             = FF_PROFILE_AAC_MAIN;
			m_pC[ENAC]->time_base.num       = 1;
			m_pC[ENAC]->time_base.den       = 48000;
			m_pC[ENAC]->codec_type          = AVMEDIA_TYPE_AUDIO;
			//m_pC[ENAC]->codec_id            = AV_CODEC_ID_AAC;

			m_pC[ENAC]->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
		}
		int val;
        if((val=avcodec_open2(m_pC[ENAC], codec, NULL))<0 )
        {
			printf("Unable to open Audio codec");
			return -1;
		}
	};
    // 16 bit samples & stereo so multiply by 4
	m_audio_timestamp_delta = (27000*m_pC[ENAC]->frame_size)/m_pC[ENAC]->sample_rate; 
	m_audio_timestamp_delta *= 1000; 

    // 2 bytes per channel
    m_sound_capture_buf_size = m_pC[ENAC]->frame_size*m_pC[ENAC]->channels*2;
	m_paudio_buffer = (uint8_t *)malloc(m_pC[ENAC]->frame_size*m_pC[ENAC]->channels * 2);
	m_paudio_frame  = (uint8_t *)malloc(m_pC[ENAC]->frame_size*m_pC[ENAC]->channels * 2);
    configure_audio_frame();
   
	m_p_eb[0] = (uint8_t *)malloc(INBUF_SIZE);
	m_p_eb[1] = (uint8_t *)malloc(INBUF_SIZE);

	return 0;
}
void codec_start(CodecParams *params){
	if(g_codec_configured == TRUE) codec_stop();
	g_audio_bitrate = params->a_br;
	g_video_bitrate = params->v_br;
	overlay_init(get_provider_name());
	if(init_codecs(params) >= 0) g_codec_configured = TRUE; 
}

void codec_stop(void){
	if(g_codec_configured == TRUE){
		avcodec_free_context(&m_pC[ENVC]);
		avcodec_free_context(&m_pC[ENAC]);
		if(m_pC[DEVC] != NULL ) avcodec_free_context(&m_pC[DEVC]);
		free(m_paudio_buffer);
		free(m_paudio_frame);
		free(m_p_eb[0]);
	    free(m_p_eb[1]);
		sws_freeContext(m_sws);
		overlay_deinit();
	}
	g_codec_configured = FALSE;
}
void codec_init(void){
    av_register_all();
    avfilter_register_all();
	g_codec_configured = FALSE;
	m_video_cb.len = 0;
	m_video_cb.b   = NULL;
	InitializeCriticalSection(&g_mutex);
}
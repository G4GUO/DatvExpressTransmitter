#pragma once

#include "stdafx.h"
#include <stdint.h>
//
// Codec information structure
//
typedef struct{
	int v_codec;
	int a_codec;
    uint32_t v_src_width;// Picture width (source)
    uint32_t v_src_height;// Picture height (source)
    int      v_src_fps;// Frames per second (source)
    int      v_src_fpf;// Fields per frame (source)
	int      v_src_pixfmt;// Pixel format source
    uint32_t v_dst_width;// Picture width (destination)
    uint32_t v_dst_height;// Picture height (destination)
    int      v_dst_fps;// Frames per second (destination)
    int      v_dst_fpf;// Fields per frame (destination)
	int      v_dst_i;//interlaced
    int      v_br; // Video bitrate
    int      v_ar[2];// Video aspect ration
    int      a_br;// Audio bitrate
    int      a_ch;// Number of Audio channels
    int      a_bd;// Audio bit depth 16 bits
    int      a_sr; // Audio sample rate
}CodecParams;

// MJPEG format 
#define AV_PIX_FMT_JPEG 0x9999

void codec_video_ident(BOOL ident);
void video_codec_samples( uint8_t *s, long len, uint64_t time);
void process_audio_codec_samples(uint8_t *s, long len, uint64_t time);
void audio_codec_samples(uint8_t *s, long len);
void codec_start(CodecParams *params);
void codec_stop(void);
void codec_init(void);

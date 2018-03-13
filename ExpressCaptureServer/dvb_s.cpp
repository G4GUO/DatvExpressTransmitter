#include "stdafx.h"
#include "DVB-T\dvb_t.h"

void dvb_s_init(void) {
	dvb_interleave_init();
	dvb_rs_init();
	dvb_encode_init();
	dvb_conv_init();
}
#include "stdafx.h"
#include <stdint.h>
#include "express.h"

#define EXT_ADDR 0x36

extern int m_express_status;

void ext_send_reg(uint32_t r) {
	uint8_t b[5];
	b[0] = EXT_ADDR;
	b[4] = (uint8_t)(r & 0xFF);
	r >>= 8;
	b[3] = (uint8_t)(r & 0xFF);
	r >>= 8;
	b[2] = (uint8_t)(r & 0xFF);
	r >>= 8;
	b[1] = (uint8_t)(r & 0xFF);

	if (m_express_status != EXP_OK) return;

	express_i2c_bulk_transfer(EP1OUT, b, 5);
//	Sleep(10);
	express_handle_events(1);

}
void ext_set_1_ghz(void) {
	uint32_t r;
	// Set the Extender to 1 GHz
	r = 0xC80000;
	ext_send_reg(r);
	r = 0x8008011;
	ext_send_reg(r);
	r = 0x4E42;
	ext_send_reg(r);
	r = 0x4B3;
	ext_send_reg(r);
	r = 0xA5003C;
	ext_send_reg(r);
	r = 0x580005;
	ext_send_reg(r);
}
void ext_set_2_ghz(void) {
	uint32_t r;
	// Set the Extender to 2 GHz
	r = 0xC80000;
	ext_send_reg(r);
	r = 0x8008011;
	ext_send_reg(r);
	r = 0x4E42;
	ext_send_reg(r);
	r = 0x4B3;
	ext_send_reg(r);
	r = 0x95003C;
	ext_send_reg(r);
	r = 0x580005;
	ext_send_reg(r);
}
void ext_set_4_ghz(void) {
	uint32_t r;
	// Set the Extender to 4 GHz
	r = 0xC80000;
	ext_send_reg(r);
	r = 0x8008011;
	ext_send_reg(r);
	r = 0x4E42;
	ext_send_reg(r);
	r = 0x4B3;
	ext_send_reg(r);
	r = 0x85003C;
	ext_send_reg(r);
	r = 0x580005;
	ext_send_reg(r);
}
void ext_set_disabled(void) {
	uint32_t r;
	// Disable the output
	r = 0x85001C;
	ext_send_reg(r);
}
double ext_set_express_freq(double f) {

	if (f < 70000000) {
		// Set the Extender to 1 GHz
		ext_set_1_ghz();
		// Set Express to 1 GHz + f
		f += 1000000000;
		return f;
	}
	if ((f >= 3400000000)&& (f <= 3500000000)) {
		// Set the Extender to 2 GHz
		ext_set_2_ghz();
		// Set Express to 1 GHz + f
		f -= 2000000000;
		return f;
	}
	if ((f >= 5000000000) && (f <= 6000000000)) {
		// Set the Extender to 2 GHz
		ext_set_4_ghz();
		// Set Express to 1 GHz + f
		f -= 4000000000;
		return f;
	}
	ext_set_disabled();
    return f;
}
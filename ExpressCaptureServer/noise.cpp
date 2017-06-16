#include "stdafx.h"
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "noise.h"

static uint32_t s0a, s1a, s2a;
static uint32_t s0b, s1b, s2b, s3b;
static FLT m_r, m_f;
static FLT m_sn_ratio;
static int m_noise_on;

void random_init(void) {
	s0a = 0x12345;
	s1a = 0x56789;
	s2a = 0xabcdef;

	s0b = 0xF1DEDDB;
	s1b = 0x271567A;
	s2b = 0x197BCDE;
	s3b = 0xAB123CD;
}
FLT random_a(void) {

	s0a = ((s0a & 0xFFFFFFFE) << 12) ^ (((s0a << 13) ^ s0a) >> 19);
	s1a = ((s1a & 0xFFFFFFF8) << 4) ^ (((s1a << 2) ^ s1a) >> 25);
	s2a = ((s2a & 0xFFFFFFF0) << 17) ^ (((s2a << 3) ^ s2a) >> 11);
	FLT m =  (FLT)(s0a^s1a^s2a);
	return m == 0 ?  1.0f : (m / 0xFFFFFFFF);
}
FLT random_b(void) {

	s0b = ((s0b & 0xFFFFFFFE) << 18) ^ (((s0b << 6) ^ s0b) >> 13);
	s1b = ((s1b & 0xFFFFFFF8) << 2) ^ (((s1b << 2) ^ s1b) >> 27);
	s2b = ((s2b & 0xFFFFFFF0) << 7) ^ (((s2b << 13) ^ s2b) >> 21);
	s3b = ((s3b & 0xFFFFFF80) << 13) ^ (((s3b << 3) ^ s3b) >> 12);

	FLT m = (FLT)(s0b^s1b^s2b^s3b);
	return (FLT)((m / 0xFFFFFFFF)*(2.0*M_PI));
}
FLT noise_get_sn(void) {
	return m_sn_ratio;
}
int noise_is_enabled(void) {
	return m_noise_on;
}

typedef struct {
	float re;
	float im;
}FComplex;

FComplex noise(void) {
	FComplex n;
	FLT a = sqrt(-2.0 * log(random_a()))*0.6316187777;//Normalise the noise (then take sqrt)
	FLT b = random_b();
	n.re = (float)(a*cos(b));
	n.im = (float)(a*sin(b));

	return n;
}
void noise_init(void) {
	m_noise_on = 0;
	random_init();
	noise_set_sn(50);
}
void noise_set_sn(FLT sn) {
	m_sn_ratio = sn;
	m_r = pow( 10.0, sn / 20.0);
	m_f = 1.0 / (1.0 + m_r);
}
void noise_on(void) {
	m_noise_on = 1;
}
void noise_off(void) {
	m_noise_on = 0;
}
scmplx *noise_add(scmplx *s, int len) {
	if (m_noise_on) {
		for (int i = 0; i < len; i++) {
			s[i].re = (short)((s[i].re*m_r + noise().re * 0x7FFF)*m_f);
			s[i].im = (short)((s[i].im*m_r + noise().im * 0x7FFF)*m_f);
		}
	}
	return s;
}
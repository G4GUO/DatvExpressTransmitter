#include "stdafx.h"
#include "Dvb.h"
#define USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define RRC_TAPS 31
#define RRC_OVERSAMPLE 2

static short *m_filter = NULL;
static float *m_f_filter = NULL;
// Working space for the filter
static scmplx *m_fbuff;
static int     m_fbuff_size;
//
// Set the overall gain of the filter
//
void set_filter_gain(float *c, int len, float gain) {
	float a = 0;
	for (int i = 0; i < len; i++) {
		a += c[i];
	}
	a = gain / fabsf(a);
	for (int i = 0; i < len; i++) {
		c[i] = c[i] * a;
	}
}
//
// Create a single oversampled Root raised cosine filter
//
void build_rrc_filter(float *filter, float rolloff, int ntaps, int samples_per_symbol) {
	double a, b, c, d;
	double B = rolloff+0.0001;// Rolloff factor .0001 stops infinite filter coefficient with 0.25 rolloff
	double t = -(ntaps - 1) / 2;// First tap
	double Ts = samples_per_symbol;// Samples per symbol
	// Create the filter
	for (int i = 0; i < (ntaps); i++) {
		a = 2.0 * B / (M_PI*sqrt(Ts));
		b = cos((1.0 + B)*M_PI*t / Ts);
		// Check for infinity in calculation (computers don't have infinite precision)
		if (t == 0)
			c = (1.0 - B)*M_PI / (4 * B);
		else
			c = sin((1.0 - B)*M_PI*t / Ts) / (4.0*B*t / Ts);

		d = (1.0 - (4.0*B*t / Ts)*(4.0*B*t / Ts));
		//filter[i] = (b+c)/(a*d);//beardy
		filter[i] = (float)(a*(b + c) / d);//nasa
		t = t + 1.0;
	}
}
void set_filter_gain(float *filter, float gain, int ntaps) {
	float sum = 0;
	for (int i = 0; i < ntaps; i++) {
		sum += filter[i];
	}
	gain = gain / sum;
	for (int i = 0; i < ntaps; i++) {
		filter[i] = filter[i] * gain;
	}
}
void window_filter(float *filter, int N) {
	// Build the window
	for (int i = -N / 2, j = 0; i < N / 2; i++, j++)
	{
		filter[j]= (0.54f - 0.46f * cosf((2.0f * (float)M_PI*i) / (N - 1)))*filter[j];
		//filter[j] = (0.5f*(1.0f + cosf((2.0f*(float)M_PI*i) / N)))*filter[j];
	}
}
void make_short(short *out, float *in, int len) {
	for (int i = 0; i < len; i++) {
		out[i] = (short)round((in[i] * 32768));
	}
}
void build_lpf_filter(float *filter, float bw, int ntaps ) {
	double a;
	double B = bw;// filter bandwidth
	double t = -( ntaps - 1) / 2;// First tap
	// Create the filter
	for (int i = 0; i < ntaps; i++) {
		if (t == 0)
			a = 2.0 * B;
		else
		    a = 2.0 * B * sin(M_PI * t * B)/ (M_PI * t * B);
		filter[i] = (float)a;
		t = t + 1.0;
	}
}
inline scmplx filter2(scmplx *buff, short *c) {
	uint32_t re = 0;
	uint32_t im = 0;
	for (int i = 0; i < (RRC_TAPS/RRC_OVERSAMPLE); i++ ) {
		re += (*c)*buff[i].re;
		im += (*c)*buff[i].im;
		c += 2;
	}
	scmplx a;
	a.re = (short)(re >> 16);
	a.im = (short)(im >> 16);
	return a;
}
void rrc_init(void) {
	// reset working buffer
	return;
}
void rrc_rolloff(float roff) {
	return;
}


// Design root-Nyquist raised-cosine filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
void liquid_firdes_rrcos(unsigned int _k,
	unsigned int _m,
	float _beta,
	float _dt,
	float * _h)
{
	if (_k < 1) {
		fprintf(stderr, "error: liquid_firdes_rrcos(): k must be greater than 0\n");
		exit(1);
	}
	else if (_m < 1) {
		fprintf(stderr, "error: liquid_firdes_rrcos(): m must be greater than 0\n");
		exit(1);
	}
	else if ((_beta < 0.0f) || (_beta > 1.0f)) {
		fprintf(stderr, "error: liquid_firdes_rrcos(): beta must be in [0,1]\n");
		exit(1);
	}
	else;

	unsigned int n;
	float z, t1, t2, t3, t4, T = 1.0f;

	float nf, kf, mf;

	unsigned int h_len = 2 * _k*_m + 1;

	// Calculate filter coefficients
	for (n = 0; n<h_len; n++) {
		nf = (float)n;
		kf = (float)_k;
		mf = (float)_m;

		z = (nf + _dt) / kf - mf;
		t1 = cosf((1 + _beta)*M_PI*z);
		t2 = sinf((1 - _beta)*M_PI*z);

		// Check for special condition where z equals zero
		if (fabsf(z) < 1e-5) {
			_h[n] = 1 - _beta + 4 * _beta / M_PI;
		}
		else {
			t3 = 1 / ((4 * _beta*z));

			float g = 1 - 16 * _beta*_beta*z*z;
			g *= g;

			// Check for special condition where 16*_beta^2*z^2 equals 1
			if (g < 1e-5) {
				float g1, g2, g3, g4;
				g1 = 1 + 2.0f / M_PI;
				g2 = sinf(0.25f*M_PI / _beta);
				g3 = 1 - 2.0f / M_PI;
				g4 = cosf(0.25f*M_PI / _beta);
				_h[n] = _beta / sqrtf(2.0f)*(g1*g2 + g3*g4);
			}
			else {
				t4 = 4 * _beta / (M_PI*sqrtf(T)*(1 - (16 * _beta*_beta*z*z)));
				_h[n] = t4*(t1 + (t2*t3));
			}
		}
	}
}

//
// the length must always be even and a multiple of 16
//
short *rrc_make_filter(float roff, int ratio, int taps) {
	// Create the over sampled mother filter
	float *filter = (float*)malloc(sizeof(float)*taps);
	// Set last coefficient to zero
	filter[taps-1] = 0;
	// RRC filter must always be odd length
//	liquid_firdes_rrcos(ratio, taps/ratio/2 - 1, roff, 0, filter); // Seems not working at 1MSPS
	build_rrc_filter(filter, roff, taps-1, ratio);
	window_filter(filter, taps-1); // Need information about this task
	set_filter_gain(filter, 0.99f, taps);
	// Free memory from last filter if it exsists
	if (m_filter != NULL) free(m_filter);
	// Allocate memory for new global filter
	m_filter = (short*)malloc(sizeof(short)*taps);
	// convert the filter into shorts
	make_short(m_filter, filter, taps);
	free(filter);
	return m_filter;
}

float *rrc_make_f_filter(float roff, int ratio, int taps) {
	// Create the over sampled mother filter
	float *filter = (float*)malloc(sizeof(float)*taps);
	// Set last coefficient to zero
	filter[taps - 1] = 0;
	// RRC filter must always be odd length
		liquid_firdes_rrcos(ratio, taps/ratio/2 , roff, 0, filter); // Seems not working at 1MSPS
	//build_rrc_filter(filter, roff, taps , ratio);
	//window_filter(filter, taps ); // Need information about this task
	set_filter_gain(filter, 0.8f, taps);
	// Free memory from last filter if it exsists
	if (m_filter != NULL) free(m_filter);
	// Allocate memory for new global filter
	m_f_filter = (float*)malloc(sizeof(float)*taps);
	memcpy(m_f_filter, filter, sizeof(float)*taps);
	free(filter);
	return m_f_filter;
}


short *lpf_make_filter(float bw, int ratio, int taps) {
	// Create the over sampled mother filter
	float *filter = (float*)malloc(sizeof(float)*taps);
	// Set last coefficient to zero
	filter[taps - 1] = 0;
	// Create an LPF
	build_lpf_filter(filter, bw/ratio, taps-1);
	window_filter(filter, taps );
	set_filter_gain(filter, 0.999f, taps);
	// Free memory from last filter if it exsists
	if (m_filter != NULL) free(m_filter);
	// Allocate memory for new global filter
	m_filter = (short*)malloc(sizeof(short)*taps);
	// convert the filter into shorts
	make_short(m_filter, filter, taps);
	free(filter);
	return m_filter;
}
// This interpolates by 2 
int rrc_filter(scmplx *in, scmplx *out, int len) {
	if (m_fbuff_size < len) {
		// Need to create a new working buffer
		if (m_fbuff != NULL) free(m_fbuff);
		m_fbuff = (scmplx*)malloc(sizeof(scmplx)*(len + (RRC_TAPS/RRC_OVERSAMPLE)));
		m_fbuff_size = len;
	}
	// Save the old samples and copy the new
	memcpy(m_fbuff, &m_fbuff[len], sizeof(scmplx)*(RRC_TAPS/RRC_OVERSAMPLE));
	memcpy(&m_fbuff[(RRC_TAPS/RRC_OVERSAMPLE)], in, sizeof(scmplx)*len);
	// Now do the interpolating filter
	for (int i = 0; i < len; i++) {
		out[i * 2]   = filter2(&m_fbuff[i], &m_filter[1]);
		out[(i*2)+1] = filter2(&m_fbuff[i], &m_filter[0]);
	}
	return (len * 2);
}
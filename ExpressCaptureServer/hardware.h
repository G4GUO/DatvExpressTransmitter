#pragma once
#include "Dvb.h"

int  hw_init(void);
void hw_deinit(void);
void hw_set_freq(double f);
void hw_set_sr(double sr);
void hw_set_level(double lvl);
void hw_transmit(void);
void hw_receive(void);
void hw_set_carrier(int chk);
void hw_set_fec(int fec);
void hw_set_iqcalibrate(int status);
void hw_set_ical(int offset);
void hw_set_qcal(int offset);
void hw_set_filter(int rolloff);
void hw_set_analogue_lpf(double bw);
void hw_set_ports(int ports);
void hw_enter_critical(void);
void hw_leave_critical(void);

// RRC library
void rrc_init(void);
void rrc_rolloff(float roff);
// Used to program the interpolating filter
short *rrc_make_filter(float roff, int ratio, int taps);
float *rrc_make_f_filter(float roff, int ratio, int taps);
short *lpf_make_filter(float bw,   int ratio,   int taps);
// This interpolates by 2 
int rrc_filter(scmplx *in, scmplx *out, int len);
int Interpolate(scmplx *in, scmplx **out, int len);

// FMCOMMS specific libraries
int fmc_init(void);
void fmc_deinit(void);
void fmc_shutdown(void);
void fmc_set_buff_length(uint32_t len);
void fmc_tx_samples(scmplx *s, int len);
void fmc_set_tx_frequency(double freq);
void fmc_set_tx_sr(long long int sr);
void fmc_set_tx_level(double level);
void fmc_set_transmit(void);
void fmc_set_receive(void);
void fmc_load_tx_rrc_filter(float roff);
void fmc_load_tx_lpf_filter(float roff);
void fmc_configure_x8_int_dec(long long int sr);
void fmc_set_analog_lpf(double bw);
void fmc_enable_tx_lo(void);
void fmc_disable_tx_lo(void);

// Lime specific stuff

int limesdr_init();
void limesdr_set_freq(double freq);
void limesdr_deinit(void);
void limesdr_set_level(int level);
int limesdr_set_sr(double sr, int OverSample);
void limesdr_transmit(void);
void limesdr_receive(void);
void limesdr_tx_rrc_filter(float rolloff);
int lime_tx_samples(scmplx *s, int len);
void limesdr_set_ical(char offset);
void limesdr_set_qcal(char offset);


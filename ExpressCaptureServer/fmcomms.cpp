/*
* libiio - AD9361 IIO streaming example
*
* Copyright (C) 2014 IABG mbH
* Author: Michael Feilen <feilen_at_iabg.de>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
**/
#include "stdafx.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include "error_codes.h"
#include "Dvb.h"
#include "hardware.h"

#ifdef __APPLE__
#include <iio/iio.h>
#else
#include <iio.h>
#endif


static char error_string[256];

#define FMC_ERROR(expr) { \
	if (!(expr)) { \
		(void) fprintf(stderr, "FMC_ERRORion failed (%s:%d)\n", __FILE__, __LINE__); \
		(void) abort(); \
	} \
}
/*
#define FMC_ERROR(expr) { \
	if (!(expr)) { \
		(void) sprintf_s(error_string, 256, "errorion failed (%s:%d)\n", __FILE__, __LINE__); \
         report_error(error_string); \
		 return -1; \
	} \
}
*/
/* helper macros */
#define KHZ(x) ((long long)(x*1000.0 + .5))
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

/* RX is input, TX is output */
enum iodev { RX, TX };

/* common RX and TX streaming params */
struct stream_cfg {
	long long bw_hz; // Analog banwidth in Hz
	long long fs_hz; // Baseband sample rate in Hz
	long long lo_hz; // Local oscillator frequency in Hz
	const char* rfport; // Port name
};

/* static scratch mem for strings */
static char tmpstr[64];

/* IIO structs required for streaming */
static struct iio_context *m_ctx = NULL;
static struct iio_channel *m_rx0_i = NULL;
static struct iio_channel *m_rx0_q = NULL;
static struct iio_channel *m_tx0_i = NULL;
static struct iio_channel *m_tx0_q = NULL;
static struct iio_buffer  *m_rxbuf = NULL;
static struct iio_buffer  *m_txbuf = NULL;
// Streaming devices
struct iio_device *m_tx = NULL;
struct iio_device *m_rx = NULL;
static bool m_running = false;
static bool stop;

/* cleanup and exit */
static void fmc_shutdown()
{
	m_running = false;

	if (m_ctx != NULL) {
		//printf("* Destroying buffers\n");
		if (m_rxbuf) { iio_buffer_destroy(m_rxbuf); m_rxbuf = NULL; }
		if (m_txbuf) { iio_buffer_destroy(m_txbuf); m_txbuf = NULL; }

		//printf("* Disabling streaming channels\n");
		if (m_rx0_i) { iio_channel_disable(m_rx0_i); }
		if (m_rx0_q) { iio_channel_disable(m_rx0_q); }
		if (m_tx0_i) { iio_channel_disable(m_tx0_i); }
		if (m_tx0_q) { iio_channel_disable(m_tx0_q); }

		//printf("* Destroying context\n");
		if (m_ctx) { iio_context_destroy(m_ctx); }
	}
}

static void handle_sig(int sig)
{
	printf("Waiting for process to finish...\n");
	stop = true;
}

/* check return value of attr_write function */
static void errchk(int v, const char* what) {
	if (v < 0) { fprintf(stderr, "FMC_ERROR %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); fmc_shutdown(); }
}
/* read attribute: long long int */
static void rd_ch_lli(struct iio_channel *chn, const char* what, long long *val)
{
	errchk(iio_channel_attr_read_longlong(chn, what, val), what);
}

/* write attribute: long long int */
static void wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{
	errchk(iio_channel_attr_write_longlong(chn, what, val), what);
}

/* write attribute: string */
static void wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
	errchk(iio_channel_attr_write(chn, what, str), what);
}

/* helper function generating channel names */
static char* get_ch_name(const char* type, int id)
{
	snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
	return tmpstr;
}

/* returns ad9361 phy device */
static struct iio_device* get_ad9361_phy(struct iio_context *ctx)
{
	struct iio_device *dev = iio_context_find_device(ctx, "ad9361-phy");
	FMC_ERROR(dev && "No ad9361-phy found");
	return dev;
}

/* finds AD9361 streaming IIO devices */
static bool get_ad9361_stream_dev(struct iio_context *ctx, enum iodev d, struct iio_device **dev)
{
	switch (d) {
	case TX: *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
	case RX: *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
	default: FMC_ERROR(0); return false;
	}
}

/* finds AD9361 streaming IIO channels */
static bool get_ad9361_stream_ch(struct iio_context *ctx, enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn)
{
	*chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), d == TX);
	if (!*chn)
		*chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), d == TX);
	return *chn != NULL;
}

/* finds AD9361 phy IIO configuration channel with id chid */
static bool get_phy_chan(struct iio_context *ctx, enum iodev d, int chid, struct iio_channel **chn)
{
	switch (d) {
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), false); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), true);  return *chn != NULL;
	default: FMC_ERROR(0); return false;
	}
}

/* finds AD9361 local oscillator IIO configuration channels */
static bool get_lo_chan(struct iio_context *ctx, enum iodev d, struct iio_channel **chn)
{
	switch (d) {
		// LO chan is always output, i.e. true
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 0), true); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 1), true); return *chn != NULL;
	default: FMC_ERROR(0); return false;
	}
}

/* applies streaming configuration through IIO */
bool cfg_ad9361_streaming_ch(struct iio_context *ctx, struct stream_cfg *cfg, enum iodev type, int chid)
{
	struct iio_channel *chn = NULL;

	// Configure phy and lo channels
	printf("* Acquiring AD9361 phy channel %d\n", chid);
	if (!get_phy_chan(ctx, type, chid, &chn)) { return false; }
	wr_ch_str(chn, "rf_port_select", cfg->rfport);
	wr_ch_lli(chn, "rf_bandwidth", cfg->bw_hz);
	wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);

	// Configure LO channel
	printf("* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
	if (!get_lo_chan(ctx, type, &chn)) { return false; }
	wr_ch_lli(chn, "frequency", cfg->lo_hz);
	return true;
}

/* simple configuration and streaming */
int fmc_init(void)
{
	if (m_running == true) return 0;

	// RX and TX sample counters
	size_t nrx = 0;
	size_t ntx = 0;

	// Stream configurations
	struct stream_cfg rxcfg;
	struct stream_cfg txcfg;

	// Listen to ctrl+c and FMC_ERROR
	signal(SIGINT, handle_sig);

	float BwKhz = 5000.0;

	// RX stream config
	rxcfg.bw_hz = KHZ(BwKhz);   // 2 MHz rf bandwidth
	rxcfg.fs_hz = MHZ(2.0);   // 2.0 MS/s rx sample rate
	rxcfg.lo_hz = GHZ(1.3); // 1.3 GHz rf frequency
	rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)

								 // TX stream config
	txcfg.bw_hz = KHZ(BwKhz); // 5 MHz rf bandwidth
	txcfg.fs_hz = MHZ(3.5);   // 2.5 MS/s tx sample rate
	txcfg.lo_hz = GHZ(1.3); // 2.5 GHz rf frequency
	txcfg.rfport = "A"; // port A (select for rf freq.)

	//printf("* Acquiring IIO context\n");
	if ((m_ctx = iio_create_network_context(get_sdr_ip_addrs())) == NULL) {
//	if ((m_ctx = iio_create_context_from_uri("usb:1.3.5")) == NULL) {
		report_error(ERROR_SDR_NOT_FOUND);
		cmd_set_error_text("ADALM-PLUTO Not found");
		return -1;
	}
	if (iio_context_get_devices_count(m_ctx) <= 0) {
		report_error(ERROR_SDR_NOT_FOUND);
		return -1;
	}

//	printf("* Acquiring AD9361 streaming devices\n");
	FMC_ERROR(get_ad9361_stream_dev(m_ctx, TX, &m_tx) && "No tx dev found");
	FMC_ERROR(get_ad9361_stream_dev(m_ctx, RX, &m_rx) && "No rx dev found");

//	printf("* Configuring AD9361 for streaming\n");
	//FMC_ERROR(cfg_ad9361_streaming_ch(m_ctx, &rxcfg, RX, 0) && "RX port 0 not found");
	FMC_ERROR(cfg_ad9361_streaming_ch(m_ctx, &txcfg, TX, 0) && "TX port 0 not found");

//	printf("* Initializing AD9361 IIO streaming channels\n");
//	FMC_ERROR(get_ad9361_stream_ch(m_ctx, RX, m_rx, 0, &m_rx0_i) && "RX chan i not found");
//	FMC_ERROR(get_ad9361_stream_ch(m_ctx, RX, m_rx, 1, &m_rx0_q) && "RX chan q not found");
	FMC_ERROR(get_ad9361_stream_ch(m_ctx, TX, m_tx, 0, &m_tx0_i) && "TX chan i not found");
	FMC_ERROR(get_ad9361_stream_ch(m_ctx, TX, m_tx, 1, &m_tx0_q) && "TX chan q not found");


//	printf("* Enabling IIO streaming channels\n");
//	iio_channel_enable(m_rx0_i);
//	iio_channel_enable(m_rx0_q);
	iio_channel_enable(m_tx0_i);
	iio_channel_enable(m_tx0_q);

	// Everything should be setup now so return a sucess
	m_running = true;
	fmc_load_tx_rrc_filter(0.35f);
	iio_context_set_timeout(m_ctx, 0);
	fmc_set_buff_length(100000);
	iio_device_set_kernel_buffers_count(m_tx, 4);
	fmc_set_receive();
	return 0;

/*
	printf("* Starting IO streaming (press CTRL+C to cancel)\n");
	while (!stop)
	{
		ssize_t nbytes_rx, nbytes_tx;
		int16_t *p_dat, *p_end;
		ptrdiff_t p_inc;

		// Schedule TX buffer
		nbytes_tx = iio_buffer_push(m_txbuf);
		if (nbytes_tx < 0) { printf("FMC_ERROR pushing buf %d\n", (int)nbytes_tx); fmc_shutdown(); }

		// Refill RX buffer
		nbytes_rx = iio_buffer_refill(m_rxbuf);
		if (nbytes_rx < 0) { printf("FMC_ERROR refilling buf %d\n", (int)nbytes_rx); fmc_shutdown(); }

		// READ: Get pointers to RX buf and read IQ from RX buf port 0
		p_inc = iio_buffer_step(m_rxbuf);
		p_end = (int16_t*)iio_buffer_end(m_rxbuf);
		for (p_dat = (int16_t*)iio_buffer_first(m_rxbuf, m_rx0_i); p_dat < p_end; p_dat += p_inc) {
			// Example: swap I and Q
			const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
			((int16_t*)p_dat)[0] = q;
			((int16_t*)p_dat)[1] = i;
		}

		// WRITE: Get pointers to TX buf and write IQ to TX buf port 0
		p_inc = iio_buffer_step(m_txbuf);
		p_end = (int16_t*)iio_buffer_end(m_txbuf);
		for (p_dat = (int16_t*)iio_buffer_first(m_txbuf, m_tx0_i); p_dat < p_end; p_dat += p_inc) {
			// Example: fill with zeros
			// 12-bit sample needs to be MSB alligned so shift by 4
			// https://wiki.analog.com/resources/eval/user-guides/ad-fmcomms2-ebz/software/basic_iq_datafiles#binary_format
			((int16_t*)p_dat)[0] = 0 << 4; // Real (I)
			((int16_t*)p_dat)[1] = 0 << 4; // Imag (Q)
		}

		// Sample counter increment and status output
		nrx += nbytes_rx / iio_device_get_sample_size(m_rx);
		ntx += nbytes_tx / iio_device_get_sample_size(m_tx);
		printf("\tRX %8.2f MSmp, TX %8.2f MSmp\n", nrx / 1e6, ntx / 1e6);
	}

	fmc_shutdown();
*/
	return 0;
}
void fmc_deinit(void) {
	fmc_shutdown();
}
//
//
void fmc_load_tx_filter(short *fir, int taps, int ratio, bool enable) {
	if (m_running == false) return;
	if (m_ctx == NULL) return;
	int res;
	struct iio_channel *chn = NULL;
	iio_device* dev = NULL;
	dev = get_ad9361_phy(m_ctx);
	int buffsize = 8192;
	char *buf = (char *)malloc(buffsize);
	int clen = 0;
	clen += sprintf_s(buf + clen, buffsize - clen, "RX 3 GAIN 0 DEC %d\n", ratio);
	clen += sprintf_s(buf + clen, buffsize - clen, "TX 3 GAIN 0 INT %d\n", ratio);
	clen += sprintf_s(buf + clen, buffsize - clen, "BWTX %d\n", 2000000);
	for (int i = 0; i < taps; i++) clen += sprintf_s(buf + clen, buffsize - clen, "%d,%d\n", fir[i], fir[i]);
	//for (int i = 0; i < taps; i++) clen += sprintf_s(buf + clen, buffsize - clen, "%d\n", fir[i]);
	clen += sprintf_s(buf + clen, buffsize - clen, "\n");
	res = iio_device_attr_write_raw(dev, "filter_fir_config", buf, clen);
//	chn = iio_device_find_channel(dev, "voltage0", true);
//	res = iio_channel_attr_write_bool(chn, "filter_fir_en", false);
//	chn = iio_device_find_channel(dev, "voltage0", false);
//	res = iio_channel_attr_write_bool(chn, "filter_fir_en", false);
//	if (!get_phy_chan(m_ctx, TX, 0, &chn)) { return; }
//	res = iio_channel_attr_write_bool(chn,"filter_fir_en", false);
	chn = iio_device_find_channel(dev, "out", false);
	res = iio_channel_attr_write_bool(chn, "voltage_filter_fir_en", true);

	//	if (!get_phy_chan(m_ctx, RX, 0, &chn)) { return; }
	free(buf);
}

void fmc_load_tx_rrc_filter(float roff) {
	if (m_running == false) return;

	//float MyRRC[128];

	short *fir = rrc_make_filter(roff, 4, 128);
	fmc_load_tx_filter(fir, 128, 4, true);
}
void fmc_load_tx_lpf_filter(float roff) {
	if (m_running == false) return;
	short *fir = lpf_make_filter(roff, 4, 128);
	fmc_load_tx_filter(fir, 128, 4, true);
}
void fmc_enable_tx_lo(void) {
	struct iio_channel *chn = NULL;
	iio_device* dev = NULL;
	dev = get_ad9361_phy(m_ctx);
	chn = iio_device_find_channel(dev, "altvoltage1", true);
	iio_channel_attr_write_bool(chn, "powerdown", false);
}
void fmc_disable_tx_lo(void) {
	struct iio_channel *chn = NULL;
	iio_device* dev = NULL;
	dev = get_ad9361_phy(m_ctx);
	chn = iio_device_find_channel(dev, "altvoltage1", true);
	iio_channel_attr_write_bool(chn, "powerdown", true);
}

void fmc_set_transmit(void) {
	if (m_running == false) return;
	if (m_ctx == NULL) return;
	fmc_set_tx_level(get_current_tx_level() - 47);
	fmc_enable_tx_lo();
}
void fmc_set_receive(void) {
	if (m_running == false) return;
	if (m_ctx == NULL) return;
	fmc_set_tx_level(-89);
//	fmc_disable_tx_lo();
}

void fmc_set_tx_level(double level) {
	if (m_running == false) return;
	if (m_ctx == NULL) return;
	if (level > 0) level = 0;
	if (level < -89) level = -89;

	struct iio_channel *chn = NULL;
	if (!get_phy_chan(m_ctx, TX, 0, &chn)) { return; }
	wr_ch_lli(chn, "hardwaregain", (long long int)level);
}

void fmc_set_tx_frequency(double freq) {
	if (m_running == false) return;
	struct iio_channel *chn = NULL;
	if (m_ctx == NULL) return;
	if (!get_lo_chan(m_ctx, TX, &chn)) { return; }
	wr_ch_lli(chn, "frequency", (long long int)freq);
}
void fmc_configure_x8_int_dec(long long int  sr) {
	if (m_running == false) return;
	if (m_ctx == NULL) return;
	iio_device* dev = NULL;
	struct iio_channel *chn = NULL;
	// Receive
	dev = iio_context_find_device(m_ctx, "cf-ad9361-lpc");
	if (dev == NULL) return;
	chn = iio_device_find_channel(dev, "voltage0", false);
	wr_ch_lli(chn, "sampling_frequency", sr);
	// Transmit
	dev = iio_context_find_device(m_ctx, "cf-ad9361-dds-core-lpc");
	if (dev == NULL) return;
	chn = iio_device_find_channel(dev, "voltage0", true);
	wr_ch_lli(chn, "sampling_frequency", sr);
}
void fmc_set_tx_sr(long long int sr) {
	if (m_running == false) return;
	struct iio_channel *chn = NULL;
	if (m_ctx == NULL) return;
	if (!get_phy_chan(m_ctx, TX, 0, &chn)) { return; }
	wr_ch_lli(chn, "sampling_frequency", (long long int)sr);
}

void fmc_set_analog_lpf(double bw) {
	if (m_running == false) return;
	struct iio_channel *chn = NULL;
	if (!get_phy_chan(m_ctx, TX, 0, &chn)) return;
	wr_ch_lli(chn, "rf_bandwidth", (int64_t)bw);
}
static uint32_t m_max_len; // Maximum size of the buffer
static uint32_t m_offset;  // Current offset into the buffer

void fmc_set_buff_length(uint32_t len) {
	// Change the size of the buffer
	m_max_len = len;
	if (m_txbuf) { iio_buffer_destroy(m_txbuf); m_txbuf = NULL; }
	m_txbuf = iio_device_create_buffer( m_tx, len, false);
	scmplx *s_b = (scmplx*)iio_buffer_first(m_txbuf, m_tx0_i);
	memset(s_b, 0, sizeof(scmplx)*len);
	iio_buffer_set_blocking_mode(m_txbuf, true);
	m_offset = 0;
}

void fmc_tx_samples( scmplx *s, int len) {
	if (m_running == false) return;
	if (len == 0) return;
	if (get_tx_status() == FALSE) return;
	// Get position of first sample in the buffer
	scmplx *s_b = (scmplx*)iio_buffer_first(m_txbuf, m_tx0_i);

	if ((m_offset + len) > m_max_len) {
		int l = m_max_len - m_offset;
		scmplx *b = (scmplx*)&s_b[m_offset];

		/*for (int i = 0; i < l; i++)
		{
			b[i].im = s[i].im<<4;
			b[i].re = s[i].re<<4;
		}*/
		memcpy(b, s, sizeof(scmplx)*l);
		int n = iio_buffer_push(m_txbuf);
		s = &s[l];
		len = len - l;
		m_offset = 0;
	}
	// Calculate the offset for the begining of the next update
	scmplx *b = (scmplx*)&s_b[m_offset];
	// Copy the Samples into the buffer
	memcpy(b, s, sizeof(scmplx)*len);
	m_offset += len;
}

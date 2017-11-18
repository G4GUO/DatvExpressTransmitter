#include "stdafx.h"
#include "Dvb.h"
#include "hardware.h"


#include <LimeSuite.h>


//#ifdef ENABLE_LIMESDR
#define _USE_MATH_DEFINES // for C++
#include <cmath>

#define EXP_OK    1
#define EXP_FAIL -1
#define EXP_MISS -2
#define EXP_IHX  -3
#define EXP_RBF  -4
#define EXP_CONF -5


static bool m_running = false;
static lms_device_t* device = NULL;
static lms_stream_t streamId;
static int   m_limesdr_status = EXP_CONF;
BOOL m_limesdr_tx = false;
double m_sr = 0;
float_type ShiftNCO[16] = { 1000000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
float m_gain = 0.0;
int m_oversample = 1;

int OVERSAMPLE = 16;

int lime_tx_samples(scmplx *s, int len) {
	if (m_running == false) return 0;
	static int debugCnt = 0;
	
		//memset(s, 0, len * sizeof(scmplx));
		int SampleSent = 0;
		if ((SampleSent = LMS_SendStream(&streamId, s, len, NULL, 500)) != len)
		{
			TRACE("len %d -> SampleSent %d \n", len, SampleSent);
		}
	
	debugCnt++;
	if ((debugCnt % 10) == 0)
	{
	lms_stream_status_t TxStatus;
	LMS_GetStreamStatus(&streamId, &TxStatus);
	TRACE("Filled %d SymbolRate %f\n", TxStatus.fifoFilledCount,TxStatus.sampleRate);
	
	}
	return 0;
}


int limesdr_init() {

	if (m_running == true) return 0;

	int n;
	lms_info_str_t list[8];
	if ((n = LMS_GetDeviceList(list)) < 0)
		return -1;
	
	if (device == NULL) {
		int i ;
		for (i = 0; i < n; i++) if (strstr(list[i], "LimeSDR") != NULL) break;
		if (i == n) return -1;
		if(LMS_Open(&device, list[i], NULL)!=0) return -1; // We can't open any of device
	}


	if (LMS_Init(device) != 0)
	{
		AfxMessageBox("LimeSDR failed to init");
		return -1;
	}
	LMS_Reset(device);

	if (LMS_EnableChannel(device, LMS_CH_TX, 0, false) != 0) //Not enable
		return -1;

	if (LMS_SetAntenna(device, LMS_CH_TX, 0, 1) != 0)
		return -1;

	streamId.channel = 0;
	streamId.fifoSize = 1024 * 100;
	streamId.throughputVsLatency = 0.0;
	streamId.isTx = true;
	streamId.dataFmt = lms_stream_t::LMS_FMT_I16;

	int res = LMS_SetupStream(device, &streamId);
	//LMS_StopStream(&streamId);
	m_running = true;

	return 0;
}

void limesdr_deinit(void) {
	if (m_running == false) return;
	m_running = false;
	int res;
	res= LMS_StopStream(&streamId);
	res = LMS_EnableChannel(device, LMS_CH_TX, 0, false);
	res= LMS_DestroyStream(device, &streamId);
	//
	
	res= LMS_Close(device);
	device = NULL;
	m_limesdr_status = EXP_CONF;

}

void limesdr_set_freq(double freq) {
	if (m_running == false) return;
		LMS_SetLOFrequency(device, LMS_CH_TX, 0, freq);

}

void limesdr_set_level(int level) {

	if (m_running == false) return;
	float_type gain = level / 47.0;
	m_gain = gain;
	LMS_SetNormalizedGain(device, LMS_CH_TX, 0, gain);
	LMS_Calibrate(device, LMS_CH_TX, 0, m_sr * m_oversample, 0);
}

int limesdr_set_sr(double sr,int OverSample) {


	if (m_running == false) return 0;
		float_type freq = 0;
		m_sr = sr;
		m_oversample = OverSample;
		lms_range_t Range;
		LMS_GetSampleRateRange(device, LMS_CH_TX, &Range);
		if ((m_sr < Range.min) || (m_sr > Range.max))
		{
			char sDebug[255];
			sprintf_s(sDebug, "Valid SR=%f-%f by %f step", Range.min, Range.max, Range.step);
			AfxMessageBox(sDebug);
		}
		/*
		if (m_sr <= 400000)	OVERSAMPLE = 32;
		if((m_sr>400000)&&(m_sr<=800000)) 	OVERSAMPLE = 16;
		if((m_sr>800000)&&(m_sr<=2000000)) OVERSAMPLE = 4;
		*/
		if (LMS_SetSampleRate(device, m_sr, OverSample) != 0)
		{
			AfxMessageBox("SR Not Set");
		}
		
		/*
		float_type HostSR, DacSR;
		LMS_GetSampleRate(device, LMS_CH_TX, 0, &HostSR, &DacSR);
		TRACE("SR %f DAC %f\n", HostSR, DacSR);
		LMS_Calibrate(device, LMS_CH_TX, 0, m_sr * 2 * OverSample, 0);
		*/
		/*LMS_GetLOFrequency(device, LMS_CH_TX, 0, &freq);
		if (freq != 0) {
			LMS_SetNCOFrequency(device, LMS_CH_TX, 0, ShiftNCO, 0);
			LMS_SetNCOIndex(device, LMS_CH_TX, 0, 0, true);

		}
		*/
	
	return 0;
}

void limesdr_tx_rrc_filter(float rolloff)
{
	if (m_running == false) return;
	int ntaps = 119;
	float *fir = rrc_make_f_filter(rolloff, 1, ntaps);
	float_type *LimeFir = (float_type*) malloc(ntaps * sizeof(float_type));
	for (int i = 0; i < ntaps; i++)
	{
		LimeFir[i] = fir[i];
	}
	if (LMS_SetGFIRCoeff(device, LMS_CH_TX, 0, LMS_GFIR3, LimeFir, ntaps)<0)
		AfxMessageBox("Unable to set coeff GFIR3");
	LMS_SetGFIR(device, LMS_CH_TX, 0, LMS_GFIR3, true);
	free(LimeFir);

}
void limesdr_transmit(void) {

	if (m_running == false) return;

		//LMS_SetupStream(device, &streamId);
		LMS_EnableChannel(device, LMS_CH_TX, 0, true);
		LMS_StartStream(&streamId);
		LMS_SetGFIR(device, LMS_CH_TX, 0, LMS_GFIR3, true); // When Channel is disable, GFIR enable is lost !!!!!! this is a workaround
		//LMS7_ISINC_BYP_TXTSP
		//Disable ISINC
		uint16_t Reg;
		LMS_ReadLMSReg(device, 0x208, &Reg);
		Reg = Reg | (1 << 7);
		LMS_WriteLMSReg(device, 0x0208, Reg);
		LMS_Calibrate(device, LMS_CH_TX, 0, m_sr * m_oversample, 0);


	
}

void limesdr_receive(void)
{
	
		if (m_running == false) return;
		m_limesdr_tx = FALSE;
		LMS_StopStream(&streamId);
		LMS_EnableChannel(device, LMS_CH_TX, 0, false);
		
		//LMS_DestroyStream(device, &streamId);

		//LMS_SetNormalizedGain(device, LMS_CH_TX, 0, 0);
		//LMS_DestroyStream(device, &streamId);
	

}


/*
What you are changing is digital dc offset.
0x0204[15:8] DCCORI
0x0204[7:0] DCCORQ
*/
static uint16_t m_Calib = 0;

void limesdr_set_qcal(char offset)
{

	m_Calib = (m_Calib & 0xFF00) + offset;
	
	LMS_WriteLMSReg(device, 0x204, m_Calib);

}

void limesdr_set_ical(char offset)
{
	m_Calib = (m_Calib & 0xFF) + offset<<8;
	LMS_WriteLMSReg(device, 0x204, m_Calib);
}

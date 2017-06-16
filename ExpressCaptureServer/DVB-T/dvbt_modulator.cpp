#include "stdafx.h"
#include <math.h>
#include "memory.h"
#include "dvb_t.h"


int m_tx_samples;
Scmplx m_sams[M32KS+M8KS];
static float max;
#define CLIP_TH 0.999f
//
// Input IQ samples float,
// length is the number of complex samples
// Output complex samples short
//
void dvb_t_clip( FFTComplex *in, int length )
{
    for( int i = 0; i < length; i++)
    {
        if(fabs(in[i].re) > CLIP_TH)
        {
           if(in[i].re > 0 )
                in[i].re = CLIP_TH;
            else
                in[i].re = -CLIP_TH;
        }
        if(fabs(in[i].im) > CLIP_TH)
        {
            if(in[i].im > 0 )
                in[i].im = CLIP_TH;
            else
                in[i].im = -CLIP_TH;
        }
    }
}
Scmplx *dvb_t_get_samples(void) {
	return m_sams;
}

void dvb_t_modulate(FFTComplex *in, int length, int guard)
{
	// Clip the waveform
	dvb_t_clip(in, length);

	// Convert to 16 bit fixed point and apply clipping where required

	FFTComplex *p = &in[length - guard];
	m_tx_samples = 0;

	for (int i = 0; i < guard; i++)
	{
		m_sams[m_tx_samples].re = (short)(p[i].re * 0x7FFF);
		m_sams[m_tx_samples].im = (short)(p[i].im * 0x7FFF);
		m_tx_samples++;
	}

	for (int i = 0; i < length; i++)
	{
		m_sams[m_tx_samples].re = (short)(in[i].re * 0x7FFF);
		m_sams[m_tx_samples].im = (short)(in[i].im * 0x7FFF);
		m_tx_samples++;
	}
	// The samples are now ready
}
void dvb_t_modulate(FFTComplex *in, float *taper, int length, int guard)
{
	// Clip the waveform
	dvb_t_clip(in, length);

	// Convert to 16 bit fixed point and apply clipping where required

	FFTComplex *p = &in[length - guard];
	m_tx_samples = 0;

	for (int i = 0; i < guard; i++)
	{
		m_sams[m_tx_samples].re = (short)(p[i].re * 0x7FFF * taper[m_tx_samples]);
		m_sams[m_tx_samples].im = (short)(p[i].im * 0x7FFF * taper[m_tx_samples]);
		m_tx_samples++;
	}

	for (int i = 0; i < length; i++)
	{
		m_sams[m_tx_samples].re = (short)(in[i].re * 0x7FFF * taper[m_tx_samples]);
		m_sams[m_tx_samples].im = (short)(in[i].im * 0x7FFF * taper[m_tx_samples]);
		m_tx_samples++;
	}
	// The samples are now ready
}
void dvb_t_modulate_init(void)
{
}

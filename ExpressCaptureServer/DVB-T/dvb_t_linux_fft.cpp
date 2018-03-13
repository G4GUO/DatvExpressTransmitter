#include "stdafx.h"
#include "math.h"
#include <stdio.h>
#include <memory.h>
#include "dvb_t.h"
#include "libavcodec\avfft.h"

extern DVBTFormat m_format;

static FFTContext *m_avfft_2k_context;
static FFTContext *m_avfft_4k_context;
static FFTContext *m_avfft_8k_context;
static FFTContext *m_avfft_16k_context;
static FFTContext *m_avfft_32k_context;
static FFTComplex *m_fft;
//
// Taper table
//
float m_taper[M32KS+(M32KS/4)];

void create_taper_table(int N, int IR, int GI)
{
    N         = N*IR;// FFT size
    int CP    = N/GI;//Cyclic prefix
    int ALPHA = 32;// Rx Alpha
    int NT    = 2*(N/(ALPHA*2));//Taper length
    int idx   = 0;
    int n     = -NT/2;

    for( int i = 0; i < NT; i++ ){
        m_taper[idx++] = (float)(0.5f*(1.0f+cosf(((float)M_PI*n)/NT)));
        n++;
    }
    for( int i = 0; i < (N+CP)-(2*NT); i++ ){
        m_taper[idx++] = 1.0;
    }
    for( int i = 0; i < NT; i++ ){
        m_taper[idx++] = m_taper[i];
    }
}

// sinc correction coefficients
float m_c[M32KS];

void create_correction_table( int N, int IR )
{
    float x,sinc;
    float f = 0.0;
    float fsr = (float)dvb_t_get_sample_rate();
    float fstep = fsr / (N*IR);

    for( int i = 0; i < N/2; i++ )
    {
        if( f == 0 )
            sinc = 1.0f;
        else{
            x = (float)M_PI*f/fsr;
            sinc = sinf(x)/x;
        }
//        m_c[i+(N/2)]   = 1.0f/sinc;
//        m_c[(N/2)-i-1] = 1.0f/sinc;
		m_c[i + (N / 2)] = 1.0f;
		m_c[(N / 2) - i - 1] = 1.0f;
		f += fstep;
    }
}
//
// 2K FFT x1 interpolation
//
static int fft_2k_x1( FFTComplex *in )
{

    // Copy the data into the correct bins
//    memcpy(&m_fft_in[M2KS/2], &in[0],      sizeof(fft_complex)*M2KS/2);
//    memcpy(&m_fft_in[0],      &in[M2KS/2], sizeof(fft_complex)*M2KS/2);

    int i,m;
	
	m = (M2KS/2);

    for( i = 0; i < (M2KS); i++ )
    {
		m_fft[m].re = in[i].re*m_c[i];
		m_fft[m].im = in[i].im*m_c[i];
        m = (m+1)%(M2KS);
    }

    av_fft_permute( m_avfft_2k_context, m_fft );
    av_fft_calc(    m_avfft_2k_context, m_fft );
	return M2KS;
}
//
// 2K FFT x2 Interpolation
//
static int fft_2k_x2( FFTComplex *in )
{
    // Zero the unused parts of the array
    memset(&m_fft[M4KS/4],0,sizeof(FFTComplex)*M4KS/2);
    // Copy the data into the correct bins
    //memcpy(&m_fft_in[M4KS*3/4], &in[0],      sizeof(fft_complex)*M2KS/2);
    //memcpy(&m_fft_in[0],        &in[M2KS/2], sizeof(fft_complex)*M2KS/2);

    int i,m;
    m = (M4KS)-(M2KS/2);
    for( i = 0; i < (M2KS); i++ )
    {
        m_fft[m].re = in[i].re*m_c[i];
        m_fft[m].im = in[i].im*m_c[i];
        m = (m+1)%(M4KS);
    }

    av_fft_permute( m_avfft_4k_context, m_fft );
    av_fft_calc(    m_avfft_4k_context, m_fft );

    return M4KS;
}
//
// 2K FFT x4 interpolation
//
static int fft_2k_x4(FFTComplex *in)
{
	// Zero the unused parts of the array
	memset(&m_fft[M8KS / 8], 0, sizeof(FFTComplex)*M8KS * 6 / 8);
	// Copy the data into the correct bins
	// Copy the data into the correct bins
	//    memcpy(&m_fft_in[M8KS*7/8], &in[0],      sizeof(fft_complex)*M2KS/2);
	//    memcpy(&m_fft_in[0],        &in[M2KS/2], sizeof(fft_complex)*M2KS/2);

	int i, m;
	m = (M8KS)-(M2KS / 2);
	for (i = 0; i < (M2KS); i++)
	{
		m_fft[m].re = in[i].re*m_c[i];
		m_fft[m].im = in[i].im*m_c[i];
		m = (m + 1) % M8KS;
	}

	av_fft_permute(m_avfft_8k_context, m_fft);
	av_fft_calc(m_avfft_8k_context, m_fft);
	return M8KS;

}

//
// Interpolate by 4 using an 8K FFT
//
static int fft_2k_nb( FFTComplex *in)
{
    // Zero the unused parts of the array
    memset(&m_fft[M8KS/8],0,sizeof(FFTComplex)*M8KS*6/8);
    // Copy the data into the correct bins
    // Copy the data into the correct bins
//    memcpy(&m_fft_in[M8KS*7/8], &in[0],      sizeof(fft_complex)*M2KS/2);
//    memcpy(&m_fft_in[0],        &in[M2KS/2], sizeof(fft_complex)*M2KS/2);

    int i,m;
    m = (M8KS)-(M8KS/8);
    for( i = 0; i < (M2KS); i++ )
    {
        m_fft[m].re = in[i].re*m_c[i];
        m_fft[m].im = in[i].im*m_c[i];
        m = (m+1)%M2KS;
    }

    av_fft_permute( m_avfft_8k_context, m_fft );
    av_fft_calc(    m_avfft_8k_context, m_fft );
	return M8KS;
}
//
// 8K FFT x1 interpolation
//
static int fft_8k_x1( FFTComplex *in )
{
    // Copy the data into the correct bins
//    memcpy(&m_fft_in[M8KS/2], &in[0],      sizeof(fft_complex)*M8KS/2);
//    memcpy(&m_fft_in[0],      &in[M8KS/2], sizeof(fft_complex)*M8KS/2);

    int i,m;
    m = (M8KS/2);
    for( i = 0; i < (M8KS); i++ )
    {
        m_fft[m].re = in[i].re*m_c[i];
        m_fft[m].im = in[i].im*m_c[i];
        m = (m+1)%M8KS;
    }

    av_fft_permute( m_avfft_8k_context, m_fft );
    av_fft_calc(    m_avfft_8k_context, m_fft );
	return M8KS;

}
//
// 8K x2 interpolation
//
static int fft_8k_x2( FFTComplex *in )
{
    // Zero the unused parts of the array
    memset(&m_fft[M8KS/2],0,sizeof(FFTComplex)*M8KS);
    // Copy the data into the correct bins
//    memcpy(&m_fft_in[M8KS*3/2], &in[0],      sizeof(fft_complex)*M8KS/2);
//    memcpy(&m_fft_in[0],        &in[M8KS/2], sizeof(fft_complex)*M8KS/2);

    int i,m;
    m = (M16KS)-(M8KS/2);
    for( i = 0; i < (M8KS); i++ )
    {
        m_fft[m].re = in[i].re*m_c[i];
        m_fft[m].im = in[i].im*m_c[i];
        m = (m+1)%(M16KS);
    }

    av_fft_permute( m_avfft_16k_context, m_fft );
    av_fft_calc(    m_avfft_16k_context, m_fft );
	return M16KS;
}
//
// 8K x4 interpolation
//
static int fft_8k_x4( FFTComplex *in )
{
	// Zero the unused parts of the array
	memset(&m_fft[M8KS / 2], 0, sizeof(FFTComplex)*(M16KS+M8KS));
	// Copy the data into the correct bins
	//    memcpy(&m_fft_in[M8KS*3/2], &in[0],      sizeof(fft_complex)*M8KS/2);
	//    memcpy(&m_fft_in[0],        &in[M8KS/2], sizeof(fft_complex)*M8KS/2);

	int i, m;
	m = (M32KS)-(M8KS / 2);
	for (i = 0; i < (M8KS); i++)
	{
		m_fft[m].re = in[i].re*m_c[i];
		m_fft[m].im = in[i].im*m_c[i];
		m = (m + 1) % (M32KS);
	}

	av_fft_permute(m_avfft_32k_context, m_fft);
	av_fft_calc(m_avfft_32k_context, m_fft);
	return M32KS;
}
int(*do_fft)(FFTComplex *in);
//
// Chooses the corect FFT, adds the guard interval and sends to the modulator
//
void dvbt_fft_modulate( FFTComplex *in, int guard )
{
	int size = do_fft(in);
    dvb_t_modulate( m_fft, size, guard );
}
/*
void fft_2k_test(  fftw_complex *out )
{
    memset(fftw_in, 0, sizeof(fftw_complex)*M2KS);
    int m = (M2KS/2)+32;//1704;
    fftw_in[m].re =  0.7;

    fftw_one( m_fftw_2k_plan, fftw_in, out );
    return;
}
*/
void init_dvb_t_fft( void )
{
    //
    // Plans
    //
    m_avfft_2k_context  = av_fft_init (11, 1);
    m_avfft_4k_context  = av_fft_init (12, 1);
    m_avfft_8k_context  = av_fft_init (13, 1);
    m_avfft_16k_context = av_fft_init (14, 1);
	m_avfft_32k_context = av_fft_init (15, 1);
	m_fft  = (FFTComplex*)av_malloc(sizeof(FFTComplex)*M32KS);
	
	int N;

	if( m_format.tm == TM_2K)
    {
		if(m_format.ir == 1) do_fft = fft_2k_x1;
		if(m_format.ir == 2) do_fft = fft_2k_x2;
		if(m_format.ir == 4) do_fft = fft_2k_x4;
		N = M2KS;
    }
    if( m_format.tm == TM_8K)
    {
		if (m_format.ir == 1) do_fft = fft_8k_x1;
		if (m_format.ir == 2) do_fft = fft_8k_x2;
		if (m_format.ir == 4) do_fft = fft_8k_x4;
		N = M8KS;
    }
    int GI;
    switch(m_format.gi)
    {
        case GI_132:
            GI = 32;
            break;
        case GI_116:
            GI = 16;
            break;
        case GI_18:
            GI = 8;
            break;
        case GI_14:
            GI = 4;
            break;
        default:
            GI = 4;
            break;
    }

    create_correction_table( N, m_format.ir);
    create_taper_table( N, m_format.ir, GI );
}
void deinit_dvb_t_fft( void )
{
    if(m_fft != NULL)   av_free(m_fft);

}

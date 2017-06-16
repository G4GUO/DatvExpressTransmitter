#include "stdafx.h"
#include <time.h>
#include <iostream>
#include <chrono>

LARGE_INTEGER m_StartingTime;
LARGE_INTEGER m_Frequency;

void init_dvb_clock(void) {
	QueryPerformanceFrequency(&m_Frequency);
	QueryPerformanceCounter(&m_StartingTime);

}
int64_t dvb_get_time(void) {
	// Activity to be timed
	LARGE_INTEGER EndingTime;
	LARGE_INTEGER ElapsedMicroseconds;

	QueryPerformanceCounter(&EndingTime);
	
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - m_StartingTime.QuadPart;

	//
	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.
	//
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= m_Frequency.QuadPart;
	ElapsedMicroseconds.QuadPart = ElapsedMicroseconds.QuadPart/27;
	return (int64_t)ElapsedMicroseconds.QuadPart;
}
#include "stdafx.h"
#include <stdint.h>
#include <memory.h>
#include <queue>
#include <deque>
#include <list>
#include "Dvb.h"

using namespace std;
static queue <SampleBuffer> m_vq;
static queue <SampleBuffer> m_aq;
static DWORD glpVideoThreadId;
static DWORD glpAudioThreadId;
static HANDLE ghWriteVideoEvent;
static HANDLE ghWriteAudioEvent;
static BOOL gThreadRunning;
// Mutexes
static CRITICAL_SECTION mutex_video;
static CRITICAL_SECTION mutex_audio;

void vb_video_post(uint8_t *b, int len) {
	
	EnterCriticalSection(&mutex_video);
	SampleBuffer video_cb;
	video_cb.b = (uint8_t *)malloc(len);
	video_cb.len = len;
	video_cb.time = get_pcr_clock();
	memcpy(video_cb.b, b, len);
	m_vq.push(video_cb);
	LeaveCriticalSection(&mutex_video);
	SetEvent(ghWriteVideoEvent);
}
void vb_audio_post(uint8_t *b, int len) {
	EnterCriticalSection(&mutex_audio);
	SampleBuffer audio_cb;
	audio_cb.b = (uint8_t *)malloc(len);
	audio_cb.len = len;
	audio_cb.time = get_pcr_clock();
	memcpy(audio_cb.b, b, len);
	m_aq.push(audio_cb);
	LeaveCriticalSection(&mutex_audio);
	SetEvent(ghWriteAudioEvent);
}

UINT vb_video_thread(LPVOID pParam){
	DWORD dwWaitResult;
	while (gThreadRunning) {
		dwWaitResult = WaitForSingleObject(
			ghWriteVideoEvent, // event handle
			INFINITE);    // indefinite wait

		EnterCriticalSection(&mutex_video);
		while (m_vq.size() > 0) {
			SampleBuffer video_cb = m_vq.front();
			// Do processing
			video_codec_samples(video_cb.b, video_cb.len, video_cb.time);
			free(video_cb.b);
			m_vq.pop();
		}
		LeaveCriticalSection(&mutex_video);
	}
	return 0;
}
UINT vb_audio_thread(LPVOID pParam) {
	DWORD dwWaitResult;
	while (gThreadRunning) {
		dwWaitResult = WaitForSingleObject(
			ghWriteAudioEvent, // event handle
			INFINITE);    // indefinite wait

		EnterCriticalSection(&mutex_audio);
		while (m_aq.size() > 0) {
			SampleBuffer audio_cb = m_aq.front();
			// Do processing
			process_audio_codec_samples(audio_cb.b, audio_cb.len, audio_cb.time);
			free(audio_cb.b);
			m_aq.pop();
		}
		LeaveCriticalSection(&mutex_audio);
	}
	return 0;
}

int vb_av_init(void) {

	int n = 0;
	gThreadRunning = TRUE;

	ghWriteVideoEvent = CreateEvent(
		NULL,               // default security attributes
		FALSE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("WriteVideoEvent")  // object name
		);
	ghWriteAudioEvent = CreateEvent(
		NULL,               // default security attributes
		FALSE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("WriteAudioEvent")  // object name
		);

	CreateThread(NULL, //Choose default security
		0, //Default stack size
		(LPTHREAD_START_ROUTINE)&vb_video_thread, //Routine to execute
		(LPVOID)&n, //Thread parameter
		0, //Immediately run the thread
		&glpVideoThreadId //Thread Id
	);
	CreateThread(NULL, //Choose default security
		0, //Default stack size
		(LPTHREAD_START_ROUTINE)&vb_audio_thread, //Routine to execute
		(LPVOID)&n, //Thread parameter
		0, //Immediately run the thread
		&glpAudioThreadId //Thread Id
		);
	InitializeCriticalSection(&mutex_video);
	InitializeCriticalSection(&mutex_audio);

	return 0;
}
void vb_deinit(void) {
	gThreadRunning = FALSE;
	CloseHandle(ghWriteVideoEvent);
	CloseHandle(ghWriteAudioEvent);
}
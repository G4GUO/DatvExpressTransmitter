#include "stdafx.h"
#include <dshow.h>
#include <atlbase.h>
#include <initguid.h>
#include <dvdmedia.h>
#include "SampleGrabber.h"
#include "Dvb.h"
#include "Codecs.h"
#include "error_codes.h"

class VideoCallbackObject : public ISampleGrabberCB {
public: 

    VideoCallbackObject() {};

    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) 
    {      
        if (NULL == ppv) return E_POINTER;
        if (riid == __uuidof(IUnknown)) {
            *ppv = static_cast<IUnknown*>(this);
             return S_OK;
        }
        if (riid == __uuidof(ISampleGrabberCB))   {
            *ppv = static_cast<ISampleGrabberCB*>(this);
             return S_OK;
        }
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() {    return S_OK;  }
    STDMETHODIMP_(ULONG) Release() {   return S_OK;  }

    //ISampleGrabberCB
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);
    STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) { return S_OK; }
};

STDMETHODIMP VideoCallbackObject::SampleCB(double SampleTime, IMediaSample *pSample)
{
    if (!pSample)
        return E_POINTER;
    long sz = pSample->GetActualDataLength();
    BYTE *pBuf = NULL;
    pSample->GetPointer(&pBuf);
    if (sz <= 0 || pBuf==NULL) return E_UNEXPECTED;
	vb_video_post( pBuf, sz );
//	video_codec_samples( pBuf, sz );
    return S_OK;
}
class AudioCallbackObject : public ISampleGrabberCB {
public: 

    AudioCallbackObject() {};

    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) 
    {      
        if (NULL == ppv) return E_POINTER;
        if (riid == __uuidof(IUnknown)) {
            *ppv = static_cast<IUnknown*>(this);
             return S_OK;
        }
        if (riid == __uuidof(ISampleGrabberCB))   {
            *ppv = static_cast<ISampleGrabberCB*>(this);
             return S_OK;
        }
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() {    return S_OK;  }
    STDMETHODIMP_(ULONG) Release() {   return S_OK;  }

    //ISampleGrabberCB
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);
    STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) { return S_OK; }
};

STDMETHODIMP AudioCallbackObject::SampleCB(double SampleTime, IMediaSample *pSample)
{
    if (!pSample)
        return E_POINTER;
    long sz = pSample->GetActualDataLength();
    BYTE *pBuf = NULL;
    pSample->GetPointer(&pBuf);
    if (sz <= 0 || pBuf==NULL) return E_UNEXPECTED;
	audio_codec_samples( pBuf, sz );
    return S_OK;
}
int check_hr( int hr, const char *text){
	if(hr != S_OK) cmd_set_error_text(text);
	return hr;
}
int check_hr(int hr, const char *text, int error) {
	if (hr != S_OK) {
		cmd_set_error_text(text);
		report_error(error);
	}
	return hr;
}

#define CHECK_HR(x,y)   if(check_hr(x,y) != S_OK) return -1
#define CHECK_HR2(x,y,z) if(check_hr(x,y,z) != S_OK) return -1

CComPtr<IPin> GetPin(IBaseFilter *pFilter, LPCOLESTR pinname)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin> pPin;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	
	while(pEnum->Next(1,&pPin,0) == S_OK)
	{
		PIN_INFO pinfo;
		pPin->QueryPinInfo(&pinfo);
		BOOL found = !_wcsicmp(pinname, pinfo.achName);
		if(pinfo.pFilter) pinfo.pFilter->Release();
		if(found) return pPin;
		pPin.Release();
	}
	return NULL;

}
CComPtr<IPin> GetOutputPin(IBaseFilter *pFilter)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin> pPin;
	BOOL found = FALSE;
	HRESULT hr = pFilter->EnumPins(&pEnum);

	while (pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_INFO pinfo;
		BOOL found;
		pPin->QueryPinInfo(&pinfo);
		if (pinfo.dir == PINDIR_OUTPUT) found = TRUE;
		if (pinfo.pFilter) pinfo.pFilter->Release();
		if (found) return pPin;
		pPin.Release();
	}
	return NULL;
}
// {C1F400A0-3F08-11D3-9F0B-006008039E37}
DEFINE_GUID(CLSID_SampleGrabber,
0xC1F400A0, 0x3F08, 0x11D3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37); //qedit.dll

//{8E14549A-DB61-4309-AFA1-3578E927E938}
DEFINE_GUID(CLSID_vMixVideo,
0x8E14549A, 0xDB61, 0x4309, 0xAF, 0xA1, 0x35, 0x78, 0xE9, 0x27, 0xE9, 0x38); //qedit.dll

//{008E145B-DB61-4309-AEA1-3578E927E940}
DEFINE_GUID(CLSID_vMixAudio,
0x008E145B, 0xDB61, 0x4309, 0xAE, 0xA1, 0x35, 0x78, 0xE9, 0x27, 0xE9, 0x40); //qedit.dll

// {C1F400A4-3F08-11D3-9F0B-006008039E37}
DEFINE_GUID(CLSID_NullRenderer,
0xC1F400A4, 0x3F08, 0x11D3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37); //qedit.dll

//{30323449-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_I420,
0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71); //Missing 

//{43594448-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_HDYC,
0x43594448, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71); //Missing 

// Enumerate all of the audio input devices
// Return the filter with a matching friendly name
HRESULT GetAudioInputFilter(IBaseFilter** gottaFilter, wchar_t* matchName)
{
	BOOL done = false;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the audio input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					//wprintf(L"Testing Audio Input Device: %s\n", varName.bstrVal);

					// Do a comparison, find out if it's the right one
					if ((wcsncmp(varName.bstrVal, matchName, wcslen(matchName)) == 0)&&(wcslen(matchName)!=0)){

						// We found it, so send it back to the caller
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**) gottaFilter);
						done = true;
					}
				}
				VariantClear(&varName);	
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	if (done) {
		return hr;	// found it, return native error
	} else {
		return VFW_E_NOT_FOUND;	// didn't find it error
	}
}

// Enumerate all of the video input devices
// Return the filter with a matching friendly name
HRESULT GetVideoInputFilter(IBaseFilter** gottaFilter, wchar_t* matchName)
{
	BOOL done = false;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the video input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					//wprintf(L"Testing Video Input Device: %s\n", varName.bstrVal);

					// Do a comparison, find out if it's the right one
					if ((wcsncmp(varName.bstrVal, matchName, wcslen(matchName)) == 0)&&(wcslen(matchName)!=0)) {
						// We found it, so send it back to the caller
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**) gottaFilter);
						done = true;
					}
				}
				VariantClear(&varName);	
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	if (done) {
		return hr;	// found it, return native error
	} else {
		return VFW_E_NOT_FOUND;	// didn't find it error
	}
}

// Enumerate all of the audio input devices
// Return a list of their names
// @names  the name strings
// @num  max number of names
// @return the number of found names
//
int GetAudioInputCaptureList(CString *names, int num )
{
	int n = 0;

	BOOL done = false;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return n;
	}

	// Obtain a class enumerator for the audio input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					names[n] = varName.bstrVal;
					n++;
				}
				VariantClear(&varName);	
				pPropBag->Release();
			}
			pMoniker->Release();
			if(n >= num ){
				pEnumCat->Release();
				pSysDevEnum->Release();
				return n;
			}
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	return n;
}

// Enumerate all of the video input devices
// Return a list of their names
// @names  the name strings
// @num  max number of names
// @return the number of found names
//
int GetVideoInputCaptureList(CString *names, int num)
{
	int n = 0;

	BOOL done = false;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the video input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					names[n] = varName.bstrVal;
					n++;
				}
				VariantClear(&varName);	
				pPropBag->Release();
			}
			pMoniker->Release();
			if(n >= num ){
				pSysDevEnum->Release();
				pSysDevEnum->Release();
				return n;
			}
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	return n;
}

// Code adopted from example in the DX9 SDK.
// This code allows us to find the input pins an audio input filter
// We'll print out a list of them, indicating the enabled one
int GetAudioCaptureFormat(IBaseFilter *pFilter, WAVEFORMATEX &w_format)
{
    IEnumPins  *pEnum = NULL;
    IPin       *pPin = NULL;
	PIN_DIRECTION PinDirThis;
	PIN_INFO	pInfo;
	IAMAudioInputMixer *pAMAIM = NULL;
	BOOL pfEnable = FALSE;
	int n = 0;

	// Begin by enumerating all the pins on a filter
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return n;
    }

	// Now, look for a pin that matches the direction characteristic.
	// When we've found it, we'll examine it.
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
		// Get the pin direction
        pPin->QueryDirection(&PinDirThis);
		if (PinDirThis == PINDIR_OUTPUT) {

			// OK, we've found an output pin on the filter, yay!
			// Now let's get the information on that pin
			// So we can print the name of the pin to the console
			hr = pPin->QueryPinInfo(&pInfo);
			if (SUCCEEDED(hr)) {
				//wprintf(L"Input pin: %s\n", pInfo.achName);
				if(1)
//				if((wcsncmp(L"Capture", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
//					(wcsncmp(L"Capturer", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
//					(wcsncmp(L"AudioOut", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
//					(wcsncmp(L"Audio Capture", pInfo.achName, wcslen(pInfo.achName)) == 0))
				{
					IEnumMediaTypes *pEnumA = NULL;
					if((hr = pPin->EnumMediaTypes(&pEnumA))==S_OK){
					    AM_MEDIA_TYPE *pmt;
					    while(pEnumA->Next(1, &pmt, 0) == S_OK ){
							if(pmt->formattype == FORMAT_WaveFormatEx ){
								WAVEFORMATEX *paudioformat = (WAVEFORMATEX *)pmt->pbFormat;
								w_format = *paudioformat;
								n++;
							}
  					    }
					    pEnumA->Release();
					}
				}
				pInfo.pFilter->Release();
			}
		}
        pPin->Release();
    }
    pEnum->Release();
    return n;  
}
// Code adopted from example in the DX9 SDK.
// This code allows us to find the input pins an audio input filter
// We'll print out a list of them, indicating the enabled one
int GetVideoCaptureFormat(IBaseFilter *pFilter, AM_MEDIA_TYPE media[], VIDEOINFOHEADER format[], int num)
{
    IEnumPins  *pEnum = NULL;
    IPin       *pPin = NULL;
	PIN_DIRECTION PinDirThis;
	PIN_INFO	pInfo;
	IAMAudioInputMixer *pAMAIM = NULL;
	BOOL pfEnable = FALSE;
	int n = 0;

	// Begin by enumerating all the pins on a filter
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return n;
    }

	// Now, look for a pin that matches the direction characteristic.
	// When we've found it, we'll examine it.
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
		// Get the pin direction
        pPin->QueryDirection(&PinDirThis);
		if (PinDirThis == PINDIR_OUTPUT) {

			// OK, we've found an input pin on the filter, yay!
			// Now let's get the information on that pin
			// So we can print the name of the pin to the console
			hr = pPin->QueryPinInfo(&pInfo);
			if (SUCCEEDED(hr)) {
				//wprintf(L"Input pin: %s\n", pInfo.achName);
				if(1)
				//if ((wcsncmp(L"Capture", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
				//	(wcsncmp(L"Capturer", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
				//	(wcsncmp(L"~Capture", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
				//	(wcsncmp(L"~Capturer", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
				//	(wcsncmp(L"Video Capture", pInfo.achName, wcslen(pInfo.achName)) == 0))
				{
					IEnumMediaTypes *pEnumV = NULL;
					if((hr = pPin->EnumMediaTypes(&pEnumV))==S_OK){
						AM_MEDIA_TYPE *pmt;
					    while(pEnumV->Next(1, &pmt, 0) == S_OK ){
							if(pmt->formattype == FORMAT_VideoInfo ){
								VIDEOINFOHEADER *pvideoformat = (VIDEOINFOHEADER *)pmt->pbFormat;
								if(n < num ){
									format[n] = *pvideoformat;
									media[n]  = *pmt;
									n++;
								}
							}
  					    }
					    pEnumV->Release();
					}
				}
				pInfo.pFilter->Release();
			}
		}
        pPin->Release();
    }
    pEnum->Release();
    return n;  
}
int GetVideoInputCaptureFormatList(CString *device, CString *formats, int num){
	int n = 0;
	CComPtr<IBaseFilter> pvMixVideo;
	wchar_t wname[100];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, device->GetBuffer(0), -1, wname, 100);

	if(GetVideoInputFilter(&pvMixVideo, wname) == S_OK){
		VIDEOINFOHEADER format[100];
		AM_MEDIA_TYPE   media[100];
		n = GetVideoCaptureFormat(pvMixVideo, media, format, num);
		for( int i = 0; i < n; i++){
			REFERENCE_TIME fps = 10000000/format[i].AvgTimePerFrame;

		   char c[5];
		   c[0] = (media[i].subtype.Data1)&0xFF;
   		   c[1] = (media[i].subtype.Data1>>8)&0xFF;
		   c[2] = (media[i].subtype.Data1>>16)&0xFF;
		   c[3] = (media[i].subtype.Data1>>24)&0xFF;
		   c[4] = 0;
		   CString type(c);
   		   if(media[i].subtype == MEDIASUBTYPE_RGB24) type = "RGB24";
           if(media[i].subtype == MEDIASUBTYPE_RGB32) type = "RGB32";

		   formats[i].Format("%d %d %d ",format[i].bmiHeader.biWidth,format[i].bmiHeader.biHeight,fps);
		   formats[i] += type;
		}
		pvMixVideo.Release();
	}
	return n;
}
//
// Given a filter and the wanted codec params in the src field find a match
// returns 0 if fails
//
int MatchVideoCaptureFormat(IBaseFilter *pFilter, CodecParams *params, AM_MEDIA_TYPE *media, VIDEOINFOHEADER *format )
{
    IEnumPins  *pEnum = NULL;
    IPin       *pPin = NULL;
	PIN_DIRECTION PinDirThis;
	PIN_INFO	pInfo;
	IAMAudioInputMixer *pAMAIM = NULL;
	BOOL pfEnable = FALSE;
	int n = -1;
	int found = 0;

	// Begin by enumerating all the pins on a filter
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return n;
    }

	// Now, look for a pin that matches the direction characteristic.
	// When we've found it, we'll examine it.
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
		// Get the pin direction
        pPin->QueryDirection(&PinDirThis);
		if (PinDirThis == PINDIR_OUTPUT) {

			// OK, we've found an input pin on the filter
			// Now let's get the information on that pin
			// So we can print the name of the pin to the console
			hr = pPin->QueryPinInfo(&pInfo);
			if (SUCCEEDED(hr)) {
				//wprintf(L"Input pin: %s\n", pInfo.achName);
				if(1)
				//if ((wcsncmp(L"Capture", pInfo.achName, wcslen(pInfo.achName)) == 0) || 
				//	(wcsncmp(L"Capturer", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
				//	(wcsncmp(L"~Capture", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
				//   (wcsncmp(L"Video Capture", pInfo.achName, wcslen(pInfo.achName)) == 0))
				{
					IEnumMediaTypes *pEnumV = NULL;
					if((hr = pPin->EnumMediaTypes(&pEnumV))==S_OK){
						AM_MEDIA_TYPE *pmt;
					    while((pEnumV->Next(1, &pmt, 0) == S_OK)&&(found == 0)){
							if(pmt->formattype == FORMAT_VideoInfo ){
								VIDEOINFOHEADER *pvideoformat = (VIDEOINFOHEADER *)pmt->pbFormat;
								REFERENCE_TIME fps = 10000000/pvideoformat->AvgTimePerFrame;
								if((pvideoformat->bmiHeader.biWidth == params->v_src_width) &&
								   (pvideoformat->bmiHeader.biHeight == params->v_src_height) &&
								   (fps >= params->v_src_fps)){
								   if((pmt->subtype == MEDIASUBTYPE_UYVY)&&(params->v_src_pixfmt = AV_PIX_FMT_UYVY422)){
									   found = 1;
								   }
								   if((pmt->subtype == MEDIASUBTYPE_YUY2)&&(params->v_src_pixfmt == AV_PIX_FMT_YUYV422)){
									   found = 1;
								   }
								   if((pmt->subtype == MEDIASUBTYPE_RGB24)&&(params->v_src_pixfmt == AV_PIX_FMT_RGB24)){
									   found = 1;
								   }
								   if((pmt->subtype == MEDIASUBTYPE_I420)&&(params->v_src_pixfmt == AV_PIX_FMT_YUV420P)){
									   found = 1;
								   }
								   if((pmt->subtype == MEDIASUBTYPE_MJPG)&&(params->v_src_pixfmt == AV_PIX_FMT_JPEG)){
									   found = 1;
								   }
								   if((pmt->subtype == MEDIASUBTYPE_HDYC)&&(params->v_src_pixfmt == AV_PIX_FMT_UYVY422)){
									   found = 1;
								   }

								   if(found){
									   *format = *pvideoformat;
									   format->AvgTimePerFrame = 10000000/params->v_dst_fps;
									   format->dwBitRate = (format->dwBitRate*params->v_dst_fps)/ params->v_src_fps;

									   *media  = *pmt;
								   }
								}
							}
  					    }
					    pEnumV->Release();
					}
				}
				pInfo.pFilter->Release();
			}
		}
        pPin->Release();
    }
    pEnum->Release();
    return found;  
}
//
// Set the size of the audio blocks
//
HRESULT SetAudioSampleBlockSize( IBaseFilter *pFilter, int size )
{
    IEnumPins  *pEnum = NULL;
    IPin       *pPin = NULL;
	PIN_DIRECTION PinDirThis;
	PIN_INFO	pInfo;
	IAMAudioInputMixer *pAMAIM = NULL;
	BOOL pfEnable = FALSE;
	int n = S_OK;
	int found = 0;

	// Begin by enumerating all the pins on a filter
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }

	// Now, look for a pin that matches the direction characteristic.
	// When we've found it, we'll examine it.
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
		// Get the pin direction
        pPin->QueryDirection(&PinDirThis);
		if (PinDirThis == PINDIR_OUTPUT) {

			// OK, we've found an output pin on the filter, yay!
			// Now let's get the information on that pin
			// So we can print the name of the pin to the console
			hr = pPin->QueryPinInfo(&pInfo);
			if (SUCCEEDED(hr)) {
				//wprintf(L"Input pin: %s\n", pInfo.achName);
				IEnumMediaTypes *pEnumV = NULL;
				if ((hr = pPin->EnumMediaTypes(&pEnumV)) == S_OK) {
					AM_MEDIA_TYPE *pmt;
					while ((pEnumV->Next(1, &pmt, 0) == S_OK) && (found == 0)) {
						if ((pmt->majortype == MEDIATYPE_Audio)&&(pmt->subtype == MEDIASUBTYPE_PCM)) {
							if(1)
							//if ((wcsncmp(L"Capture", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
						//		(wcsncmp(L"AudioOut", pInfo.achName, wcslen(pInfo.achName)) == 0) ||
							//	(wcsncmp(L"Audio Capture", pInfo.achName, wcslen(pInfo.achName)) == 0)) 
							{
								// Get the interfaces we need.
								CComQIPtr<IAMStreamConfig> streamconfig(pPin);
								if (!streamconfig) {
									pInfo.pFilter->Release();
									pPin->Release();
									pEnum->Release();
									return E_NOINTERFACE;
								}
								CComQIPtr<IAMBufferNegotiation> bufneg(pPin);
								if (!bufneg) {
									pInfo.pFilter->Release();
									pPin->Release();
									pEnum->Release();
									return E_NOINTERFACE;
								}
								ALLOCATOR_PROPERTIES props;
								props.cBuffers = -1;
								props.cbBuffer = size;
								props.cbAlign = -1;
								props.cbPrefix = -1;
								bufneg->SuggestAllocatorProperties(&props);
								found = 1;
							}
						}
					}
					pEnumV->Release();
				}
			}
			pInfo.pFilter->Release();
		}
        pPin->Release();
    }
    pEnum->Release();
    return n;  
}
//
// This builds the graph
//
HRESULT BuildGraph(IGraphBuilder *pGraph,CodecParams *params)
{
    HRESULT hr = S_OK;
	wchar_t wname[100];

	// Set up a default format for video 
    AM_MEDIA_TYPE videopmt;
    ZeroMemory(&videopmt, sizeof(AM_MEDIA_TYPE));
    videopmt.majortype = MEDIATYPE_Video;
    videopmt.subtype = MEDIASUBTYPE_UYVY;
    videopmt.formattype = FORMAT_VideoInfo;
    videopmt.bFixedSizeSamples = TRUE;
    videopmt.cbFormat = 88;
    videopmt.lSampleSize = 829440;
    videopmt.bTemporalCompression = FALSE;
    VIDEOINFOHEADER videoformat;
    ZeroMemory(&videoformat, sizeof(VIDEOINFOHEADER));
	videoformat.dwBitRate = 165888000;
	videoformat.AvgTimePerFrame = 400000;
    videoformat.bmiHeader.biSize = 40;
    videoformat.bmiHeader.biWidth = 720;
    videoformat.bmiHeader.biHeight = 576;
    videoformat.bmiHeader.biPlanes = 1;
    videoformat.bmiHeader.biBitCount = 16;
    videoformat.bmiHeader.biCompression = 1498831189; 
    videoformat.bmiHeader.biSizeImage = 829440;

	// This is the only format that will be supported for Audio 
	AM_MEDIA_TYPE audiopmt;
    ZeroMemory(&audiopmt, sizeof(AM_MEDIA_TYPE));
    audiopmt.majortype = MEDIATYPE_Audio;
    audiopmt.subtype = MEDIASUBTYPE_PCM;
    audiopmt.formattype = FORMAT_WaveFormatEx;
    audiopmt.bFixedSizeSamples = TRUE;
    audiopmt.cbFormat = 18;
    audiopmt.lSampleSize = 4;
    audiopmt.bTemporalCompression = FALSE;
    WAVEFORMATEX audioformat;
    ZeroMemory(&audioformat, sizeof(WAVEFORMATEX));
	audioformat.wFormatTag = 1;
	if (params->a_br == 32000) {
		// Mono
		audiopmt.lSampleSize = 2;
		audioformat.nChannels = 1;
		audioformat.wBitsPerSample = 16;
		audioformat.nSamplesPerSec = 48000;
		audioformat.nAvgBytesPerSec = audioformat.nSamplesPerSec*audioformat.nChannels*audioformat.wBitsPerSample / 8;
		audioformat.nBlockAlign = 2;
		audiopmt.pbFormat = (BYTE*)&audioformat;
	}
	else {
		// Stereo
		audiopmt.lSampleSize = 4;
		audioformat.nChannels = 2;
		audioformat.wBitsPerSample = 16;
		audioformat.nSamplesPerSec = 48000;
		audioformat.nAvgBytesPerSec = audioformat.nSamplesPerSec*audioformat.nChannels*audioformat.wBitsPerSample / 8;
		audioformat.nBlockAlign = 4;
		audiopmt.pbFormat = (BYTE*)&audioformat;
	}
	
    //graph builder
    CComPtr<ICaptureGraphBuilder2> pBuilder;
    hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    CHECK_HR(hr, _T("Can't create Capture Graph Builder"));
    hr = pBuilder->SetFiltergraph(pGraph);
    CHECK_HR(hr, _T("Can't SetFiltergraph"));

	//Add Video Capture Device
    CComPtr<IBaseFilter> pvVideo;
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, get_video_capture_device(), -1, wname, 100);

	hr = GetVideoInputFilter(&pvVideo, wname);

	//hr = pvMixVideo.CoCreateInstance(CLSID_vMixVideo);
    CHECK_HR2(hr, _T("Invalid Video Capture Device"),ERROR_INVALID_VIDEO_CAPTURE);
    hr = pGraph->AddFilter(pvVideo, L"Video Capture");
    CHECK_HR(hr, _T("Can't add Video Capture device to graph"));

	// Find the parameters that most closely match the requested ones.
	MatchVideoCaptureFormat( pvVideo, params, &videopmt, &videoformat );
    videopmt.pbFormat = (BYTE*)&videoformat;
	
	//Add Audio Capture Device
    CComPtr<IBaseFilter> pvAudio;
	if (strcmp(get_audio_capture_device(), S_USE_VIDEO_DEVICE) != 0) {
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, get_audio_capture_device(), -1, wname, 100);
		hr = GetAudioInputFilter(&pvAudio, wname);
	}
	else {
		// Combined video and Audio device
		pvAudio = pvVideo;
	}

    //hr = pvMixAudio.CoCreateInstance(CLSID_vMixAudio);
    CHECK_HR2(hr, _T("Invalid Audio Capture Device"),ERROR_INVALID_AUDIO_CAPTURE);

	if (params->a_br == 32000) {
		// Mono
		hr = SetAudioSampleBlockSize(pvAudio, 1152*2);
	}
	else {
		// Stereo
		hr = SetAudioSampleBlockSize(pvAudio, 1152*4);
	}
    CHECK_HR(hr, _T("Cant set Audio Capture Sample block size"));

    hr = pGraph->AddFilter(pvAudio, L"Audio Capture");
    CHECK_HR(hr, _T("Can't add Audio Capture device to graph"));


//	VIDEOINFOHEADER w_format;
//	GetVideoCaptureFormat(pvMixVideo, w_format);
   
	//Create a VideoSampleGrabber
    CComPtr<IBaseFilter> pVideoSampleGrabber;
    hr = pVideoSampleGrabber.CoCreateInstance(CLSID_SampleGrabber);
    CHECK_HR(hr, _T("Can't create Video SampleGrabber"));
   
	CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> pVideoSampleGrabber_isg(pVideoSampleGrabber);
	// Set the format of the input to the sample grabber
	hr = pVideoSampleGrabber_isg->SetMediaType(&videopmt);
    CHECK_HR(hr, _T("Can't set Media Type to Video Sample Grabber"));

	IAMStreamConfig *pConfig = NULL;
    hr = pBuilder->FindInterface(
    &PIN_CATEGORY_CAPTURE, // Capture pin.
    &MEDIATYPE_Video,    // Any media type.
    pvVideo, // Pointer to the capture filter.
    IID_IAMStreamConfig, (void**)&pConfig);
    hr = pConfig->SetFormat(&videopmt);

	// Add the Video sample grabber to the graph that should force the cpture output
	hr = pGraph->AddFilter(pVideoSampleGrabber, L"VideoSampleGrabber");
    CHECK_HR(hr, _T("Can't add Video SampleGrabber to graph"));

	// Now provide the Video callback:
    hr = pVideoSampleGrabber_isg->SetCallback(new VideoCallbackObject(), 0);
    CHECK_HR(hr, _T("Can't set Video callback"));

    //add AudioSampleGrabber
    CComPtr<IBaseFilter> pAudioSampleGrabber;
    hr = pAudioSampleGrabber.CoCreateInstance(CLSID_SampleGrabber);
    CHECK_HR(hr, _T("Can't create SampleGrabber"));
    hr = pGraph->AddFilter(pAudioSampleGrabber, L"AudioSampleGrabber");
    CHECK_HR(hr, _T("Can't add Audio SampleGrabber to graph"));

	CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> pAudioSampleGrabber_isg(pAudioSampleGrabber);
	hr = pAudioSampleGrabber_isg->SetMediaType(&audiopmt);
    CHECK_HR(hr, _T("Can't set Media Type to Audio Sample Grabber"));
	
    // Now provide the Audio callback:
    hr = pAudioSampleGrabber_isg->SetCallback(new AudioCallbackObject(), 0);
    CHECK_HR(hr, _T("Can't set Audio callback"));

	// Connect the Video and the sample grabber
	hr = pGraph->ConnectDirect(GetOutputPin(pvVideo),GetPin(pVideoSampleGrabber, L"Input"), NULL );
//	if(hr != S_OK) hr = pGraph->ConnectDirect(GetPin(pvVideo, L"Capture"), GetPin(pVideoSampleGrabber, L"Input"), NULL);
//	if(hr != S_OK) hr = pGraph->ConnectDirect(GetPin(pvVideo, L"Capturer"), GetPin(pVideoSampleGrabber, L"Input"), NULL);
//	if(hr != S_OK) hr = pGraph->ConnectDirect(GetPin(pvVideo, L"~Capture"), GetPin(pVideoSampleGrabber, L"Input"), NULL);
//	if(hr != S_OK ) hr = pGraph->ConnectDirect(GetPin(pvVideo, L"Video Capture"),GetPin(pVideoSampleGrabber, L"Input"), NULL );
    CHECK_HR(hr, _T("Can't connect Video to Sample Grabber"));

	// Connect Audio and the sample grabber
	hr = pGraph->ConnectDirect(GetOutputPin(pvAudio),GetPin(pAudioSampleGrabber, L"Input"), NULL );
//	if(hr != S_OK) hr = pGraph->ConnectDirect(GetPin(pvAudio, L"Capture"), GetPin(pAudioSampleGrabber, L"Input"), NULL);
//	if(hr != S_OK) hr = pGraph->ConnectDirect(GetPin(pvAudio, L"Capturer"), GetPin(pAudioSampleGrabber, L"Input"), NULL);
//	if(hr != S_OK) hr = pGraph->ConnectDirect(GetPin(pvAudio, L"Audio Capture"),GetPin(pAudioSampleGrabber, L"Input"), NULL );
//	if(hr != S_OK) hr = pGraph->ConnectDirect(GetPin(pvAudio, L"AudioOut"), GetPin(pAudioSampleGrabber, L"Input"), NULL);
	CHECK_HR(hr, _T("Can't connect Audio to Sample Grabber"));

	//Add Video Null Renderer
    CComPtr<IBaseFilter> pvNullRenderer;
    hr = pvNullRenderer.CoCreateInstance(CLSID_NullRenderer);
    CHECK_HR(hr, _T("Can't create Video Null Renderer"));
    hr = pGraph->AddFilter(pvNullRenderer, L"Video Null Renderer");
    CHECK_HR(hr, _T("Can't add Video Null Renderer to graph"));

	//Add Audio Null Renderer
    CComPtr<IBaseFilter> paNullRenderer;
    hr = paNullRenderer.CoCreateInstance(CLSID_NullRenderer);
    CHECK_HR(hr, _T("Can't create Audio Null Renderer"));
    hr = pGraph->AddFilter(paNullRenderer, L"Audio Null Renderer");
    CHECK_HR(hr, _T("Can't add Audio Null Renderer to graph"));

	// Connect Video Sample Grabber to Null Renderer
	hr = pGraph->ConnectDirect(GetPin(pVideoSampleGrabber, L"Output"),GetPin(pvNullRenderer, L"In"), NULL );
    CHECK_HR(hr, _T("Can't connect Video Sample Grabber to Null renderer"));

	// Connect Audio Sample Grabber to Null Renderer
	hr = pGraph->ConnectDirect(GetPin(pAudioSampleGrabber, L"Output"),GetPin(paNullRenderer, L"In"), NULL );
    CHECK_HR(hr, _T("Can't connect Audio Sample Grabber to Null renderer"));
  
    return S_OK;
}

static CComPtr<IGraphBuilder> g_graph;
static BOOL g_capture_active;

int capture_start(CodecParams *params)
{
	g_capture_active = FALSE;

	if (strlen(get_video_capture_device()) == 0) {
		cmd_set_error_text("No Video Capture device configured");
		return -1;
	}
	if (strlen(get_audio_capture_device()) == 0) {
		cmd_set_error_text("No Audio Capture device configured");
		return -1;
	}
	if (strlen(get_video_capture_format()) == 0) {
		cmd_set_error_text("No Video Capture format configured");
		return -1;
	}

	CoInitialize(NULL);

	g_graph.CoCreateInstance(CLSID_FilterGraph);

	HRESULT hr = BuildGraph(g_graph,params);

	if(hr==S_OK){
		CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl(g_graph);
		g_capture_active = TRUE;
		return S_OK;
	}
	else {
		g_graph.Release();
		CoUninitialize();
		return -1;
	}
}

void capture_stop(void) {
	if (g_capture_active == TRUE) {
		CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl(g_graph);
		mediaControl->Stop();
		g_graph.Release();
		CoUninitialize();
		g_capture_active = FALSE;
	}
}
void capture_pause(void){
	if (g_capture_active == TRUE) {
		CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl(g_graph);
		mediaControl->Stop();
	}
}
void capture_run(void){
	if (g_capture_active == TRUE) {
		CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl(g_graph);
		mediaControl->Run();
	}
}
void capture_time(void) {
	if (g_capture_active == TRUE) {
		CComQIPtr<IReferenceClock, &IID_IReferenceClock> refClock(g_graph);
		REFERENCE_TIME mTime;
		refClock->GetTime(&mTime);
	}
}

/*
IReferenceClock *clk = NULL;
hr = pBuilder->QueryInterface(IID_IReferenceClock, (void **)&clk);
REFERENCE_TIME *pTime = NULL;
clk->GetTime(pTime);
*/
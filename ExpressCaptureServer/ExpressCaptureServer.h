
// ExpressCaptureServer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "DVB-S2/DVBS2.h"


// CExpressCaptureServerApp:
// See ExpressCaptureServer.cpp for the implementation of this class
//

class CExpressCaptureServerApp : public CWinApp
{
public:
	CExpressCaptureServerApp();
	CString m_sConfigFile;
// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CExpressCaptureServerApp theApp;
extern DVBS2 theDvbS2;
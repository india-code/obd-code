
// synway_obd.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Csynway_obdApp:
// See synway_obd.cpp for the implementation of this class
//

class Csynway_obdApp : public CWinApp
{
public:
	Csynway_obdApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Csynway_obdApp theApp;
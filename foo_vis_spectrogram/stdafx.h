// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
// ATL
#include <atlcom.h>
//#include <atlhost.h>
#include <atlctl.h>
#include <atlwin.h>

// WTL
#include <atlapp.h>
//#include <atlframe.h>
#include <atlcrack.h>
#include <atlmisc.h>
//#include <atldlgs.h>
#include <atlctrls.h>

#include <foobar2000/SDK/foobar2000.h>
#include <foobar2000/helpers/helpers.h>

#undef BEGIN_MSG_MAP
#define BEGIN_MSG_MAP(x) BEGIN_MSG_MAP_EX(x)

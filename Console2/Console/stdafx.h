// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER			0x0500
#define _WIN32_WINNT	0x0500 
#define _WIN32_IE		0x0400
#define _RICHEDIT_VER	0x0100

//////////////////////////////////////////////////////////////////////////////

#include <atlbase.h>
#include <atlcoll.h>
#include <atlstr.h>
#include <atltypes.h>
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES

#include <atlapp.h>

extern CAppModule _Module;

#include <atlmisc.h>
#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>

#pragma warning(push)
#pragma warning(disable: 4189)
#include "WtlTabbing/atlgdix.h"

#include "WtlTabbing/CustomTabCtrl.h"
#include "WtlTabbing/DotNetTabCtrl.h"
//#include "WtlTabbing/SimpleTabCtrls.h"
//#include "WtlTabbing/SimpleDlgTabCtrls.h"
#include "WtlTabbing/TabbedFrame.h"
//#include "WtlTabbing/TabbedMDI.h"
#pragma warning(pop)

#include <iostream>
#include <string>
#include <map>
using namespace std;

#pragma warning(push)
#pragma warning(disable: 4244 4267 4511 4512 4702)
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
using namespace boost;
#pragma warning(pop)

#include "ConsoleHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// trace function and TRACE macro

#ifdef _DEBUG

void Trace(const wchar_t* pszFormat, ...);

#define TRACE		::Trace

#else

#define TRACE		__noop

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////


// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER			0x0501
#define _WIN32_WINNT	0x0501 
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

#include <atlwin.h>
#include <atlddx.h>
#include <atlmisc.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atltheme.h>

#pragma warning(push)
#pragma warning(disable: 4189 4267)
#include "atlgdix.h"

#include "CustomTabCtrl.h"
#include "DotNetTabCtrl.h"
//#include "SimpleTabCtrls.h"
//#include "SimpleDlgTabCtrls.h"
#include "TabbedFrame.h"
//#include "TabbedMDI.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4702)
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
using namespace std;
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244 4267 4511 4512 4701 4702)
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
using namespace boost;
#pragma warning(pop)

#include "Helpers.h"
#include "ConsoleHandler.h"
#include "ImageHandler.h"
#include "SettingsHandler.h"

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


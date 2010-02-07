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

#pragma warning(push)
#pragma warning(disable: 4996)
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
#pragma warning(pop)

#include "userenv.h"

#pragma warning(push)
#pragma warning(disable: 4189 4267)
#include "atlgdix.h"

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// the next three are copies from resource.h, MODIFY IN SYNC
// they have to be defined *before* CustomTabCtrl.h is included
#define IDS_TABBINGFRAMEWORK_TIPS_SCROLLRIGHT 187
#define IDS_TABBINGFRAMEWORK_TIPS_SCROLLLEFT 188
#define IDS_TABBINGFRAMEWORK_TIPS_CLOSE 189

#include "CustomTabCtrl.h"
#include "DotNetTabCtrl.h"
//#include "SimpleTabCtrls.h"
//#include "SimpleDlgTabCtrls.h"
#include "TabbedFrame.h"
//#include "TabbedMDI.h"
#pragma warning(pop)

#include <HtmlHelp.h>

#pragma warning(disable: 4503) // disables 'name truncated' warnings

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
#pragma warning(disable: 4244 4267 4511 4512 701 4702)
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
using namespace boost;
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4510 4610)
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
using namespace boost::multi_index;
#pragma warning(pop)

#include "../shared/SharedMemory.h"
#include "../shared/Structures.h"

#include "Helpers.h"
#include "ConsoleHandler.h"
#include "ImageHandler.h"
#include "SettingsHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Version numbers

#define	VERSION_MAJOR	2
#define	VERSION_MINOR	0
#define	VERSION_BUILD	148

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// WinXP defines

#define SPI_GETFONTSMOOTHINGTYPE	0x200A
#define FE_FONTSMOOTHINGCLEARTYPE	0x0002

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// User-defined messages

#define UM_UPDATE_CONSOLE_VIEW	WM_USER + 0x1000
#define UM_CONSOLE_RESIZED		WM_USER + 0x1001
#define UM_CONSOLE_CLOSED		WM_USER + 0x1002
#define UM_UPDATE_TITLES		WM_USER + 0x1003
#define UM_SHOW_POPUP_MENU		WM_USER + 0x1004
#define UM_START_MOUSE_DRAG		WM_USER + 0x1005
#define UM_TRAY_NOTIFY			WM_USER + 0x1006

#define UPDATE_CONSOLE_RESIZE		0x0001
#define UPDATE_CONSOLE_TEXT_CHANGED	0x0002

#define IDC_TRAY_ICON		0x0001

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Global hotkeys

#define IDC_GLOBAL_ACTIVATE	0xB000

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Windows' console window menu commands

#define SC_CONSOLE_COPY			0xFFF0
#define SC_CONSOLE_PASTE		0xFFF1
#define SC_CONSOLE_MARK			0xFFF2
#define SC_CONSOLE_SCROLL		0xFFF3
#define SC_CONSOLE_FIND			0xFFF4
#define SC_CONSOLE_SELECT_ALL	0xFFF5

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Memory allocation tracking

#ifdef _DEBUG
#include <crtdbg.h>
void* __cdecl operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#define DEBUG_NEW new(__FILE__, __LINE__)

void __cdecl operator delete(void* p, LPCSTR lpszFileName, int nLine);
#endif

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


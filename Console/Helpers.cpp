#include "StdAfx.h"
#include "Helpers.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::GetModulePath(HINSTANCE hInstance)
{
	wchar_t szModulePath[MAX_PATH] = L"";

	::GetModuleFileName(hInstance, szModulePath, MAX_PATH);

	wstring strPath(szModulePath);

	return strPath.substr(0, strPath.rfind(L'\\')+1);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::GetModuleFileName(HINSTANCE hInstance)
{
	wchar_t szModulePath[MAX_PATH] = L"";

	::GetModuleFileName(hInstance, szModulePath, MAX_PATH);

	wstring strPath(szModulePath);

	return strPath;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::ExpandEnvironmentStrings(const wstring& str)
{
	wchar_t szExpanded[0x8000] = L"";

	::ExpandEnvironmentStrings(str.c_str(), szExpanded, ARRAYSIZE(szExpanded));

	return wstring(szExpanded);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::ExpandEnvironmentStringsForUser(HANDLE userToken, const wstring& str)
{
	wchar_t szExpanded[0x8000] = L"";

	::ExpandEnvironmentStringsForUser(userToken, str.c_str(), szExpanded, ARRAYSIZE(szExpanded));

	return wstring(szExpanded);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void Helpers::GetMonitorRect(HWND hWnd, CRect& rectMonitor)
{
	HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	GetMonitorRect(hMonitor, true, rectMonitor);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void Helpers::GetDesktopRect(HWND hWnd, CRect& rectDesktop)
{
	HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	GetMonitorRect(hMonitor, false, rectDesktop);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void Helpers::GetDesktopRect(const CPoint& point, CRect& rectDesktop)
{
	HMONITOR hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
	GetMonitorRect(hMonitor, false, rectDesktop);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void Helpers::GetMonitorRect(HMONITOR hMonitor, bool bIgnoreTaskbar, CRect& rectDesktop)
{
	::ZeroMemory(&rectDesktop, sizeof(CRect));

	MONITORINFO		mi;

	::ZeroMemory(&mi, sizeof(MONITORINFO));
	mi.cbSize = sizeof(MONITORINFO);

	::GetMonitorInfo(hMonitor, &mi);
	rectDesktop = mi.rcMonitor;

	if (!bIgnoreTaskbar)
	{
		HWND hWndTaskbar = ::FindWindow(L"Shell_TrayWnd", L"");

		if (hWndTaskbar)
		{
			CRect rectTaskbar(0, 0, 0, 0);
			::GetWindowRect(hWndTaskbar, &rectTaskbar);
			rectDesktop.SubtractRect(&(mi.rcMonitor), rectTaskbar);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HBITMAP Helpers::CreateBitmap(HDC dc, DWORD dwWidth, DWORD dwHeight, CBitmap& bitmap)
{
//	HBITMAP hBmp = bitmap.CreateCompatibleBitmap(dc, dwWidth, dwHeight);
//	if (hBmp != NULL) return hBmp;

	// failed to create compatible bitmap, fall back to DIB section...
	BITMAPINFO	bmpInfo;
	void*		pBits = NULL;
	
	::ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));

//	DWORD dwBytesPerLine =   (((32 * bkImage->dwImageWidth) + 31) / 32 * 4); 
	bmpInfo.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth		= dwWidth;
	bmpInfo.bmiHeader.biHeight		= dwHeight;
	bmpInfo.bmiHeader.biPlanes		= static_cast<WORD>(::GetDeviceCaps(dc, PLANES));
	bmpInfo.bmiHeader.biBitCount	= static_cast<WORD>(::GetDeviceCaps(dc, BITSPIXEL));
	bmpInfo.bmiHeader.biCompression	= BI_RGB;
	bmpInfo.bmiHeader.biSizeImage	= 0;//dwBytesPerLine*bkImage->dwImageHeight;


	return bitmap.CreateDIBSection(dc, &bmpInfo, DIB_RGB_COLORS, &pBits, NULL, 0);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::LoadString(UINT uID)
{
  wchar_t str[0x800];

  if( ::LoadString(::GetModuleHandle(NULL), uID, str, ARRAYSIZE(str)) )
    return wstring(str);
  else
    return wstring(L"LoadString failed");
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HICON Helpers::LoadTabIcon(bool bBigIcon, bool bUseDefaultIcon, const wstring& strIcon, const wstring& strShell)
{
  if (bUseDefaultIcon)
  {
    if ( !strShell.empty() )
    {
      wstring strCommandLine = Helpers::ExpandEnvironmentStrings(strShell);
      int argc = 0;
      std::unique_ptr<LPWSTR[], LocalFreeHelper> argv(::CommandLineToArgvW(strCommandLine.c_str(), &argc));

      if ( argv && argc > 0 )
      {
        SHFILEINFO info;
        memset(&info, 0, sizeof(info));
        if( ::SHGetFileInfo(
          argv[0],
          0,
          &info,
          sizeof(info),
          SHGFI_ICON | (( bBigIcon )? SHGFI_LARGEICON : SHGFI_SMALLICON)) != 0 )
        {
          return info.hIcon;
        }
      }
    }
  }
  else
  {
    if (!strIcon.empty())
    {
      if ( bBigIcon )
      {
        return static_cast<HICON>(
          ::LoadImage(
            NULL,
            Helpers::ExpandEnvironmentStrings(strIcon).c_str(),
            IMAGE_ICON,
            0,
            0,
            LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_DEFAULTSIZE));
      }
      else
      {
        return static_cast<HICON>(
          ::LoadImage(
            NULL,
            Helpers::ExpandEnvironmentStrings(strIcon).c_str(),
            IMAGE_ICON,
            16,
            16,
            LR_DEFAULTCOLOR | LR_LOADFROMFILE));
      }
    }
  }

  if ( bBigIcon )
  {
    return static_cast<HICON>(
      ::LoadImage(
        ::GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON,
        0,
        0,
        LR_DEFAULTCOLOR | LR_DEFAULTSIZE));
  }
  else
  {
    return static_cast<HICON>(
      ::LoadImage(
        ::GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON,
        16,
        16,
        LR_DEFAULTCOLOR));
  }
}

//////////////////////////////////////////////////////////////////////////////


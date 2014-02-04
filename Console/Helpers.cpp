#include "StdAfx.h"
#include "Helpers.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::GetModulePath(HINSTANCE hInstance)
{
	wchar_t szModulePath[MAX_PATH];
	::ZeroMemory(szModulePath, sizeof(szModulePath));

	::GetModuleFileName(hInstance, szModulePath, MAX_PATH);

	wstring strPath(szModulePath);

	return strPath.substr(0, strPath.rfind(L'\\')+1);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::ExpandEnvironmentStrings(const wstring& str)
{
	shared_array<wchar_t> szExpanded(new wchar_t[0x8000]);

	::ZeroMemory(szExpanded.get(), 0x8000*sizeof(wchar_t));
	::ExpandEnvironmentStrings(str.c_str(), szExpanded.get(), 0x8000);

	return wstring(szExpanded.get());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::ExpandEnvironmentStringsForUser(const shared_ptr<void>& userToken, const wstring& str)
{
	shared_array<wchar_t> szExpanded(new wchar_t[MAX_PATH]);

	::ZeroMemory(szExpanded.get(), MAX_PATH*sizeof(wchar_t));
	::ExpandEnvironmentStringsForUser(userToken.get(), str.c_str(), szExpanded.get(), MAX_PATH);

	return wstring(szExpanded.get());
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
	int bufferSize = 0x800;
	scoped_array<wchar_t>	str(new wchar_t[bufferSize]);
	::ZeroMemory(str.get(), bufferSize * sizeof(wchar_t));
	
	bufferSize = ::LoadString(::GetModuleHandle(NULL), uID, str.get(), bufferSize);
	
	return wstring(str.get());
}

void Helpers::LocalizeComboBox (CComboBox& combo, UINT initialStringID)
{
	int count = combo.GetCount();
	CAtlString localized;

	combo.ResetContent();
	while (count--) {
		localized.LoadString(initialStringID++);
		combo.AddString(localized);
	}
}

//////////////////////////////////////////////////////////////////////////////

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

wstring Helpers::GetCurrentDirectory(void)
{
	wchar_t szCD[MAX_PATH] = L"";

	if( ::GetCurrentDirectory(MAX_PATH, szCD) == 0 )
		return wstring();
	else
		return wstring(szCD);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring Helpers::EscapeCommandLineArg(const wstring& str)
{
	wstring result(L"\"");
	result += str;
	if( str.back() == L'\\' )
		result += L"\\";
	result += L"\"";

	return result;
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


const wchar_t * Helpers::GetEnvironmentVariable(const wchar_t * envb, const wchar_t * str, size_t len /*= SIZE_MAX*/)
{
	const wchar_t * ptr = envb;

	if( len == SIZE_MAX ) len = wcslen(str);

	while ((ptr[0] != L'\x00') && !(_wcsnicmp(ptr, str, len) == 0 && ptr[len] == L'=')) ptr += wcslen(ptr)+1;

	if( ptr[0] != L'\x00' )
		return ptr + len + 1;

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::ExpandEnvironmentStrings(const wchar_t * envb, const std::wstring & str)
{
	std::wstring result;

	for(size_t i = 0; i < str.length(); ++i)
	{
		if(str[i] != L'%')
		{
			result += str[i];
		}
		else
		{
			const wchar_t * value = nullptr;
			size_t len = 0;

			for(size_t j = i + 1; j < str.length(); ++j, ++len)
			{
				if(str[j] == L'%')
				{
					if(len > 0)
					{
						value = Helpers::GetEnvironmentVariable(
							envb,
							str.data() + i + 1,
							len);
					}
					break;
				}
			}

			if(value)
			{
				result.append(value);
				i += len + 1;
			}
			else
			{
				result.append(str.data() + i, len + 1);
				i += len;
			}
		}
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::ExpandEnvironmentStrings(const std::map<std::wstring, std::wstring, __case_insensitive_compare> & dictionary, const std::wstring & str)
{
	std::wstring result;

	for(size_t i = 0; i < str.length(); ++i)
	{
		if(str[i] != L'%')
		{
			result += str[i];
		}
		else
		{
			auto value = dictionary.end();
			size_t len = 0;

			for(size_t j = i + 1; j < str.length(); ++j, ++len)
			{
				if(str[j] == L'%')
				{
					if(len > 0)
					{
						value = dictionary.find(std::wstring(str.data() + i + 1, len));
					}
					break;
				}
			}

			if(value != dictionary.end())
			{
				result.append(value->second);
				i += len + 1;
			}
			else
			{
				result.append(str.data() + i, len + 1);
				i += len;
			}
		}
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring Helpers::GetComputerName(void)
{
	std::wstring strComputerName;

	wchar_t szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD   dwComputerNameLen = ARRAYSIZE(szComputerName);
	if(::GetComputerName(szComputerName, &dwComputerNameLen))
		strComputerName = szComputerName;

	return strComputerName;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetMonitorRect(HWND hWnd, CRect& rectMonitor)
{
	HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	return GetMonitorRect(hMonitor, true, rectMonitor);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetDesktopRect(HWND hWnd, CRect& rectDesktop)
{
	HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	return GetMonitorRect(hMonitor, false, rectDesktop);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetDesktopRect(const CPoint& point, CRect& rectDesktop)
{
	HMONITOR hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
	return GetMonitorRect(hMonitor, false, rectDesktop);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::GetMonitorRect(HMONITOR hMonitor, bool bIgnoreTaskbar, CRect& rectDesktop)
{
  ::ZeroMemory(&rectDesktop, sizeof(CRect));

  MONITORINFO		mi;

  ::ZeroMemory(&mi, sizeof(MONITORINFO));
  mi.cbSize = sizeof(MONITORINFO);

  if( ::GetMonitorInfo(hMonitor, &mi) )
  {
    rectDesktop = bIgnoreTaskbar? mi.rcMonitor : mi.rcWork;
    return true;
  }
  else
  {
    return false;
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
      int index = 0;

      // check strIcon ends with ,<integer>
      bool ok = false;

      size_t pos = strIcon.find_last_of(L',');
      if( pos != wstring::npos )
      {
        bool negative = false;
        size_t i = pos + 1;
        if( i < strIcon.length() && strIcon.at(i) == L'-' )
        {
          i ++;
          negative = true;
        }
        for(; i < strIcon.length(); ++i)
        {
          if( strIcon.at(i) >= L'0' && strIcon.at(i) <= L'9' )
          {
            ok = true;
            index = index * 10 + (strIcon.at(i) - L'0');
          }
          else
          {
            ok = false;
            break;
          }
        }
        if( negative )
          index = -index;
      }

      wstring strIconPath = ok ? strIcon.substr(0, pos) : strIcon;

      HICON hIcon = nullptr;

      if ( bBigIcon )
      {
        ::ExtractIconEx(
          Helpers::ExpandEnvironmentStrings(strIconPath).c_str(),
          index,
          &hIcon,
          nullptr,
          1);
      }
      else
      {
        ::ExtractIconEx(
          Helpers::ExpandEnvironmentStrings(strIconPath).c_str(),
          index,
          nullptr,
          &hIcon,
          1);
      }

      if( hIcon )
        return hIcon;
    }
  }


  if ( bBigIcon )
  {
    return static_cast<HICON>(
      ::LoadImage(
        ::GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON,
		::GetSystemMetrics(SM_CXICON),
		::GetSystemMetrics(SM_CYICON),
        LR_DEFAULTCOLOR));
  }
  else
  {
    return static_cast<HICON>(
      ::LoadImage(
        ::GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON,
		::GetSystemMetrics(SM_CXSMICON),
		::GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR));
  }
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::IsElevated(void)
{
	std::unique_ptr<void, CloseHandleHelper> hToken(nullptr);

	{
		HANDLE _hToken = nullptr;

		if ( !::OpenProcessToken(
			::GetCurrentProcess(),
			TOKEN_QUERY,
			&_hToken) )
		{
			Win32Exception::ThrowFromLastError("OpenProcessToken");
		}

		hToken.reset(_hToken);
	}

	TOKEN_ELEVATION_TYPE tet;
	DWORD dwReturnLength = 0;

	if ( !::GetTokenInformation(
		hToken.get(),
		TokenElevationType,
		&tet,
		sizeof(TOKEN_ELEVATION_TYPE),
		&dwReturnLength ) )
	{
		Win32Exception::ThrowFromLastError("GetTokenInformation");
	}

	return tet == TokenElevationTypeFull;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool Helpers::CheckOSVersion(DWORD dwMinMajorVersion, DWORD dwMinMinorVersion)
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;
	BYTE op = VER_GREATER_EQUAL;

	// Initialize the OSVERSIONINFOEX structure.

	::ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = dwMinMajorVersion;
	osvi.dwMinorVersion = dwMinMinorVersion;

	// Initialize the condition mask.

	VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
	VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );

	// Perform the test.

	if( ::VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION |
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
		dwlConditionMask) )
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int Helpers::GetHighDefinitionResourceId(int nId)
{
	switch( ::GetSystemMetrics(SM_CYSMICON) )
	{
	case 16: return nId;
	case 20: return nId + 10;
	case 24: return nId + 20;
	default: return nId + 30;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::string Helpers::ToUtf8(const std::wstring& text)
{
	std::string result;
	int rc = ::WideCharToMultiByte(
		CP_UTF8,
		0,
		text.c_str(), static_cast<int>(text.length()),
		nullptr, 0,
		nullptr, nullptr);

	if(rc > 0)
	{
		result.resize(rc);
		::WideCharToMultiByte(
			CP_UTF8,
			0,
			text.c_str(), static_cast<int>(text.length()),
			&result[0], rc,
			nullptr, nullptr);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void Helpers::WriteLine(HANDLE hFile, const std::wstring& text)
{
	std::string utf8 = Helpers::ToUtf8(text);

	if(!::WriteFile(
		hFile,
		utf8.data(),
		static_cast<DWORD>(utf8.size()),
		NULL,
		NULL))
		Win32Exception::ThrowFromLastError("WriteFile");

	if(!::WriteFile(
		hFile,
		"\r\n",
		2,
		NULL,
		NULL))
		Win32Exception::ThrowFromLastError("WriteFile");
}

//////////////////////////////////////////////////////////////////////////////


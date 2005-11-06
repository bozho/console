// stdafx.cpp : source file that includes just the standard includes
//	Console.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#if (_ATL_VER < 0x0700)
#include <atlimpl.cpp>
#endif //(_ATL_VER < 0x0700)

//////////////////////////////////////////////////////////////////////////////
// trace function and TRACE macro

#ifdef _DEBUG
#include <stdio.h>

void Trace(const wchar_t* pszFormat, ...)
{
	wchar_t szOutput[1024];
	va_list	vaList;

	va_start(vaList, pszFormat);
	vswprintf(szOutput, pszFormat, vaList);
	::OutputDebugString(szOutput);
}

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////

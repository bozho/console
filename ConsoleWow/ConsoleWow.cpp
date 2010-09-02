// ConsoleWow.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int wmain(int argc, wchar_t* argv[])
{
	UINT_PTR fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	return (int)fnLoadLibrary;
}


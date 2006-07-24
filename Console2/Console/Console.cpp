// Console.cpp : main source file for Console.exe
//

#include "stdafx.h"

#include "resource.h"

#include "ConsoleView.h"
#include "aboutdlg.h"
#include "MainFrame.h"
#include "Console.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Global variables

CAppModule					_Module;

shared_ptr<SettingsHandler>	g_settingsHandler;
shared_ptr<ImageHandler>	g_imageHandler;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ParseCommandLine(LPTSTR lptstrCmdLine, wstring& strConfigFile, vector<wstring>& startupTabs, vector<wstring>& startupDirs, int& nMultiStartSleep, wstring& strDbgCmdLine)
{
	typedef tokenizer<char_separator<wchar_t>, wstring::const_iterator, wstring > tokenizer;

	wstring						strCmdLine(lptstrCmdLine);
	char_separator<wchar_t>		sep(L" \t");
	tokenizer					tokens(strCmdLine, sep);
	tokenizer::const_iterator	it = tokens.begin();

	for (; it != tokens.end(); ++it)
	{
		if (*it == wstring(L"-c"))
		{
			// custom config file
			++it;
			if (it == tokens.end()) break;
			strConfigFile = *it;
		}
		else if (*it == wstring(L"-t"))
		{
			// startup tab name
			++it;
			if (it == tokens.end()) break;
			startupTabs.push_back(*it);
		}
		else if (*it == wstring(L"-d"))
		{
			// startup dir
			++it;
			if (it == tokens.end()) break;
			startupDirs.push_back(*it);
		}
		else if (*it == wstring(L"-ts"))
		{
			// startup tab sleep for multiple tabs
			++it;
			if (it == tokens.end()) break;
			nMultiStartSleep = _wtoi(it->c_str());
			if (nMultiStartSleep < 0) nMultiStartSleep = 500;
		}
		// TODO: not working yet, need to investigate
/*
		else if (*it == wstring(L"-dbg"))
		{
			// console window replacement option (see Tip 1 in the help file)
			++it;
			if (it == tokens.end()) break;
			strDbgCmdLine = *it;
		}
*/
	}

	// make sure that startupDirs is at least as big as startupTabs
	if (startupDirs.size() < startupTabs.size()) startupDirs.resize(startupTabs.size());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	wstring			strConfigFile(L"");
	vector<wstring>	startupTabs;
	vector<wstring>	startupDirs;
	int				nMultiStartSleep = 0;
	wstring			strDbgCmdLine(L"");

	ParseCommandLine(lpstrCmdLine, strConfigFile, startupTabs, startupDirs, nMultiStartSleep, strDbgCmdLine);

	if (strConfigFile.length() == 0)
	{
		strConfigFile = Helpers::GetModulePath(NULL) + wstring(L"console.xml");
	}

	if (!g_settingsHandler->LoadSettings(strConfigFile))
	{
		//TODO: error handling
		return -1;
	}

	// create main window
	MainFrame wndMain(startupTabs, startupDirs, nMultiStartSleep, strDbgCmdLine);

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	g_settingsHandler.reset(new SettingsHandler());
	g_imageHandler.reset(new ImageHandler());

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	g_settingsHandler.reset();

	::CoUninitialize();

	return nRet;
}

//////////////////////////////////////////////////////////////////////////////

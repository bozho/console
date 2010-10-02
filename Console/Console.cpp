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
// Invisible parent window class for the main window if no taskbar should be shown

class NoTaskbarParent
	: public CWindowImpl<NoTaskbarParent, CWindow, CWinTraits<WS_POPUP, WS_EX_TOOLWINDOW> >
{
	public:
		DECLARE_WND_CLASS(L"NoTaskbarParent")

		NoTaskbarParent() {}
		~NoTaskbarParent() {}

		BEGIN_MSG_MAP(NoTaskbarParent)
		END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ParseCommandLine
(
	LPTSTR lptstrCmdLine, 
	wstring& strConfigFile, 
	wstring& strWindowTitle, 
	vector<wstring>& startupTabs, 
	vector<wstring>& startupDirs, 
	vector<wstring>& startupCmds, 
	int& nMultiStartSleep, 
	wstring& strDbgCmdLine
)
{
	int						argc = 0;
	shared_array<LPWSTR>	argv(::CommandLineToArgvW(lptstrCmdLine, &argc), ::GlobalFree);

	if (argc < 1) return;

	for (int i = 0; i < argc; ++i)
	{
		if (wstring(argv[i]) == wstring(L"-c"))
		{
			// custom config file
			++i;
			if (i == argc) break;
			strConfigFile = argv[i];
		}
		else if (wstring(argv[i]) == wstring(L"-w"))
		{
			// startup tab name
			++i;
			if (i == argc) break;
			strWindowTitle = argv[i];
		}
		else if (wstring(argv[i]) == wstring(L"-t"))
		{
			// startup tab name
			++i;
			if (i == argc) break;
			startupTabs.push_back(argv[i]);
		}
		else if (wstring(argv[i]) == wstring(L"-d"))
		{
			// startup dir
			++i;
			if (i == argc) break;
			startupDirs.push_back(argv[i]);
		}
		else if (wstring(argv[i]) == wstring(L"-r"))
		{
			// startup cmd
			++i;
			if (i == argc) break;
			startupCmds.push_back(argv[i]);
		}
		else if (wstring(argv[i]) == wstring(L"-ts"))
		{
			// startup tab sleep for multiple tabs
			++i;
			if (i == argc) break;
			nMultiStartSleep = _wtoi(argv[i]);
			if (nMultiStartSleep < 0) nMultiStartSleep = 500;
		}
		// TODO: not working yet, need to investigate
/*
		else if (wstring(argv[i]) == wstring(L"-dbg"))
		{
			// console window replacement option (see Tip 1 in the help file)
			++i;
			if (i == argc) break;
			strDbgCmdLine = argv[i];
		}
*/
	}

	// make sure that startupDirs and startupCmds are at least as big as startupTabs
	if (startupDirs.size() < startupTabs.size()) startupDirs.resize(startupTabs.size());
	if (startupCmds.size() < startupTabs.size()) startupCmds.resize(startupTabs.size());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	wstring			strConfigFile(L"");
	wstring			strWindowTitle(L"");
	vector<wstring>	startupTabs;
	vector<wstring>	startupDirs;
	vector<wstring>	startupCmds;
	int				nMultiStartSleep = 0;
	wstring			strDbgCmdLine(L"");

	ParseCommandLine(
		lpstrCmdLine, 
		strConfigFile, 
		strWindowTitle, 
		startupTabs, 
		startupDirs, 
		startupCmds, 
		nMultiStartSleep, 
		strDbgCmdLine);

	if (strConfigFile.length() == 0)
	{
		strConfigFile = wstring(L"console.xml");
//		strConfigFile = Helpers::GetModulePath(NULL) + wstring(L"console.xml");
//		strConfigFile = wstring(::_wgetenv(L"APPDATA")) + wstring(L"\\Console\\console.xml");
	}

	if (!g_settingsHandler->LoadSettings(Helpers::ExpandEnvironmentStrings(strConfigFile)))
	{
		//TODO: error handling
		return 1;
	}

	// create main window
	NoTaskbarParent noTaskbarParent;
	MainFrame wndMain(strWindowTitle, startupTabs, startupDirs, startupCmds, nMultiStartSleep, strDbgCmdLine);

	if (!g_settingsHandler->GetAppearanceSettings().stylesSettings.bTaskbarButton)
	{
		noTaskbarParent.Create(NULL);
	}

	if(wndMain.CreateEx(noTaskbarParent.m_hWnd) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 1;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	if (noTaskbarParent.m_hWnd != NULL) noTaskbarParent.DestroyWindow();

	_Module.RemoveMessageLoop();

	return nRet;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

#define FALLBACK_LANGUAGE (wchar_t *)-1
// created from http://www.loc.gov/standards/iso639-2/php/code_list.php
struct {
	LANGID language;
	wchar_t *iso639;
} languages[] = {
	{ LANG_AFRIKAANS, L"af" }, 
	{ LANG_ALBANIAN, L"sq" }, 
	{ LANG_ARABIC, L"ar" }, 
	{ LANG_ARMENIAN, L"hy" }, 
	{ LANG_ASSAMESE, L"as" }, 
	{ LANG_AZERI, L"az" }, 
	{ LANG_BASQUE, L"eu" }, 
	{ LANG_BELARUSIAN, L"be" }, 
	{ LANG_BENGALI, L"bn" }, 
	{ LANG_BULGARIAN, L"bg" }, 
	{ LANG_CATALAN, L"ca" }, 
	{ LANG_CHINESE, L"zh" }, 
	{ LANG_CROATIAN, L"hr" }, 
	{ LANG_CZECH, L"cs" }, 
	{ LANG_DANISH, L"da" }, 
	{ LANG_DIVEHI, L"dv" }, 
	{ LANG_DUTCH, L"nl" }, 
	{ LANG_ENGLISH, FALLBACK_LANGUAGE }, 
	{ LANG_ESTONIAN, L"et" }, 
	{ LANG_FAEROESE, L"fo" }, 
	{ LANG_FARSI, L"fa" }, 
	{ LANG_FINNISH, L"fi" }, 
	{ LANG_FRENCH, L"fr" }, 
	{ LANG_GALICIAN, L"gl" }, 
	{ LANG_GEORGIAN, L"ka" }, 
	{ LANG_GERMAN, L"de" }, 
	{ LANG_GREEK, L"el" }, 
	{ LANG_GUJARATI, L"gu" }, 
	{ LANG_HEBREW, L"he" }, 
	{ LANG_HINDI, L"hi" }, 
	{ LANG_HUNGARIAN, L"hu" }, 
	{ LANG_ICELANDIC, L"is" }, 
	{ LANG_INDONESIAN, L"id" }, 
	{ LANG_ITALIAN, L"it" }, 
	{ LANG_JAPANESE, L"jp" }, 
	{ LANG_KANNADA, L"kn" }, 
	{ LANG_KASHMIRI, L"ks" }, 
	{ LANG_KAZAK, L"kk" }, 
	{ LANG_KONKANI, L"kok" }, 
	{ LANG_KOREAN, L"ko" }, 
	{ LANG_KYRGYZ, L"ky" }, 
	{ LANG_LATVIAN, L"lv" }, 
	{ LANG_LITHUANIAN, L"lt" }, 
	{ LANG_MACEDONIAN, L"mk" }, 
	{ LANG_MALAY, L"ms" }, 
	{ LANG_MALAYALAM, L"ml" }, 
	{ LANG_MANIPURI, L"mni" }, 
	{ LANG_MARATHI, L"mr" }, 
	{ LANG_MONGOLIAN, L"mn" }, 
	{ LANG_NEPALI, L"ne" }, 
	{ LANG_NORWEGIAN, L"nn" }, 
	{ LANG_ORIYA, L"or" }, 
	{ LANG_POLISH, L"pl" }, 
	{ LANG_PORTUGUESE, L"pt" }, 
	{ LANG_PUNJABI, L"pa" }, 
	{ LANG_ROMANIAN, L"ro" }, 
	{ LANG_RUSSIAN, L"ru" }, 
	{ LANG_SANSKRIT, L"sa" }, 
	{ LANG_SERBIAN, L"sr" }, 
	{ LANG_SINDHI, L"sd" }, 
	{ LANG_SLOVAK, L"sk" }, 
	{ LANG_SLOVENIAN, L"sl" }, 
	{ LANG_SPANISH, L"es" }, 
	{ LANG_SWAHILI, L"sw" }, 
	{ LANG_SWEDISH, L"sv" }, 
	{ LANG_SYRIAC, L"syr" }, 
	{ LANG_TAMIL, L"ta" }, 
	{ LANG_TATAR, L"tt" }, 
	{ LANG_TELUGU, L"te" }, 
	{ LANG_THAI, L"th" }, 
	{ LANG_TURKISH, L"tr" }, 
	{ LANG_UKRAINIAN, L"uk" }, 
	{ LANG_URDU, L"ur" }, 
	{ LANG_UZBEK, L"uz" }, 
	{ LANG_VIETNAMESE, L"vi" }
};

HMODULE FindLocalizedResources(const LANGID language) 
{
	for (int i = 0; i < _countof(languages); ++i) {
		if (language == languages[i].language) {
			if (FALLBACK_LANGUAGE == languages[i].iso639)
				return (HMODULE)-1;

			wstring dll (L"console_");
			dll += languages[i].iso639;
			return ::LoadLibrary(dll.c_str());
		}
	}

	return 0;
}

void LoadLocalizedResources()
{
	wstring strLibrary (L"console_");
	HMODULE hResources = NULL;
	size_t sizeLang = 0;
	shared_array<wchar_t> szLangEnv(new wchar_t[32768]);

	// if we can securely get LANG, and its size greater than 2 chars (e.g. "en\0")
	if (! _wgetenv_s(&sizeLang, szLangEnv.get(), 32768, L"LANG") && sizeLang > 2) {
		if (0 == _wcsnicmp(szLangEnv.get(), L"en", 2))
			return; // default language
		// try to use the whole variable
		hResources = ::LoadLibrary(wstring (strLibrary + szLangEnv.get()).c_str());
		// if not supported, try to use just the language
		if (NULL == hResources)
			hResources = ::LoadLibrary(wstring(strLibrary + wstring (szLangEnv.get()).substr(0, 2)).c_str());
	}

	if (NULL == hResources) {
		hResources = FindLocalizedResources(PRIMARYLANGID(::GetUserDefaultUILanguage()));
	}
	if (NULL == hResources) {
		hResources = FindLocalizedResources(PRIMARYLANGID(::GetSystemDefaultUILanguage()));
	}

	if (NULL != hResources && (HMODULE)-1 != hResources)
		_Module.SetResourceInstance(hResources);
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

	LoadLocalizedResources();

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	g_settingsHandler.reset();

	::CoUninitialize();

	return nRet;
}

//////////////////////////////////////////////////////////////////////////////

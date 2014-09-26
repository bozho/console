// Console.cpp : main source file for Console.exe
//

#include "stdafx.h"

#include "resource.h"

#include "ConsoleView.h"
#include "aboutdlg.h"
#include "MainFrame.h"
#include "Console.h"
#include "ConsoleException.h"
#include "WallPaper.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Global variables

CAppModule					_Module;

std::shared_ptr<SettingsHandler>	g_settingsHandler;
std::shared_ptr<ImageHandler>	g_imageHandler;
_t_TranslateMessageEx TranslateMessageEx;

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
	bool& bReuse,
	wstring& strSyncName
)
{
	int argc = 0;
	std::unique_ptr<LPWSTR[], LocalFreeHelper> argv(::CommandLineToArgvW(lptstrCmdLine, &argc));

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
		else if (wstring(argv[i]) == wstring(L"-reuse"))
		{
			bReuse = true;
		}
		else if (wstring(argv[i]) == wstring(L"-a"))
		{
			++i;
			if (i == argc) break;
			strSyncName = argv[i];
		}
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

static bool HandleReuse(LPCTSTR lpstrCmdLine)
{
	SharedMemory<HWND> sharedInstance;
  try
  {
    sharedInstance.Open(L"ConsoleZ", syncObjNone);
  }
  catch(Win32Exception& ex)
  {
    if(ex.GetErrorCode() == ERROR_FILE_NOT_FOUND)
      return false;
    throw;
  }
	if (0 != sharedInstance.Get())
	{
		::SetForegroundWindow(*sharedInstance);

		std::wstring commandLine = lpstrCmdLine;
		commandLine += L" -cwd ";
		commandLine += Helpers::EscapeCommandLineArg(Helpers::GetCurrentDirectory());

		COPYDATASTRUCT cds = {0};
		cds.dwData = 0;
		cds.lpData = (LPVOID)commandLine.c_str();
		cds.cbData = static_cast<DWORD>((commandLine.length() + 1) * sizeof(wchar_t));

		::SendMessage(*sharedInstance, WM_COPYDATA, 0, (LPARAM)&cds);

		return true;
	}

	return false;
}

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
  try
  {
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    wstring strConfigFile(L"");
    bool    bReuse = false;
    wstring strSyncName;

    ParseCommandLine(
      lpstrCmdLine,
      strConfigFile,
      bReuse,
      strSyncName);

    if (strConfigFile.length() == 0)
    {
      strConfigFile = wstring(L"console.xml");
    }

    if (!g_settingsHandler->LoadSettings(Helpers::ExpandEnvironmentStrings(strConfigFile)))
    {
      throw std::exception("enable to load settings!");
    }

		if (!bReuse)
		{
			bReuse = !g_settingsHandler->GetBehaviorSettings2().instanceSettings.bAllowMultipleInstances;
		}

		if (!strSyncName.empty())
		{
			wstring strWindowTitle;
			vector<wstring> startupTabs;
			vector<wstring> startupDirs;
			vector<wstring> startupCmds;
			vector<DWORD>   basePriorities;
			int nMultiStartSleep = 0;
			wstring strWorkingDir;

			MainFrame::ParseCommandLine
			(
				lpstrCmdLine,
				strWindowTitle,
				startupTabs,
				startupDirs,
				startupCmds,
				basePriorities,
				nMultiStartSleep,
				strWorkingDir
			);

			TabSettings& tabSettings = g_settingsHandler->GetTabSettings();

			// find tab with corresponding name...
			for (auto tabData = tabSettings.tabDataVector.begin(); tabData != tabSettings.tabDataVector.end(); ++tabData)
			{
				if (tabData->get()->strTitle == startupTabs[0])
				{
					wstring strInitialDir(g_settingsHandler->GetConsoleSettings().strInitialDir);

					if (startupDirs.size() > 0 && startupDirs[0].length() > 0)
					{
						strInitialDir = startupDirs[0];
					}
					else if (tabData->get()->strInitialDir.length() > 0)
					{
						strInitialDir = tabData->get()->strInitialDir;
					}

					wstring	strShell(g_settingsHandler->GetConsoleSettings().strShell);

					if (tabData->get()->strShell.length() > 0)
					{
						strShell	= tabData->get()->strShell;
					}

					wstring strInitialCmd = strWorkingDir;

					if (startupCmds.size() > 0 && startupCmds[0].length() > 0)
					{
						strInitialCmd = startupCmds[0];
					}

					DWORD dwBasePriority = ULONG_MAX;

					if (basePriorities.size() > 0)
					{
						dwBasePriority = basePriorities[0];
					}

					if (dwBasePriority == ULONG_MAX)
						dwBasePriority = tabData->get()->dwBasePriority;

					try
					{
						ConsoleHandler ConsoleHandler;
						ConsoleHandler.StartShellProcessAsAdministrator
						(
							strSyncName,
							strShell,
							strInitialDir,
							strInitialCmd,
							dwBasePriority,
							tabData->get()->environmentVariables
						);
					}
					catch (const ConsoleException& ex)
					{
						MessageBox(NULL, ex.GetMessage().c_str(), L"error", MB_ICONERROR|MB_OK);
						return 1;
					}

					break;
				}
			}

      return 0;
    }

    if (bReuse && HandleReuse(lpstrCmdLine))
      return 0;

    // create main window
    NoTaskbarParent noTaskbarParent;
    MainFrame wndMain(lpstrCmdLine);

    if (!g_settingsHandler->GetAppearanceSettings().stylesSettings.bTaskbarButton)
    {
      noTaskbarParent.Create(NULL);
    }

    if(wndMain.CreateEx(noTaskbarParent.m_hWnd) == NULL)
    {
      ATLTRACE(_T("Main window creation failed!\n"));
      return 1;
    }

#ifdef _USE_AERO
    // restore the drop files message in elevated console
    ::ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
    ::ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
    ::ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
#endif

    wndMain.ShowWindow(nCmdShow);

    SharedMemory<HWND> sharedInstance;
    if (bReuse)
    {
      sharedInstance.Create(L"ConsoleZ", 1, syncObjNone, _T(""));
      sharedInstance = wndMain.m_hWnd;
    }

    WallPaperThread wallPaperThread(wndMain);

    if (Helpers::CheckOSVersion(6, 1))
    {
      // Win7 or more, we use the wallpaper slideshow monitoring
      wallPaperThread.Start();
    }

		TranslateMessageEx = (_t_TranslateMessageEx)::GetProcAddress(::GetModuleHandle(L"user32.dll"), "TranslateMessageEx");
		if( !TranslateMessageEx )
			Win32Exception::ThrowFromLastError("TranslateMessageEx");

    int nRet = theLoop.Run();

    if (noTaskbarParent.m_hWnd != NULL) noTaskbarParent.DestroyWindow();

    _Module.RemoveMessageLoop();

    return nRet;
  }
  catch(std::exception& e)
  {
    ::MessageBoxA(0, e.what(), "exception", MB_OK);
    return 1;
  }
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

#ifdef _USE_AERO
  // init GDI+
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif

	g_settingsHandler.reset(new SettingsHandler());
	g_imageHandler.reset(new ImageHandler());

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

#ifdef _USE_AERO
  // shutdown GDI+;
  Gdiplus::GdiplusShutdown(gdiplusToken);
#endif

	_Module.Term();
	g_settingsHandler.reset();

	::CoUninitialize();

	return nRet;
}

//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "Console.h"
#include "TabView.h"
#include "ConsoleView.h"
#include "ConsoleException.h"
#include "DlgRenameTab.h"
#include "DlgSettingsMain.h"
#include "MainFrame.h"
#include "JumpList.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
void MainFrame::ParseCommandLine
(
	LPCTSTR lptstrCmdLine,
	wstring& strWindowTitle,
	vector<wstring>& startupTabs,
	vector<wstring>& startupDirs,
	vector<wstring>& startupCmds,
	int& nMultiStartSleep,
	std::wstring& strWorkingDir
)
{
	int argc = 0;
	std::unique_ptr<LPWSTR[], LocalFreeHelper> argv(::CommandLineToArgvW(lptstrCmdLine, &argc));

	if (argc < 1) return;

	for (int i = 0; i < argc; ++i)
	{
		if (wstring(argv[i]) == wstring(L"-w"))
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
		else if (wstring(argv[i]) == wstring(L"-cwd"))
		{
			++i;
			if (i == argc) break;
			strWorkingDir = argv[i];
		}
	}

	// make sure that startupDirs and startupCmds are at least as big as startupTabs
	if (startupDirs.size() < startupTabs.size()) startupDirs.resize(startupTabs.size());
	if (startupCmds.size() < startupTabs.size()) startupCmds.resize(startupTabs.size());
}

MainFrame::MainFrame
(
	LPCTSTR lpstrCmdLine
)
: m_bOnCreateDone(false)
, m_startupTabs(vector<wstring>(0))
, m_startupDirs(vector<wstring>(0))
, m_startupCmds(vector<wstring>(0))
, m_nMultiStartSleep(0)
, m_strWorkingDir(L"")
, m_activeTabView()
, m_bMenuVisible     (true)
, m_bMenuChecked     (true)
, m_bToolbarVisible  (true)
, m_bStatusBarVisible(true)
, m_bTabsVisible     (true)
, m_bFullScreen      (false)
, m_dockPosition(dockNone)
, m_zOrder(zorderNormal)
, m_mousedragOffset(0, 0)
, m_tabs()
, m_tabsMutex(NULL, FALSE, NULL)
, m_dwWindowWidth(0)
, m_dwWindowHeight(0)
, m_dwResizeWindowEdge(WMSZ_BOTTOM)
, m_bRestoringWindow(false)
, m_rectRestoredWnd(0, 0, 0, 0)
, m_bAppActive(true)
, m_hwndPreviousForeground(NULL)
{
	m_Margins.cxLeftWidth    = 0;
	m_Margins.cxRightWidth   = 0;
	m_Margins.cyTopHeight    = 0;
	m_Margins.cyBottomHeight = 0;

	ParseCommandLine(
		lpstrCmdLine,
		m_strCmdLineWindowTitle,
		m_startupTabs,
		m_startupDirs,
		m_startupCmds,
		m_nMultiStartSleep,
		m_strWorkingDir);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (!m_acceleratorTable.IsNull() && m_acceleratorTable.TranslateAccelerator(m_hWnd, pMsg)) return TRUE;

	if(CTabbedFrameImpl<MainFrame>::PreTranslateMessage(pMsg)) return TRUE;

	if (!m_activeTabView) return FALSE;

	return m_activeTabView->PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::OnIdle()
{
  UpdateStatusBar();
  UIUpdateToolBar();

  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
LRESULT MainFrame::CreateInitialTabs
(
	vector<wstring> startupTabs,
	vector<wstring> startupCmds,
	vector<wstring> startupDirs,
	int nMultiStartSleep,
	std::wstring strWorkingDir
)
{
	bool bAtLeastOneStarted = false;

	// create initial console window(s)
	if (startupTabs.size() == 0)
	{
		wstring strStartupDir(L"");
		wstring strStartupCmd(L"");

		if (startupDirs.size() > 0) strStartupDir = startupDirs[0];
		if (startupCmds.size() > 0) strStartupCmd = startupCmds[0];

		bAtLeastOneStarted = CreateNewConsole(0, strStartupDir.empty()? strWorkingDir : strStartupDir, strStartupCmd);
	}
	else
	{
		TabSettings&	tabSettings = g_settingsHandler->GetTabSettings();

		for (size_t tabIndex = 0; tabIndex < startupTabs.size(); ++tabIndex)
		{
			// find tab with corresponding name...
			for (size_t i = 0; i < tabSettings.tabDataVector.size(); ++i)
			{
				if (tabSettings.tabDataVector[i]->strTitle == startupTabs[tabIndex])
				{
					// -ts Specifies sleep time between starting next tab if multiple -t's are specified.
					if (bAtLeastOneStarted && nMultiStartSleep > 0) ::Sleep(nMultiStartSleep);
					// found it, create
					if (CreateNewConsole(
						static_cast<DWORD>(i),
						startupDirs[tabIndex].empty()? strWorkingDir : startupDirs[tabIndex],
						startupCmds[tabIndex]))
					{
						bAtLeastOneStarted = true;
					}
					break;
				}
			}
		}
	}

	return bAtLeastOneStarted ? 0 : -1;
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ControlsSettings&	controlsSettings= g_settingsHandler->GetAppearanceSettings().controlsSettings;
	PositionSettings&	positionSettings= g_settingsHandler->GetAppearanceSettings().positionSettings;

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	m_tabsRPopupMenu.LoadMenu(IDR_TAB_POPUP_MENU);

	UpdateMenuHotKeys();

  m_contextMenu.CreatePopupMenu();
  CMenuHandle mainMenu = m_CmdBar.GetMenu();
  int count = mainMenu.GetMenuItemCount();
  for(int i = 0; i < count; ++i)
  {
    CString title;
    mainMenu.GetMenuString(i, title, MF_BYPOSITION);
    m_contextMenu.InsertMenu(i, MF_BYPOSITION, mainMenu.GetSubMenu(i), title);
  }

#ifdef _USE_AERO
  HWND hWndToolBar = CreateAeroToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
#else
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
#endif

	TBBUTTONINFO tbi;
	m_toolbar.Attach(hWndToolBar);
	m_toolbar.SendMessage(TB_SETEXTENDEDSTYLE, 0, static_cast<WPARAM>(TBSTYLE_EX_DRAWDDARROWS));

	tbi.dwMask	= TBIF_STYLE;
	tbi.cbSize	= sizeof(TBBUTTONINFO);
	
	m_toolbar.GetButtonInfo(ID_FILE_NEW_TAB, &tbi);
#ifdef _USE_AERO
  // TBSTYLE_DROPDOWN : the button separator is not drawed
	tbi.fsStyle |= BTNS_WHOLEDROPDOWN;
#else
	tbi.fsStyle |= TBSTYLE_DROPDOWN;
#endif
	m_toolbar.SetButtonInfo(ID_FILE_NEW_TAB, &tbi);

	m_toolbar.AddBitmap(1, IDR_FULLSCREEN1);
	m_nFullSreen1Bitmap = m_toolbar.GetImageList().GetImageCount() - 1;
	m_nFullSreen2Bitmap = m_toolbar.GetBitmap(ID_VIEW_FULLSCREEN);


#ifdef _USE_AERO
  CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE & ~RBS_BANDBORDERS);
#else
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
#endif
	AddSimpleReBarBand(hWndCmdBar, NULL, FALSE);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

#ifdef _USE_AERO
  // we remove the grippers
  CReBarCtrl rebar(m_hWndToolBar);
  rebar.LockBands(true);
#endif

	CreateStatusBar();

	// create font
	ConsoleView::RecreateFont(g_settingsHandler->GetAppearanceSettings().fontSettings.dwSize, false);

	// initialize tabs
	UpdateTabsMenu(m_CmdBar.GetMenu(), m_tabsMenu);
	SetReflectNotifications(true);

	DWORD dwTabStyles = CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_SCROLL | CTCS_CLOSEBUTTON | CTCS_HOTTRACK;
	if (controlsSettings.bTabsOnBottom) dwTabStyles |= CTCS_BOTTOM;
	if (g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView) dwTabStyles |= CTCS_CLOSELASTTAB;

	CreateTabWindow(m_hWnd, rcDefault, dwTabStyles);

	if (LRESULT created = CreateInitialTabs(m_startupTabs, m_startupCmds, m_startupDirs, m_nMultiStartSleep, m_strWorkingDir))
		return created;

	UIAddToolBar(hWndToolBar);
	UISetBlockAccelerators(true);

	SetWindowStyles();

	m_bMenuChecked = controlsSettings.bShowMenu;
	ShowMenu(m_bMenuChecked);
	ShowToolbar(controlsSettings.bShowToolbar);
	ShowStatusbar(controlsSettings.bShowStatusbar);

	bool bShowTabs = controlsSettings.bShowTabs;

	{
		MutexLock lock(m_tabsMutex);

		UpdateUI();

		if (m_tabs.size() <= 1 && controlsSettings.bHideSingleTab)
			bShowTabs = false;
	}

	ShowTabs(bShowTabs);

	DWORD dwFlags	= SWP_NOSIZE|SWP_NOZORDER;

	if( !positionSettings.bSavePosition &&
	    (positionSettings.nX == -1 || positionSettings.nY == -1) )
	{
		// do not reposition the window
		dwFlags |= SWP_NOMOVE;
	}
	else
	{
		// check we're not out of desktop bounds 
		int	nDesktopLeft	= ::GetSystemMetrics(SM_XVIRTUALSCREEN);
		int	nDesktopTop		= ::GetSystemMetrics(SM_YVIRTUALSCREEN);

		int	nDesktopRight	= nDesktopLeft + ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int	nDesktopBottom	= nDesktopTop + ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

		if ((positionSettings.nX < nDesktopLeft) || (positionSettings.nX > nDesktopRight)) positionSettings.nX = 50;
		if ((positionSettings.nY < nDesktopTop) || (positionSettings.nY > nDesktopBottom)) positionSettings.nY = 50;
	}

	SetTransparency();
	SetWindowPos(NULL, positionSettings.nX, positionSettings.nY, 0, 0, dwFlags);
	DockWindow(positionSettings.dockPosition);
	SetZOrder(positionSettings.zOrder);

	m_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
	if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon) SetTrayIcon(NIM_ADD);
	SetWindowIcons();

	CreateAcceleratorTable();
	RegisterGlobalHotkeys();

	AdjustWindowSize(ADJUSTSIZE_NONE);

	CRect rectWindow;
	GetWindowRect(&rectWindow);

	m_dwWindowWidth	= rectWindow.Width();
	m_dwWindowHeight= rectWindow.Height();

	TRACE(L"initial dims: %ix%i\n", m_dwWindowWidth, m_dwWindowHeight);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// this is the only way I know that other message handlers can be aware 
	// if they're being called after OnCreate has finished
	m_bOnCreateDone = true;

	if( positionSettings.bSaveSize ||
	    (positionSettings.nW != -1 && positionSettings.nH != -1) )
	{
		// resize the window
		SetWindowPos(NULL, 0, 0, positionSettings.nW, positionSettings.nH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
		AdjustWindowSize(ADJUSTSIZE_WINDOW);
	}

	if( g_settingsHandler->GetAppearanceSettings().fullScreenSettings.bStartInFullScreen )
		ShowFullScreen(true);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if( g_settingsHandler->GetBehaviorSettings().closeSettings.bConfirmClosingMultipleViews )
  {
    MutexLock lock(m_tabsMutex);

    if( m_tabs.size() > 1 )
    {
      if( ::MessageBox(m_hWnd, L"Are you sure you want close all tabs ?", L"Close", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL )
        return 0;
    }
    else if( m_tabs.size() == 1 && m_tabs.begin()->second->GetViewsCount() > 1 )
    {
        if( ::MessageBox(m_hWnd, L"Are you sure you want close all views ?", L"Close", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL )
          return 0;
    }
  }

	// save settings on exit
	bool				bSaveSettings		= false;
	ConsoleSettings&	consoleSettings		= g_settingsHandler->GetConsoleSettings();
	PositionSettings&	positionSettings	= g_settingsHandler->GetAppearanceSettings().positionSettings;

	if (consoleSettings.bSaveSize)
	{
#if 0
		consoleSettings.dwRows		= m_dwRows;
		consoleSettings.dwColumns	= m_dwColumns;
#endif
		bSaveSettings = true;
	}

	if (positionSettings.bSavePosition || positionSettings.bSaveSize)
	{
		// we store position and size of the non fullscreen window
		if( !m_bFullScreen )
			GetWindowRect(&m_rectWndNotFS);

		if (positionSettings.bSavePosition)
		{
			positionSettings.nX = m_rectWndNotFS.left;
			positionSettings.nY = m_rectWndNotFS.top;
		}

		if (positionSettings.bSaveSize)
		{
			positionSettings.nW = m_rectWndNotFS.Width();
			positionSettings.nH = m_rectWndNotFS.Height();
		}

		bSaveSettings = true;
	}


	if (bSaveSettings) g_settingsHandler->SaveSettings();

	// destroy all views
	MutexLock viewMapLock(m_tabsMutex);
	for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
	{
		RemoveTab(it->second->m_hWnd);
		if (m_activeTabView == it->second) m_activeTabView.reset();
		it->second->DestroyWindow();
	}

	if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon) SetTrayIcon(NIM_DELETE);

	UnregisterGlobalHotkeys();

	DestroyWindow();
	PostQuitMessage(0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnActivateApp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	m_bAppActive = static_cast<BOOL>(wParam)? true : false;

	if (!m_activeTabView) return 0;

	this->ActivateApp();

	bHandled = FALSE;

	return 0;
}

void MainFrame::ActivateApp(void)
{
  m_activeTabView->SetAppActiveStatus(m_bAppActive);

  TransparencySettings& transparencySettings = g_settingsHandler->GetAppearanceSettings().transparencySettings;

  if ((transparencySettings.transType == transAlpha) && 
    ((transparencySettings.byActiveAlpha != 255) || (transparencySettings.byInactiveAlpha != 255)))
  {
    if (m_bAppActive)
    {
      ::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), transparencySettings.byActiveAlpha, LWA_ALPHA);
    }
    else
    {
      ::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), transparencySettings.byInactiveAlpha, LWA_ALPHA);
    }

  }

#ifdef _USE_AERO
  m_TabCtrl.SetAppActiveStatus(m_bAppActive);
  m_TabCtrl.RedrawWindow();
#endif

  if ((transparencySettings.transType == transGlass) && 
    (transparencySettings.byActiveAlpha != transparencySettings.byInactiveAlpha))
  {
    m_activeTabView->Repaint(true);
  }

}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowHideWindow(void)
{
  bool bQuake = g_settingsHandler->GetAppearanceSettings().stylesSettings.bQuake;
  bool bActivate = true;

  if( bQuake )
  {
		DWORD dwActivateFlags = AW_ACTIVATE | AW_SLIDE;
		DWORD dwHideFlags     = AW_HIDE     | AW_SLIDE;

    switch( m_dockPosition )
    {
    case dockNone:
      // effect disabled ...
      bQuake = false;
      break;
    case dockTL:
      dwActivateFlags |= AW_VER_POSITIVE;
      dwHideFlags     |= AW_VER_NEGATIVE;
      break;
    case dockTR:
      dwActivateFlags |= AW_VER_POSITIVE;
      dwHideFlags     |= AW_VER_NEGATIVE;
      break;
    case dockBL:
      dwActivateFlags |= AW_VER_NEGATIVE;
      dwHideFlags     |= AW_VER_POSITIVE;
      break;
    case dockBR:
      dwActivateFlags |= AW_VER_NEGATIVE;
      dwHideFlags     |= AW_VER_POSITIVE;
      break;
    }

		if(!m_bAppActive)
		{
			this->m_hwndPreviousForeground = ::GetForegroundWindow();
		}

		if(!this->IsWindowVisible())
		{
			::AnimateWindow(m_hWnd, 300, dwActivateFlags);
			this->RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME |RDW_INVALIDATE | RDW_ERASE);
		}
		else if(m_bAppActive)
		{
			::AnimateWindow(m_hWnd, 300, dwHideFlags);
			::SetForegroundWindow(this->m_hwndPreviousForeground);
			bActivate = false;
		}
	}
	else
	{
		if( this->IsIconic() )
		{
			this->ShowWindow(SW_RESTORE);
		}
		else if(m_bAppActive)
		{
			this->ShowWindow(SW_MINIMIZE);
			bActivate = false;
		}
		else
		{
			this->ShowWindow(SW_SHOW);
		}
	}

  if( bActivate )
  {
    PostMessage(WM_ACTIVATEAPP, TRUE, 0);

    POINT	cursorPos;
    CRect	windowRect;

    ::GetCursorPos(&cursorPos);
    GetWindowRect(&windowRect);

    if ((cursorPos.x < windowRect.left) || (cursorPos.x > windowRect.right)) cursorPos.x = windowRect.left + windowRect.Width()/2;
    if ((cursorPos.y < windowRect.top) || (cursorPos.y > windowRect.bottom)) cursorPos.y = windowRect.top + windowRect.Height()/2;

    ::SetCursorPos(cursorPos.x, cursorPos.y);
    ::SetForegroundWindow(m_hWnd);
  }
}

LRESULT MainFrame::OnHotKey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  switch (wParam)
  {
  case IDC_GLOBAL_ACTIVATE :
    {
      ShowHideWindow();
      break;
    }

  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSysKeydown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
/*
	if ((wParam == VK_SPACE) && (lParam & (0x1 << 29)))
	{
		// send the SC_KEYMENU directly to the main frame, because DefWindowProc does not handle the message correctly
		return SendMessage(WM_SYSCOMMAND, SC_KEYMENU, VK_SPACE);
	}
*/

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
  // OnSize needs to know this
  switch( GET_SC_WPARAM(wParam) )
  {
  case SC_RESTORE:
		if( this->IsZoomed() )
			m_bRestoringWindow = true;
    break;

  case SC_MAXIMIZE:
    GetWindowRect(&m_rectRestoredWnd);
    break;
  }

  bHandled = FALSE;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	// Start timer that will force a call to ResizeWindow (called from WM_EXITSIZEMOVE handler
	// when the Console window is resized using a mouse)
	// External utilities that might resize Console window usually don't send WM_EXITSIZEMOVE
	// message after resizing a window.
	SetTimer(TIMER_SIZING, TIMER_SIZING_INTERVAL);

	if (wParam == SIZE_MAXIMIZED)
	{
		PostMessage(WM_EXITSIZEMOVE, 1, 0);
	}
	else if (m_bRestoringWindow && (wParam == SIZE_RESTORED))
	{
		m_bRestoringWindow = false;
		PostMessage(WM_EXITSIZEMOVE, 1, 0);
	}

// 	CRect rectWindow;
// 	GetWindowRect(&rectWindow);
// 
// 	TRACE(L"OnSize dims: %ix%i\n", rectWindow.Width(), rectWindow.Height());


	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSizing(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_dwResizeWindowEdge = static_cast<DWORD>(wParam);

	if (!m_activeTabView)
		return 0;

	m_activeTabView->SetResizing(true);
#if 0
	CPoint pointSize = m_activeView->GetCellSize();
	RECT *rectNew = (RECT *)lParam;

	CRect rectWindow;
	GetWindowRect(&rectWindow);

	if (rectWindow.top != rectNew->top)
		rectNew->top += (rectWindow.top - rectNew->top) - (rectWindow.top - rectNew->top) / pointSize.y * pointSize.y;

	if (rectWindow.bottom != rectNew->bottom)
		rectNew->bottom += (rectWindow.bottom - rectNew->bottom) - (rectWindow.bottom - rectNew->bottom) / pointSize.y * pointSize.y;

	if (rectWindow.left != rectNew->left)
		rectNew->left += (rectWindow.left - rectNew->left) - (rectWindow.left - rectNew->left) / pointSize.x * pointSize.x;

	if (rectWindow.right != rectNew->right)
		rectNew->right += (rectWindow.right - rectNew->right) - (rectWindow.right - rectNew->right) / pointSize.x * pointSize.x;
#endif
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	WINDOWPOS*			pWinPos			= reinterpret_cast<WINDOWPOS*>(lParam);
	PositionSettings&	positionSettings= g_settingsHandler->GetAppearanceSettings().positionSettings;

	if (positionSettings.zOrder == zorderOnBottom) pWinPos->hwndInsertAfter = HWND_BOTTOM;

	if (m_bRestoringWindow)
	{
		SetWindowPos(
			NULL, 
			m_rectRestoredWnd.left, 
			m_rectRestoredWnd.top, 
			0, 
			0, 
			SWP_NOSIZE|SWP_NOZORDER|SWP_NOSENDCHANGING);

		return 0;
	}

	if (!(pWinPos->flags & SWP_NOMOVE))
	{
		// do nothing for minimized or maximized or fullscreen windows
		if (IsIconic() || IsZoomed() || m_bFullScreen) return 0;

		if (positionSettings.nSnapDistance >= 0)
		{
			m_dockPosition	= dockNone;

			CRect	rectMonitor;
			CRect	rectDesktop;
			CRect	rectWindow;
			CPoint	pointCursor;

			// we'll snap Console window to the desktop edges

			// WM_WINDOWPOSCHANGING will be called when locking a computer
			// GetCursorPos will fail in that case; in that case we return and prevent invalid window position after unlock
			if (!::GetCursorPos(&pointCursor)) return 0;
			GetWindowRect(&rectWindow);
			Helpers::GetDesktopRect(pointCursor, rectDesktop);
			Helpers::GetMonitorRect(m_hWnd, rectMonitor);

			if (!rectMonitor.PtInRect(pointCursor))
			{
				pWinPos->x = pointCursor.x;
				pWinPos->y = pointCursor.y;
			}

			int	nLR = -1;
			int	nTB = -1;

			// now, see if we're close to the edges
			if (pWinPos->x <= rectDesktop.left + positionSettings.nSnapDistance)
			{
				pWinPos->x = rectDesktop.left;
				nLR = 0;
			}

			if (pWinPos->x >= rectDesktop.right - rectWindow.Width() - positionSettings.nSnapDistance)
			{
				pWinPos->x = rectDesktop.right - rectWindow.Width();
				nLR = 1;
			}

			if (pWinPos->y <= rectDesktop.top + positionSettings.nSnapDistance)
			{
				pWinPos->y = rectDesktop.top;
				nTB = 0;
			}
			
			if (pWinPos->y >= rectDesktop.bottom - rectWindow.Height() - positionSettings.nSnapDistance)
			{
				pWinPos->y = rectDesktop.bottom - rectWindow.Height();
				nTB = 2;
			}

			if ((nLR != -1) && (nTB != -1))
			{
				m_dockPosition = static_cast<DockPosition>(nTB | nLR);
			}
		}


		if (m_activeTabView)
		{
			CRect rectClient;
			GetClientRect(&rectClient);

			m_activeTabView->MainframeMoving();
			// we need to invalidate client rect here for proper background 
			// repaint when using relative backgrounds
			InvalidateRect(&rectClient, FALSE);
		}

		return 0;
	}

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnMouseButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (::GetCapture() == m_hWnd)
	{
		::ReleaseCapture();
	}
	else
	{
		bHandled = FALSE;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint	point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	if (::GetCapture() == m_hWnd)
	{
		ClientToScreen(&point);

		SetWindowPos(
			NULL, 
			point.x - m_mousedragOffset.x, 
			point.y - m_mousedragOffset.y, 
			0, 
			0,
			SWP_NOSIZE|SWP_NOZORDER);

		RedrawWindow(NULL, NULL, RDW_UPDATENOW|RDW_ALLCHILDREN);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnExitSizeMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ResizeWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (wParam == TIMER_SIZING)
	{
		KillTimer(TIMER_SIZING);
		ResizeWindow();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (lParam == 0) return 0;

	wstring strArea(reinterpret_cast<wchar_t*>(lParam));

	// according to WM_SETTINGCHANGE doc:
	// to change environment, lParam should be "Environment"
	if (strArea == L"Environment")
	{
		ConsoleHandler::UpdateEnvironmentBlock();
	}
	else
	{
		// otherwise, we don't know what has changed
		// technically, we can skip reloading for "Policy" and "intl", but
		// hopefully they don't happen often, so reload everything
		g_imageHandler->ReloadDesktopImages();


		// can't use Invalidate because full repaint is in order
		m_activeTabView->Repaint(true);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnConsoleResized(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */)
{
	AdjustWindowSize(ADJUSTSIZE_NONE);
	UpdateStatusBar();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnConsoleClosed(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /* bHandled */)
{
	MutexLock lock(m_tabsMutex);
	for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
	{
		bool boolTabClosed;
		if( it->second->CloseView(reinterpret_cast<HWND>(wParam), boolTabClosed) )
		{
			if( boolTabClosed )
				CloseTab(it->second->m_hWnd);
			break;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnUpdateTitles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /* bHandled */)
{
	MutexLock viewMapLock(m_tabsMutex);
	HWND      hwndTabView = reinterpret_cast<HWND>(wParam);

	if( hwndTabView == NULL )
	{
		// update all tabs
		for(auto itView = m_tabs.begin(); itView != m_tabs.end(); ++itView)
			UpdateTabTitle(itView->second);
	}
	else
	{
		auto itView = m_tabs.find(hwndTabView);

		if (itView != m_tabs.end())
			UpdateTabTitle(itView->second);
	}

	return 0;
}

std::wstring MainFrame::FormatTitle(std::wstring strFormat, std::shared_ptr<TabView> tabView, std::shared_ptr<ConsoleView> consoleView)
{
/*
                  +-----+
         +--pop-->|     |
+-----+  |   )    |     |    %?(): u-U
|  S  |  +--------|     |<--------------+
|  T  |           |  R  |               |
|  A  |----push-->|  E  |     %     +-------+
|  R  |           |  A  |---------->|SPECIAL|
|  T  |        +--|  D  |           +-------+
+-----+       *|  |     |
   ^           +->|     |     ?     +---------+    u-U    +---------+     :     +---------+
   |              |     |---------->|QUESTION1|---------->|QUESTION2|---------->|QUESTION3|
   |              +-----+           +---------+           +---------+           +---------+
   |                 |                             (           |                   |   ^
   +-----------------)-------------!empty----------------------+                   |   |
   |                 |                                                    (        |   |
   +-----------------)--------------empty------------------------------------------+   |
                     |                                                    :            |
                     +-----------------------------------------------------------------+
*/

	enum
	{
		START,
		READ,
		SPECIAL,
		QUESTION1,
		QUESTION2,
		QUESTION3
	}
	automaton_state = START;

	struct layer
	{
		enum
		{
			NONE,
			DEFINED,
			UNDEFINED,
		}            condition_value,
		             condition_part;
		std::wstring str;

		layer():condition_value(NONE),condition_part(NONE),str(){}
	};

	WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;

	std::wstring strMainTitle  = m_strCmdLineWindowTitle.empty()? windowSettings.strTitle : m_strCmdLineWindowTitle;
	std::wstring strShellTitle = consoleView->GetConsoleCommand();
	int          nTabNumber    = m_TabCtrl.FindItem(*tabView) + 1;

	std::stack<std::shared_ptr<layer>> layers;

	std::wstring result(L"");
	size_t position = 1;

	for(auto i = strFormat.begin(); i != strFormat.end(); ++i, ++position)
	{
		switch( automaton_state )
		{
		case START:
			layers.push(std::shared_ptr<layer>(new layer));
			automaton_state = READ;

		case READ:
			switch( *i )
			{
			case L'%':
				automaton_state = SPECIAL;
				break;
			case L'?':
				automaton_state = QUESTION1;
				break;
			case L')':
				if( layers.size() > 1 )
				{
					std::shared_ptr<layer> top = layers.top();
					layers.pop();

					if( layers.top()->condition_part == layers.top()->condition_value )
						layers.top()->str += top->str;
				}
				else goto error;
				break;
			case L':':
				if( layers.top()->condition_value != layer::NONE && layers.top()->condition_part != layer::UNDEFINED )
					automaton_state = QUESTION3;
				else goto error;
				break;
			default:
				layers.top()->str += *i;
				break;
			}
			break;
		case SPECIAL:
			switch( *i )
			{
			case L'%':
			case L'?':
			case L'(':
			case L')':
			case L':':
				layers.top()->str += *i;
				break;
			case L'u': layers.top()->str += consoleView->GetUser(); break;
			case L'p': layers.top()->str += std::to_wstring(consoleView->GetConsoleHandler().GetConsolePid()); break;
			case L'n': layers.top()->str += std::to_wstring(nTabNumber); break;
			case L'i': layers.top()->str += L"index"; break;
			case L'm': layers.top()->str += strMainTitle; break;
			case L't': layers.top()->str += tabView->GetTitle(); break;
			case L's': layers.top()->str += strShellTitle; break;
			case L'A': layers.top()->str += consoleView->GetConsoleHandler().IsElevated()? L"y" : L""; break;
			case L'U': layers.top()->str += ( consoleView->IsRunningAsUser() )? L"y" : L""; break;
			case L'N': layers.top()->str += ( consoleView->IsRunningAsUserNetOnly() )? L"y" : L""; break;
			default:
				goto error;
			}
			automaton_state = READ;
			break;
		case QUESTION1:
			{
				bool defined = false;

				switch( *i )
				{
				case L'u': defined = consoleView->GetUser().GetLength() > 0; break;
				case L'm': defined = !strMainTitle.empty(); break;
				case L't': defined = tabView->GetTitle().GetLength() > 0; break;
				case L's': defined = !strShellTitle.empty(); break;
				case L'A': defined = consoleView->GetConsoleHandler().IsElevated(); break;
				case L'U': defined = consoleView->IsRunningAsUser(); break;
				case L'N': defined = consoleView->IsRunningAsUserNetOnly(); break;
					break;
				default:
					goto error;
				}

				layers.top()->condition_value = defined? layer::DEFINED : layer::UNDEFINED;
				automaton_state = QUESTION2;
			}
			break;
		case QUESTION2:
			switch( *i )
			{
			case L'(':
				layers.top()->condition_part = layer::DEFINED;
				automaton_state = START;
				break;
			case L':':
				automaton_state = QUESTION3;
				break;
			default:
				goto error;
			}
			break;
		case QUESTION3:
			switch( *i )
			{
			case L'(':
				layers.top()->condition_part = layer::UNDEFINED;
				automaton_state = START;
				break;
			default:
				goto error;
			}
			break;
		}
	}

	if( layers.size() > 1 ) goto error;
	if( layers.size() == 1 )
		result = layers.top()->str;

	return result;

error:
	result = L"syntax error at position ";
	result += std::to_wstring(position);

	return result;
}

void MainFrame::UpdateTabTitle(std::shared_ptr<TabView> tabView)
{
	std::shared_ptr<ConsoleView> consoleView = tabView->GetActiveConsole(_T(__FUNCTION__));
	if (!consoleView) return;

	WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;

	wstring strTabTitle = FormatTitle(windowSettings.strTabTitleFormat, tabView, consoleView);

	if (tabView == m_activeTabView)
	{
		m_strWindowTitle = windowSettings.bUseTabTitles? strTabTitle : FormatTitle(windowSettings.strMainTitleFormat, tabView, consoleView);

		SetWindowText(m_strWindowTitle.c_str());
		if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon)
			SetTrayIcon(NIM_MODIFY);
	}

	// we always set the tool tip text to the complete, untrimmed title
	UpdateTabToolTip(*tabView, strTabTitle.c_str());

	if
	(
		(windowSettings.dwTrimTabTitles > 0)
		&&
		(windowSettings.dwTrimTabTitles > windowSettings.dwTrimTabTitlesRight)
		&&
		(strTabTitle.length() > static_cast<int>(windowSettings.dwTrimTabTitles))
	)
	{
		strTabTitle = strTabTitle.substr(0, windowSettings.dwTrimTabTitles - windowSettings.dwTrimTabTitlesRight)
		            + L'\u2026'
		            + strTabTitle.substr(strTabTitle.length() - windowSettings.dwTrimTabTitlesRight);
	}

	UpdateTabText(*tabView, strTabTitle.c_str());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnShowPopupMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint	point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	MouseSettings::Command command = static_cast<MouseSettings::Command>(wParam);

	switch (command)
	{
	case MouseSettings::cmdMenu1:
		m_CmdBar.TrackPopupMenu(m_contextMenu, 0, point.x, point.y);
		break;

	case MouseSettings::cmdMenu2:
		m_CmdBar.TrackPopupMenu(m_tabsMenu, 0, point.x, point.y);
		break;

	case MouseSettings::cmdMenu3:
		{
			CMenu menu;
			UpdateOpenedTabsMenu(menu);
			m_CmdBar.TrackPopupMenu(menu, 0, point.x, point.y);
		}
		break;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnStartMouseDrag(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint	point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	CRect	windowRect;

	GetWindowRect(windowRect);

	m_mousedragOffset = point;
	m_mousedragOffset.x -= windowRect.left;
	m_mousedragOffset.y -= windowRect.top;

	SetCapture();
	return 0;
}


#ifdef _USE_AERO

LRESULT MainFrame::OnStartMouseDragExtendedFrameToClientArea(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  if( aero::IsAeroGlassActive() )
  {
    if( pnmh->code == NM_CLICK && pnmh->hwndFrom == m_TabCtrl.m_hWnd )
    {
      NMCTCITEM* pTabItem	= reinterpret_cast<NMCTCITEM*>(pnmh);
      if( pTabItem->iItem == -1 )
      {
        CRect	tabWindowRect;
        m_TabCtrl.GetWindowRect(tabWindowRect);

        CRect	windowRect;
        GetWindowRect(windowRect);

        m_mousedragOffset = pTabItem->pt;
        m_mousedragOffset.x += tabWindowRect.left;
        m_mousedragOffset.y += tabWindowRect.top;
        m_mousedragOffset.x -= windowRect.left;
        m_mousedragOffset.y -= windowRect.top;

        SetCapture();
      }
    }
    else if( pnmh->code == NM_LDOWN && pnmh->hwndFrom == m_toolbar.m_hWnd )
    {
      LPNMMOUSE pMouse = reinterpret_cast<LPNMMOUSE>(pnmh);

      if( pMouse->dwItemSpec == SIZE_MAX )
      {
        CRect	toolbarWindowRect;
        m_toolbar.GetWindowRect(toolbarWindowRect);

        CRect	windowRect;
        GetWindowRect(windowRect);

        m_mousedragOffset = pMouse->pt;
        m_mousedragOffset.x += toolbarWindowRect.left;
        m_mousedragOffset.y += toolbarWindowRect.top;
        m_mousedragOffset.x -= windowRect.left;
        m_mousedragOffset.y -= windowRect.top;

        SetCapture();
      }
    }
  }

  return 0;
}

LRESULT MainFrame::OnDBLClickExtendedFrameToClientArea(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  if( aero::IsAeroGlassActive() )
  {
    if( pnmh->hwndFrom == m_TabCtrl.m_hWnd )
    {
      NMCTCITEM* pTabItem	= reinterpret_cast<NMCTCITEM*>(pnmh);
      if( pTabItem->iItem == -1 )
      {
        // Telling the window to maximize itself might bypass some internal adjustments that the program makes
        // when it maximizes via a system menu command.
        // To emulate clicking on the maximize button, send it a SC_MAXIMIZE command.
        this->SendMessage(WM_SYSCOMMAND, this->IsZoomed()? SC_RESTORE : SC_MAXIMIZE, 0);
      }
    }

    // there is no left db click from toolbar
  }

  return 0;
}

#endif //_USE_AERO

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTrayNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	switch (lParam)
	{
		case WM_RBUTTONUP :
		{
			CPoint	posCursor;
			
			::GetCursorPos(&posCursor);
			// show popup menu
			::SetForegroundWindow(m_hWnd);

			m_CmdBar.TrackPopupMenu(m_contextMenu, 0, posCursor.x, posCursor.y);

			// we need this for the menu to close when clicking outside of it
			PostMessage(WM_NULL, 0, 0);

			return 0;
		}

		case WM_LBUTTONDOWN : 
		{
			ShowHideWindow();
			return 0;
		}

		case WM_LBUTTONDBLCLK :
		{
			ShowHideWindow();
			return 0;
		}

		default : return 0;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon) SetTrayIcon(NIM_ADD);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTabChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	NMCTC2ITEMS*				pTabItems	= reinterpret_cast<NMCTC2ITEMS*>(pnmh);

	AppearanceSettings&			appearanceSettings = g_settingsHandler->GetAppearanceSettings();

	CTabViewTabItem*			pTabItem1	= (pTabItems->iItem1 != 0xFFFFFFFF) ? m_TabCtrl.GetItem(pTabItems->iItem1) : NULL;
	CTabViewTabItem*			pTabItem2	= m_TabCtrl.GetItem(pTabItems->iItem2);

	MutexLock					viewMapLock(m_tabsMutex);
	TabViewMap::iterator	it;

	if (pTabItem1)
	{
		it = m_tabs.find(pTabItem1->GetTabView());
		if (it != m_tabs.end())
		{
			it->second->SetActive(false);
		}
	}

	if (pTabItem2)
	{
		it = m_tabs.find(pTabItem2->GetTabView());
		if (it != m_tabs.end())
		{
			m_activeTabView = it->second;
			it->second->SetActive(true);

			if (appearanceSettings.windowSettings.bUseTabIcon) SetWindowIcons();

			// clear the highlight in case it's on
			HighlightTab(m_activeTabView->m_hWnd, false);
		}
		else
		{
      m_activeTabView = std::shared_ptr<TabView>();
		}
	}

	if (appearanceSettings.stylesSettings.bTrayIcon) SetTrayIcon(NIM_MODIFY);

	UpdateUI();

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTabOrderChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
	// update tab titles
	this->PostMessage(
		UM_UPDATE_TITLES,
		0,
		0);

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTabClose(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */)
{
	NMCTC2ITEMS*		pTabItems	= reinterpret_cast<NMCTC2ITEMS*>(pnmh);
	CTabViewTabItem*	pTabItem	= (pTabItems->iItem1 != 0xFFFFFFFF) ? m_TabCtrl.GetItem(pTabItems->iItem1) : NULL;

	CloseTab(pTabItem);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTabMiddleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMCTCITEM*       pTabItems = reinterpret_cast<NMCTCITEM*>(pnmh);
	CTabViewTabItem* pTabItem  = (pTabItems->iItem != 0xFFFFFFFF) ? m_TabCtrl.GetItem(pTabItems->iItem) : NULL;

	if (pTabItem == NULL)
	{

		// I prefer choose my console with the good environment ...
		// CreateNewConsole(0);

		if (!m_tabsMenu.IsNull())
		{
			CPoint point(pTabItems->pt.x, pTabItems->pt.y);
			CPoint screenPoint(point);
			this->m_TabCtrl.ClientToScreen(&screenPoint);
			m_CmdBar.TrackPopupMenu(m_tabsMenu, 0, screenPoint.x, screenPoint.y);
		}
	}
	else
	{
		CloseTab(pTabItem);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTabRightClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMCTCITEM*       pTabItems = reinterpret_cast<NMCTCITEM*>(pnmh);
	CTabViewTabItem* pTabItem  = (pTabItems->iItem != 0xFFFFFFFF) ? m_TabCtrl.GetItem(pTabItems->iItem) : NULL;

	if (pTabItem)
	{
		// select the tab
		// this will update the menu UI
		m_TabCtrl.SetCurSel(pTabItems->iItem);

		CPoint point(pTabItems->pt.x, pTabItems->pt.y);
		CPoint screenPoint(point);
		this->m_TabCtrl.ClientToScreen(&screenPoint);
		m_CmdBar.TrackPopupMenu(m_tabsRPopupMenu.GetSubMenu(0), 0, screenPoint.x, screenPoint.y);
	}

	// tab is already selected so return 1
	return 1;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnRebarHeightChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
  TRACE(L"MainFrame::OnRebarHeightChanged\n");
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnToolbarDropDown(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
	CPoint	cursorPos;
	::GetCursorPos(&cursorPos);

	CRect	buttonRect;
	m_toolbar.GetItemRect(0, &buttonRect);
	m_toolbar.ClientToScreen(&buttonRect);

	m_CmdBar.TrackPopupMenu(m_tabsMenu, 0, buttonRect.left, buttonRect.bottom);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnFileNewTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == ID_FILE_NEW_TAB)
	{
		CreateNewConsole(0);
	}
	else
	{
		CreateNewConsole(wID-ID_NEW_TAB_1);
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSwitchTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nNewSel = wID-ID_SWITCH_TAB_1;

	if (nNewSel >= m_TabCtrl.GetItemCount()) return 0;
	m_TabCtrl.SetCurSel(nNewSel);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnFileCloseTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CTabViewTabItem* pCurSelTabItem = m_TabCtrl.GetItem(m_TabCtrl.GetCurSel());
	if (!pCurSelTabItem) return 0;

	switch(wID)
	{
	case ID_FILE_CLOSE_TAB:
		CloseTab(pCurSelTabItem);
		break;

	case ID_FILE_CLOSE_ALL_TABS_BUT_THIS:
	case ID_FILE_CLOSE_ALL_TABS_LEFT:
	case ID_FILE_CLOSE_ALL_TABS_RIGHT:
		{
			MutexLock viewMapLock(m_tabsMutex);

			// close all to the left
			if( wID == ID_FILE_CLOSE_ALL_TABS_BUT_THIS ||
			    wID == ID_FILE_CLOSE_ALL_TABS_LEFT )
			{
				for(;;)
				{
					if( m_TabCtrl.GetItemCount() == 0 )
						break;

					CTabViewTabItem* pMostLeftTabItem = m_TabCtrl.GetItem(0);

					if( pMostLeftTabItem == pCurSelTabItem )
						break;

					CloseTab(pMostLeftTabItem->GetTabView());
				}
			}

			// close all to the right
			if( wID == ID_FILE_CLOSE_ALL_TABS_BUT_THIS ||
			    wID == ID_FILE_CLOSE_ALL_TABS_RIGHT )
			{
				for(;;)
				{
					if( m_TabCtrl.GetItemCount() == 0 )
						break;

					CTabViewTabItem* pMostRightTabItem = m_TabCtrl.GetItem(m_TabCtrl.GetItemCount() - 1);

					if( pMostRightTabItem == pCurSelTabItem )
						break;

					CloseTab(pMostRightTabItem->GetTabView());
				}
			}


				/*
				if( !g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView )
				if (m_tabs.size() <= 1) return;
				*/
		}
		break;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnNextTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nCurSel = m_TabCtrl.GetCurSel();

	if (++nCurSel >= m_TabCtrl.GetItemCount()) nCurSel = 0;
	m_TabCtrl.SetCurSel(nCurSel);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnPrevTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nCurSel = m_TabCtrl.GetCurSel();

	if (--nCurSel < 0) nCurSel = m_TabCtrl.GetItemCount() - 1;
	m_TabCtrl.SetCurSel(nCurSel);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSwitchView(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if( m_activeTabView )
    m_activeTabView->SwitchView(wID);

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnResizeView(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if( m_activeTabView )
    m_activeTabView->ResizeView(wID);

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnCloseView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  MutexLock viewMapLock(m_tabsMutex);

  if( !g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView )
  {
    if( m_tabs.size() == 1 && m_tabs.begin()->second->GetViewsCount() == 1 )
      return 0;
  }

	if( m_activeTabView )
	{
		bool boolTabClosed;
		m_activeTabView->CloseView(0, boolTabClosed);
		if( boolTabClosed )
			CloseTab(m_activeTabView->m_hWnd);
	}

  if( !g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView )
  {
    if( m_tabs.size() == 1 && m_tabs.begin()->second->GetViewsCount() == 1 )
      UIEnable(ID_CLOSE_VIEW, FALSE);
  }

  ::SetForegroundWindow(m_hWnd);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSplitHorizontally(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  MutexLock viewMapLock(m_tabsMutex);

  if( m_activeTabView )
    m_activeTabView->SplitHorizontally();

  if( !g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView )
  {
    if( m_tabs.size() > 1 || m_tabs.begin()->second->GetViewsCount() > 1 )
      UIEnable(ID_CLOSE_VIEW, TRUE);
  }

  ::SetForegroundWindow(m_hWnd);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSplitVertically(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  MutexLock viewMapLock(m_tabsMutex);

  if( m_activeTabView )
    m_activeTabView->SplitVertically();

  if( !g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView )
  {
    if( m_tabs.size() > 1 || m_tabs.begin()->second->GetViewsCount() > 1 )
      UIEnable(ID_CLOSE_VIEW, TRUE);
  }

  ::SetForegroundWindow(m_hWnd);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnGroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  MutexLock lock(m_tabsMutex);
  for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
  {
    it->second->Group(true);
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnUngroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  MutexLock lock(m_tabsMutex);
  for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
  {
    it->second->Group(false);
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnGroupTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (!m_activeTabView) return 0;
  m_activeTabView->Group(true);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnUngroupTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (!m_activeTabView) return 0;
  m_activeTabView->Group(false);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (!m_activeTabView) return 0;
  std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
  if( activeConsoleView )
  {
    activeConsoleView->Copy();
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (!m_activeTabView) return 0;
  std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
  if( activeConsoleView )
  {
    activeConsoleView->SelectAll();
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditClearSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (!m_activeTabView) return 0;
  std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
  if( activeConsoleView )
  {
    activeConsoleView->ClearSelection();
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  PasteToConsoles();
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditStopScrolling(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (!m_activeTabView) return 0;
  std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
  if( activeConsoleView )
  {
    activeConsoleView->GetConsoleHandler().StopScrolling();
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditRenameTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (!m_activeTabView) return 0;

  DlgRenameTab dlg(m_activeTabView->GetTitle());

  if (dlg.DoModal() == IDOK)
  {
    m_activeTabView->SetTitle(dlg.m_strTabName);

    this->PostMessage(
      UM_UPDATE_TITLES,
      reinterpret_cast<WPARAM>(m_activeTabView->m_hWnd),
      0);
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!m_activeTabView) return 0;

	DlgSettingsMain dlg;

	// unregister global hotkeys here, they might change
	UnregisterGlobalHotkeys();

	if (dlg.DoModal() == IDOK)
	{
		ControlsSettings& controlsSettings = g_settingsHandler->GetAppearanceSettings().controlsSettings;

		DWORD dwTabStyles = ::GetWindowLong(GetTabCtrl().m_hWnd, GWL_STYLE);
		if (controlsSettings.bTabsOnBottom) dwTabStyles |= CTCS_BOTTOM; else dwTabStyles &= ~CTCS_BOTTOM;
		if (g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView) dwTabStyles |= CTCS_CLOSELASTTAB; else dwTabStyles &= ~CTCS_CLOSELASTTAB;
		::SetWindowLong(GetTabCtrl().m_hWnd, GWL_STYLE, dwTabStyles);

		SetWindowStyles();

		UpdateTabsMenu(m_CmdBar.GetMenu(), m_tabsMenu);
		UpdateMenuHotKeys();

		CreateAcceleratorTable();

		SetTransparency();

		// tray icon
		if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon)
		{
			SetTrayIcon(NIM_ADD);
		}
		else
		{
			SetTrayIcon(NIM_DELETE);
		}

    MutexLock	tabMapLock(m_tabsMutex);

    if( !m_bFullScreen )
    {
      m_bMenuChecked = controlsSettings.bShowMenu;
      ShowMenu(m_bMenuChecked);
      ShowToolbar(controlsSettings.bShowToolbar);

      bool bShowTabs = false;

      if ( controlsSettings.bShowTabs && 
        (!controlsSettings.bHideSingleTab || (m_tabs.size() > 1))
        )
      {
        bShowTabs = true;
      }

      ShowTabs(bShowTabs);

      ShowStatusbar(controlsSettings.bShowStatusbar);
    }

    SetZOrder(g_settingsHandler->GetAppearanceSettings().positionSettings.zOrder);

    const COLORREF * consoleColors = g_settingsHandler->GetConsoleSettings().consoleColors;
    for (auto it = m_tabs.begin(); it != m_tabs.end(); ++it)
    {
      it->second->InitializeScrollbars();
      it->second->GetTabData()->SetColors(consoleColors, false);
    }

    TabDataVector& tabDataVector = g_settingsHandler->GetTabSettings().tabDataVector;
    for (auto it = tabDataVector.begin(); it != tabDataVector.end(); ++it)
    {
      it->get()->SetColors(consoleColors, false);
    }

    ConsoleView::RecreateFont(g_settingsHandler->GetAppearanceSettings().fontSettings.dwSize, false);
    AdjustWindowSize(ADJUSTSIZE_WINDOW);

    UpdateUI();
  }

  RegisterGlobalHotkeys();

	// update tab titles
	this->PostMessage(
		UM_UPDATE_TITLES,
		0,
		0);

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewMenu(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if( wID == ID_VIEW_MENU2 )
	{
		if( !m_bMenuChecked )
		{
			ShowMenu(!m_bMenuVisible);
		}
	}
	else
	{
		m_bMenuChecked = !m_bMenuChecked;
		ShowMenu(m_bMenuChecked);
		if( !m_bFullScreen )
		{
			g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowMenu = m_bMenuChecked;
			g_settingsHandler->SaveSettings();
		}
	}
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ShowToolbar(!m_bToolbarVisible);
  if( !m_bFullScreen )
  {
    g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowToolbar = m_bToolbarVisible;
    g_settingsHandler->SaveSettings();
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ShowStatusbar(!m_bStatusBarVisible);
  if( !m_bFullScreen )
  {
    g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowStatusbar = m_bStatusBarVisible;
    g_settingsHandler->SaveSettings();
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ShowTabs(!m_bTabsVisible);
  if( !m_bFullScreen )
  {
    g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowTabs = m_bTabsVisible;
    g_settingsHandler->SaveSettings();
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewConsole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (m_activeTabView)
  {
    std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
    if( activeConsoleView )
    {
      activeConsoleView->SetConsoleWindowVisible(!activeConsoleView->GetConsoleWindowVisible());
      UISetCheck(ID_VIEW_CONSOLE, activeConsoleView->GetConsoleWindowVisible() ? TRUE : FALSE);
    }
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnFullScreen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowFullScreen(!m_bFullScreen);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnZoom(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_activeTabView)
	{
		std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
		if( activeConsoleView )
		{
			DWORD dwNewSize = g_settingsHandler->GetAppearanceSettings().fontSettings.dwSize;

			if( wID != ID_VIEW_ZOOM_100 )
			{
				dwNewSize = ::MulDiv(dwNewSize, activeConsoleView->GetFontZoom(), 100);
				if( wID == ID_VIEW_ZOOM_INC ) dwNewSize ++;
				if( wID == ID_VIEW_ZOOM_DEC ) dwNewSize --;
			}

			// recreate font with new size
			if (ConsoleView::RecreateFont(dwNewSize, true))
			{
				// only if the new size is different (to avoid flickering at extremes)
				AdjustWindowSize(ADJUSTSIZE_FONT);
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnDumpBuffer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (m_activeTabView)
  {
    std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
    if( activeConsoleView )
    {
      activeConsoleView->DumpBuffer();
    }
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::HtmlHelp(m_hWnd, (Helpers::GetModulePath(NULL) + wstring(L"console.chm")).c_str(), HH_DISPLAY_TOPIC, NULL);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::AdjustWindowRect(CRect& rect)
{
	AdjustWindowRectEx(&rect, GetWindowLong(GWL_STYLE), FALSE, GetWindowLong(GWL_EXSTYLE));

	// adjust for the toolbar height
	CReBarCtrl	rebar(m_hWndToolBar);
	rect.bottom	+= rebar.GetBarHeight() - 4;

	if (m_bStatusBarVisible)
	{
		CRect	rectStatusBar(0, 0, 0, 0);

		::GetWindowRect(m_hWndStatusBar, &rectStatusBar);
		rect.bottom	+= rectStatusBar.Height();
	}

	rect.bottom	+= GetTabAreaHeight(); //+0
}

//////////////////////////////////////////////////////////////////////////////

bool MainFrame::CreateNewConsole(DWORD dwTabIndex, const wstring& strCmdLineInitialDir /*= wstring(L"")*/, const wstring& strCmdLineInitialCmd /*= wstring(L"")*/)
{
	if (dwTabIndex >= g_settingsHandler->GetTabSettings().tabDataVector.size()) return false;

	MutexLock	tabMapLock(m_tabsMutex);

	std::shared_ptr<TabData> tabData = g_settingsHandler->GetTabSettings().tabDataVector[dwTabIndex];

	std::shared_ptr<TabView> tabView(new TabView(*this, tabData, strCmdLineInitialDir, strCmdLineInitialCmd));

	HWND hwndTabView = tabView->Create(
											m_hWnd, 
											rcDefault, 
											NULL, 
											WS_CHILD | WS_VISIBLE);

	if (hwndTabView == NULL)
	{
		return false;
	}

	m_tabs.insert(TabViewMap::value_type(hwndTabView, tabView));

	CString strTabTitle;
	tabView->GetWindowText(strTabTitle);

	if( g_settingsHandler->GetAppearanceSettings().controlsSettings.bHideTabIcons )
		AddTab(hwndTabView, strTabTitle);
	else
		AddTabWithIcon(hwndTabView, strTabTitle, tabView->GetIcon(false));
	DisplayTab(hwndTabView, FALSE);
	::SetForegroundWindow(m_hWnd);

  if (m_tabs.size() > 1)
  {
    CRect clientRect(0, 0, 0, 0);
    tabView->AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
  }

  if( !m_bFullScreen &&
      g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowTabs &&
      (
        m_tabs.size() > 1 ||
        !g_settingsHandler->GetAppearanceSettings().controlsSettings.bHideSingleTab
      )
    )
  {
    ShowTabs(true);
  }

	UpdateUI();

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CloseTab(CTabViewTabItem* pTabItem)
{
  MutexLock viewMapLock(m_tabsMutex);
  if (!pTabItem) return;
  if( !g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView )
    if (m_tabs.size() <= 1) return;
  CloseTab(pTabItem->GetTabView());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CloseTab(HWND hwndTabView)
{
  MutexLock viewMapLock(m_tabsMutex);
  TabViewMap::iterator it = m_tabs.find(hwndTabView);
  if (it == m_tabs.end()) return;

  RemoveTab(hwndTabView);
  if (m_activeTabView == it->second) m_activeTabView.reset();
  it->second->DestroyWindow();
  m_tabs.erase(it);

  UpdateUI();

  if ((m_tabs.size() == 1) &&
    m_bTabsVisible && 
    (g_settingsHandler->GetAppearanceSettings().controlsSettings.bHideSingleTab))
  {
    ShowTabs(false);
  }

  if (m_tabs.empty()) PostMessage(WM_CLOSE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::UpdateTabsMenu(CMenuHandle mainMenu, CMenu& tabsMenu)
{
	if (!tabsMenu.IsNull()) tabsMenu.DestroyMenu();
	tabsMenu.CreateMenu();

	// build tabs menu
	TabDataVector&  tabDataVector = g_settingsHandler->GetTabSettings().tabDataVector;
	WORD            wId           = ID_NEW_TAB_1;

	for (auto it = tabDataVector.begin(); it != tabDataVector.end(); ++it, ++wId)
	{
		CMenuItemInfo	subMenuItem;

		auto hotK = g_settingsHandler->GetHotKeys().commands.get<HotKeys::commandID>().find(wId);

		std::wstring strTitle = (*it)->strTitle;
		if( hotK != g_settingsHandler->GetHotKeys().commands.get<HotKeys::commandID>().end() )
		{
			strTitle += L"\t";
			strTitle += hotK->get()->GetHotKeyName();
		}

		subMenuItem.fMask       = MIIM_STRING | MIIM_ID;
		subMenuItem.wID         = wId;
		subMenuItem.dwTypeData  = const_cast<wchar_t*>(strTitle.c_str());
		subMenuItem.cch         = static_cast<UINT>(strTitle.length());

		tabsMenu.InsertMenuItem(wId-ID_NEW_TAB_1, TRUE, &subMenuItem);

		m_CmdBar.RemoveImage(wId);
		HICON hiconMenu = (*it)->GetMenuIcon();
		if( hiconMenu )
			m_CmdBar.AddIcon(hiconMenu, wId);
	}

	// set tabs menu as popup submenu
	if (!mainMenu.IsNull())
	{
		CMenuItemInfo	menuItem;

		menuItem.fMask    = MIIM_SUBMENU;
		menuItem.hSubMenu	= HMENU(tabsMenu);

		mainMenu.SetMenuItemInfo(ID_FILE_NEW_TAB, FALSE, &menuItem);
	}

	// create jumplist
	JumpList::CreateList(g_settingsHandler->GetTabSettings().tabDataVector);
}

void MainFrame::UpdateOpenedTabsMenu(CMenu& tabsMenu)
{
	if (!tabsMenu.IsNull()) tabsMenu.DestroyMenu();
	tabsMenu.CreatePopupMenu();

	// in full screen, adds the entry "exit fullscreen"
	if( m_bFullScreen )
	{
		CMenuItemInfo	subMenuItem;
		WORD wId = ID_VIEW_FULLSCREEN;
		auto hotK = g_settingsHandler->GetHotKeys().commands.get<HotKeys::commandID>().find(wId);

		std::wstring strTitle = L"Exit Full Screen";
		if( hotK != g_settingsHandler->GetHotKeys().commands.get<HotKeys::commandID>().end() )
		{
			strTitle += L"\t";
			strTitle += hotK->get()->GetHotKeyName();
		}

		subMenuItem.fMask       = MIIM_STRING | MIIM_ID;
		subMenuItem.wID         = wId;
		subMenuItem.dwTypeData  = const_cast<wchar_t*>(strTitle.c_str());
		subMenuItem.cch         = static_cast<UINT>(strTitle.length() + 1);

		tabsMenu.InsertMenuItem(wId, TRUE, &subMenuItem);
	}

	// build tabs menu
	WORD wId = ID_SWITCH_TAB_1;
	MutexLock	tabMapLock(m_tabsMutex);
	for (auto it = m_tabs.begin(); it != m_tabs.end(); ++it, ++wId)
	{
		CMenuItemInfo	subMenuItem;

		auto hotK = g_settingsHandler->GetHotKeys().commands.get<HotKeys::commandID>().find(wId);

		std::wstring strTitle = it->second->GetTitle();
		if( hotK != g_settingsHandler->GetHotKeys().commands.get<HotKeys::commandID>().end() )
		{
			strTitle += L"\t";
			strTitle += hotK->get()->GetHotKeyName();
		}

		subMenuItem.fMask       = MIIM_STRING | MIIM_ID;
		subMenuItem.wID         = wId;
		subMenuItem.dwTypeData  = const_cast<wchar_t*>(strTitle.c_str());
		subMenuItem.cch         = static_cast<UINT>(strTitle.length() + 1);

		tabsMenu.InsertMenuItem(wId-ID_SWITCH_TAB_1, TRUE, &subMenuItem);

		auto tabData = it->second->GetTabData();

		if (m_activeTabView == it->second)
			tabsMenu.EnableMenuItem(wId, MF_GRAYED | MF_BYCOMMAND);

		m_CmdBar.RemoveImage(wId);
		HICON hiconMenu = tabData->GetMenuIcon();
		if( hiconMenu )
			m_CmdBar.AddIcon(hiconMenu, wId);
	}
}

void MainFrame::UpdateMenuHotKeys(void)
{
  CMenuHandle menu = m_CmdBar.GetMenu();

  auto ids =  g_settingsHandler->GetHotKeys().commands;
  for(auto id = ids.begin(); id != ids.end(); ++id)
  {
    CString strMenuItemText;
    if( menu.GetMenuString(id->get()->wCommandID, strMenuItemText, MF_BYCOMMAND) )
    {
      int tab = strMenuItemText.Find(L'\t');
      if (tab != -1)
        strMenuItemText.Truncate(tab);
      strMenuItemText.AppendChar(L'\t');
      strMenuItemText.Append(id->get()->GetHotKeyName().c_str());

      menu.ModifyMenu(id->get()->wCommandID, MF_BYCOMMAND | MF_STRING, static_cast<UINT_PTR>(id->get()->wCommandID), strMenuItemText.GetString());
    }
    if( m_tabsRPopupMenu.GetMenuString(id->get()->wCommandID, strMenuItemText, MF_BYCOMMAND) )
    {
      int tab = strMenuItemText.Find(L'\t');
      if (tab != -1)
        strMenuItemText.Truncate(tab);
      strMenuItemText.AppendChar(L'\t');
      strMenuItemText.Append(id->get()->GetHotKeyName().c_str());

      m_tabsRPopupMenu.ModifyMenu(id->get()->wCommandID, MF_BYCOMMAND | MF_STRING, static_cast<UINT_PTR>(id->get()->wCommandID), strMenuItemText.GetString());
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::UpdateStatusBar()
{
  static CString strCAPS(LPCTSTR(IDPANE_CAPS_INDICATOR));
  static CString strNUM (LPCTSTR(IDPANE_NUM_INDICATOR ));
  static CString strSCRL(LPCTSTR(IDPANE_SCRL_INDICATOR));

  UISetText(1, (GetKeyState(VK_CAPITAL) & 1) ? strCAPS : L"");
  UISetText(2, (GetKeyState(VK_NUMLOCK) & 1) ? strNUM  : L"");
  UISetText(3, (GetKeyState(VK_SCROLL)  & 1) ? strSCRL : L"");

  wchar_t strSelection   [16] = L"";
  wchar_t strColsRows    [16] = L"";
  wchar_t strBufColsRows [16] = L"";
  wchar_t strPid         [16] = L"";
  wchar_t strZoom        [16] = L"";

  if (m_activeTabView)
  {
    std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
    if( activeConsoleView )
    {
      SharedMemory<ConsoleParams>& consoleParams = activeConsoleView->GetConsoleHandler().GetConsoleParams();

      DWORD dwSelectionSize = activeConsoleView->GetSelectionSize();
      if( dwSelectionSize )
        _snwprintf_s(strSelection, ARRAYSIZE(strSelection),   _TRUNCATE, L"%lu", dwSelectionSize);

      _snwprintf_s(strColsRows,    ARRAYSIZE(strColsRows),    _TRUNCATE, L"%lux%lu",
        consoleParams->dwColumns,
        consoleParams->dwRows);
      _snwprintf_s(strPid,         ARRAYSIZE(strPid),         _TRUNCATE, L"%lu",
        activeConsoleView->GetConsoleHandler().GetConsolePid());
      _snwprintf_s(strBufColsRows, ARRAYSIZE(strBufColsRows), _TRUNCATE, L"%lux%lu",
        consoleParams->dwBufferColumns ? consoleParams->dwBufferColumns : consoleParams->dwColumns,
        consoleParams->dwBufferRows ? consoleParams->dwBufferRows : consoleParams->dwRows);
      _snwprintf_s(strZoom, ARRAYSIZE(strZoom),               _TRUNCATE, L"%lu%%",
        activeConsoleView->GetFontZoom());

      UIEnable(ID_EDIT_COPY,            activeConsoleView->CanCopy()           ? TRUE : FALSE);
      UIEnable(ID_EDIT_CLEAR_SELECTION, activeConsoleView->CanClearSelection() ? TRUE : FALSE);
      UIEnable(ID_EDIT_PASTE,           activeConsoleView->CanPaste()          ? TRUE : FALSE);
      UISetCheck(ID_VIEW_CONSOLE, activeConsoleView->GetConsoleWindowVisible() ? TRUE : FALSE);
    }
  }

  UISetText(4, strPid);
  UISetText(5, strSelection);
  UISetText(6, strColsRows);
  UISetText(7, strBufColsRows);
  UISetText(8, strZoom);

  UIUpdateStatusBar();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::SetWindowStyles(void)
{
  StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

  DWORD	dwStyle   = GetWindowLong(GWL_STYLE);
  DWORD	dwExStyle = GetWindowLong(GWL_EXSTYLE);

  DWORD	dwOldStyle   = dwStyle;
  DWORD	dwOldExStyle = dwExStyle;

  if( m_bFullScreen )
  {
    dwStyle &= ~WS_MAXIMIZEBOX;
    dwStyle &= ~WS_CAPTION;
    dwStyle &= ~WS_THICKFRAME;
  }
  else
  {
    if (stylesSettings.bResizable) dwStyle |= WS_MAXIMIZEBOX; else dwStyle &= ~WS_MAXIMIZEBOX;
    if (stylesSettings.bCaption)   dwStyle |= WS_CAPTION;     else dwStyle &= ~WS_CAPTION;
    if (stylesSettings.bResizable) dwStyle |= WS_THICKFRAME;  else dwStyle &= ~WS_THICKFRAME;
    if (stylesSettings.bBorder)    dwStyle |= WS_BORDER; /* WS_CAPTION = WS_BORDER | WS_DLGFRAME  */
  }

  dwStyle |= WS_MINIMIZEBOX;
  dwExStyle |= WS_EX_APPWINDOW;

  if (!stylesSettings.bTaskbarButton)
  {
    if (!stylesSettings.bTrayIcon)
    {
      // remove minimize button
      dwStyle &= ~WS_MINIMIZEBOX;
    }
    dwExStyle &= ~WS_EX_APPWINDOW;
  }

  SetWindowLong(GWL_STYLE, dwStyle);
  SetWindowLong(GWL_EXSTYLE, dwExStyle);

  if( m_bOnCreateDone )
  {
    TRACE(
      L"MainFrame::SetWindowStyles Style %08lx -> %08lx ExStyle %08lx -> %08lx\n",
      dwOldStyle, dwStyle,
      dwOldExStyle, dwExStyle);

    if( dwExStyle != dwOldExStyle )
    {
      this->ShowWindow(SW_HIDE);
      if (stylesSettings.bTaskbarButton)
        this->ModifyStyleEx(WS_EX_TOOLWINDOW, WS_EX_APPWINDOW);
      else
        this->ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
      this->ShowWindow(SW_SHOW);
    }

    if( dwStyle != dwOldStyle )
    {
      this->SetWindowPos(
        nullptr,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
  }
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::DockWindow(DockPosition dockPosition)
{
	m_dockPosition = dockPosition;
	if (m_dockPosition == dockNone) return;

	CRect			rectDesktop;
	CRect			rectWindow;
	int				nX = 0;
	int				nY = 0;

	Helpers::GetDesktopRect(m_hWnd, rectDesktop);
	GetWindowRect(&rectWindow);

	switch (m_dockPosition)
	{
		case dockTL :
		{
			nX = rectDesktop.left;
			nY = rectDesktop.top;
			break;
		}

		case dockTR :
		{
			nX = rectDesktop.right - rectWindow.Width();
			nY = rectDesktop.top;
			break;
		}

		case dockBR :
		{
			nX = rectDesktop.right - rectWindow.Width();
			nY = rectDesktop.bottom - rectWindow.Height();
			break;
		}

		case dockBL :
		{
			nX = rectDesktop.left;
			nY = rectDesktop.bottom - rectWindow.Height();
			break;
		}

		default : return;
	}

	SetWindowPos(
		NULL, 
		nX, 
		nY, 
		0, 
		0, 
		SWP_NOSIZE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::SetZOrder(ZOrder zOrder)
{
	if (zOrder == m_zOrder) return;

	HWND hwndZ = HWND_NOTOPMOST;

	m_zOrder = zOrder;

	switch (m_zOrder)
	{
		case zorderNormal	: hwndZ = HWND_NOTOPMOST; break;
		case zorderOnTop	: hwndZ = HWND_TOPMOST; break;
		case zorderOnBottom	: hwndZ = HWND_BOTTOM; break;
		case zorderDesktop	: hwndZ = HWND_NOTOPMOST; break;
	}

	// if we're pinned to the desktop, desktop shell's main window is our parent
	if( m_zOrder == zorderDesktop )
	{
		HWND hShellWnd = GetShellWindow();
		DWORD dwShellPID = 0;

		if( hShellWnd )
			::GetWindowThreadProcessId(hShellWnd, &dwShellPID);

		// when Windows 7's desktop picture rotation functionality is activated
		// we search a window with class "WorkerW" in the shell's process
		if( dwShellPID && Helpers::CheckOSVersion(6, 1) )
		{
			HWND hShell = NULL;

			while( (hShell = ::FindWindowEx(NULL, hShell, L"WorkerW", NULL)) != NULL )
			{
				if( ::IsWindowVisible(hShell) && ::FindWindowEx(hShell, NULL, NULL, NULL) )
				{
					DWORD dwTestPID;
					::GetWindowThreadProcessId(hShell, &dwTestPID);

					if( dwTestPID == dwShellPID )
						break;
				}
			}

			if( hShell )
				hShellWnd = hShell;
		}

		SetParent(hShellWnd);
	}
	else
	{
		SetParent(NULL);
	}
	SetWindowPos(hwndZ, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HWND MainFrame::GetDesktopWindow()
{
	// pinned to the desktop, Program Manager is the parent
	// TODO: support more shells/automatic shell detection
	return ::FindWindow(L"Progman", L"Program Manager");
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::SetWindowIcons()
{
	WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;

	if (!m_icon.IsNull()) m_icon.DestroyIcon();
	if (!m_smallIcon.IsNull()) m_smallIcon.DestroyIcon();

	if (windowSettings.bUseTabIcon && m_activeTabView)
	{
		m_icon.Attach(m_activeTabView->GetIcon(true).DuplicateIcon());
		m_smallIcon.Attach(m_activeTabView->GetIcon(false).DuplicateIcon());
	}
	else
	{
		m_icon.Attach(Helpers::LoadTabIcon(true, false, windowSettings.strIcon, L""));
		m_smallIcon.Attach(Helpers::LoadTabIcon(false, false, windowSettings.strIcon, L""));
	}

	if (!m_icon.IsNull())
	{
		CIcon oldIcon(SetIcon(m_icon, TRUE));
	}

	if (!m_smallIcon.IsNull())
	{
		CIcon oldIcon(SetIcon(m_smallIcon, FALSE));
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowMenu(bool bShow)
{
	m_bMenuVisible = bShow;

	CReBarCtrl rebar(m_hWndToolBar);
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST);	// menu is 1st added band
	rebar.ShowBand(nBandIndex, m_bMenuVisible);
	UISetCheck(ID_VIEW_MENU, m_bMenuChecked);

	UpdateLayout();
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	DockWindow(m_dockPosition);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowToolbar(bool bShow)
{
	m_bToolbarVisible = bShow;

	CReBarCtrl rebar(m_hWndToolBar);
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, m_bToolbarVisible);
	UISetCheck(ID_VIEW_TOOLBAR, m_bToolbarVisible);

	UpdateLayout();
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	DockWindow(m_dockPosition);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowStatusbar(bool bShow)
{
	m_bStatusBarVisible = bShow;

	::ShowWindow(m_hWndStatusBar, m_bStatusBarVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, m_bStatusBarVisible);

	UpdateLayout();
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	DockWindow(m_dockPosition);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowTabs(bool bShow)
{
	m_bTabsVisible = bShow;

	if (m_bTabsVisible)
	{
		ShowTabControl();
	}
	else
	{
		HideTabControl();
	}

	UISetCheck(ID_VIEW_TABS, m_bTabsVisible);

	UpdateLayout();
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	DockWindow(m_dockPosition);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowFullScreen(bool bShow)
{
  m_bFullScreen = bShow;

  m_CmdBar.RemoveImage(ID_VIEW_FULLSCREEN);

  if( m_bFullScreen )
  {
    m_CmdBar.AddBitmap(IDR_FULLSCREEN1, ID_VIEW_FULLSCREEN);
    m_toolbar.ChangeBitmap(ID_VIEW_FULLSCREEN, m_nFullSreen1Bitmap);

    // save the non fullscreen position and size
    // normal or maximized
    GetWindowRect(&m_rectWndNotFS);

    m_bMenuChecked = false;
    ShowMenu     (false);
    ShowToolbar  (false);
    ShowStatusbar(false);
    ShowTabs     (false);
  }
  else
  {
    m_CmdBar.AddBitmap(IDR_FULLSCREEN2, ID_VIEW_FULLSCREEN);
    m_toolbar.ChangeBitmap(ID_VIEW_FULLSCREEN, m_nFullSreen2Bitmap);

    ControlsSettings&	controlsSettings= g_settingsHandler->GetAppearanceSettings().controlsSettings;

    bool bShowTabs = controlsSettings.bShowTabs;

    if( bShowTabs )
    {
      MutexLock lock(m_tabsMutex);
      if ((m_tabs.size() == 1) && (controlsSettings.bHideSingleTab))
      {
        bShowTabs = false;
      }
    }

    m_bMenuChecked = controlsSettings.bShowMenu;
    ShowMenu     (m_bMenuChecked);
    ShowToolbar  (controlsSettings.bShowToolbar);
    ShowStatusbar(controlsSettings.bShowStatusbar);
    ShowTabs     (bShowTabs);
  }

  UISetCheck(ID_VIEW_FULLSCREEN, m_bFullScreen);

  if( !m_bFullScreen ) this->ShowWindow(SW_HIDE);
  SetWindowStyles();
  if( !m_bFullScreen ) this->ShowWindow(SW_SHOW);
  SetTransparency();

  // and go to fullscreen or restore
  if( m_bFullScreen )
  {
    FullScreenSettings&	fullScreenSettings = g_settingsHandler->GetAppearanceSettings().fullScreenSettings;
    DWORD dwFullScreenMonitor = fullScreenSettings.dwFullScreenMonitor;

    if( dwFullScreenMonitor > 0 )
    {
      std::vector<CRect> vMonitors;
      ::EnumDisplayMonitors(NULL, NULL, MainFrame::MonitorEnumProc, reinterpret_cast<LPARAM>(&vMonitors));
      if( dwFullScreenMonitor > vMonitors.size() )
        dwFullScreenMonitor = 0;
      else
        SetWindowPos(NULL, vMonitors[dwFullScreenMonitor - 1], SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
    }

    if( dwFullScreenMonitor == 0 )
    {
      CRect rectCurrent;
      if( Helpers::GetMonitorRect(m_hWnd, rectCurrent) )
        SetWindowPos(NULL, rectCurrent, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
    }
  }
  else
  {
    // restore the non fullscreen position
    // normal or maximized
    SetWindowPos(NULL, m_rectWndNotFS, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
  }

  AdjustWindowSize(ADJUSTSIZE_WINDOW);
}

BOOL CALLBACK MainFrame::MonitorEnumProc(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData)
{
  std::vector<CRect> * pvMonitors = reinterpret_cast<std::vector<CRect> *>(lpData);

  pvMonitors->push_back(lprcMonitor);

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ResizeWindow()
{
	CRect rectWindow;
	GetWindowRect(&rectWindow);

	DWORD dwWindowWidth	= rectWindow.Width();
	DWORD dwWindowHeight= rectWindow.Height();

#ifdef _DEBUG
	CRect rectClient;
	GetClientRect(&rectClient);

	TRACE(L"old dims: %ix%i\n", m_dwWindowWidth, m_dwWindowHeight);
	TRACE(L"new dims: %ix%i\n", dwWindowWidth, dwWindowHeight);
	TRACE(L"client dims: %ix%i\n", rectClient.Width(), rectClient.Height());
#endif

	if ((dwWindowWidth != m_dwWindowWidth) ||
		(dwWindowHeight != m_dwWindowHeight))
	{
		AdjustWindowSize(ADJUSTSIZE_WINDOW);
	}

	SendMessage(WM_NULL, 0, 0);
	m_dwResizeWindowEdge = WMSZ_BOTTOM;

	if (m_activeTabView) m_activeTabView->SetResizing(false);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::AdjustWindowSize(ADJUSTSIZE as)
{
  TRACE(L"AdjustWindowSize\n");
	CRect clientRect(0, 0, 0, 0);

	if (as != ADJUSTSIZE_NONE)
	{
		// adjust the active view
		if (!m_activeTabView) return;

		m_activeTabView->AdjustRectAndResize(as, clientRect, m_dwResizeWindowEdge);

		// for other views, first set view size and then resize their Windows consoles
		MutexLock	viewMapLock(m_tabsMutex);

    for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
		{
			if (m_activeTabView == it->second) continue;

			it->second->SetWindowPos(
							0,
							0,
							0,
							clientRect.Width(),
							clientRect.Height(),
							SWP_NOMOVE|SWP_NOZORDER|SWP_NOSENDCHANGING);

			it->second->AdjustRectAndResize(as, clientRect, m_dwResizeWindowEdge);
		}
	}
	else
	{
		if (!m_activeTabView) return;

		m_activeTabView->GetRect(clientRect);
	}

  TRACE(L"AdjustWindowSize 0: %ix%i\n", clientRect.Width(), clientRect.Height());

	AdjustWindowRect(clientRect);

	TRACE(L"AdjustWindowSize 1: %ix%i\n", clientRect.Width(), clientRect.Height());
	SetWindowPos(
		0,
		0,
		0,
		clientRect.Width(),
		clientRect.Height() + 4,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOSENDCHANGING);

	// update window width and height
	CRect rectWindow;

	GetWindowRect(&rectWindow);
	TRACE(L"AdjustWindowSize 2: %ix%i\n", rectWindow.Width(), rectWindow.Height());
	m_dwWindowWidth	= rectWindow.Width();
	m_dwWindowHeight= rectWindow.Height();

	SetMargins();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::SetMargins(void)
{
  CReBarCtrl rebar(m_hWndToolBar);
  DWORD dwStyle = this->m_TabCtrl.GetStyle();

  m_Margins.cyTopHeight = rebar.GetBarHeight();
  m_Margins.cyBottomHeight = 0;

  if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
  {
    if( m_bTabsVisible )
    {
      m_Margins.cyBottomHeight += m_nTabAreaHeight;
    }

    if (m_bStatusBarVisible)
    {
      CRect rectStatusBar(0, 0, 0, 0);
      ::GetWindowRect(m_hWndStatusBar, &rectStatusBar);
      m_Margins.cyBottomHeight += rectStatusBar.Height();
    }
  }
  else
  {
    if( m_bTabsVisible )
    {
      m_Margins.cyTopHeight += m_nTabAreaHeight;
    }
  }
  SetTransparency();
}

void MainFrame::SetTransparency()
{
  // set transparency
  TransparencySettings& transparencySettings = g_settingsHandler->GetAppearanceSettings().transparencySettings;

  // RAZ
  SetWindowLong(
    GWL_EXSTYLE,
    GetWindowLong(GWL_EXSTYLE) & ~WS_EX_LAYERED);

#ifdef _USE_AERO
  BOOL fEnabled = FALSE;
  DwmIsCompositionEnabled(&fEnabled);
  if( fEnabled )
  {
    if( transparencySettings.transType != transGlass )
    {
      // there is a side effect whith glass into client area and no caption (and no resizable)
      // blur is not applied, the window is transparent ...
      DWORD	dwStyle = GetWindowLong(GWL_STYLE);

      if( (dwStyle & WS_CAPTION) != WS_CAPTION && (dwStyle & WS_THICKFRAME) != WS_THICKFRAME )
      {
        MARGINS m = {0, 0, 0, 0};
        ::DwmExtendFrameIntoClientArea(m_hWnd, &m);

        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = FALSE;
        bb.hRgnBlur = NULL;
        ::DwmEnableBlurBehindWindow(m_hWnd, &bb);

        fEnabled = FALSE;
      }
      else
      {
        if( transparencySettings.transType == transColorKey )
        {
          MARGINS m = {0, 0, 0, 0};
          ::DwmExtendFrameIntoClientArea(m_hWnd, &m);
        }
        else
        {
          ::DwmExtendFrameIntoClientArea(m_hWnd, &m_Margins);
        }
      }
    }
  }
#endif

  switch (transparencySettings.transType)
  {
  case transAlpha:
    // if Console is pinned to the desktop window, wee need to set it as top-level window temporarily
    if (m_zOrder == zorderDesktop) SetParent(NULL);

    if ((transparencySettings.byActiveAlpha == 255) &&
      (transparencySettings.byInactiveAlpha == 255))
    {

      break;
    }

    SetWindowLong(
      GWL_EXSTYLE, 
      GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYERED);

    ::SetLayeredWindowAttributes(
      m_hWnd,
      0, 
      transparencySettings.byActiveAlpha, 
      LWA_ALPHA);

    // back to desktop-pinned mode, if needed
    if (m_zOrder == zorderDesktop) SetParent(GetDesktopWindow());

    break;

  case transColorKey :
    {
#ifdef _USE_AERO
      // under VISTA/Windows 7 color key transparency replace aero glass by black
      fEnabled = FALSE;
#endif

      SetWindowLong(
        GWL_EXSTYLE, 
        GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYERED);

      ::SetLayeredWindowAttributes(
        m_hWnd,
        transparencySettings.crColorKey, 
        transparencySettings.byActiveAlpha, 
        LWA_COLORKEY);

      break;
    }

  case transGlass :
    {
#ifdef _USE_AERO
      if( fEnabled )
      {
        // there is a side effect whith glass into client area and no caption (and no resizable)
        // blur is not applied, the window is transparent ...
        DWORD	dwStyle = GetWindowLong(GWL_STYLE);

        if( (dwStyle & WS_CAPTION) != WS_CAPTION && (dwStyle & WS_THICKFRAME) != WS_THICKFRAME )
        {
          DWM_BLURBEHIND bb = {0};
          bb.dwFlags = DWM_BB_ENABLE | DWM_BB_TRANSITIONONMAXIMIZED;
          bb.fEnable = TRUE;
          bb.fTransitionOnMaximized = TRUE;
          bb.hRgnBlur = NULL;
          ::DwmEnableBlurBehindWindow(m_hWnd, &bb);
        }
        else
        {
          MARGINS m = {-1,-1,-1,-1};
          ::DwmExtendFrameIntoClientArea(m_hWnd, &m);
        }
      }
#endif

      break;
    }
  }

#ifdef _USE_AERO
  aero::SetAeroGlassActive(fEnabled != FALSE);
  m_ATB.Invalidate(TRUE);
  m_TabCtrl.Invalidate(TRUE);
#endif
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CreateAcceleratorTable()
{
	HotKeys&                 hotKeys = g_settingsHandler->GetHotKeys();
	std::unique_ptr<ACCEL[]> accelTable(new ACCEL[hotKeys.commands.size()]);
	int                      nAccelCount = 0;

	for (auto it = hotKeys.commands.begin(); it != hotKeys.commands.end(); ++it)
	{
		std::shared_ptr<HotKeys::CommandData> c(*it);

		if ((*it)->accelHotkey.cmd == 0) continue;
		if ((*it)->accelHotkey.key == 0) continue;
		if ((*it)->bGlobal) continue;

		::CopyMemory(&(accelTable[nAccelCount]), &((*it)->accelHotkey), sizeof(ACCEL));
		++nAccelCount;
	}

	if (!m_acceleratorTable.IsNull()) m_acceleratorTable.DestroyObject();
	m_acceleratorTable.CreateAcceleratorTable(accelTable.get(), nAccelCount);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::RegisterGlobalHotkeys()
{
	HotKeys&							hotKeys	= g_settingsHandler->GetHotKeys();
	HotKeys::CommandsSequence::iterator it		= hotKeys.commands.begin();

	for (; it != hotKeys.commands.end(); ++it)
	{
		if ((*it)->accelHotkey.cmd == 0) continue;
		if ((*it)->accelHotkey.key == 0) continue;
		if (!(*it)->bGlobal) continue;

		UINT uiModifiers = 0;

		if ((*it)->accelHotkey.fVirt & FSHIFT)   uiModifiers |= MOD_SHIFT;
		if ((*it)->accelHotkey.fVirt & FCONTROL) uiModifiers |= MOD_CONTROL;
		if ((*it)->accelHotkey.fVirt & FALT)     uiModifiers |= MOD_ALT;
		if ((*it)->bWin)                         uiModifiers |= MOD_WIN;

		::RegisterHotKey(m_hWnd, (*it)->wCommandID, uiModifiers, (*it)->accelHotkey.key);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::UnregisterGlobalHotkeys()
{
	HotKeys&							hotKeys	= g_settingsHandler->GetHotKeys();
	HotKeys::CommandsSequence::iterator it		= hotKeys.commands.begin();

	for (; it != hotKeys.commands.end(); ++it)
	{
		if ((*it)->accelHotkey.cmd == 0) continue;
		if ((*it)->accelHotkey.key == 0) continue;
		if (!(*it)->bGlobal) continue;

		::UnregisterHotKey(m_hWnd, (*it)->wCommandID);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CreateStatusBar()
{
	m_hWndStatusBar = m_statusBar.Create(*this);
#ifdef _USE_AERO
	aero::Subclass(m_ASB, m_hWndStatusBar);
#endif
	UIAddStatusBar(m_hWndStatusBar);

	int arrPanes[]	= { ID_DEFAULT_PANE, IDPANE_CAPS_INDICATOR, IDPANE_NUM_INDICATOR, IDPANE_SCRL_INDICATOR, IDPANE_PID_INDICATOR, IDPANE_SELECTION, IDPANE_COLUMNS_ROWS, IDPANE_BUF_COLUMNS_ROWS, IDPANE_ZOOM};

	m_statusBar.SetPanes(arrPanes, sizeof(arrPanes)/sizeof(int), true);
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::SetTrayIcon(DWORD dwMessage) {
	
	NOTIFYICONDATA	tnd;
	wstring			strToolTip(m_strWindowTitle);

	tnd.cbSize				= sizeof(NOTIFYICONDATA);
	tnd.hWnd				= m_hWnd;
	tnd.uID					= IDC_TRAY_ICON;
	tnd.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= UM_TRAY_NOTIFY;
	tnd.hIcon				= m_smallIcon;
	
	if (strToolTip.length() > 63) {
		strToolTip.resize(59);
		strToolTip += _T(" ...");
	}
	
	// we're still using v4.0 controls, so the size of the tooltip can be at most 64 chars
	// TODO: there should be a macro somewhere
	wcsncpy_s(tnd.szTip, _countof(tnd.szTip), strToolTip.c_str(), (sizeof(tnd.szTip)-1)/sizeof(wchar_t));
	return ::Shell_NotifyIcon(dwMessage, &tnd);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////


void MainFrame::SetActiveConsole(HWND hwndTabView, HWND hwndConsoleView)
{
  // find the tab
  MutexLock viewMapLock(m_tabsMutex);
  auto it = m_tabs.find(hwndTabView);
  if( it != m_tabs.end() )
  {
    it->second->SetActiveConsole(hwndConsoleView);
    if( m_activeTabView != it->second )
    {
      int nCount = m_TabCtrl.GetItemCount();
      for(int i = 0; i < nCount; ++i)
      {
        if( m_TabCtrl.GetItem(i)->GetTabView() == hwndTabView )
        {
          m_TabCtrl.SetCurSel(i);
          break;
        }
      }
    }

    this->ActivateApp();
  }
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void MainFrame::PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam)
{
  MutexLock lock(m_tabsMutex);
  for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
  {
    it->second->PostMessageToConsoles(Msg, wParam, lParam);
  }
}

void MainFrame::WriteConsoleInputToConsoles(KEY_EVENT_RECORD* pkeyEvent)
{
  MutexLock lock(m_tabsMutex);
  for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
  {
    it->second->WriteConsoleInputToConsoles(pkeyEvent);
  }
}

void MainFrame::PasteToConsoles()
{
	if (!m_activeTabView) return;
	std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
	if( activeConsoleView && activeConsoleView->CanPaste() )
	{
		std::wstring text;

		if( !::OpenClipboard(NULL) ) return;

		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if( hData )
		{
			wchar_t * pszText = static_cast<wchar_t*>( GlobalLock(hData) );
			if( pszText )
			{
				try
				{
					text = pszText;
				}
				catch(std::bad_alloc&)
				{
				}
				::GlobalUnlock(hData);
			}
		}

		::CloseClipboard();

		if( !text.empty() )
		{
			if( activeConsoleView->IsGrouped() )
			{
				MutexLock lock(m_tabsMutex);
				for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
				{
					it->second->SendTextToConsoles(text.c_str());
				}
			}
			else
				activeConsoleView->GetConsoleHandler().SendTextToConsole(text.c_str());
		}
	}
}

void MainFrame::SendTextToConsoles(const wchar_t* pszText)
{
  MutexLock lock(m_tabsMutex);
  for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
  {
    it->second->SendTextToConsoles(pszText);
  }
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;
	if (!cds) return 0;

	vector<wstring> startupTabs;
	vector<wstring> startupCmds;
	vector<wstring> startupDirs;
	int nMultiStartSleep = 0;
	wstring strWorkingDir;

	wstring ignoreTitle;

	ParseCommandLine((LPCTSTR)cds->lpData, ignoreTitle, startupTabs, startupDirs, startupCmds, nMultiStartSleep, strWorkingDir);
	CreateInitialTabs(startupTabs, startupCmds, startupDirs, nMultiStartSleep, strWorkingDir);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
  if( g_settingsHandler->GetAppearanceSettings().stylesSettings.bCaption ) return 0;

  LPMINMAXINFO lpMMI = reinterpret_cast<LPMINMAXINFO>(lParam);
  /*
  For systems with multiple monitors, the ptMaxSize and ptMaxPosition members describe the maximized size
  and position of the window on the primary monitor, even if the window ultimately maximizes onto a
  secondary monitor. In that case, the window manager adjusts these values to compensate for differences
  between the primary monitor and the monitor that displays the window. Thus, if the user leaves ptMaxSize
  untouched, a window on a monitor larger than the primary monitor maximizes to the size of the larger monitor.
  */

  CRect rectCurrentWorkArea;
  CRect rectCurrentMonitor;
  if( Helpers::GetDesktopRect(m_hWnd, rectCurrentWorkArea) &&
      Helpers::GetMonitorRect(m_hWnd, rectCurrentMonitor)  &&
      rectCurrentWorkArea != rectCurrentMonitor ) // there is a taskbar ...
  {
    TRACE(
      L"1ptMaxPosition %ix%i\n"
      L"1ptMaxSize     %ix%i\n",
      lpMMI->ptMaxPosition.x, lpMMI->ptMaxPosition.y,
      lpMMI->ptMaxSize.x, lpMMI->ptMaxSize.y);

    lpMMI->ptMaxPosition.x = rectCurrentWorkArea.left - rectCurrentMonitor.left;
    lpMMI->ptMaxPosition.y = rectCurrentWorkArea.top  - rectCurrentMonitor.top ;
    lpMMI->ptMaxSize.x = rectCurrentWorkArea.right  - rectCurrentWorkArea.left;
    lpMMI->ptMaxSize.y = rectCurrentWorkArea.bottom - rectCurrentWorkArea.top;

    TRACE(
      L"2ptMaxPosition %ix%i\n"
      L"2ptMaxSize     %ix%i\n",
      lpMMI->ptMaxPosition.x, lpMMI->ptMaxPosition.y,
      lpMMI->ptMaxSize.x, lpMMI->ptMaxSize.y);
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void MainFrame::UpdateUI()
{
	MutexLock lock(m_tabsMutex);

	if( g_settingsHandler->GetBehaviorSettings().closeSettings.bAllowClosingLastView )
	{
		UIEnable(ID_FILE_CLOSE_TAB, TRUE);
		UIEnable(ID_CLOSE_VIEW, TRUE);
	}
	else
	{
		UIEnable(ID_FILE_CLOSE_TAB, m_tabs.size() > 1);
		UIEnable(ID_CLOSE_VIEW, m_tabs.size() > 1 || (!m_tabs.empty() && m_tabs.begin()->second->GetViewsCount() > 1));
	}

	UIEnable(ID_FILE_CLOSE_ALL_TABS_BUT_THIS, m_tabs.size() > 1);
	UIEnable(ID_FILE_CLOSE_ALL_TABS_LEFT, m_TabCtrl.GetCurSel() > 0);
	UIEnable(ID_FILE_CLOSE_ALL_TABS_RIGHT, m_TabCtrl.GetCurSel() < (m_TabCtrl.GetItemCount() - 1));
}

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
static void ParseCommandLine
(
	LPCTSTR lptstrCmdLine,
	wstring& strWindowTitle,
	vector<wstring>& startupTabs,
	vector<wstring>& startupDirs,
	vector<wstring>& startupCmds,
	int& nMultiStartSleep
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
, m_activeTabView()
, m_bMenuVisible(TRUE)
, m_bToolbarVisible(TRUE)
, m_bStatusBarVisible(TRUE)
, m_bTabsVisible(TRUE)
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
{
	m_Margins.cxLeftWidth    = 0;
	m_Margins.cxRightWidth   = 0;
	m_Margins.cyTopHeight    = 0;
	m_Margins.cyBottomHeight = 0;

	wstring strWindowTitle(L"");

	ParseCommandLine(
		lpstrCmdLine,
		strWindowTitle,
		m_startupTabs,
		m_startupDirs,
		m_startupCmds,
		m_nMultiStartSleep);
	m_strCmdLineWindowTitle = strWindowTitle.c_str();
	m_strWindowTitle = strWindowTitle.c_str();
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
	int nMultiStartSleep
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

		bAtLeastOneStarted = CreateNewConsole(0, strStartupDir, strStartupCmd);
	}
	else
	{
		TabSettings&	tabSettings = g_settingsHandler->GetTabSettings();

		for (size_t tabIndex = 0; tabIndex < startupTabs.size(); ++tabIndex)
		{
			// find tab with corresponding name...
			for (size_t i = 0; i < tabSettings.tabDataVector.size(); ++i)
			{
				wstring str = tabSettings.tabDataVector[i]->strTitle;
				if (tabSettings.tabDataVector[i]->strTitle == startupTabs[tabIndex])
				{
					// -ts Specifies sleep time between starting next tab if multiple -t's are specified.
					if (bAtLeastOneStarted) ::Sleep(nMultiStartSleep);
					// found it, create
					if (CreateNewConsole(
						static_cast<DWORD>(i), 
						startupDirs[tabIndex],
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

	// initialize tabs
	UpdateTabsMenu(m_CmdBar.GetMenu(), m_tabsMenu);
	SetReflectNotifications(true);
//	SetTabStyles(CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_SCROLL | CTCS_CLOSEBUTTON | CTCS_BOLDSELECTEDTAB);

	DWORD dwTabStyles = CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_SCROLL | CTCS_CLOSEBUTTON | CTCS_HOTTRACK;
	if (controlsSettings.bTabsOnBottom) dwTabStyles |= CTCS_BOTTOM;

	CreateTabWindow(m_hWnd, rcDefault, dwTabStyles);

	if (LRESULT created = CreateInitialTabs(m_startupTabs, m_startupCmds, m_startupDirs, m_nMultiStartSleep))
		return created;

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_MENU, 1);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_TABS, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetBlockAccelerators(true);

	SetWindowStyles();

	ShowMenu(controlsSettings.bShowMenu ? TRUE : FALSE);
	ShowToolbar(controlsSettings.bShowToolbar ? TRUE : FALSE);
	ShowStatusbar(controlsSettings.bShowStatusbar ? TRUE : FALSE);
	ShowTabs(controlsSettings.bShowTabs ? TRUE : FALSE);

	{
		MutexLock lock(m_tabsMutex);
		if (m_tabs.size() == 1)
		{
			UIEnable(ID_FILE_CLOSE_TAB, FALSE);
		}
		if ((m_tabs.size() == 1) && m_bTabsVisible && (controlsSettings.bHideSingleTab))
		{
			ShowTabs(FALSE);
		}
	}

	DWORD dwFlags	= SWP_NOSIZE|SWP_NOZORDER;

	if ((!positionSettings.bSavePosition) && 
		(positionSettings.nX == -1) || (positionSettings.nY == -1))
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

	if (m_strCmdLineWindowTitle.GetLength() == 0)
	{
		m_strWindowTitle = g_settingsHandler->GetAppearanceSettings().windowSettings.strTitle.c_str();
	}
	SetWindowText(m_strWindowTitle);

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

	if (positionSettings.bSavePosition)
	{
		CRect rectWindow;

		GetWindowRect(rectWindow);

		positionSettings.nX	= rectWindow.left;
		positionSettings.nY	= rectWindow.top;

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

	// we're being called while OnCreate is running, return here
	if (!m_bOnCreateDone)
	{
		bHandled = FALSE;
		return 0;
	}

	bHandled = FALSE;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowHideWindow(void)
{
  bool bQuake = g_settingsHandler->GetAppearanceSettings().stylesSettings.bQuake;
  bool bActivate = true;

  DWORD dwActivateFlags = AW_ACTIVATE | AW_SLIDE;
  DWORD dwHideFlags     = AW_HIDE     | AW_SLIDE;

  if( bQuake )
  {
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
  }

  if( bQuake )
  {
    if(!::IsWindowVisible(m_hWnd))
    {
      ::AnimateWindow(m_hWnd, 300, dwActivateFlags);
    }
    else if(m_bAppActive)
    {
      ::AnimateWindow(m_hWnd, 300, dwHideFlags);
      bActivate = false;
    }
  }
  else
  {
    ShowWindow(this->IsIconic()?SW_RESTORE:SW_SHOW);
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

//	TRACE(L"OnSysCommand: 0x%08X\n", wParam);

	// OnSize needs to know this
	if ((wParam & 0xFFF0) == SC_RESTORE)
	{
		m_bRestoringWindow = true;
	}
	else if ((wParam & 0xFFF0) == SC_MAXIMIZE)
	{
		GetWindowRect(&m_rectRestoredWnd);
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
		// do nothing for minimized or maximized windows
		if (IsIconic() || IsZoomed()) return 0;

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
    if( it->second->CloseView(reinterpret_cast<HWND>(wParam)) )
      break;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnUpdateTitles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /* bHandled */)
{
	MutexLock					viewMapLock(m_tabsMutex);
	TabViewMap::iterator	itView = m_tabs.find(reinterpret_cast<HWND>(wParam));

	if (itView == m_tabs.end()) return 0;
  std::shared_ptr<TabView>	tabView(itView->second);
  std::shared_ptr<ConsoleView> consoleView = itView->second->GetActiveConsole(_T(__FUNCTION__));
  if (!consoleView) return 0;

	WindowSettings&			windowSettings	= g_settingsHandler->GetAppearanceSettings().windowSettings;

	if (windowSettings.bUseConsoleTitle)
	{
		CString	strTabTitle(consoleView->GetTitle());

		UpdateTabTitle(*tabView, strTabTitle);

		if ((m_strCmdLineWindowTitle.GetLength() == 0) &&
			(windowSettings.bUseTabTitles) && 
			(tabView == m_activeTabView))
		{
			m_strWindowTitle = strTabTitle;
			SetWindowText(m_strWindowTitle);
			if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon) SetTrayIcon(NIM_MODIFY);
		}
	}
	else
	{
		CString	strCommandText(consoleView->GetConsoleCommand());
		CString	strTabTitle(consoleView->GetTitle());

		if (m_strCmdLineWindowTitle.GetLength() != 0)
		{
			m_strWindowTitle = m_strCmdLineWindowTitle;
		}
		else
		{
			m_strWindowTitle = windowSettings.strTitle.c_str();
		}

		if (tabView == m_activeTabView)
		{
			if ((m_strCmdLineWindowTitle.GetLength() == 0) && (windowSettings.bUseTabTitles))
			{
				m_strWindowTitle = strTabTitle;
			}

			if (windowSettings.bShowCommand)	m_strWindowTitle += strCommandText;

			SetWindowText(m_strWindowTitle);
			if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon) SetTrayIcon(NIM_MODIFY);
		}
		
		if (windowSettings.bShowCommandInTabs) strTabTitle += strCommandText;

		UpdateTabTitle(*tabView, strTabTitle);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnShowPopupMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint	point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	
	CMenu		contextMenu;
	CMenu		tabsMenu;
	CMenuHandle	popupMenu;

	contextMenu.LoadMenu(IDR_POPUP_MENU_TAB);
	popupMenu = contextMenu.GetSubMenu(0);
	
	UpdateTabsMenu(popupMenu, tabsMenu);
	popupMenu.TrackPopupMenu(0, point.x, point.y, m_hWnd);

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

			CMenu		contextMenu;
			CMenu		tabsMenu;
			CMenuHandle	popupMenu;

			contextMenu.LoadMenu(IDR_POPUP_MENU_TAB);
			popupMenu = contextMenu.GetSubMenu(0);
			
			UpdateTabsMenu(popupMenu, tabsMenu);
			popupMenu.TrackPopupMenu(0, posCursor.x, posCursor.y, m_hWnd);

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

	if (appearanceSettings.windowSettings.bUseTabTitles && m_activeTabView)
	{
    std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
    if( activeConsoleView )
    {
		  SetWindowText(activeConsoleView->GetTitle());
    }
	}

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
	NMCTCITEM*			pTabItems	= reinterpret_cast<NMCTCITEM*>(pnmh);
	CTabViewTabItem*	pTabItem	= (pTabItems->iItem != 0xFFFFFFFF) ? m_TabCtrl.GetItem(pTabItems->iItem) : NULL;

	if (pTabItem == NULL)
	{

		// I prefer choose my console with the good environment ...
		// CreateNewConsole(0);

		if (!m_tabsMenu.IsNull())
		{
			CPoint point(pTabItems->pt.x, pTabItems->pt.y);
			CPoint screenPoint(point);
			this->m_TabCtrl.ClientToScreen(&screenPoint);
			m_tabsMenu.TrackPopupMenu(0, screenPoint.x, screenPoint.y, m_hWnd);
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

	m_tabsMenu.TrackPopupMenu(0, buttonRect.left, buttonRect.bottom, m_hWnd);
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

LRESULT MainFrame::OnFileCloseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CTabViewTabItem* pTabItem = m_TabCtrl.GetItem(m_TabCtrl.GetCurSel());
	
	CloseTab(pTabItem);
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

LRESULT MainFrame::OnNextView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if( m_activeTabView )
    m_activeTabView->NextView();
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnPrevView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if( m_activeTabView )
    m_activeTabView->PrevView();
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnCloseView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if( m_activeTabView )
    m_activeTabView->CloseView();
  ::SetForegroundWindow(m_hWnd);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSplitHorizontally(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if( m_activeTabView )
    m_activeTabView->SplitHorizontally();
  ::SetForegroundWindow(m_hWnd);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSplitVertically(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if( m_activeTabView )
    m_activeTabView->SplitVertically();
  ::SetForegroundWindow(m_hWnd);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OngroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT MainFrame::OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  PasteToConsoles();
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
	
		UpdateTabsMenu(m_CmdBar.GetMenu(), m_tabsMenu);

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

		ShowMenu(controlsSettings.bShowMenu ? TRUE : FALSE);
		ShowToolbar(controlsSettings.bShowToolbar ? TRUE : FALSE);

		BOOL bShowTabs = FALSE;

		MutexLock	viewMapLock(m_tabsMutex);
		
		if ( controlsSettings.bShowTabs && 
			(!controlsSettings.bHideSingleTab || (m_tabs.size() > 1))
		   )
		{
			bShowTabs = TRUE;
		}

		ShowTabs(bShowTabs);

		ShowStatusbar(controlsSettings.bShowStatusbar ? TRUE : FALSE);

		SetZOrder(g_settingsHandler->GetAppearanceSettings().positionSettings.zOrder);

		for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
		{
			it->second->InitializeScrollbars();
		}

    ConsoleView::RecreateFont();
		AdjustWindowSize(ADJUSTSIZE_WINDOW);
	}

	RegisterGlobalHotkeys();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowMenu(!m_bMenuVisible);
	g_settingsHandler->SaveSettings();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowToolbar(!m_bToolbarVisible);
	g_settingsHandler->SaveSettings();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowStatusbar(!m_bStatusBarVisible);
	g_settingsHandler->SaveSettings();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowTabs(!m_bTabsVisible);
	g_settingsHandler->SaveSettings();
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

	AddTabWithIcon(hwndTabView, strTabTitle, tabView->GetIcon(false));
	DisplayTab(hwndTabView, FALSE);
	::SetForegroundWindow(m_hWnd);

	if (m_tabs.size() > 1)
	{
    CRect clientRect(0, 0, 0, 0);
    tabView->AdjustRectAndResize(ADJUSTSIZE_WINDOW, clientRect, WMSZ_BOTTOM);
		UIEnable(ID_FILE_CLOSE_TAB, TRUE);
	}
	if ( g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowTabs &&
		((m_tabs.size() > 1) || (!g_settingsHandler->GetAppearanceSettings().controlsSettings.bHideSingleTab))
	   )
	{
		ShowTabs(TRUE);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CloseTab(CTabViewTabItem* pTabItem)
{
  MutexLock viewMapLock(m_tabsMutex);
  if (!pTabItem) return;
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

  if (m_tabs.size() == 1)
  {
    UIEnable(ID_FILE_CLOSE_TAB, FALSE);
  }
  if ((m_tabs.size() == 1) &&
    m_bTabsVisible && 
    (g_settingsHandler->GetAppearanceSettings().controlsSettings.bHideSingleTab))
  {
    ShowTabs(FALSE);
  }

  if (m_tabs.size() == 0) PostMessage(WM_CLOSE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::UpdateTabTitle(HWND hwndTabView, CString& strTabTitle)
{
	// we always set the tool tip text to the complete, untrimmed title
	UpdateTabToolTip(hwndTabView, strTabTitle);

	WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;

	if 
	(
		(windowSettings.dwTrimTabTitles > 0) 
		&& 
		(windowSettings.dwTrimTabTitles > windowSettings.dwTrimTabTitlesRight) 
		&& 
		(strTabTitle.GetLength() > static_cast<int>(windowSettings.dwTrimTabTitles))
	)
	{
		strTabTitle = strTabTitle.Left(windowSettings.dwTrimTabTitles - windowSettings.dwTrimTabTitlesRight) + CString(L"...") + strTabTitle.Right(windowSettings.dwTrimTabTitlesRight);
	}
	
	UpdateTabText(hwndTabView, strTabTitle);
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

		// create menu icons with proper transparency (thanks to chrisz for the patch)
		CIcon tabSmallIcon(Helpers::LoadTabIcon(false, (*it)->bUseDefaultIcon, (*it)->strIcon, (*it)->strShell));

		// destroy icon bitmap
		if (!(*it)->menuBitmap.IsNull()) (*it)->menuBitmap.DeleteObject();

		if (!tabSmallIcon.IsNull())
		{
			CDC	dc;

			(*it)->menuBitmap.CreateCompatibleBitmap(::GetDC(NULL), 16, 16);

			dc.CreateCompatibleDC(::GetDC(NULL));
			dc.SelectBitmap((*it)->menuBitmap);
			dc.FillSolidRect(0, 0, 16, 16, ::GetSysColor(COLOR_MENU));
			dc.DrawIconEx(0, 0, tabSmallIcon.m_hIcon, 16, 16);
		}

		if (!(*it)->menuBitmap.IsNull()) 
		{
			tabsMenu.SetMenuItemBitmaps(wId, MF_BYCOMMAND, (*it)->menuBitmap, NULL);
		}
	}

	// set tabs menu as popup submenu
	if (!mainMenu.IsNull())
	{
		CMenuItemInfo	menuItem;

		menuItem.fMask		= MIIM_SUBMENU;
		menuItem.hSubMenu	= HMENU(tabsMenu);

		mainMenu.SetMenuItemInfo(ID_FILE_NEW_TAB, FALSE, &menuItem);
	}

  // create jumplist
  JumpList::CreateList(g_settingsHandler->GetTabSettings().tabDataVector);
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

  if (m_activeTabView)
  {
    std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
    if( activeConsoleView )
    {
      SharedMemory<ConsoleParams>& consoleParams = activeConsoleView->GetConsoleHandler().GetConsoleParams();

      DWORD dwSelectionSize = activeConsoleView->GetSelectionSize();
      if( dwSelectionSize )
        _snwprintf_s(strSelection, ARRAYSIZE(strSelection), _TRUNCATE, L"%lu", dwSelectionSize);

      _snwprintf_s(strColsRows,    ARRAYSIZE(strColsRows),    _TRUNCATE, L"%lux%lu",
        consoleParams->dwColumns,
        consoleParams->dwRows);
      _snwprintf_s(strPid,         ARRAYSIZE(strPid),         _TRUNCATE, L"%lu",
        activeConsoleView->GetConsoleHandler().GetConsolePid());
      _snwprintf_s(strBufColsRows, ARRAYSIZE(strBufColsRows), _TRUNCATE, L"%lux%lu",
        consoleParams->dwBufferColumns ? consoleParams->dwBufferColumns : consoleParams->dwColumns,
        consoleParams->dwBufferRows ? consoleParams->dwBufferRows : consoleParams->dwRows);

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

  UIUpdateStatusBar();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::SetWindowStyles()
{
	StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

	DWORD	dwStyle		= GetWindowLong(GWL_STYLE);
	DWORD	dwExStyle	= GetWindowLong(GWL_EXSTYLE);

	if (!stylesSettings.bResizable)	dwStyle &= ~WS_MAXIMIZEBOX;
	if (!stylesSettings.bCaption)	dwStyle &= ~WS_CAPTION;
	if (!stylesSettings.bResizable)	dwStyle &= ~WS_THICKFRAME;

	if (!stylesSettings.bTaskbarButton)
	{
		if (!stylesSettings.bTrayIcon)
		{
			// remove minimize button
			dwStyle &= ~WS_MINIMIZEBOX;
		}
		dwExStyle &= ~WS_EX_APPWINDOW;
	}

	if (stylesSettings.bBorder) dwStyle |= WS_BORDER;

	SetWindowLong(GWL_STYLE, dwStyle);
	SetWindowLong(GWL_EXSTYLE, dwExStyle);
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
	SetParent((m_zOrder == zorderDesktop) ? GetDesktopWindow() : NULL);
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

void MainFrame::ShowMenu(BOOL bShow)
{
	m_bMenuVisible = bShow;

	CReBarCtrl rebar(m_hWndToolBar);
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST);	// menu is 1st added band
	rebar.ShowBand(nBandIndex, m_bMenuVisible);
	UISetCheck(ID_VIEW_MENU, m_bMenuVisible);

	g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowMenu = m_bMenuVisible ? true : false;

	UpdateLayout();
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	DockWindow(m_dockPosition);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowToolbar(BOOL bShow)
{
	m_bToolbarVisible = bShow;

	CReBarCtrl rebar(m_hWndToolBar);
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, m_bToolbarVisible);
	UISetCheck(ID_VIEW_TOOLBAR, m_bToolbarVisible);

	g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowToolbar = m_bToolbarVisible? true : false;

	UpdateLayout();
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	DockWindow(m_dockPosition);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowStatusbar(BOOL bShow)
{
	m_bStatusBarVisible = bShow;

	::ShowWindow(m_hWndStatusBar, m_bStatusBarVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, m_bStatusBarVisible);

	g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowStatusbar = m_bStatusBarVisible? true : false;
	
	UpdateLayout();
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	DockWindow(m_dockPosition);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::ShowTabs(BOOL bShow)
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

	ControlsSettings& controlsSettings = g_settingsHandler->GetAppearanceSettings().controlsSettings;

	if (!controlsSettings.bHideSingleTab)
	{
		controlsSettings.bShowTabs = m_bTabsVisible ? true : false;
	}

	UpdateLayout();
	AdjustWindowSize(ADJUSTSIZE_WINDOW);
	DockWindow(m_dockPosition);
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
  if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
  {
    m_Margins.cyTopHeight = rebar.GetBarHeight();
    if( m_bTabsVisible )
    {
      m_Margins.cyBottomHeight = m_nTabAreaHeight;
      if (m_bStatusBarVisible)
      {
        CRect rectStatusBar(0, 0, 0, 0);
        ::GetWindowRect(m_hWndStatusBar, &rectStatusBar);
        m_Margins.cyBottomHeight += rectStatusBar.Height();
      }
    }
    else
    {
      m_Margins.cyBottomHeight = 0;
    }
  }
  else
  {
    m_Margins.cyTopHeight = rebar.GetBarHeight() + m_nTabAreaHeight;
    m_Margins.cyBottomHeight = 0;
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
  StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

  BOOL fEnabled = FALSE;
  DwmIsCompositionEnabled(&fEnabled);
  if( fEnabled )
  {
    if( transparencySettings.transType != transGlass )
    {
      // there is a side effect whith glass into client area and no caption (and no resizable)
      // blur is not applied, the window is transparent ...
      if( !stylesSettings.bCaption && !stylesSettings.bResizable )
      {
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
        if( !stylesSettings.bCaption && !stylesSettings.bResizable )
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

		if ((*it)->accelHotkey.fVirt & FSHIFT)	uiModifiers |= MOD_SHIFT;
		if ((*it)->accelHotkey.fVirt & FCONTROL)uiModifiers |= MOD_CONTROL;
		if ((*it)->accelHotkey.fVirt & FALT)	uiModifiers |= MOD_ALT;

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

	int arrPanes[]	= { ID_DEFAULT_PANE, IDPANE_CAPS_INDICATOR, IDPANE_NUM_INDICATOR, IDPANE_SCRL_INDICATOR, IDPANE_PID_INDICATOR, IDPANE_SELECTION, IDPANE_COLUMNS_ROWS, IDPANE_BUF_COLUMNS_ROWS};

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

void MainFrame::PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam)
{
  MutexLock lock(m_tabsMutex);
  for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
  {
    it->second->PostMessageToConsoles(Msg, wParam, lParam);
  }
}

void MainFrame::PasteToConsoles()
{
  if (!m_activeTabView) return;
  std::shared_ptr<ConsoleView> activeConsoleView = m_activeTabView->GetActiveConsole(_T(__FUNCTION__));
  if( activeConsoleView )
  {
    if( activeConsoleView->IsGrouped() )
    {
      MutexLock lock(m_tabsMutex);
      for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
      {
        it->second->PasteToConsoles();
      }
    }
    else
      activeConsoleView->Paste();
  }
}

void MainFrame::SendTextToConsole(const wchar_t* pszText)
{
  MutexLock lock(m_tabsMutex);
  for (TabViewMap::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
  {
    it->second->SendTextToConsole(pszText);
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
	int nMultiStartSleep;

	wstring ignoreTitle;

	ParseCommandLine((LPCTSTR)cds->lpData, ignoreTitle, startupTabs, startupDirs, startupCmds, nMultiStartSleep);
	CreateInitialTabs(startupTabs, startupCmds, startupDirs, nMultiStartSleep);

	return 0;
}

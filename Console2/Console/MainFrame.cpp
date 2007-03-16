#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "Console.h"
#include "ConsoleView.h"
#include "DlgRenameTab.h"
#include "DlgSettingsMain.h"
#include "MainFrame.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

MainFrame::MainFrame(const vector<wstring>& startupTabs, const vector<wstring>& startupDirs, const vector<wstring>& startupCmds, int nMultiStartSleep, const wstring& strDbgCmdLine)
: m_startupTabs(startupTabs)
, m_startupDirs(startupDirs)
, m_startupCmds(startupCmds)
, m_nMultiStartSleep(nMultiStartSleep)
, m_strDbgCmdLine(strDbgCmdLine)
, m_activeView()
, m_bMenuVisible(TRUE)
, m_bToolbarVisible(TRUE)
, m_bStatusBarVisible(TRUE)
, m_bTabsVisible(TRUE)
, m_dockPosition(dockNone)
, m_zOrder(zorderNormal)
, m_mousedragOffset(0, 0)
, m_mapViews()
, m_strWindowTitle(L"")
, m_dwRows(0)
, m_dwColumns(0)
, m_dwWindowWidth(0)
, m_dwWindowHeight(0)
, m_dwResizeWindowEdge(WMSZ_BOTTOM)
, m_bRestoringWindow(false)
, m_rectRestoredWnd(0, 0, 0, 0)
{

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

	if (m_activeView.get() == NULL) return FALSE;

	return m_activeView->PreTranslateMessage(pMsg);
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

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	TBBUTTONINFO tbi;
	m_toolbar.Attach(hWndToolBar);
	m_toolbar.SendMessage(TB_SETEXTENDEDSTYLE, 0, static_cast<WPARAM>(TBSTYLE_EX_DRAWDDARROWS));

	tbi.dwMask	= TBIF_STYLE;
	tbi.cbSize	= sizeof(TBBUTTONINFO);
	
	m_toolbar.GetButtonInfo(ID_FILE_NEW_TAB, &tbi);

	tbi.fsStyle |= TBSTYLE_DROPDOWN;
	m_toolbar.SetButtonInfo(ID_FILE_NEW_TAB, &tbi);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateStatusBar();

	// initialize tabs
	UpdateTabsMenu(m_CmdBar.GetMenu(), m_tabsMenu);
	SetReflectNotifications(true);
//	SetTabStyles(CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_SCROLL | CTCS_CLOSEBUTTON | CTCS_BOLDSELECTEDTAB);
	CreateTabWindow(m_hWnd, rcDefault, CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_SCROLL | CTCS_CLOSEBUTTON | CTCS_BOLDSELECTEDTAB);

	// create initial console window(s)
	if (m_startupTabs.size() == 0)
	{
		wstring strStartupDir(L"");
		wstring strStartupCmd(L"");

		if (m_startupDirs.size() > 0) strStartupDir = m_startupDirs[0];
		if (m_startupCmds.size() > 0) strStartupCmd = m_startupCmds[0];

		if (!CreateNewConsole(0, strStartupDir, strStartupCmd, m_strDbgCmdLine)) return -1;
	}
	else
	{
		bool			bAtLeastOneStarted = false;
		TabSettings&	tabSettings = g_settingsHandler->GetTabSettings();

		for (size_t tabIndex = 0; tabIndex < m_startupTabs.size(); ++tabIndex)
		{
			// find tab with corresponding name...
			for (size_t i = 0; i < tabSettings.tabDataVector.size(); ++i)
			{
				wstring str = tabSettings.tabDataVector[i]->strTitle;
				if (tabSettings.tabDataVector[i]->strTitle == m_startupTabs[tabIndex])
				{
					// found it, create
					if (CreateNewConsole(
							static_cast<DWORD>(i), 
							m_startupDirs[tabIndex],
							m_startupCmds[tabIndex],
							(i == 0) ? m_strDbgCmdLine : wstring(L"")))
					{
						bAtLeastOneStarted = true;
					}
					if (m_startupTabs.size() > 1) ::Sleep(m_nMultiStartSleep);
					break;
				}
			}
		}

		// could not start none of the startup tabs, exit
		if (!bAtLeastOneStarted) return -1;
	}

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_MENU, 1);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_TABS, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	SetWindowStyles();

	ControlsSettings&	controlsSettings= g_settingsHandler->GetAppearanceSettings().controlsSettings;
	PositionSettings&	positionSettings= g_settingsHandler->GetAppearanceSettings().positionSettings;

	ShowMenu(controlsSettings.bShowMenu ? TRUE : FALSE);
	ShowToolbar(controlsSettings.bShowToolbar ? TRUE : FALSE);
	ShowStatusbar(controlsSettings.bShowStatusbar ? TRUE : FALSE);
	ShowTabs(controlsSettings.bShowTabs ? TRUE : FALSE);

	if ((m_mapViews.size() == 1) && m_bTabsVisible && (controlsSettings.bHideSingleTab))
	{
		ShowTabs(FALSE);
	}

	DWORD dwFlags	= SWP_NOSIZE|SWP_NOZORDER;

	if ((!positionSettings.bSavePosition) && 
		(positionSettings.nX == -1) || (positionSettings.nY == -1))
	{
		dwFlags |= SWP_NOMOVE;
	}

	SetWindowPos(NULL, positionSettings.nX, positionSettings.nY, 0, 0, dwFlags);
	DockWindow(positionSettings.dockPosition);
	SetZOrder(positionSettings.zOrder);

	m_strWindowTitle = g_settingsHandler->GetAppearanceSettings().windowSettings.strTitle.c_str();
	SetWindowText(m_strWindowTitle);

	SetWindowIcons();

	CreateAcceleratorTable();
	SetTransparency();
	if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon) SetTrayIcon(NIM_ADD);

	AdjustWindowSize(false);

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

LRESULT MainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// save settings on exit
	bool				bSaveSettings		= false;
	ConsoleSettings&	consoleSettings		= g_settingsHandler->GetConsoleSettings();
	PositionSettings&	positionSettings	= g_settingsHandler->GetAppearanceSettings().positionSettings;

	if (consoleSettings.bSaveSize)
	{
		consoleSettings.dwRows		= m_dwRows;
		consoleSettings.dwColumns	= m_dwColumns;

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
	for (ConsoleViewMap::iterator it = m_mapViews.begin(); it != m_mapViews.end(); ++it)
	{
		RemoveTab(it->second->m_hWnd);
		if (m_activeView.get() == it->second.get()) m_activeView.reset();
		it->second->DestroyWindow();
	}

	if (g_settingsHandler->GetAppearanceSettings().stylesSettings.bTrayIcon) SetTrayIcon(NIM_DELETE);

	bHandled = false;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnActivateApp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (m_activeView.get() == NULL) return 0;

	m_activeView->SetAppActiveStatus(static_cast<BOOL>(wParam) == TRUE);

	TransparencySettings& transparencySettings = g_settingsHandler->GetAppearanceSettings().transparencySettings;

	if ((transparencySettings.transType == transAlpha) && 
		((transparencySettings.byActiveAlpha != 255) || (transparencySettings.byInactiveAlpha != 255)))
	{
		if (static_cast<BOOL>(wParam))
		{
			::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), transparencySettings.byActiveAlpha, LWA_ALPHA);
		}
		else
		{
			::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), transparencySettings.byInactiveAlpha, LWA_ALPHA);
		}
		
	}

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

LRESULT MainFrame::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;

	CRect					maxClientRect;

	if ((m_activeView.get() == NULL) || (!m_activeView->GetMaxRect(maxClientRect)))
	{
		bHandled = false;
		return 1;
	}

	TRACE(L"minmax: %ix%i\n", maxClientRect.Width(), maxClientRect.Height());

	AdjustWindowRect(maxClientRect);

	TRACE(L"minmax: %ix%i\n", maxClientRect.Width(), maxClientRect.Height());

	pMinMax->ptMaxSize.x = maxClientRect.Width();
	pMinMax->ptMaxSize.y = maxClientRect.Height() + 4;

	pMinMax->ptMaxTrackSize.x = pMinMax->ptMaxSize.x;
	pMinMax->ptMaxTrackSize.y = pMinMax->ptMaxSize.y;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == SIZE_MAXIMIZED)
	{
		PostMessage(WM_EXITSIZEMOVE, 1, 0);
	}
	else if (m_bRestoringWindow && (wParam == SIZE_RESTORED))
	{
		m_bRestoringWindow = false;
		PostMessage(WM_EXITSIZEMOVE, 1, 0);
/*
		CRect rectWindow;
		GetWindowRect(&rectWindow);

		DWORD dwWindowWidth	= LOWORD(lParam);
		DWORD dwWindowHeight= HIWORD(lParam);

		if ((dwWindowWidth != m_dwWindowWidth) ||
			(dwWindowHeight != m_dwWindowHeight))
		{
//			AdjustWindowSize(true, (wParam == SIZE_MAXIMIZED));

			CRect clientRect;
			GetClientRect(&clientRect);
			AdjustAndResizeConsoleView(clientRect);
			AdjustWindowRect(clientRect);
		}
*/
	}

// 	CRect rectWindow;
// 	GetWindowRect(&rectWindow);
// 
// 	TRACE(L"OnSize dims: %ix%i\n", rectWindow.Width(), rectWindow.Height());


	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSizing(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_dwResizeWindowEdge = static_cast<DWORD>(wParam);
	return 0;
}

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
		// do nothing for maximized windows
		if (IsZoomed()) return 0;

		m_dockPosition	= dockNone;
		
		if (positionSettings.nSnapDistance >= 0)
		{
			CRect	rectMonitor;
			CRect	rectDesktop;
			CRect	rectWindow;
			CPoint	pointCursor;

			// we'll snap Console window to the desktop edges
			::GetCursorPos(&pointCursor);
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


		// only for relative backgrounds
		if (m_activeView.get() != NULL)
		{
			shared_ptr<TabData> tabData = m_activeView->GetTabData();

			if (tabData->imageData.bRelative)
			{
				CRect rectClient;
				GetClientRect(&rectClient);
				InvalidateRect(&rectClient, FALSE);
			}
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

LRESULT MainFrame::OnExitSizeMove(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CRect rectWindow;
	GetWindowRect(&rectWindow);

	CRect rectClient;
	GetClientRect(&rectClient);

	DWORD dwWindowWidth	= rectWindow.Width();
	DWORD dwWindowHeight= rectWindow.Height();

	TRACE(L"old dims: %ix%i\n", m_dwWindowWidth, m_dwWindowHeight);
	TRACE(L"new dims: %ix%i\n", dwWindowWidth, dwWindowHeight);
	TRACE(L"client dims: %ix%i\n", rectClient.Width(), rectClient.Height());

	if ((dwWindowWidth != m_dwWindowWidth) ||
		(dwWindowHeight != m_dwWindowHeight))
	{
		AdjustWindowSize(true, false);
	}

	SendMessage(WM_NULL, 0, 0);
	m_dwResizeWindowEdge = WMSZ_BOTTOM;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	g_imageHandler->ReloadDesktopImages();
	m_activeView->Invalidate();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnConsoleResized(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */)
{
	// update rows/columns
	SharedMemory<ConsoleParams>& consoleParams = m_activeView->GetConsoleHandler().GetConsoleParams();
	m_dwRows	= consoleParams->dwRows;
	m_dwColumns	= consoleParams->dwColumns;

	AdjustWindowSize(false);
	UpdateStatusBar();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnConsoleClosed(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /* bHandled */)
{
	CloseTab(reinterpret_cast<HWND>(wParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnUpdateTitles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /* bHandled */)
{
	shared_ptr<ConsoleView>	consoleView(m_mapViews.find(reinterpret_cast<HWND>(wParam))->second);
	WindowSettings&			windowSettings	= g_settingsHandler->GetAppearanceSettings().windowSettings;

	if (windowSettings.bUseConsoleTitle)
	{
		CString	strTabTitle(consoleView->GetTitle());

		if ((windowSettings.dwTrimTabTitles > 0) && (strTabTitle.GetLength() > static_cast<int>(windowSettings.dwTrimTabTitles)))
		{
			strTabTitle = strTabTitle.Left(windowSettings.dwTrimTabTitles) + CString(L"...");
		}
		UpdateTabText(consoleView->m_hWnd, strTabTitle);

		if ((windowSettings.bUseTabTitles) && (consoleView == m_activeView))
		{
			m_strWindowTitle = consoleView->GetTitle();
			SetWindowText(m_strWindowTitle);
			SetTrayIcon(NIM_MODIFY);
		}
	}
	else
	{
		CString	strCommandText(consoleView->GetConsoleCommand());
		CString	strTabTitle(consoleView->GetTitle());

		m_strWindowTitle = windowSettings.strTitle.c_str();

		if (consoleView == m_activeView)
		{
			if (windowSettings.bUseTabTitles)	m_strWindowTitle = strTabTitle;
			if (windowSettings.bShowCommand)	m_strWindowTitle += strCommandText;

			SetWindowText(m_strWindowTitle);
			SetTrayIcon(NIM_MODIFY);
		}
		
		if (windowSettings.bShowCommandInTabs) strTabTitle += strCommandText;

		if ((windowSettings.dwTrimTabTitles > 0) && (strTabTitle.GetLength() > static_cast<int>(windowSettings.dwTrimTabTitles)))
		{
			strTabTitle = strTabTitle.Left(windowSettings.dwTrimTabTitles) + CString(L"...");
		}
		UpdateTabText(consoleView->m_hWnd, strTabTitle);
	}

/*
	if ((windowSettings.bUseTabTitles) && (consoleView == m_activeView->m_hWnd))
	{
		m_strWindowTitle = strConsoleTitle;
		SetWindowText(m_strWindowTitle);
		SetTrayIcon(NIM_MODIFY);
	}
*/

/*
	TRACE(L"Boink: %s\n", strConsoleTitle);
	UpdateTabText(consoleView, strConsoleTitle);
*/

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnShowPopupMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	POINT	point;
	point.x = GET_X_LPARAM(lParam);
	point.y = GET_Y_LPARAM(lParam);
	
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
	CPoint	point(LOWORD(lParam), HIWORD(lParam));
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
			//if (m_bPopupMenuDisabled) return 0;

			POINT	posCursor;
			
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
			// TODO: handle
//			m_bHideWindow = false;
//			ShowHideWindow();
			::SetForegroundWindow(m_hWnd);
			return 0;
		}
			
		case WM_LBUTTONDBLCLK :
		{
			// TODO: handle
//			m_bHideWindow = !m_bHideWindow;
//			ShowHideWindow();
//			::SetForegroundWindow(m_hWnd);
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

	ConsoleViewMap::iterator	it;

	if (pTabItem1)
	{
		it = m_mapViews.find(pTabItem1->GetTabView());
		if (it != m_mapViews.end())
		{
			it->second->SetActive(false);
		}
	}

	if (pTabItem2)
	{
		it = m_mapViews.find(pTabItem2->GetTabView());
		if (it != m_mapViews.end())
		{
			UISetCheck(ID_VIEW_CONSOLE, it->second->GetConsoleWindowVisible() ? TRUE : FALSE);
			m_activeView = it->second;
			it->second->SetActive(true);

			if (appearanceSettings.windowSettings.bUseTabIcon) SetWindowIcons();

		}
		else
		{
			m_activeView = shared_ptr<ConsoleView>();
		}
	}

	if (appearanceSettings.stylesSettings.bTrayIcon) SetTrayIcon(NIM_MODIFY);
	
	if (appearanceSettings.windowSettings.bUseTabTitles && (m_activeView.get() != NULL))
	{
		SetWindowText(m_activeView->GetTitle());
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
	NMCTC2ITEMS*		pTabItems	= reinterpret_cast<NMCTC2ITEMS*>(pnmh);
	CTabViewTabItem*	pTabItem	= (pTabItems->iItem1 != 0xFFFFFFFF) ? m_TabCtrl.GetItem(pTabItems->iItem1) : NULL;

	CloseTab(pTabItem);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnRebarHeightChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
	AdjustWindowSize(false);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnToolbarDropDown(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
	POINT	cursorPos;
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

LRESULT MainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_activeView.get() == NULL) return 0;

	m_activeView->Copy();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_activeView.get() == NULL) return 0;

	m_activeView->Paste();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_activeView.get() == NULL) return 0;

	m_activeView->Copy();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_activeView.get() == NULL) return 0;

	m_activeView->Paste();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditRenameTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_activeView.get() == NULL) return 0;

	DlgRenameTab dlg(m_activeView->GetTitle());

	if (dlg.DoModal() == IDOK)
	{
		WindowSettings&			windowSettings	= g_settingsHandler->GetAppearanceSettings().windowSettings;
	
		m_activeView->SetTitle(dlg.m_strTabName);

		CString	strTabTitle(dlg.m_strTabName);

		if (windowSettings.bShowCommandInTabs) strTabTitle += m_activeView->GetConsoleCommand();

		if ((windowSettings.dwTrimTabTitles > 0) && (strTabTitle.GetLength() > static_cast<int>(windowSettings.dwTrimTabTitles)))
		{
			strTabTitle = strTabTitle.Left(windowSettings.dwTrimTabTitles) + CString(L"...");
		}
		UpdateTabText(*m_activeView, strTabTitle);

		if (windowSettings.bUseTabTitles) SetWindowText(m_activeView->GetTitle());
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_activeView.get() == NULL) return 0;

	DlgSettingsMain dlg;

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

		if ( controlsSettings.bShowTabs && 
			(!controlsSettings.bHideSingleTab || (m_mapViews.size() > 1))
		   )
		{
			bShowTabs = TRUE;
		}

		ShowTabs(bShowTabs);

		ShowStatusbar(controlsSettings.bShowStatusbar ? TRUE : FALSE);

		SetZOrder(g_settingsHandler->GetAppearanceSettings().positionSettings.zOrder);

		m_activeView->RecreateOffscreenBuffers();
		AdjustWindowSize(false);
		m_activeView->RepaintView();
	}

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
	if (m_activeView.get() != NULL)
	{
		m_activeView->SetConsoleWindowVisible(!m_activeView->GetConsoleWindowVisible());
		UISetCheck(ID_VIEW_CONSOLE, m_activeView->GetConsoleWindowVisible() ? TRUE : FALSE);
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
	if (m_activeView.get() == NULL) return 0;

	m_activeView->DumpBuffer();

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
/*

shared_ptr<ConsoleView> MainFrame::GetActiveView()
{
	if (m_mapViews.size() == 0) return shared_ptr<ConsoleView>();

	ConsoleViewMap::iterator	findIt		= m_mapViews.find(m_hWndActive);
	if (findIt == m_mapViews.end()) return shared_ptr<ConsoleView>();

	return findIt->second;
}

*/
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
//	rect.right	+= 0;

//	TRACE(L"AdjustWindowRect: %ix%i\n", rect.Width(), rect.Height());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

/*
void MainFrame::AdjustAndResizeConsoleView(CRect& rectView)
{
	// adjust the active view
//	if (m_activeView.get() == NULL) return;


//	GetClientRect(&rectView);

/ *
	if (m_bToolbarVisible)
	{

		CRect		rectToolBar(0, 0, 0, 0 );
		CRect		rectToolBarBorders(0, 0, 0, 0);
		CReBarCtrl	rebar(m_hWndToolBar);
		int			nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);

		rebar.GetRect(nBandIndex, &rectToolBar);
		rebar.GetBandBorders(nBandIndex, &rectToolBarBorders);
		rectView.bottom	-= rectToolBar.bottom - rectToolBar.top;

		rectView.bottom	-= rectToolBarBorders.top + rectToolBarBorders.bottom;
	}

	if (m_bStatusBarVisible)
	{
		CRect	rectStatusBar(0, 0, 0, 0);

		::GetWindowRect(m_hWndStatusBar, &rectStatusBar);
		rectView.bottom	-= rectStatusBar.bottom - rectStatusBar.top;
	}

	rectView.bottom	-= GetTabAreaHeight(); //+0
* /

	// adjust the active view
	if (m_activeView.get() == NULL) return;

	m_activeView->AdjustRectAndResize(rectView);
	
	// for other views, first set view size and then resize their Windows consoles
	for (ConsoleViewMap::iterator it = m_mapViews.begin(); it != m_mapViews.end(); ++it)
	{
		if (it->second->m_hWnd == m_activeView->m_hWnd) continue;

		it->second->SetWindowPos(
						0, 
						0, 
						0, 
						rectView.right - rectView.left, 
						rectView.bottom - rectView.top, 
						SWP_NOMOVE|SWP_NOZORDER|SWP_NOSENDCHANGING);
		
		it->second->AdjustRectAndResize(rectView);
	}
}
*/

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MainFrame::CreateNewConsole(DWORD dwTabIndex, const wstring& strStartupDir /*= wstring(L"")*/, const wstring& strStartupCmd /*= wstring(L"")*/, const wstring& strDbgCmdLine /*= wstring(L"")*/)
{
	if (dwTabIndex >= g_settingsHandler->GetTabSettings().tabDataVector.size()) return false;

	DWORD dwRows	= g_settingsHandler->GetConsoleSettings().dwRows;
	DWORD dwColumns	= g_settingsHandler->GetConsoleSettings().dwColumns;

	if (m_mapViews.size() > 0)
	{
		SharedMemory<ConsoleParams>& consoleParams = m_mapViews.begin()->second->GetConsoleHandler().GetConsoleParams();
		dwRows		= consoleParams->dwRows;
		dwColumns	= consoleParams->dwColumns;
	}
	else
	{
		// initialize member variables for the first view
		m_dwRows	= dwRows;
		m_dwColumns	= dwColumns;
	}

	shared_ptr<ConsoleView> consoleView(new ConsoleView(*this, dwTabIndex, strStartupDir, strStartupCmd, strDbgCmdLine, dwRows, dwColumns));

	HWND hwndConsoleView = consoleView->Create(
											m_hWnd, 
											rcDefault, 
											NULL, 
											WS_CHILD | WS_VISIBLE,// | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
											0);

	if (hwndConsoleView == NULL)
	{
		CString	strMessage;

		strMessage.Format(IDS_TAB_CREATE_FAILED, g_settingsHandler->GetTabSettings().tabDataVector[dwTabIndex]->strTitle.c_str());
		::MessageBox(m_hWnd, strMessage, L"Error", MB_OK|MB_ICONERROR);
		return false;
	}

	m_mapViews.insert(ConsoleViewMap::value_type(hwndConsoleView, consoleView));

	CString strTabTitle;
	consoleView->GetWindowText(strTabTitle);

	AddTabWithIcon(*consoleView, strTabTitle, consoleView->GetIcon(false));
	DisplayTab(hwndConsoleView, FALSE);
	::SetForegroundWindow(m_hWnd);

	if ( g_settingsHandler->GetAppearanceSettings().controlsSettings.bShowTabs &&
		((m_mapViews.size() > 1) || (!g_settingsHandler->GetAppearanceSettings().controlsSettings.bHideSingleTab))
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
	if (!pTabItem) return;
	CloseTab(pTabItem->GetTabView());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CloseTab(HWND hwndConsoleView)
{
	ConsoleViewMap::iterator	it = m_mapViews.find(hwndConsoleView);
	if (it == m_mapViews.end()) return;

	RemoveTab(hwndConsoleView);
	if (m_activeView.get() == it->second.get()) m_activeView.reset();
	it->second->DestroyWindow();
	m_mapViews.erase(it);

	if ((m_mapViews.size() == 1) &&
		m_bTabsVisible && 
		(g_settingsHandler->GetAppearanceSettings().controlsSettings.bHideSingleTab))
	{
		ShowTabs(FALSE);
	}

	if (m_mapViews.size() == 0) PostMessage(WM_CLOSE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::UpdateTabsMenu(CMenuHandle mainMenu, CMenu& tabsMenu)
{
	if (!tabsMenu.IsNull()) tabsMenu.DestroyMenu();
	tabsMenu.CreateMenu();

	// build tabs menu
	TabDataVector&			tabDataVector	= g_settingsHandler->GetTabSettings().tabDataVector;
	TabDataVector::iterator	it				= tabDataVector.begin();
	DWORD					dwId			= ID_NEW_TAB_1;

	for (it; it != tabDataVector.end(); ++it, ++dwId)
	{
		CMenuItemInfo	subMenuItem;
/*
		ICONINFO		iconInfo;
		BITMAP			bmp;

		::GetIconInfo(tabDataVector[dwId-ID_NEW_TAB_1]->tabSmallIcon, &iconInfo);
		::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);
*/

		subMenuItem.fMask		= MIIM_STRING | MIIM_ID;
/*
		subMenuItem.fMask		= MIIM_BITMAP | MIIM_ID | MIIM_TYPE;
		subMenuItem.fType		= MFT_BITMAP;
*/
		subMenuItem.wID			= dwId;
//		subMenuItem.hbmpItem	= iconInfo.hbmColor;
		subMenuItem.dwTypeData	= const_cast<wchar_t*>((*it)->strTitle.c_str());
		subMenuItem.cch			= static_cast<UINT>((*it)->strTitle.length());

		tabsMenu.InsertMenuItem(dwId-ID_NEW_TAB_1, TRUE, &subMenuItem);
//		tabsMenu.SetMenuItemBitmaps(dwId, MF_BYCOMMAND, iconInfo.hbmColor, NULL);
	}

	// set tabs menu as popup submenu
	if (!mainMenu.IsNull())
	{
		CMenuItemInfo	menuItem;

		menuItem.fMask		= MIIM_SUBMENU;
		menuItem.hSubMenu	= HMENU(tabsMenu);

		mainMenu.SetMenuItemInfo(ID_FILE_NEW_TAB, FALSE, &menuItem);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::UpdateStatusBar()
{
	CString strRowsCols;

	strRowsCols.Format(IDPANE_ROWS_COLUMNS, m_dwRows, m_dwColumns);
	UISetText(1, strRowsCols);

	UIUpdateStatusBar();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::SetWindowStyles()
{
	StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

	DWORD	dwStyle		= GetWindowLong(GWL_STYLE);
	DWORD	dwExStyle	= GetWindowLong(GWL_EXSTYLE);

	dwStyle &= ~WS_MAXIMIZEBOX;
	if (!stylesSettings.bCaption)	dwStyle &= ~WS_CAPTION;
	if (!stylesSettings.bResizable)	dwStyle &= ~WS_THICKFRAME;
	if (!stylesSettings.bTaskbarButton)
	{
		dwStyle		&= ~WS_MINIMIZEBOX;
		dwExStyle	|= WS_EX_TOOLWINDOW;
		dwExStyle	&= ~WS_EX_APPWINDOW;
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

	HWND hwndParent = NULL;

	if (m_zOrder == zorderDesktop)
	{
		// pinned to the desktop, Program Manager is the parent
		// TODO: automatic shell detection
		hwndParent = ::FindWindow(L"Progman", L"Program Manager");
	}

	SetParent(hwndParent);
	SetWindowPos(hwndZ, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::SetWindowIcons()
{
	WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;

	if (!m_icon.IsNull()) m_icon.DestroyIcon();
	if (!m_smallIcon.IsNull()) m_smallIcon.DestroyIcon();

	if (windowSettings.bUseTabIcon && (m_activeView.get() != NULL))
	{
		m_icon.Attach(m_activeView->GetIcon(true).DuplicateIcon());
		m_smallIcon.Attach(m_activeView->GetIcon(false).DuplicateIcon());
	}
	else
	{
		if (windowSettings.strIcon.length() > 0)
		{
			m_icon.Attach(static_cast<HICON>(::LoadImage(
													NULL, 
													Helpers::ExpandEnvironmentStrings(windowSettings.strIcon).c_str(), 
													IMAGE_ICON, 
													0, 
													0, 
													LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_DEFAULTSIZE)));

			m_smallIcon.Attach(static_cast<HICON>(::LoadImage(
													NULL, 
													Helpers::ExpandEnvironmentStrings(windowSettings.strIcon).c_str(), 
													IMAGE_ICON, 
													16, 
													16, 
													LR_DEFAULTCOLOR | LR_LOADFROMFILE)));
		}
		else
		{
			m_icon.Attach(static_cast<HICON>(::LoadImage(
													::GetModuleHandle(NULL), 
													MAKEINTRESOURCE(IDR_MAINFRAME), 
													IMAGE_ICON, 
													0, 
													0, 
													LR_DEFAULTCOLOR | LR_DEFAULTSIZE)));

			m_smallIcon.Attach(static_cast<HICON>(::LoadImage(
													::GetModuleHandle(NULL), 
													MAKEINTRESOURCE(IDR_MAINFRAME), 
													IMAGE_ICON, 
													16, 
													16, 
													LR_DEFAULTCOLOR)));
		}
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
	AdjustWindowSize(false);
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
	AdjustWindowSize(false);
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
	AdjustWindowSize(false);
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
	AdjustWindowSize(false);
	DockWindow(m_dockPosition);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::AdjustWindowSize(bool bResizeConsole, bool bMaxOrRestore /*= false*/)
{
	CRect clientRect(0, 0, 0, 0);

	if (bResizeConsole)
	{
		if (bMaxOrRestore)
		{
			GetClientRect(&clientRect);
		
			// adjust for the toolbar height
			CReBarCtrl	rebar(m_hWndToolBar);
//			clientRect.top	+= rebar.GetBarHeight() - 4;
			clientRect.bottom -= rebar.GetBarHeight();

			if (m_bStatusBarVisible)
			{
				CRect	rectStatusBar(0, 0, 0, 0);

				::GetWindowRect(m_hWndStatusBar, &rectStatusBar);
				clientRect.bottom	-= rectStatusBar.Height();
			}

			clientRect.top += GetTabAreaHeight(); //+0
		}

		// adjust the active view
		if (m_activeView.get() == NULL) return;

		// if we're being maximized, AdjustRectAndResize will use client rect supplied
		m_activeView->AdjustRectAndResize(clientRect, m_dwResizeWindowEdge, !bMaxOrRestore);
		
		// for other views, first set view size and then resize their Windows consoles
		for (ConsoleViewMap::iterator it = m_mapViews.begin(); it != m_mapViews.end(); ++it)
		{
			if (it->second->m_hWnd == m_activeView->m_hWnd) continue;

			it->second->SetWindowPos(
							0, 
							0, 
							0, 
							clientRect.Width(), 
							clientRect.Height(), 
							SWP_NOMOVE|SWP_NOZORDER|SWP_NOSENDCHANGING);
		
			// if we're being maximized, AdjustRectAndResize will use client rect supplied
			it->second->AdjustRectAndResize(clientRect, m_dwResizeWindowEdge, !bMaxOrRestore);
		}
	}
	else
	{
		if (m_activeView.get() == NULL) return;
		m_activeView->GetRect(clientRect);
	}

	AdjustWindowRect(clientRect);

//	TRACE(L"AdjustWindowSize: %ix%i\n", clientRect.Width(), clientRect.Height());

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
//	TRACE(L"AdjustWindowSize 2: %ix%i\n", rectWindow.Width(), rectWindow.Height());
	m_dwWindowWidth	= rectWindow.Width();
	m_dwWindowHeight= rectWindow.Height();

/*
	CRect clientRect;
	GetClientRect(&clientRect);

	if (bMaximizing)
	{
		// adjust for the toolbar height
		CReBarCtrl	rebar(m_hWndToolBar);
		clientRect.top	+= rebar.GetBarHeight() - 4;

		if (m_bStatusBarVisible)
		{
			CRect	rectStatusBar(0, 0, 0, 0);

			::GetWindowRect(m_hWndStatusBar, &rectStatusBar);
			clientRect.bottom	-= rectStatusBar.bottom - rectStatusBar.top;
		}

		clientRect.top += GetTabAreaHeight(); //+0
	}

	if (bResizeConsole)
	{
//		AdjustAndResizeConsoleView(clientRect);

		// adjust the active view
		if (m_activeView.get() == NULL) return;

		m_activeView->AdjustRectAndResize(clientRect);
		
		// for other views, first set view size and then resize their Windows consoles
		for (ConsoleViewMap::iterator it = m_mapViews.begin(); it != m_mapViews.end(); ++it)
		{
			if (it->second->m_hWnd == m_activeView->m_hWnd) continue;

			it->second->SetWindowPos(
							0, 
							0, 
							0, 
							clientRect.Width(), 
							clientRect.Height(), 
							SWP_NOMOVE|SWP_NOZORDER|SWP_NOSENDCHANGING);
			
			it->second->AdjustRectAndResize(clientRect);
		}
	}
	else
	{
		if (m_activeView.get() == NULL) return;

		m_activeView->GetRect(clientRect);
	}

	AdjustWindowRect(clientRect);

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
	m_dwWindowWidth	= rectWindow.Width();
	m_dwWindowHeight= rectWindow.Height();
*/
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::SetTransparency()
{
	// set transparency
	TransparencySettings& transparencySettings = g_settingsHandler->GetAppearanceSettings().transparencySettings;
	switch (transparencySettings.transType)
	{
		case transAlpha : 

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

			break;

		case transColorKey :
		{
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

		default :
		{
			SetWindowLong(
					GWL_EXSTYLE, 
					GetWindowLong(GWL_EXSTYLE) & ~WS_EX_LAYERED);
		}


	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CreateAcceleratorTable()
{
	HotKeys&							hotKeys	= g_settingsHandler->GetHotKeys();
	HotKeys::CommandsSequence::iterator it		= hotKeys.commands.begin();

	shared_array<ACCEL>					accelTable(new ACCEL[hotKeys.commands.size()]);
	int									nAcceleratorCount = 0;

	for (int i = 0; it != hotKeys.commands.end(); ++i, ++it)
	{
		if ((*it)->accelHotkey.cmd == 0) continue;
		::CopyMemory(&(accelTable[i]), &((*it)->accelHotkey), sizeof(ACCEL));
		++nAcceleratorCount;
	}

	if (!m_acceleratorTable.IsNull()) m_acceleratorTable.DestroyObject();
	m_acceleratorTable.CreateAcceleratorTable(accelTable.get(), nAcceleratorCount);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CreateStatusBar()
{
	m_hWndStatusBar = m_statusBar.Create(*this);
    UIAddStatusBar(m_hWndStatusBar);

	int arrPanes[]	= { ID_DEFAULT_PANE, IDPANE_ROWS_COLUMNS };
 
    m_statusBar.SetPanes(arrPanes, sizeof(arrPanes)/sizeof(int), false);
	m_statusBar.SetPaneWidth(IDPANE_ROWS_COLUMNS, 50);
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
	
	wcsncpy(tnd.szTip, strToolTip.c_str(), (sizeof(tnd.szTip)-1)/sizeof(wchar_t));
	return ::Shell_NotifyIcon(dwMessage, &tnd);
}

/////////////////////////////////////////////////////////////////////////////


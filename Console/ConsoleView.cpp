#include "stdafx.h"
#include "resource.h"

#include <fstream>

#include "Console.h"
#include "MainFrame.h"
#include "ConsoleException.h"
#include "ConsoleView.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CDC		ConsoleView::m_dcOffscreen(::CreateCompatibleDC(NULL));
CDC		ConsoleView::m_dcText(::CreateCompatibleDC(NULL));

CBitmap	ConsoleView::m_bmpOffscreen;
CBitmap	ConsoleView::m_bmpText;

CFont	ConsoleView::m_fontText;

int		ConsoleView::m_nCharHeight(0);
int		ConsoleView::m_nCharWidth(0);

//////////////////////////////////////////////////////////////////////////////

ConsoleView::ConsoleView(MainFrame& mainFrame, DWORD dwTabIndex, const wstring& strCmdLineInitialDir, const wstring& strInitialCmd, const wstring& strDbgCmdLine, DWORD dwRows, DWORD dwColumns)
: m_mainFrame(mainFrame)
, m_strCmdLineInitialDir(strCmdLineInitialDir)
, m_strInitialCmd(strInitialCmd)
, m_strDbgCmdLine(strDbgCmdLine)
, m_bInitializing(true)
, m_bResizing(false)
, m_bAppActive(true)
, m_bActive(true)
, m_bNeedFullRepaint(false) // first OnPaint will do a full repaint
, m_bUseTextAlphaBlend(false)
, m_bConsoleWindowVisible(false)
, m_dwStartupRows(dwRows)
, m_dwStartupColumns(dwColumns)
, m_bShowVScroll(false)
, m_bShowHScroll(false)
, m_nVScrollWidth(::GetSystemMetrics(SM_CXVSCROLL))
, m_nHScrollWidth(::GetSystemMetrics(SM_CXHSCROLL))
, m_strTitle(g_settingsHandler->GetTabSettings().tabDataVector[dwTabIndex]->strTitle.c_str())
, m_strUser()
, bigIcon()
, smallIcon()
, m_consoleHandler()
, m_screenBuffer()
, m_consoleSettings(g_settingsHandler->GetConsoleSettings())
, m_appearanceSettings(g_settingsHandler->GetAppearanceSettings())
, m_hotkeys(g_settingsHandler->GetHotKeys())
, m_tabData(g_settingsHandler->GetTabSettings().tabDataVector[dwTabIndex])
, m_background()
, m_backgroundBrush(NULL)
, m_cursor()
, m_selectionHandler()
, m_mouseCommand(MouseSettings::cmdNone)
, m_bufferMutex(NULL, FALSE, NULL)
, m_bFlashTimerRunning(false)
, m_dwFlashes(0)
{
}

ConsoleView::~ConsoleView()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL ConsoleView::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) || 
		(pMsg->message == WM_KEYUP) ||
		(pMsg->message == WM_SYSKEYDOWN) || 
		(pMsg->message == WM_SYSKEYUP))
	{
		// Avoid calling ::TranslateMessage for WM_KEYDOWN, WM_KEYUP,
		// WM_SYSKEYDOWN and WM_SYSKEYUP (except for wParam == VK_PACKET, 
		// which is sent by SendInput when pasting text).
		///
		// This prevents WM_CHAR and WM_SYSCHAR messages, enabling stuff like
		// handling 'dead' characters input and passing all keys to console.
		if (pMsg->wParam == VK_PACKET) return FALSE;
		::DispatchMessage(pMsg);
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	DragAcceptFiles(TRUE);

	// load icon
	if (m_tabData->strIcon.length() > 0)
	{
		bigIcon = static_cast<HICON>(::LoadImage(
											NULL, 
											Helpers::ExpandEnvironmentStrings(m_tabData->strIcon).c_str(), 
											IMAGE_ICON, 
											0, 
											0, 
											LR_DEFAULTCOLOR|LR_LOADFROMFILE|LR_DEFAULTSIZE));

		smallIcon = static_cast<HICON>(::LoadImage(
											NULL, 
											Helpers::ExpandEnvironmentStrings(m_tabData->strIcon).c_str(), 
											IMAGE_ICON, 
											16, 
											16, 
											LR_DEFAULTCOLOR|LR_LOADFROMFILE));
	}
	else
	{
		bigIcon = static_cast<HICON>(::LoadImage(
											::GetModuleHandle(NULL), 
											MAKEINTRESOURCE(IDR_MAINFRAME), 
											IMAGE_ICON, 
											0, 
											0, 
											LR_DEFAULTCOLOR|LR_DEFAULTSIZE));

		smallIcon = static_cast<HICON>(::LoadImage(
											::GetModuleHandle(NULL), 
											MAKEINTRESOURCE(IDR_MAINFRAME), 
											IMAGE_ICON, 
											16, 
											16, 
											LR_DEFAULTCOLOR));
	}

	// set console delegates
	m_consoleHandler.SetupDelegates(
						fastdelegate::MakeDelegate(this, &ConsoleView::OnConsoleChange), 
						fastdelegate::MakeDelegate(this, &ConsoleView::OnConsoleClose));

	// load background image
	if (m_tabData->backgroundImageType == bktypeImage)
	{
		m_background = g_imageHandler->GetImage(m_tabData->imageData);
	}
	else if (m_tabData->backgroundImageType == bktypeDesktop)
	{
		m_background = g_imageHandler->GetDesktopImage(m_tabData->imageData);
	}

	if (!m_background) m_tabData->backgroundImageType = bktypeNone;

	// TODO: error handling
	wstring strInitialDir(m_consoleSettings.strInitialDir);

	if (m_strCmdLineInitialDir.length() > 0)
	{
		strInitialDir = m_strCmdLineInitialDir;
	}
	else if (m_tabData->strInitialDir.length() > 0)
	{
		strInitialDir = m_tabData->strInitialDir;
	}

	wstring	strShell(m_consoleSettings.strShell);
	bool	bDebugFlag = false;

	if (m_strDbgCmdLine.length() > 0)
	{
		strShell	= m_strDbgCmdLine;
		bDebugFlag	= true;
	}
	else if (m_tabData->strShell.length() > 0)
	{
		strShell	= m_tabData->strShell;
	}

	try
	{
		CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		UserCredentials* userCredentials = reinterpret_cast<UserCredentials*>(createStruct->lpCreateParams);

		m_consoleHandler.StartShellProcess(
									strShell, 
									strInitialDir,
									userCredentials->user,
									userCredentials->password,
									m_strInitialCmd,
									g_settingsHandler->GetAppearanceSettings().windowSettings.bUseConsoleTitle ? m_tabData->strTitle : wstring(L""),
									m_dwStartupRows, 
									m_dwStartupColumns,
									bDebugFlag);

		m_strUser = userCredentials->user.c_str();
	}
	catch (const ConsoleException& ex)
	{
		m_exceptionMessage = ex.GetMessage().c_str();
		return -1;
	}

	// set view title
	SetTitle(m_strTitle);

	m_bInitializing = false;

	// set current language in the console window
	::PostMessage(
		m_consoleHandler.GetConsoleParams()->hwndConsoleWindow,
		WM_INPUTLANGCHANGEREQUEST, 
		0, 
		reinterpret_cast<LPARAM>(::GetKeyboardLayout(0)));

	// scrollbar stuff
	InitializeScrollbars();

	// create offscreen buffers
	CreateOffscreenBuffers();

	// TODO: put this in console size change handler
	m_screenBuffer.reset(new CharInfo[m_consoleHandler.GetConsoleParams()->dwRows*m_consoleHandler.GetConsoleParams()->dwColumns]);
//	::ZeroMemory(m_screenBuffer.get(), sizeof(CHAR_INFO)*m_consoleHandler.GetConsoleParams()->dwRows*m_consoleHandler.GetConsoleParams()->dwColumns);

	m_consoleHandler.StartMonitorThread();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_bFlashTimerRunning) KillTimer(FLASH_TAB_TIMER);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (!m_bActive) return 0;

	CPaintDC	dc(m_hWnd);

	if (m_bNeedFullRepaint)
	{
		// we need to update offscreen buffers here for first paint and relative backgrounds
		RepaintText(m_dcText);
		UpdateOffscreen(dc.m_ps.rcPaint);
		m_bNeedFullRepaint = false;
	}

	// this is the best way I know how to detect if the window is being 
	// repainted while sizing
	// the flag is set in MainFrame::OnSizing and MainFrame::OnExitSizeMove
	if (m_bResizing)
	{
		dc.FillRect(&dc.m_ps.rcPaint, m_backgroundBrush);
	}
	
	dc.BitBlt(
		dc.m_ps.rcPaint.left, 
		dc.m_ps.rcPaint.top, 
		dc.m_ps.rcPaint.right, 
		dc.m_ps.rcPaint.bottom,
		m_dcOffscreen, 
		dc.m_ps.rcPaint.left, 
		dc.m_ps.rcPaint.top, 
		SRCCOPY);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	WINDOWPOS* pWinPos = reinterpret_cast<WINDOWPOS*>(lParam);

	// showing the view, repaint
	if (pWinPos->flags & SWP_SHOWWINDOW) Repaint(false);

	// force full repaint for relative backgrounds
	if (m_tabData->imageData.bRelative && !(pWinPos->flags & SWP_NOMOVE)) m_bNeedFullRepaint = true;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnSysKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ((wParam == VK_SPACE) && (lParam & (0x1 << 29)))
	{
		return m_mainFrame.SendMessage(WM_SYSCOMMAND, SC_KEYMENU, VK_SPACE);
	}

	return OnConsoleFwdMsg(uMsg, wParam, lParam, bHandled);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnConsoleFwdMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (((uMsg == WM_KEYDOWN) || (uMsg == WM_KEYUP)) && (wParam == VK_PACKET)) return 0;

	if (!TranslateKeyDown(uMsg, wParam, lParam))
	{
//		TRACE(L"Msg: 0x%04X, wParam: 0x%08X, lParam: 0x%08X\n", uMsg, wParam, lParam);
		::PostMessage(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, uMsg, wParam, lParam);
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoScroll(SB_VERT, LOWORD(wParam), HIWORD(wParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoScroll(SB_HORZ, LOWORD(wParam), HIWORD(wParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	UINT						uKeys			= GET_KEYSTATE_WPARAM(wParam); 
	UINT						uXButton		= GET_XBUTTON_WPARAM(wParam);

	CPoint						point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	MouseSettings&				mouseSettings	= g_settingsHandler->GetMouseSettings();
	bool						bMouseButtonUp	= false;
	MouseSettings::Action		mouseAction;

	LRESULT						ret = 0;

	// get modifiers
	if (uKeys & MK_CONTROL)			mouseAction.modifiers |= MouseSettings::mkCtrl;
	if (uKeys & MK_SHIFT)			mouseAction.modifiers |= MouseSettings::mkShift;
	if (GetKeyState(VK_MENU) < 0)	mouseAction.modifiers |= MouseSettings::mkAlt;

	// get mouse button
	switch (uMsg)
	{
		case WM_LBUTTONDOWN :
		case WM_LBUTTONUP :
		case WM_LBUTTONDBLCLK :
			mouseAction.button = MouseSettings::btnLeft;
			break;

		case WM_RBUTTONDOWN :
		case WM_RBUTTONUP :
		case WM_RBUTTONDBLCLK :
			mouseAction.button = MouseSettings::btnRight;
			break;

		case WM_MBUTTONDOWN :
		case WM_MBUTTONUP :
		case WM_MBUTTONDBLCLK :
			mouseAction.button = MouseSettings::btnMiddle;
			break;

		case WM_XBUTTONDOWN :
		case WM_XBUTTONUP :
		case WM_XBUTTONDBLCLK :
			if (uXButton == XBUTTON1)
			{
				mouseAction.button = MouseSettings::btn4th;
			}
			else
			{
				mouseAction.button = MouseSettings::btn5th;
			}
			ret = 1;
			break;
	}

	// get click type
	switch (uMsg)
	{
		case WM_LBUTTONDOWN :
		case WM_RBUTTONDOWN :
		case WM_MBUTTONDOWN :
		case WM_XBUTTONDOWN :
			mouseAction.clickType = MouseSettings::clickSingle;
			break;

		case WM_LBUTTONDBLCLK :
		case WM_RBUTTONDBLCLK :
		case WM_MBUTTONDBLCLK :
		case WM_XBUTTONDBLCLK :
			mouseAction.clickType = MouseSettings::clickDouble;
			break;

		case WM_LBUTTONUP :
		case WM_RBUTTONUP :
		case WM_MBUTTONUP :
		case WM_XBUTTONUP :
			bMouseButtonUp = true;
			break;
	}

	do
	{
		if (m_mouseCommand == MouseSettings::cmdNone)
		{
			// no current mouse action
			typedef MouseSettings::Commands::index<MouseSettings::commandID>::type	CommandIDIndex;

			CommandIDIndex::iterator it;

			// copy command
			if (m_selectionHandler->GetState() == SelectionHandler::selstateSelected)
			{
				it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdCopy);

				if ((*it)->action == mouseAction)
				{
					m_mouseCommand = MouseSettings::cmdCopy;
					return 0;
				}
			}

			// select command
			it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdSelect);
			if ((*it)->action == mouseAction)
			{
				::SetCursor(::LoadCursor(NULL, IDC_IBEAM));

				MutexLock bufferLock(m_bufferMutex);
				m_selectionHandler->ClearSelection();
				m_selectionHandler->StartSelection(GetConsoleCoord(point), m_appearanceSettings.stylesSettings.crSelectionColor, m_screenBuffer);

				m_mouseCommand = MouseSettings::cmdSelect;
				return 0;
			}

			if (MouseSettings::clickDouble == mouseAction.clickType)
			{
				mouseAction.clickType = MouseSettings::clickSingle;
				if ((*it)->action == mouseAction)
				{
					m_mouseCommand = MouseSettings::cmdSelect;

					MutexLock bufferLock(m_bufferMutex);
					m_selectionHandler->SelectWord(GetConsoleCoord(point), m_appearanceSettings.stylesSettings.crSelectionColor, m_screenBuffer);
				}

				mouseAction.clickType = MouseSettings::clickDouble;
			}

			// paste command
			it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdPaste);
			if ((*it)->action == mouseAction)
			{
				m_mouseCommand = MouseSettings::cmdPaste;
				return 0;
			}

			// drag command
			it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdDrag);
			if ((*it)->action == mouseAction)
			{
				CPoint clientPoint(point);

				ClientToScreen(&clientPoint);
				m_mainFrame.PostMessage(UM_START_MOUSE_DRAG, MAKEWPARAM(uKeys, uXButton), MAKELPARAM(clientPoint.x, clientPoint.y));

				// we don't set active command here, main frame handles mouse drag
				return 0;
			}

			// menu command
			it = mouseSettings.commands.get<MouseSettings::commandID>().find(MouseSettings::cmdMenu);
			if ((*it)->action == mouseAction)
			{
				m_mouseCommand = MouseSettings::cmdMenu;
				return 0;
			}
		}
		else
		{
			// we have an active command, handle it...
			switch (m_mouseCommand)
			{
				case MouseSettings::cmdCopy :
				{
					Copy(&point);
					break;
				}

				case MouseSettings::cmdSelect :
				{
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));

					if (m_selectionHandler->GetState() == SelectionHandler::selstateStartedSelecting)
					{
						m_selectionHandler->EndSelection();
						m_selectionHandler->ClearSelection();
					}
					else if (m_selectionHandler->GetState() == SelectionHandler::selstateSelecting ||
						 m_selectionHandler->GetState() == SelectionHandler::selstateSelectWord)
					{
						m_selectionHandler->EndSelection();

						// copy on select
						if (g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bCopyOnSelect)
						{
							Copy(NULL);
						}
					}

					break;
				}

				case MouseSettings::cmdPaste :
				{
					Paste();
					break;
				}

				case MouseSettings::cmdMenu :
				{
					CPoint	screenPoint(point);
					ClientToScreen(&screenPoint);
					m_mainFrame.SendMessage(UM_SHOW_POPUP_MENU, 0, MAKELPARAM(screenPoint.x, screenPoint.y));
					break;
				}

				default :
				{
					if (bMouseButtonUp) return 0;
				}
			}

			m_mouseCommand = MouseSettings::cmdNone;
			return 0;
		}

	} while(false);

	ForwardMouseClick(uMsg, wParam, point);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UINT	uFlags = static_cast<UINT>(wParam);
	CPoint	point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	if (m_mouseCommand == MouseSettings::cmdSelect)
	{
		CRect	rectClient;
		GetClientRect(&rectClient);

		DWORD dwInsideBorder = g_settingsHandler->GetAppearanceSettings().stylesSettings.dwInsideBorder;

		if (point.x < rectClient.left + static_cast<LONG>(dwInsideBorder))
		{
			DoScroll(SB_HORZ, SB_LINELEFT, 0);
		}			
		else if (point.x > rectClient.right - static_cast<LONG>(dwInsideBorder))
		{
			DoScroll(SB_HORZ, SB_LINERIGHT, 0);
		}
		
		if (point.y < rectClient.top + static_cast<LONG>(dwInsideBorder))
		{
			DoScroll(SB_VERT, SB_LINEUP, 0);
		}
		else if (point.y > rectClient.bottom - static_cast<LONG>(dwInsideBorder))
		{
			DoScroll(SB_VERT, SB_LINEDOWN, 0);
		}

		{
			MutexLock bufferLock(m_bufferMutex);
			m_selectionHandler->UpdateSelection(GetConsoleCoord(point), m_screenBuffer);
		}

		BitBltOffscreen();
	}
	else if ((m_mouseCommand == MouseSettings::cmdNone) && 
			 ((uFlags & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) != 0))
	{
		ForwardMouseClick(uMsg, wParam, point);
	}
	else
	{
		bHandled = FALSE;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// discards mouse message when activating the window using mouse
	LRESULT ret = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);

	HWND hwndTopLevel	= ::GetForegroundWindow();
	HWND hwndParent		= NULL;

	do
	{
		hwndParent = ::GetParent(hwndTopLevel);
		if (hwndParent != NULL) hwndTopLevel = hwndParent;
	}
	while (hwndParent != NULL);

	// if we're not active, discard the mouse message
	if (hwndTopLevel != m_mainFrame.m_hWnd)
	{
		if (ret == MA_ACTIVATE)
		{
			ret = MA_ACTIVATEANDEAT;
		}
		else if (ret == MA_NOACTIVATE)
		{
			ret = MA_NOACTIVATEANDEAT;
		}
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (wParam == FLASH_TAB_TIMER)
	{
		// if we got activated, kill timer
		if (m_bActive)
		{
			KillTimer(FLASH_TAB_TIMER);
			m_bFlashTimerRunning = false;
			return 0;
		}

		m_mainFrame.HighlightTab(m_hWnd, (m_dwFlashes % 2) == 0);
		if (++m_dwFlashes == g_settingsHandler->GetBehaviorSettings().tabHighlightSettings.dwFlashes * 2)
		{
			if (g_settingsHandler->GetBehaviorSettings().tabHighlightSettings.bStayHighlighted)
			{
				m_mainFrame.HighlightTab(m_hWnd, true);
			}

			KillTimer(FLASH_TAB_TIMER);
			m_bFlashTimerRunning = false;
		}

		return 0;
	}

	if (!m_bActive) return 0;

	if ((wParam == CURSOR_TIMER) && (m_cursor.get() != NULL))
	{
		m_cursor->PrepareNext();
		m_cursor->Draw(m_bAppActive);
		BitBltOffscreen(true);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnInputLangChangeRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostMessage(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, uMsg, wParam, lParam);
	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnInputLangChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostMessage(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_SYSCHARSET, lParam);
	::PostMessage(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, uMsg, wParam, lParam);
	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDROP	hDrop = reinterpret_cast<HDROP>(wParam);
	UINT	uFilesCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	CString	strFilenames;

	// concatenate all filenames
	for (UINT i = 0; i < uFilesCount; ++i)
	{
		CString	strFilename;
		::DragQueryFile(hDrop, i, strFilename.GetBuffer(MAX_PATH), MAX_PATH);
		strFilename.ReleaseBuffer();

		// put quotes around the filename
		strFilename = CString(L"\"") + strFilename + CString("\"");
		
		if (i > 0) strFilenames += L" ";
		strFilenames += strFilename;

	}
	::DragFinish(hDrop);

	SendTextToConsole(strFilenames);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnUpdateConsoleView(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_bInitializing) return false;

	bool bResize	= ((wParam & UPDATE_CONSOLE_RESIZE) > 0);
	bool textChanged= ((wParam & UPDATE_CONSOLE_TEXT_CHANGED) > 0);

	// console size changed, resize offscreen buffers
	if (bResize)
	{
/*
		TRACE(L"================================================================\n");
		TRACE(L"Resizing console wnd: 0x%08X\n", m_hWnd);
*/
		InitializeScrollbars();

		if (m_bActive) RecreateOffscreenBuffers();

		// notify parent about resize
		m_mainFrame.SendMessage(UM_CONSOLE_RESIZED, 0, 0);
	}

	UpdateTitle();
	
	// if the view is not visible, don't repaint
	if (!m_bActive)
	{
		if 
		(
			textChanged &&
			!bResize && 
			(g_settingsHandler->GetBehaviorSettings().tabHighlightSettings.dwFlashes > 0) && 
			(!m_bFlashTimerRunning)
		)
		{
			m_dwFlashes = 0;
			m_bFlashTimerRunning = true;
			SetTimer(FLASH_TAB_TIMER, 500);
		}
		
		return 0;
	}

	SharedMemory<ConsoleInfo>& consoleInfo = m_consoleHandler.GetConsoleInfo();

	if (m_bShowVScroll)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si); 
		si.fMask  = SIF_POS; 
		si.nPos   = consoleInfo->csbi.srWindow.Top; 
		::FlatSB_SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);

/*
		TRACE(L"----------------------------------------------------------------\n");
		TRACE(L"VScroll pos: %i\n", consoleInfo->csbi.srWindow.Top);
*/
	}

	if (m_bShowHScroll)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si); 
		si.fMask  = SIF_POS; 
		si.nPos   = consoleInfo->csbi.srWindow.Left; 
		::FlatSB_SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);
	}

	if ((m_selectionHandler->GetState() == SelectionHandler::selstateStartedSelecting) ||
		(m_selectionHandler->GetState() == SelectionHandler::selstateSelecting))
	{
		CPoint	point;
		::GetCursorPos(&point);
		ScreenToClient(&point);

		MutexLock bufferLock(m_bufferMutex);
		m_selectionHandler->UpdateSelection(GetConsoleCoord(point), m_screenBuffer);
	}
	else if (m_selectionHandler->GetState() == SelectionHandler::selstateSelected)
	{
		m_selectionHandler->UpdateSelection();
	}

	Repaint(false);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnScrollCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	int	nScrollType	= 0;
	int nScrollCode	= 0;

	switch (wID)
	{
		case ID_SCROLL_UP :
		{
			nScrollType	= SB_VERT;
			nScrollCode = SB_LINEUP;
			break;
		}

		case ID_SCROLL_LEFT :
		{
			nScrollType	= SB_HORZ;
			nScrollCode = SB_LINELEFT;
			break;
		}

		case ID_SCROLL_DOWN :
		{
			nScrollType	= SB_VERT;
			nScrollCode = SB_LINEDOWN;
			break;
		}

		case ID_SCROLL_RIGHT :
		{
			nScrollType	= SB_HORZ;
			nScrollCode = SB_LINERIGHT;
			break;
		}

		case ID_SCROLL_PAGE_UP :
		{
			nScrollType	= SB_VERT;
			nScrollCode = SB_PAGEUP;
			break;
		}

		case ID_SCROLL_PAGE_LEFT :
		{
			nScrollType	= SB_HORZ;
			nScrollCode = SB_PAGELEFT;
			break;
		}

		case ID_SCROLL_PAGE_DOWN :
		{
			nScrollType	= SB_VERT;
			nScrollCode = SB_PAGEDOWN;
			break;
		}

		case ID_SCROLL_PAGE_RIGHT :
		{
			nScrollType	= SB_HORZ;
			nScrollCode = SB_PAGERIGHT;
			break;
		}


		default : bHandled = FALSE; return 0;
	}

	DoScroll(nScrollType, nScrollCode, 0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::GetRect(CRect& clientRect)
{
	StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

	clientRect.left		= 0;
	clientRect.top		= 0;
	clientRect.right	= m_consoleHandler.GetConsoleParams()->dwColumns*m_nCharWidth + 2*stylesSettings.dwInsideBorder;
	clientRect.bottom	= m_consoleHandler.GetConsoleParams()->dwRows*m_nCharHeight + 2*stylesSettings.dwInsideBorder;

	if (m_bShowVScroll) clientRect.right	+= m_nVScrollWidth;
	if (m_bShowHScroll) clientRect.bottom	+= m_nHScrollWidth;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::GetMaxRect(CRect& maxClientRect)
{
	if (m_bInitializing) return false;

	StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

	// TODO: take care of max window size
	maxClientRect.left	= 0;
	maxClientRect.top	= 0;
	maxClientRect.right	= m_consoleHandler.GetConsoleParams()->dwMaxColumns*m_nCharWidth + 2*stylesSettings.dwInsideBorder;
	maxClientRect.bottom= m_consoleHandler.GetConsoleParams()->dwMaxRows*m_nCharHeight + 2*stylesSettings.dwInsideBorder;

	CWindow desktopWindow(::GetDesktopWindow());
	CRect	rectDesktop;
	bool	bRecalc = false;

	desktopWindow.GetWindowRect(rectDesktop);

	if (rectDesktop.Width() < maxClientRect.Width())
	{
		m_consoleHandler.GetConsoleParams()->dwMaxColumns = (rectDesktop.Width() - 2*stylesSettings.dwInsideBorder) / m_nCharWidth;
		bRecalc = true;
	}

	if (rectDesktop.Height() < maxClientRect.Height())
	{
		m_consoleHandler.GetConsoleParams()->dwMaxRows = (rectDesktop.Height() - 2*stylesSettings.dwInsideBorder) / m_nCharHeight;
		bRecalc = true;
	}

	if (bRecalc)
	{
		maxClientRect.right	= m_consoleHandler.GetConsoleParams()->dwMaxColumns*m_nCharWidth + 2*stylesSettings.dwInsideBorder;
		maxClientRect.bottom= m_consoleHandler.GetConsoleParams()->dwMaxRows*m_nCharHeight + 2*stylesSettings.dwInsideBorder;
	}

	if (m_bShowVScroll) maxClientRect.right	+= m_nVScrollWidth;
	if (m_bShowHScroll) maxClientRect.bottom+= m_nHScrollWidth;

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::AdjustRectAndResize(CRect& clientRect, DWORD dwResizeWindowEdge, bool bGetClientRect)
{
	StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

	if (bGetClientRect) GetWindowRect(&clientRect);
/*
	TRACE(L"================================================================\n");
	TRACE(L"rect: %ix%i - %ix%i\n", clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
*/

	// exclude scrollbars from row/col calculation
	if (m_bShowVScroll) clientRect.right	-= m_nVScrollWidth;
	if (m_bShowHScroll) clientRect.bottom	-= m_nHScrollWidth;

	// TODO: handle variable fonts
	DWORD dwColumns	= (clientRect.Width() - 2*stylesSettings.dwInsideBorder) / m_nCharWidth;
	DWORD dwRows	= (clientRect.Height() - 2*stylesSettings.dwInsideBorder) / m_nCharHeight;

	clientRect.right	= clientRect.left + dwColumns*m_nCharWidth + 2*stylesSettings.dwInsideBorder;
	clientRect.bottom	= clientRect.top + dwRows*m_nCharHeight + 2*stylesSettings.dwInsideBorder;

	// adjust for scrollbars
	if (m_bShowVScroll) clientRect.right	+= m_nVScrollWidth;
	if (m_bShowHScroll) clientRect.bottom	+= m_nHScrollWidth;

	SharedMemory<ConsoleSize>&	newConsoleSize = m_consoleHandler.GetNewConsoleSize();
	SharedMemoryLock			memLock(newConsoleSize);

	newConsoleSize->dwColumns			= dwColumns;
	newConsoleSize->dwRows				= dwRows;
	newConsoleSize->dwResizeWindowEdge	= dwResizeWindowEdge;

/*
	TRACE(L"console view: 0x%08X, adjusted: %ix%i\n", m_hWnd, dwRows, dwColumns);
	TRACE(L"================================================================\n");
*/

	m_consoleHandler.GetNewConsoleSize().SetReqEvent();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetConsoleWindowVisible(bool bVisible)
{
	m_bConsoleWindowVisible = bVisible;

	if (bVisible)
	{
		CPoint point;
		::GetCursorPos(&point);
		::SetWindowPos(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, NULL, point.x, point.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	}

	::ShowWindow(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, bVisible ? SW_SHOW : SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetAppActiveStatus(bool bAppActive)
{
	m_bAppActive = bAppActive;
	if (m_cursor.get() != NULL) m_cursor->Draw(m_bAppActive);
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::RecreateOffscreenBuffers()
{
	if (!m_fontText.IsNull())		m_fontText.DeleteObject();
	if (!m_backgroundBrush.IsNull())m_backgroundBrush.DeleteObject();
	if (!m_bmpOffscreen.IsNull())	m_bmpOffscreen.DeleteObject();
	if (!m_bmpText.IsNull())		m_bmpText.DeleteObject();
	CreateOffscreenBuffers();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::Repaint(bool bFullRepaint)
{
	// OnPaint will do the work for a full repaint
	if (!m_bNeedFullRepaint)
	{
		// not a forced full text repaint, check text difference
		if (!bFullRepaint) bFullRepaint = (GetBufferDifference() > 15);

		// repaint text layer
 		if (bFullRepaint)
 		{
			RepaintText(m_dcText);
		}
		else
		{
			RepaintTextChanges(m_dcText);
		}
	}

	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::MainframeMoving()
{
	// next OnPaint will do a full repaint
	if (m_tabData->imageData.bRelative) m_bNeedFullRepaint = true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetResizing(bool bResizing)
{
	m_bResizing = bResizing;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetActive(bool bActive)
{
	m_bActive = bActive;
	if (!m_bActive) return;

	Repaint(true);
	UpdateTitle();
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetTitle(const CString& strTitle)
{
	CString	title(strTitle);

	if (m_strUser.GetLength() > 0)
	{
		title.Format(L"[%s] %s", m_strUser, strTitle);
	}

	m_strTitle = title;
	SetWindowText(m_strTitle);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

CString ConsoleView::GetConsoleCommand()
{
	CWindow consoleWnd(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow);
	CString strConsoleTitle(L"");

	consoleWnd.GetWindowText(strConsoleTitle);

	int nPos = strConsoleTitle.Find(L'-');

	if (nPos == -1)
	{
		nPos = strConsoleTitle.Find(L"Console2 command window");

		if (nPos == -1)
		{
			return CString(L" - ") + strConsoleTitle;
		}
		else
		{
			return CString();
		}
	}
	else
	{
		return strConsoleTitle.Right(strConsoleTitle.GetLength() - nPos + 1);
	}
}

/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::Copy(const CPoint* pPoint /* = NULL */)
{
	if ((m_selectionHandler->GetState() != SelectionHandler::selstateSelecting) &&
		(m_selectionHandler->GetState() != SelectionHandler::selstateSelected))
	{
		return;
	}

	bool bCopied = false;

	if (!g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bSensitiveCopy)
	{
		pPoint = 0;
	}

	if (pPoint != NULL)
	{
		bCopied = m_selectionHandler->CopySelection(GetConsoleCoord(*pPoint));
	}
	else
	{
		// called by mainframe
		m_selectionHandler->CopySelection();
		bCopied = true;
	}

	if (!bCopied || g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bClearOnCopy) m_selectionHandler->ClearSelection();
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::ClearSelection()
{
	if ((m_selectionHandler->GetState() != SelectionHandler::selstateSelecting) &&
		(m_selectionHandler->GetState() != SelectionHandler::selstateSelected))
	{
		return;
	}

	m_selectionHandler->ClearSelection();
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::Paste()
{
	if (!CanPaste()) return;
	::SendMessage(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, WM_SYSCOMMAND, SC_CONSOLE_PASTE, 0);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::DumpBuffer()
{
	wofstream of;
	of.open(Helpers::ExpandEnvironmentStrings(_T("%temp%\\console.dump")).c_str());
	DWORD       dwOffset = 0;
	MutexLock	bufferLock(m_bufferMutex);

	for (DWORD i = 0; i < m_consoleHandler.GetConsoleParams()->dwRows; ++i)
	{
		for (DWORD j = 0; j < m_consoleHandler.GetConsoleParams()->dwColumns; ++j)
		{
			of << m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar;
			++dwOffset;
		}

		of << endl;
	}

	of.close();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::OnConsoleChange(bool bResize)
{
	SharedMemory<ConsoleParams>&	consoleParams	= m_consoleHandler.GetConsoleParams();
	DWORD							dwBufferSize	= consoleParams->dwRows * consoleParams->dwColumns;

	SharedMemory<ConsoleInfo>&	consoleInfo = m_consoleHandler.GetConsoleInfo();
	SharedMemory<CHAR_INFO>&	consoleBuffer = m_consoleHandler.GetConsoleBuffer();

	SharedMemoryLock	consoleInfoLock(consoleInfo);
	SharedMemoryLock	sharedBufferLock(consoleBuffer);
	MutexLock			localBufferLock(m_bufferMutex);

	// console size changed, resize local buffer
	if (bResize)
	{
		m_screenBuffer.reset(new CharInfo[dwBufferSize]);
	}

	// copy changed data
	for (DWORD dwOffset = 0; dwOffset < dwBufferSize; ++dwOffset)
	{
		if (memcmp(&(m_screenBuffer[dwOffset].charInfo), &(consoleBuffer[dwOffset]), sizeof(CHAR_INFO)))
		{
			memcpy(&(m_screenBuffer[dwOffset].charInfo), &(consoleBuffer[dwOffset]), sizeof(CHAR_INFO));
			m_screenBuffer[dwOffset].changed = true;
		}
	}

	WPARAM wParam = 0;

	if (bResize) wParam |= UPDATE_CONSOLE_RESIZE;

	if (consoleInfo->textChanged)
	{
		wParam |= UPDATE_CONSOLE_TEXT_CHANGED;
		consoleInfo->textChanged = false;
	}

	PostMessage(UM_UPDATE_CONSOLE_VIEW, wParam);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::OnConsoleClose()
{
	if (::IsWindow(m_hWnd)) m_mainFrame.PostMessage(UM_CONSOLE_CLOSED, reinterpret_cast<WPARAM>(m_hWnd), 0);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::CreateOffscreenBuffers()
{
	CWindowDC	dcWindow(m_hWnd);
	CRect		rectWindowMax;
//	CRect		rectWindow;

	// create font
	if (!CreateFont(m_appearanceSettings.fontSettings.strName))
	{
		CreateFont(wstring(L"Courier New"));
	}

	// get ClearType status
	BOOL	bSmoothing		= FALSE;
	UINT	uSmoothingType	= 0;
	CDC		dcDdesktop(::GetDC(NULL));
	
	::SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, (void*)&bSmoothing, 0);
	::SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, (void*)&uSmoothingType, 0);

	if
	(
		(
			(dcDdesktop.GetDeviceCaps(BITSPIXEL)*dcDdesktop.GetDeviceCaps(PLANES) == 32) // 32-bit depth only
			&&
			bSmoothing
			&& 
			(uSmoothingType == FE_FONTSMOOTHINGCLEARTYPE)
		)
		||
		(m_appearanceSettings.fontSettings.fontSmoothing == fontSmoothCleartype)
	)
	{
		m_bUseTextAlphaBlend = true;
	}
	else
	{
		m_bUseTextAlphaBlend = false;
	}

	// get max window rect based on font and console size
	GetMaxRect(rectWindowMax);
	GetRect(rectWindowMax);

	// create offscreen bitmaps if needed
	if (m_bmpOffscreen.IsNull()) CreateOffscreenBitmap(m_dcOffscreen, rectWindowMax, m_bmpOffscreen);
	if (m_bmpText.IsNull()) CreateOffscreenBitmap(m_dcText, rectWindowMax, m_bmpText);

	// create background brush
	m_backgroundBrush.CreateSolidBrush(m_tabData->crBackgroundColor);

	// initial offscreen paint
	m_dcOffscreen.FillRect(&rectWindowMax, m_backgroundBrush);

	// set text DC stuff
	m_dcText.SetBkMode(OPAQUE);
	m_dcText.FillRect(&rectWindowMax, m_backgroundBrush);

	// create selection handler
	m_selectionHandler.reset(new SelectionHandler(
									m_hWnd, 
									dcWindow, 
									rectWindowMax, 
									m_consoleHandler,
									m_consoleHandler.GetConsoleParams(), 
									m_consoleHandler.GetConsoleInfo(), 
									m_consoleHandler.GetCopyInfo(),
									m_nCharWidth, 
									m_nCharHeight));

	// create and initialize cursor
	CRect		rectCursor(0, 0, m_nCharWidth, m_nCharHeight);

	m_cursor.reset();
	m_cursor = CursorFactory::CreateCursor(
								m_hWnd, 
								m_bAppActive, 
								m_tabData.get() ? static_cast<CursorStyle>(m_tabData->dwCursorStyle) : cstyleXTerm, 
								dcWindow, 
								rectCursor, 
								m_tabData.get() ? static_cast<CursorStyle>(m_tabData->crCursorColor) : RGB(255, 255, 255));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::CreateOffscreenBitmap(CDC& cdc, const CRect& rect, CBitmap& bitmap)
{
	if (!bitmap.IsNull()) return;// bitmap.DeleteObject();

	Helpers::CreateBitmap(cdc, rect.Width(), rect.Height(), bitmap);
	cdc.SelectBitmap(bitmap);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::CreateFont(const wstring& strFontName)
{
	if (!m_fontText.IsNull()) return true;// m_fontText.DeleteObject();

	BYTE	byFontQuality = DEFAULT_QUALITY;

	switch (m_appearanceSettings.fontSettings.fontSmoothing)
	{
		case fontSmoothDefault	: byFontQuality = DEFAULT_QUALITY;			break;
		case fontSmoothNone		: byFontQuality = NONANTIALIASED_QUALITY;	break;
		case fontSmoothCleartype: byFontQuality = CLEARTYPE_QUALITY;		break;
		default : DEFAULT_QUALITY;
	}
	m_fontText.CreateFont(
		-::MulDiv(m_appearanceSettings.fontSettings.dwSize , m_dcText.GetDeviceCaps(LOGPIXELSY), 72),
		0,
		0,
		0,
		m_appearanceSettings.fontSettings.bBold ? FW_BOLD : 0,
		m_appearanceSettings.fontSettings.bItalic,
		FALSE,
		FALSE,
 		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		byFontQuality,
		DEFAULT_PITCH,
		strFontName.c_str());

	TEXTMETRIC	textMetric;

	m_dcText.SelectFont(m_fontText);
	m_dcText.GetTextMetrics(&textMetric);

	if (textMetric.tmPitchAndFamily & TMPF_FIXED_PITCH)
	{
		if (!m_fontText.IsNull()) m_fontText.DeleteObject();
		return false;
	}

	// fixed pitch font (TMPF_FIXED_PITCH is cleared!!!)
	m_nCharWidth = textMetric.tmAveCharWidth;
	m_nCharHeight = textMetric.tmHeight;

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::InitializeScrollbars()
{
	SharedMemory<ConsoleParams>& consoleParams = m_consoleHandler.GetConsoleParams();

 	m_bShowVScroll = m_appearanceSettings.controlsSettings.bShowScrollbars && (consoleParams->dwBufferRows > consoleParams->dwRows);
 	m_bShowHScroll = m_appearanceSettings.controlsSettings.bShowScrollbars && (consoleParams->dwBufferColumns > consoleParams->dwColumns);

//	if (m_nScrollbarStyle != FSB_REGULAR_MODE)

	if (m_appearanceSettings.controlsSettings.bFlatScrollbars)
	{
		::InitializeFlatSB(m_hWnd);
		::FlatSB_SetScrollProp(m_hWnd, WSB_PROP_VSTYLE, FSB_ENCARTA_MODE, TRUE);
	}
	else
	{
		::UninitializeFlatSB(m_hWnd);
	}

  	::FlatSB_ShowScrollBar(m_hWnd, SB_VERT, m_bShowVScroll);
  	::FlatSB_ShowScrollBar(m_hWnd, SB_HORZ, m_bShowHScroll);

/*
	TRACE(L"InitializeScrollbars, console wnd: 0x%08X\n", m_hWnd);
	TRACE(L"Sizes: %i, %i    %i, %i\n", consoleParams->dwRows, consoleParams->dwBufferRows - 1, consoleParams->dwColumns, consoleParams->dwBufferColumns - 1);
	TRACE(L"----------------------------------------------------------------\n");
*/

	if (m_appearanceSettings.controlsSettings.bShowScrollbars && (consoleParams->dwBufferRows > consoleParams->dwRows))
	{
		// set vertical scrollbar stuff
		SCROLLINFO	si ;

		si.cbSize	= sizeof(SCROLLINFO) ;
		si.fMask	= SIF_PAGE | SIF_RANGE ;
		si.nPage	= consoleParams->dwRows;
		si.nMax		= consoleParams->dwBufferRows - 1;
		si.nMin		= 0 ;

		::FlatSB_SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
	}

	if (m_appearanceSettings.controlsSettings.bShowScrollbars && (consoleParams->dwBufferColumns > consoleParams->dwColumns))
	{
		// set vertical scrollbar stuff
		SCROLLINFO	si ;

		si.cbSize	= sizeof(SCROLLINFO) ;
		si.fMask	= SIF_PAGE | SIF_RANGE ;
		si.nPage	= consoleParams->dwColumns;
		si.nMax		= consoleParams->dwBufferColumns - 1;
		si.nMin		= 0 ;

		::FlatSB_SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);
	}
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::DoScroll(int nType, int nScrollCode, int nThumbPos)
{
	int nCurrentPos = ::FlatSB_GetScrollPos(m_hWnd, nType);
	int nDelta = 0;

	ScrollSettings& scrollSettings = g_settingsHandler->GetBehaviorSettings().scrollSettings;
	
	switch(nScrollCode)
	{ 
		case SB_PAGEUP: /* SB_PAGELEFT */

			if (scrollSettings.dwPageScrollRows > 0)
			{
				nDelta = -static_cast<int>(scrollSettings.dwPageScrollRows);
			}
			else
			{
				nDelta = (nType == SB_VERT) ? -static_cast<int>(m_consoleHandler.GetConsoleParams()->dwRows) : -static_cast<int>(m_consoleHandler.GetConsoleParams()->dwColumns);
			}
			break; 
			
		case SB_PAGEDOWN: /* SB_PAGERIGHT */
			if (scrollSettings.dwPageScrollRows > 0)
			{
				nDelta = static_cast<int>(scrollSettings.dwPageScrollRows);
			}
			else
			{
				nDelta = (nType == SB_VERT) ? static_cast<int>(m_consoleHandler.GetConsoleParams()->dwRows) : static_cast<int>(m_consoleHandler.GetConsoleParams()->dwColumns);
			}
			break; 
			
		case SB_LINEUP: /* SB_LINELEFT */
			nDelta = -1; 
			break; 
			
		case SB_LINEDOWN: /* SB_LINERIGHT */
			nDelta = 1; 
			break; 
			
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			nDelta = nThumbPos - nCurrentPos; 
			break;
			
		case SB_ENDSCROLL:
			return;
			
		default: 
			return;
	}
	
	if (nDelta != 0)
	{
		SharedMemory<SIZE>& newScrollPos = m_consoleHandler.GetNewScrollPos();

		if (nType == SB_VERT)
		{
			newScrollPos->cx = 0;
			newScrollPos->cy = nDelta;
		}
		else
		{
			newScrollPos->cx = nDelta;
			newScrollPos->cy = 0;
		}

		newScrollPos.SetReqEvent();
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

DWORD ConsoleView::GetBufferDifference()
{
	DWORD		dwCount				= m_consoleHandler.GetConsoleParams()->dwRows * m_consoleHandler.GetConsoleParams()->dwColumns;
	DWORD		dwChangedPositions	= 0;
	MutexLock	bufferLock(m_bufferMutex);

	for (DWORD i = 0; i < dwCount; ++i)
	{
		if (m_screenBuffer[i].changed) ++dwChangedPositions;
	}

	return dwChangedPositions*100/dwCount;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::UpdateTitle()
{
	if (g_settingsHandler->GetAppearanceSettings().windowSettings.bUseConsoleTitle)
	{
		CWindow consoleWnd(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow);
		CString strConsoleTitle(L"");

		consoleWnd.GetWindowText(strConsoleTitle);

		SetTitle(strConsoleTitle);
	}

	m_mainFrame.PostMessage(
					UM_UPDATE_TITLES, 
					reinterpret_cast<WPARAM>(m_hWnd), 
					0);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::RepaintText(CDC& dc)
{
	SIZE	bitmapSize;
	CRect	bitmapRect;

	dc.GetCurrentBitmap().GetSize(bitmapSize);
	bitmapRect.left		= 0;
	bitmapRect.top		= 0;
	bitmapRect.right	= bitmapSize.cx;
	bitmapRect.bottom	= bitmapSize.cy;

	if (m_tabData->backgroundImageType == bktypeNone)
	{
		dc.FillRect(&bitmapRect, m_backgroundBrush);
	}
	else
	{
		CRect	rectWindow;
		GetClientRect(&rectWindow);

		g_imageHandler->UpdateImageBitmap(dc, rectWindow, m_background);

		if (m_tabData->imageData.bRelative)
		{
			CPoint	pointClientScreen(0, 0);
			ClientToScreen(&pointClientScreen);

			dc.BitBlt(
				rectWindow.left, 
				rectWindow.top, 
				rectWindow.right, 
				rectWindow.bottom, 
				m_background->dcImage, 
				rectWindow.left + pointClientScreen.x - ::GetSystemMetrics(SM_XVIRTUALSCREEN), 
				rectWindow.top + pointClientScreen.y - ::GetSystemMetrics(SM_YVIRTUALSCREEN), 
				SRCCOPY);
		}
		else
		{
			dc.BitBlt(
				bitmapRect.left, 
				bitmapRect.top, 
				bitmapRect.right, 
				bitmapRect.bottom, 
				m_background->dcImage, 
				bitmapRect.left, 
				bitmapRect.top, 
				SRCCOPY);
		}
	}

	StylesSettings&					stylesSettings	= g_settingsHandler->GetAppearanceSettings().stylesSettings;
	SharedMemory<ConsoleParams>&	consoleParams	= m_consoleHandler.GetConsoleParams();
	MutexLock						bufferLock(m_bufferMutex);

	DWORD dwX			= stylesSettings.dwInsideBorder;
	DWORD dwY			= stylesSettings.dwInsideBorder;
	DWORD dwOffset		= 0;
	
	WORD attrBG;

	// stuff used for caching
	int			nBkMode		= TRANSPARENT;
	COLORREF	crBkColor	= RGB(0, 0, 0);
	COLORREF	crTxtColor	= RGB(0, 0, 0);
	
	int			nCharWidths	= 0;
	bool		bTextOut	= false;

	wstring		strText(L"");

	for (DWORD i = 0; i < consoleParams->dwRows; ++i)
	{
		dwX = stylesSettings.dwInsideBorder;
		dwY = i*m_nCharHeight + stylesSettings.dwInsideBorder;

		nBkMode			= TRANSPARENT;
		crBkColor		= RGB(0, 0, 0);
		crTxtColor		= RGB(0, 0, 0);
		
		nCharWidths		= 0;
		bTextOut		= false;
		
		attrBG = (m_screenBuffer[dwOffset].charInfo.Attributes & 0xFF) >> 4;
		
		// here we decide how to paint text over the background
		if (m_consoleSettings.consoleColors[attrBG] == RGB(0, 0, 0))
		{
			nBkMode		= TRANSPARENT;
		}
		else
		{
			nBkMode		= OPAQUE;
			crBkColor	= m_consoleSettings.consoleColors[attrBG];
		}

		dc.SetBkMode(nBkMode);
		dc.SetBkColor(crBkColor);

		crTxtColor		= m_appearanceSettings.fontSettings.bUseColor ? m_appearanceSettings.fontSettings.crFontColor : m_consoleSettings.consoleColors[m_screenBuffer[dwOffset].charInfo.Attributes & 0xF];
		dc.SetTextColor(crTxtColor);

		strText		= m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar;
		m_screenBuffer[dwOffset].changed = false;

		nCharWidths	= 1;
		++dwOffset;

		for (DWORD j = 1; j < consoleParams->dwColumns; ++j, ++dwOffset)
		{
			if (m_screenBuffer[dwOffset].charInfo.Attributes & COMMON_LVB_TRAILING_BYTE)
			{
				m_screenBuffer[dwOffset].changed = false;
				++nCharWidths;
				continue;
			}
			
			attrBG = (m_screenBuffer[dwOffset].charInfo.Attributes & 0xFF) >> 4;

			if (m_consoleSettings.consoleColors[attrBG] == RGB(0, 0, 0))
			{
				if (nBkMode != TRANSPARENT)
				{
					nBkMode = TRANSPARENT;
					bTextOut = true;
				}
			}
			else
			{
				if (nBkMode != OPAQUE)
				{
					nBkMode = OPAQUE;
					bTextOut = true;
				}
				if (crBkColor != m_consoleSettings.consoleColors[attrBG])
				{
					crBkColor = m_consoleSettings.consoleColors[attrBG];
					bTextOut = true;
				}
			}

			if (crTxtColor != (m_appearanceSettings.fontSettings.bUseColor ? m_appearanceSettings.fontSettings.crFontColor : m_consoleSettings.consoleColors[m_screenBuffer[dwOffset].charInfo.Attributes & 0xF]))
			{
				crTxtColor = m_appearanceSettings.fontSettings.bUseColor ? m_appearanceSettings.fontSettings.crFontColor : m_consoleSettings.consoleColors[m_screenBuffer[dwOffset].charInfo.Attributes & 0xF];
				bTextOut = true;
			}

			if (bTextOut)
			{
				CRect textOutRect(dwX, dwY, dwX+m_nCharWidth*nCharWidths, dwY+m_nCharHeight);

				dc.ExtTextOut(dwX, dwY, 0, &textOutRect, strText.c_str(), static_cast<int>(strText.length()), NULL);
				dwX += static_cast<int>(nCharWidths * m_nCharWidth);

				dc.SetBkMode(nBkMode);
				dc.SetBkColor(crBkColor);
				dc.SetTextColor(crTxtColor);

				strText		= m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar;
				m_screenBuffer[dwOffset].changed = false;
				nCharWidths	= 1;
				bTextOut	= false;
			}
			else
			{
				strText += m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar;
				m_screenBuffer[dwOffset].changed = false;
				++nCharWidths;
			}
		}

		if (strText.length() > 0)
		{
			CRect textOutRect(dwX, dwY, dwX+m_nCharWidth*nCharWidths, dwY+m_nCharHeight);
			dc.ExtTextOut(dwX, dwY, 0, &textOutRect, strText.c_str(), static_cast<int>(strText.length()), NULL);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::RepaintTextChanges(CDC& dc)
{
	StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

	DWORD	dwX			= stylesSettings.dwInsideBorder;
	DWORD	dwY			= stylesSettings.dwInsideBorder;
	DWORD	dwOffset	= 0;
	
	WORD	attrBG;

	MutexLock bufferLock(m_bufferMutex);

	CRect	rectWindow;
	GetClientRect(&rectWindow);

	if (m_tabData->backgroundImageType != bktypeNone) g_imageHandler->UpdateImageBitmap(dc, rectWindow, m_background);

	for (DWORD i = 0; i < m_consoleHandler.GetConsoleParams()->dwRows; ++i)
	{
		dwX = stylesSettings.dwInsideBorder;
		dwY = i*m_nCharHeight + stylesSettings.dwInsideBorder;

		for (DWORD j = 0; j < m_consoleHandler.GetConsoleParams()->dwColumns; ++j, ++dwOffset, dwX += m_nCharWidth)
		{
			if (m_screenBuffer[dwOffset].changed)
			{
				m_screenBuffer[dwOffset].changed = false;

				if (m_screenBuffer[dwOffset].charInfo.Attributes & COMMON_LVB_TRAILING_BYTE) continue;

				CRect rect;
				rect.top	= dwY;
				rect.left	= dwX;
				rect.bottom	= dwY + m_nCharHeight;
				// we have to erase two spaces for double-width characters
				rect.right	= (m_screenBuffer[dwOffset].charInfo.Attributes & COMMON_LVB_LEADING_BYTE) ? dwX + 2*m_nCharWidth : dwX + m_nCharWidth;
				
				if (m_tabData->backgroundImageType == bktypeNone)
				{
					dc.FillRect(&rect, m_backgroundBrush);
				}
				else
				{
					if (m_tabData->imageData.bRelative)
					{
						CPoint	pointClientScreen(0, 0);
						ClientToScreen(&pointClientScreen);

						dc.BitBlt(
							rect.left, 
							rect.top, 
							rect.Width(), 
							rect.Height(), 
							m_background->dcImage, 
							rect.left + pointClientScreen.x - ::GetSystemMetrics(SM_XVIRTUALSCREEN), 
							rect.top + pointClientScreen.y - ::GetSystemMetrics(SM_YVIRTUALSCREEN), 
							SRCCOPY);
					}
					else
					{
						dc.BitBlt(
							rect.left, 
							rect.top, 
							rect.Width(),
							rect.Height(),
							m_background->dcImage, 
							rect.left, 
							rect.top, 
							SRCCOPY);
					}
				}

				attrBG = (m_screenBuffer[dwOffset].charInfo.Attributes & 0xFF) >> 4;

				// here we decide how to paint text over the background
				if (m_consoleSettings.consoleColors[attrBG] == RGB(0, 0, 0))
				{
					dc.SetBkMode(TRANSPARENT);
				}
				else
				{
					dc.SetBkMode(OPAQUE);
					dc.SetBkColor(m_consoleSettings.consoleColors[attrBG]);
				}
				
				dc.SetBkColor(m_consoleSettings.consoleColors[attrBG]);
				dc.SetTextColor(m_appearanceSettings.fontSettings.bUseColor ? m_appearanceSettings.fontSettings.crFontColor : m_consoleSettings.consoleColors[m_screenBuffer[dwOffset].charInfo.Attributes & 0xF]);

				dc.ExtTextOut(dwX, dwY, ETO_CLIPPED, &rect, &(m_screenBuffer[dwOffset].charInfo.Char.UnicodeChar), 1, NULL);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::BitBltOffscreen(bool bOnlyCursor /*= false*/)
{
	CRect			rectBlit;
	StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

	if (bOnlyCursor)
	{
		// blit only cursor
		if (!(m_cursor) || !m_consoleHandler.GetCursorInfo()->bVisible) return;

		SharedMemory<ConsoleInfo>& consoleInfo = m_consoleHandler.GetConsoleInfo();

		rectBlit		= m_cursor->GetCursorRect();
		rectBlit.left	+= (consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth + stylesSettings.dwInsideBorder;
		rectBlit.top	+= (consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top) * m_nCharHeight + stylesSettings.dwInsideBorder;
		rectBlit.right	+= (consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth + stylesSettings.dwInsideBorder;
		rectBlit.bottom	+= (consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top) * m_nCharHeight + stylesSettings.dwInsideBorder;
	}
	else
	{
		// blit rect is entire view
		GetClientRect(&rectBlit);
	}

	// we can skip this for relative background images when a full repaint 
	// is needed (UpdateOffscreen will be called in OnPaint)
	if (!m_tabData->imageData.bRelative || (m_tabData->imageData.bRelative && !m_bNeedFullRepaint))
	{
		// we don't do this for relative backgrounds here
		UpdateOffscreen(rectBlit);
	}

	InvalidateRect(&rectBlit, FALSE);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::UpdateOffscreen(const CRect& rectBlit)
{
	m_dcOffscreen.BitBlt(
					rectBlit.left, 
					rectBlit.top, 
					rectBlit.right, 
					rectBlit.bottom, 
					m_dcText, 
					rectBlit.left, 
					rectBlit.top, 
					SRCCOPY);

	// blit cursor
	if (m_consoleHandler.GetCursorInfo()->bVisible && (m_cursor.get() != NULL))
	{
		CRect			rectCursor(0, 0, 0, 0);
		SharedMemory<ConsoleInfo>& consoleInfo = m_consoleHandler.GetConsoleInfo();
		StylesSettings& stylesSettings = g_settingsHandler->GetAppearanceSettings().stylesSettings;

		// don't blit if cursor is outside visible window
		if ((consoleInfo->csbi.dwCursorPosition.X >= consoleInfo->csbi.srWindow.Left) &&
			(consoleInfo->csbi.dwCursorPosition.X <= consoleInfo->csbi.srWindow.Right) &&
			(consoleInfo->csbi.dwCursorPosition.Y >= consoleInfo->csbi.srWindow.Top) &&
			(consoleInfo->csbi.dwCursorPosition.Y <= consoleInfo->csbi.srWindow.Bottom))
		{
			rectCursor			= m_cursor->GetCursorRect();
			rectCursor.left		+= (consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth + stylesSettings.dwInsideBorder;
			rectCursor.top		+= (consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top) * m_nCharHeight + stylesSettings.dwInsideBorder;
			rectCursor.right	+= (consoleInfo->csbi.dwCursorPosition.X - consoleInfo->csbi.srWindow.Left) * m_nCharWidth + stylesSettings.dwInsideBorder;
			rectCursor.bottom	+= (consoleInfo->csbi.dwCursorPosition.Y - consoleInfo->csbi.srWindow.Top) * m_nCharHeight + stylesSettings.dwInsideBorder;

			m_cursor->BitBlt(
						m_dcOffscreen, 
						rectCursor.left, 
						rectCursor.top);
		}
	}

	// blit selection
	m_selectionHandler->BitBlt(m_dcOffscreen);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::SendTextToConsole(const wchar_t* pszText)
{
	if (pszText == NULL) return;

	size_t textLen = wcslen(pszText);

	if (textLen == 0) return;

	SharedMemory<TextInfo>&	textInfo = m_consoleHandler.GetTextInfo();

	{
		SharedMemoryLock		memLock(textInfo);

		void* pRemoteMemory = ::VirtualAllocEx(
									m_consoleHandler.GetConsoleHandle().get(),
									NULL, 
									(textLen+1)*sizeof(wchar_t), 
									MEM_COMMIT, 
									PAGE_READWRITE);

		if (pRemoteMemory == NULL) return;

		if (!::WriteProcessMemory(
					m_consoleHandler.GetConsoleHandle().get(),
					pRemoteMemory, 
					(PVOID)pszText, 
					(textLen+1)*sizeof(wchar_t), 
					NULL))
		{
			::VirtualFreeEx(m_consoleHandler.GetConsoleHandle().get(), pRemoteMemory, NULL, MEM_RELEASE);
			return;
		}

		textInfo->mem = reinterpret_cast<UINT_PTR>(pRemoteMemory);
		textInfo.SetReqEvent();
	}

	::WaitForSingleObject(textInfo.GetRespEvent(), INFINITE);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

bool ConsoleView::TranslateKeyDown(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	if (uMsg == WM_KEYDOWN)
	{
		if (m_hotkeys.bUseScrollLock && ((::GetKeyState(VK_SCROLL) & 0x01) == 0x01))
		{
			switch(wParam)
			{
				case VK_UP:   
					DoScroll(SB_VERT, SB_LINEUP, 0); 
					return true;

				case VK_PRIOR:  
					DoScroll(SB_VERT, SB_PAGEUP, 0); 
					return true;

				case VK_DOWN: 
					DoScroll(SB_VERT, SB_LINEDOWN, 0); 
					return true;

				case VK_NEXT: 
					DoScroll(SB_VERT, SB_PAGEDOWN, 0); 
					return true;

				case VK_LEFT: 
					DoScroll(SB_HORZ, SB_LINELEFT, 0); 
					return true;

				case VK_RIGHT:  
					DoScroll(SB_HORZ, SB_LINERIGHT, 0); 
					return true;
			}
		}
	}
	
	if ((uMsg == WM_SYSKEYDOWN) || (uMsg == WM_SYSKEYUP))
	{
		// eat ALT+ENTER
		if ((wParam == VK_RETURN) && ((::GetKeyState(VK_MENU) & 0x80) == 0x80))
		{
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::ForwardMouseClick(UINT uMsg, WPARAM wParam, const CPoint& point)
{
	DWORD dwMouseButtonState= 0;
	DWORD dwControlKeyState	= 0;
	DWORD dwEventFlags		= 0;

	if (uMsg == WM_MOUSEMOVE)
	{
		dwEventFlags |= MOUSE_MOVED;

		UINT	uFlags = static_cast<UINT>(wParam);

		if ((uFlags & MK_LBUTTON) != 0)
		{
			dwMouseButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
		}
		else if ((uFlags & MK_RBUTTON) != 0)
		{
			dwMouseButtonState = RIGHTMOST_BUTTON_PRESSED;
		}
		else if ((uFlags & MK_MBUTTON) != 0)
		{
			dwMouseButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
		}
		else if ((uFlags & MK_XBUTTON1) != 0)
		{
			dwMouseButtonState = FROM_LEFT_3RD_BUTTON_PRESSED;
		}
		else if ((uFlags & MK_XBUTTON2) != 0)
		{
			dwMouseButtonState = FROM_LEFT_4TH_BUTTON_PRESSED;
		}
	}
	else
	{
		// one of mouse click messages
//		UINT	uKeys			= GET_KEYSTATE_WPARAM(wParam); 
		UINT	uXButton		= GET_XBUTTON_WPARAM(wParam);

		switch (uMsg)
		{
			case WM_LBUTTONDOWN : 
			{
				dwMouseButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
				break;
			}

			case WM_LBUTTONDBLCLK : 
			{
				dwMouseButtonState	 = FROM_LEFT_1ST_BUTTON_PRESSED;
				dwEventFlags		|= DOUBLE_CLICK;
				break;
			}

			case WM_RBUTTONDOWN : 
			{
				dwMouseButtonState = RIGHTMOST_BUTTON_PRESSED;
				break;
			}

			case WM_RBUTTONDBLCLK : 
			{
				dwMouseButtonState	 = RIGHTMOST_BUTTON_PRESSED;
				dwEventFlags		|= DOUBLE_CLICK;
				break;
			}

			case WM_MBUTTONDOWN : 
			{
				dwMouseButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
				break;
			}

			case WM_MBUTTONDBLCLK : 
			{
				dwMouseButtonState	 = FROM_LEFT_2ND_BUTTON_PRESSED;
				dwEventFlags		|= DOUBLE_CLICK;
				break;
			}

			case WM_XBUTTONDOWN : 
			{
				if (uXButton == XBUTTON1)
				{
					dwMouseButtonState = FROM_LEFT_3RD_BUTTON_PRESSED;
				}
				else
				{
					dwMouseButtonState = FROM_LEFT_4TH_BUTTON_PRESSED;
				}
				break;
			}

			case WM_XBUTTONDBLCLK : 
			{
				if (uXButton == XBUTTON1)
				{
					dwMouseButtonState	= FROM_LEFT_3RD_BUTTON_PRESSED;
					dwEventFlags		|= DOUBLE_CLICK;
				}
				else
				{
					dwMouseButtonState	= FROM_LEFT_4TH_BUTTON_PRESSED;
					dwEventFlags		|= DOUBLE_CLICK;
				}
				break;
			}
		}
	}

	// get control key states
	if (GetKeyState(VK_RMENU) < 0)		dwControlKeyState |= RIGHT_ALT_PRESSED;
	if (GetKeyState(VK_LMENU) < 0)		dwControlKeyState |= LEFT_ALT_PRESSED;
	if (GetKeyState(VK_RCONTROL) < 0)	dwControlKeyState |= RIGHT_CTRL_PRESSED;
	if (GetKeyState(VK_LCONTROL) < 0)	dwControlKeyState |= LEFT_CTRL_PRESSED;
	if (GetKeyState(VK_CAPITAL) < 0)	dwControlKeyState |= CAPSLOCK_ON;
	if (GetKeyState(VK_NUMLOCK) < 0)	dwControlKeyState |= NUMLOCK_ON;
	if (GetKeyState(VK_SCROLL) < 0)		dwControlKeyState |= SCROLLLOCK_ON;
	if (GetKeyState(VK_SHIFT) < 0)		dwControlKeyState |= SHIFT_PRESSED;


	m_consoleHandler.SendMouseEvent(GetConsoleCoord(point), dwMouseButtonState, dwControlKeyState, dwEventFlags);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

COORD ConsoleView::GetConsoleCoord(const CPoint& clientPoint)
{
	StylesSettings& stylesSettings	= g_settingsHandler->GetAppearanceSettings().stylesSettings;

	DWORD			dwColumns		= m_consoleHandler.GetConsoleParams()->dwColumns;
	DWORD			dwBufferColumns	= m_consoleHandler.GetConsoleParams()->dwBufferColumns;
	SMALL_RECT&		srWindow		= m_consoleHandler.GetConsoleInfo()->csbi.srWindow;

	CPoint			point(clientPoint);
	COORD			consolePoint;
	SHORT			maxX = (dwBufferColumns > 0) ? static_cast<SHORT>(dwBufferColumns - 1) : static_cast<SHORT>(dwColumns - 1);

	consolePoint.X = static_cast<SHORT>((point.x - static_cast<LONG>(stylesSettings.dwInsideBorder)) / m_nCharWidth + srWindow.Left);
	consolePoint.Y = static_cast<SHORT>((point.y - static_cast<LONG>(stylesSettings.dwInsideBorder)) / m_nCharHeight + srWindow.Top);

	if (consolePoint.X < 0)
	{
		consolePoint.X = maxX;
		--consolePoint.Y;
	}

	if (consolePoint.X > srWindow.Right) consolePoint.X = srWindow.Right;

	if (consolePoint.Y < 0) consolePoint.Y = 0;

	if (consolePoint.Y > srWindow.Bottom) consolePoint.Y = srWindow.Bottom;

	return consolePoint;
}

/////////////////////////////////////////////////////////////////////////////


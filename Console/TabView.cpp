#include "stdafx.h"
#include "resource.h"

#include "Console.h"
#include "TabView.h"
#include "DlgCredentials.h"
#include "MainFrame.h"

int CMultiSplitPane::splitBarWidth  = 0;
int CMultiSplitPane::splitBarHeight = 0;


//////////////////////////////////////////////////////////////////////////////

TabView::TabView(MainFrame& mainFrame, shared_ptr<TabData> tabData)
:m_mainFrame(mainFrame)
,m_viewsMutex(NULL, FALSE, NULL)
,m_tabData(tabData)
,m_strTitle(tabData->strTitle.c_str())
,m_bigIcon()
,m_smallIcon()
,m_boolIsGrouped(false)
{
}

TabView::~TabView()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL TabView::PreTranslateMessage(MSG* pMsg)
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
  /*
  shared_ptr<ConsoleView> consoleView = this->GetActiveConsole(_T(__FUNCTION__));
  if( consoleView )
    return consoleView->PreTranslateMessage(pMsg);
  return FALSE;
  */
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT TabView::OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
  // load icon
  m_bigIcon.Attach(Helpers::LoadTabIcon(true, m_tabData->bUseDefaultIcon, m_tabData->strIcon, m_tabData->strShell));
  m_smallIcon.Attach(Helpers::LoadTabIcon(false, m_tabData->bUseDefaultIcon, m_tabData->strIcon, m_tabData->strShell));

  LRESULT result = -1;

  ATLTRACE(_T("TabView::OnCreate\n"));
  MutexLock viewMapLock(m_viewsMutex);
  HWND hwndConsoleView = CreateNewConsole();
  if( hwndConsoleView )
  {
    result = multisplitClass::OnCreate(uMsg, wParam, lParam, bHandled);
    TRACE(L"multisplitClass::OnCreate returns %p\n", result);
    if( result == 0 )
    {
      multisplitClass::tree.window = hwndConsoleView;
      CRect rect;
      m_views.begin()->second->GetRect(rect);
      multisplitClass::RectSet(rect, true);
    }
  }

  bHandled = TRUE;
  ATLTRACE(_T("TabView::OnCreate done\n"));
  return result; // windows sets focus to first control
}

LRESULT TabView::OnEraseBackground (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
	// handled, no background painting needed
	return 1;
}

LRESULT TabView::OnSize (UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
  if (wParam != SIZE_MINIMIZED && m_mainFrame.m_bOnCreateDone)
  {
    multisplitClass::RectSet(); // to ClientRect
  }

  bHandled = FALSE;
  return 1;
}

HWND TabView::CreateNewConsole(void)
{
	DWORD dwRows    = g_settingsHandler->GetConsoleSettings().dwRows;
	DWORD dwColumns = g_settingsHandler->GetConsoleSettings().dwColumns;

	MutexLock	viewMapLock(m_viewsMutex);
#if 0
	if (m_views.size() > 0)
	{
		SharedMemory<ConsoleParams>& consoleParams = m_views.begin()->second->GetConsoleHandler().GetConsoleParams();
		dwRows		= consoleParams->dwRows;
		dwColumns	= consoleParams->dwColumns;
	}
	else
	{
		// initialize member variables for the first view
		m_dwRows	= dwRows;
		m_dwColumns	= dwColumns;
	}
#endif
	shared_ptr<ConsoleView> consoleView(new ConsoleView(m_mainFrame, m_hWnd, m_tabData, m_strTitle, dwRows, dwColumns));
  consoleView->Group(this->IsGrouped());
	UserCredentials userCredentials;

	if (m_tabData->bRunAsUser)
	{
#ifdef _USE_AERO
    // Display a dialog box to request credentials.
    CREDUI_INFOW ui;
    ui.cbSize = sizeof(ui);
    ui.hwndParent = GetConsoleWindow();
    ui.pszMessageText = m_tabData->strShell.c_str();
    ui.pszCaptionText = L"Enter username and password";
    ui.hbmBanner = NULL;

    // we need a target
    WCHAR szModuleFileName[_MAX_PATH] = L"";
    ::GetModuleFileName(NULL, szModuleFileName, ARRAYSIZE(szModuleFileName));

    WCHAR szUser    [CREDUI_MAX_USERNAME_LENGTH + 1] = L"";
    WCHAR szPassword[CREDUI_MAX_PASSWORD_LENGTH + 1] = L"";
    wcscpy_s(szUser, ARRAYSIZE(szUser), m_tabData->strUser.c_str());

    DWORD rc = ::CredUIPromptForCredentials(
      &ui,                                //__in_opt  PCREDUI_INFO pUiInfo,
      szModuleFileName,                   //__in      PCTSTR pszTargetName,
      NULL,                               //__in      PCtxtHandle Reserved,
      0,                                  //__in_opt  DWORD dwAuthError,
      szUser,                             //__inout   PCTSTR pszUserName,
      ARRAYSIZE(szUser),                  //__in      ULONG ulUserNameMaxChars,
      szPassword,                         //__inout   PCTSTR pszPassword,
      ARRAYSIZE(szPassword),              //__in      ULONG ulPasswordMaxChars,
      NULL,                               //__inout   PBOOL pfSave,
      CREDUI_FLAGS_EXCLUDE_CERTIFICATES | //__in      DWORD dwFlags
      CREDUI_FLAGS_ALWAYS_SHOW_UI       |
      CREDUI_FLAGS_GENERIC_CREDENTIALS  |
      CREDUI_FLAGS_DO_NOT_PERSIST
    );

    if( rc != NO_ERROR )
      return 0;

    userCredentials.user     = szUser;
    userCredentials.password = szPassword;
#else
		DlgCredentials dlg(m_tabData->strUser.c_str());

		if (dlg.DoModal() != IDOK) return 0;

		userCredentials.user	= dlg.GetUser();
		userCredentials.password= dlg.GetPassword();
#endif
	}

	HWND hwndConsoleView = consoleView->Create(
											m_hWnd, 
											rcDefault, 
											NULL, 
											WS_CHILD | WS_VISIBLE,// | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
											0,
											0U,
											reinterpret_cast<void*>(&userCredentials));

	if (hwndConsoleView == NULL)
	{
		CString	strMessage(consoleView->GetExceptionMessage());

		if (strMessage.GetLength() == 0)
		{
			strMessage.Format(IDS_TAB_CREATE_FAILED, m_tabData->strTitle.c_str(), m_tabData->strShell.c_str());
		}

		::MessageBox(m_hWnd, strMessage, L"Error", MB_OK|MB_ICONERROR);

		return 0;
	}

	m_views.insert(ConsoleViewMap::value_type(hwndConsoleView, consoleView));

	return hwndConsoleView;
}

shared_ptr<ConsoleView> TabView::GetActiveConsole(const TCHAR* szFrom)
{
  shared_ptr<ConsoleView> result;
  if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
  {
    MutexLock viewMapLock(m_viewsMutex);
    ConsoleViewMap::iterator iter = m_views.find(multisplitClass::defaultFocusPane->window);
    if( iter != m_views.end() )
      result = iter->second;
    else
      TRACE(L"defaultFocusPane->window = %p not found !!!\n", defaultFocusPane->window);
  }
  else
  {
    TRACE(L"TabView::GetActiveConsole multisplitClass::defaultFocusPane = %p\n", multisplitClass::defaultFocusPane);
  }
  //TRACE(L"TabView::GetActiveConsole called by %s returns %p\n", szFrom, result.get());
  return result;
}

void TabView::GetRect(CRect& clientRect)
{
  clientRect = this->visibleRect;
}

void TabView::InitializeScrollbars()
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->InitializeScrollbars();
  }
}

void TabView::Repaint(bool bFullRepaint)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->Repaint(bFullRepaint);
  }
}

void TabView::SetResizing(bool bResizing)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->SetResizing(bResizing);
  }
}

void TabView::MainframeMoving()
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->MainframeMoving();
  }
}

void TabView::SetTitle(const CString& strTitle)
{
  m_strTitle = strTitle;

  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->SetTitle(strTitle);
  }
}

void TabView::SetActive(bool bActive)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->SetActive(bActive);
  }
}

void TabView::SetAppActiveStatus(bool bAppActive)
{
  MutexLock	viewMapLock(m_viewsMutex);
  if( bAppActive )
  {
    shared_ptr<ConsoleView> consoleView = this->GetActiveConsole(_T(__FUNCTION__));
    for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
    {
      it->second->SetAppActiveStatus(it->second == consoleView);
    }
  }
  else
  {
    for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
    {
      it->second->SetAppActiveStatus(false);
    }
  }
}

void TabView::AdjustRectAndResize(CRect& clientRect, DWORD dwResizeWindowEdge)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->AdjustRectAndResize(clientRect, dwResizeWindowEdge);
  }
  this->GetRect(clientRect);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::SplitHorizontally()
{
  if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
  {
    HWND hwndConsoleView = CreateNewConsole();
    if( hwndConsoleView )
    {
      multisplitClass::SetDefaultFocusPane(multisplitClass::defaultFocusPane->split(
        hwndConsoleView,
        CMultiSplitPane::HORIZONTAL));

      CRect clientRect(0, 0, 0, 0);
      AdjustRectAndResize(clientRect, WMSZ_BOTTOM);
    }
  }
}

void TabView::SplitVertically()
{
  if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
  {
    HWND hwndConsoleView = CreateNewConsole();
    if( hwndConsoleView )
    {
      multisplitClass::SetDefaultFocusPane(multisplitClass::defaultFocusPane->split(
        hwndConsoleView,
        CMultiSplitPane::VERTICAL));

      CRect clientRect(0, 0, 0, 0);
      AdjustRectAndResize(clientRect, WMSZ_BOTTOM);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

bool TabView::CloseView(HWND hwnd /*= 0*/)
{
  if( hwnd == 0 )
  {
    if( multisplitClass::defaultFocusPane )
      hwnd = multisplitClass::defaultFocusPane->window;
  }

  if( hwnd )
  {
    MutexLock viewMapLock(m_viewsMutex);
    ConsoleViewMap::iterator iter = m_views.find(hwnd);
    if( iter != m_views.end() )
    {
      iter->second->DestroyWindow();
      m_views.erase(iter);
      multisplitClass::SetDefaultFocusPane(multisplitClass::defaultFocusPane->remove());

      if( m_views.size() == 0 )
        m_mainFrame.CloseTab(this->m_hWnd);
      else
      {
        CRect clientRect(0, 0, 0, 0);
        AdjustRectAndResize(clientRect, WMSZ_BOTTOM);
      }

      return true;
    }
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::NextView()
{
  if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
  {
    MutexLock viewMapLock(m_viewsMutex);
    if( m_views.size() > 1 )
    {
      ConsoleViewMap::iterator iter = m_views.find(multisplitClass::defaultFocusPane->window);
      ++iter;
      if( iter == m_views.end() )
        iter = m_views.begin();
      multisplitClass::SetDefaultFocusPane(multisplitClass::tree.get(iter->first));
    }
  }
}

void TabView::PrevView()
{
  if( multisplitClass::defaultFocusPane && multisplitClass::defaultFocusPane->window )
  {
    MutexLock viewMapLock(m_viewsMutex);
    if( m_views.size() > 1 )
    {
      ConsoleViewMap::iterator iter = m_views.find(multisplitClass::defaultFocusPane->window);
      if( iter == m_views.begin() )
        iter = m_views.end();
      --iter;
      multisplitClass::SetDefaultFocusPane(multisplitClass::tree.get(iter->first));
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::OnSplitBarMove(HWND /*hwndPane0*/, HWND /*hwndPane1*/, bool /*boolEnd*/)
{
  CRect clientRect(0, 0, 0, 0);
  AdjustRectAndResize(clientRect, WMSZ_BOTTOM);
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    ::PostMessage(it->second->GetConsoleHandler().GetConsoleParams()->hwndConsoleWindow, Msg, wParam, lParam);
  }
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::PasteToConsoles()
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->Paste();
  }
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void TabView::Group(bool b)
{
  MutexLock	viewMapLock(m_viewsMutex);
  for (ConsoleViewMap::iterator it = m_views.begin(); it != m_views.end(); ++it)
  {
    it->second->Group(b);
  }
  m_boolIsGrouped = b;
}
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

MainFrame::MainFrame()
: m_bMenuVisible(TRUE)
, m_bToolbarVisible(TRUE)
, m_bStatusBarVisible(TRUE)
, m_bTabsVisible(TRUE)
, m_mapViews()
{

}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::PreTranslateMessage(MSG* pMsg) {

	if (!m_acceleratorTable.IsNull() && m_acceleratorTable.TranslateAccelerator(m_hWnd, pMsg)) return TRUE;

	if(CTabbedFrameImpl<MainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	shared_ptr<ConsoleView> activeView(GetActiveView());

	if (activeView.get() == NULL) return FALSE;

	return activeView->PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::OnIdle() {

	UIUpdateToolBar();
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();
	UpdateTabsMenu(m_CmdBar.GetMenu(), m_tabsMenu);

	SetReflectNotifications(true);
	SetTabStyles(CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_SCROLL | CTCS_CLOSEBUTTON | CTCS_BOLDSELECTEDTAB);
	CreateTabWindow(m_hWnd, rcDefault, CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_SCROLL | CTCS_CLOSEBUTTON | CTCS_BOLDSELECTEDTAB);

	// create initial console window
	if (!CreateNewConsole(0)) return -1;

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_MENU, 1);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_TABS, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	UpdateWindowStyles();

	WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;

	m_bMenuVisible		= ShowMenu(windowSettings.bShowMenu ? TRUE : FALSE);
	m_bToolbarVisible	= ShowToolbar(windowSettings.bShowToolbar ? TRUE : FALSE);
	m_bTabsVisible		= ShowTabs(windowSettings.bShowTabs ? TRUE : FALSE);
	m_bStatusBarVisible	= ShowStatusbar(windowSettings.bShowStatusbar ? TRUE : FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	bHandled = false;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	for (ConsoleViewMap::iterator it = m_mapViews.begin(); it != m_mapViews.end(); ++it) {
		RemoveTab(it->second->m_hWnd);
		it->second->DestroyWindow();
	}

	bHandled = false;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnActivateApp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {

	shared_ptr<ConsoleView> consoleView = GetActiveView();
	if (consoleView.get() != NULL) {
		consoleView->SetAppActiveStatus(static_cast<BOOL>(wParam) == TRUE);
	}

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnConsoleClosed(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /* bHandled */) {

	HWND						hwndConsoleView	= reinterpret_cast<HWND>(lParam);
	ConsoleViewMap::iterator	findIt			= m_mapViews.find(hwndConsoleView);

	if (findIt == m_mapViews.end()) return 0;

	RemoveTab(hwndConsoleView);

	findIt->second->DestroyWindow();
	m_mapViews.erase(findIt);

	if (m_mapViews.size() == 0) PostMessage(WM_CLOSE);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnShowPopupMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {

	POINT	point;
	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);
	::ClientToScreen(m_hWnd, &point);
	
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

LRESULT MainFrame::OnTabChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {

	NMCTC2ITEMS*				pTabItems	= reinterpret_cast<NMCTC2ITEMS*>(pnmh);

	CTabViewTabItem*			pTabItem1	= (pTabItems->iItem1 != 0xFFFFFFFF) ? m_TabCtrl.GetItem(pTabItems->iItem1) : NULL;
	CTabViewTabItem*			pTabItem2	= m_TabCtrl.GetItem(pTabItems->iItem2);

	ConsoleViewMap::iterator	it;

	if (pTabItem1) {
		it = m_mapViews.find(pTabItem1->GetTabView());
		if (it != m_mapViews.end()) {
			it->second->SetViewActive(false);
		}
	}

	if (pTabItem2) {
		it = m_mapViews.find(pTabItem2->GetTabView());
		if (it != m_mapViews.end()) {
			UISetCheck(ID_VIEW_CONSOLE, it->second->GetConsoleWindowVisible() ? TRUE : FALSE);
			it->second->SetViewActive(true);
		}
	}

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnTabClose(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */) {

	NMCTC2ITEMS*		pTabItems	= reinterpret_cast<NMCTC2ITEMS*>(pnmh);
	CTabViewTabItem*	pTabItem	= (pTabItems->iItem1 != 0xFFFFFFFF) ? m_TabCtrl.GetItem(pTabItems->iItem1) : NULL;

	CloseTab(pTabItem);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnFileNewTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	if (wID == ID_FILE_NEW_TAB) {
		CreateNewConsole(0);
	} else {
		CreateNewConsole(wID-ID_NEW_TAB_1);
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnSwitchTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	int nNewSel = wID-ID_SWITCH_TAB_1;

	if (nNewSel >= m_TabCtrl.GetItemCount()) return 0;
	m_TabCtrl.SetCurSel(nNewSel);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnFileCloseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	CTabViewTabItem* pTabItem = m_TabCtrl.GetItem(m_TabCtrl.GetCurSel());
	
	CloseTab(pTabItem);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnNextTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	int nCurSel = m_TabCtrl.GetCurSel();

	if (++nCurSel >= m_TabCtrl.GetItemCount()) nCurSel = 0;
	m_TabCtrl.SetCurSel(nCurSel);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnPrevTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	int nCurSel = m_TabCtrl.GetCurSel();

	if (--nCurSel < 0) nCurSel = m_TabCtrl.GetItemCount() - 1;
	m_TabCtrl.SetCurSel(nCurSel);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	PostMessage(WM_CLOSE);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	shared_ptr<ConsoleView>	consoleView(GetActiveView());
	if (consoleView.get() == NULL) return 0;

	consoleView->Copy();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	shared_ptr<ConsoleView>	consoleView(GetActiveView());
	if (consoleView.get() == NULL) return 0;

	consoleView->Paste();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditRenameTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	shared_ptr<ConsoleView>	consoleView(GetActiveView());
	if (consoleView.get() == NULL) return 0;

	CString strTabName(L"");
	consoleView->GetWindowText(strTabName);

	DlgRenameTab dlg(strTabName);

	if (dlg.DoModal() == IDOK) {
		consoleView->SetWindowText(dlg.m_strTabName);
		UpdateTabText(consoleView->m_hWnd, dlg.m_strTabName);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnEditSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	shared_ptr<ConsoleView>	consoleView(GetActiveView());
	if (consoleView.get() == NULL) return 0;

	DlgSettingsMain dlg;

	if (dlg.DoModal() == IDOK) {
		consoleView->RecreateOffscreenBuffers();
		AdjustWindowSize(false);
		SetTransparency();
		CreateAcceleratorTable();
		consoleView->RepaintView();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	m_bMenuVisible = ShowMenu(!m_bMenuVisible);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	m_bToolbarVisible = ShowToolbar(!m_bToolbarVisible);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	m_bTabsVisible = ShowTabs(!m_bTabsVisible);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	m_bStatusBarVisible = ShowStatusbar(!m_bStatusBarVisible);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnViewConsole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	shared_ptr<ConsoleView> consoleView = GetActiveView();

	if (consoleView.get() != NULL) {
		consoleView->SetConsoleWindowVisible(!consoleView->GetConsoleWindowVisible());
		UISetCheck(ID_VIEW_CONSOLE, consoleView->GetConsoleWindowVisible() ? TRUE : FALSE);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT MainFrame::OnRebarHeightChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {

	AdjustWindowSize(false);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

shared_ptr<ConsoleView> MainFrame::GetActiveView() {

	if (m_mapViews.size() == 0) return shared_ptr<ConsoleView>();

	ConsoleViewMap::iterator	findIt		= m_mapViews.find(m_hWndActive);
	if (findIt == m_mapViews.end()) return shared_ptr<ConsoleView>();

	return findIt->second;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::AdjustWindowRect(CRect& rect) {

	AdjustWindowRectEx(&rect, GetWindowLong(GWL_STYLE), FALSE, GetWindowLong(GWL_EXSTYLE));

	// adjust for the toolbar height
	CReBarCtrl	rebar(m_hWndToolBar);
	rect.bottom	+= rebar.GetBarHeight() - 4;

	if (m_bStatusBarVisible) {

		CRect	rectStatusBar(0, 0, 0, 0);

		::GetWindowRect(m_hWndStatusBar, &rectStatusBar);
		rect.bottom	+= rectStatusBar.bottom - rectStatusBar.top;
	}

	rect.bottom	+= GetTabAreaHeight(); //+0
//	rect.right	+= 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::AdjustAndResizeConsoleView(CRect& rectView) {

	// adjust the active view
	shared_ptr<ConsoleView>	consoleView(GetActiveView());
	if (consoleView.get() == NULL) return;


/*
	GetClientRect(&rectView);

	if (m_bToolbarVisible) {

		CRect		rectToolBar			= { 0, 0, 0, 0 };
		CRect		rectToolBarBorders	= { 0, 0, 0, 0 };
		CReBarCtrl	rebar(m_hWndToolBar);
		int			nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);

		rebar.GetRect(nBandIndex, &rectToolBar);
		rebar.GetBandBorders(nBandIndex, &rectToolBarBorders);
		rectView.bottom	-= rectToolBar.bottom - rectToolBar.top;

		rectView.bottom	-= rectToolBarBorders.top + rectToolBarBorders.bottom;
	}

	if (m_bStatusBarVisible) {

		CRect	rectStatusBar	= { 0, 0, 0, 0 };

		::GetWindowRect(m_hWndStatusBar, &rectStatusBar);
		rectView.bottom	-= rectStatusBar.bottom - rectStatusBar.top;
	}

	rectView.bottom	-= GetTabAreaHeight(); //+0
*/


	consoleView->AdjustRectAndResize(rectView);
	
	// for other views, first set view size and then resize their Windows consoles
	for (ConsoleViewMap::iterator it = m_mapViews.begin(); it != m_mapViews.end(); ++it) {

		if (it->second->m_hWnd == consoleView->m_hWnd) continue;

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

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MainFrame::CreateNewConsole(DWORD dwTabIndex) {

	if (dwTabIndex >= g_settingsHandler->GetTabSettings().tabDataVector.size()) return false;

	DWORD dwRows	= g_settingsHandler->GetConsoleSettings().dwRows;
	DWORD dwColumns	= g_settingsHandler->GetConsoleSettings().dwColumns;

	if (m_mapViews.size() > 0) {
		SharedMemory<ConsoleParams>& consoleParams = m_mapViews.begin()->second->GetConsoleHandler().GetConsoleParams();
		dwRows		= consoleParams->dwRows;
		dwColumns	= consoleParams->dwColumns;
	}

	shared_ptr<ConsoleView> consoleView(new ConsoleView(dwTabIndex, dwRows, dwColumns));

	HWND hwndConsoleView = consoleView->Create(
											m_hWnd, 
											rcDefault, 
											NULL, 
											WS_CHILD | WS_VISIBLE,// | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
											0);

	if (hwndConsoleView == NULL) return false;

	m_mapViews.insert(ConsoleViewMap::value_type(hwndConsoleView, consoleView));

	CString strTabTitle;
	consoleView->GetWindowText(strTabTitle);

	AddTabWithIcon(*consoleView, strTabTitle, consoleView->GetIcon());
	DisplayTab(hwndConsoleView, FALSE);
	::SetForegroundWindow(m_hWnd);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::CloseTab(CTabViewTabItem* pTabItem) {

	ConsoleViewMap::iterator	it;

	if (!pTabItem) return;

	HWND hwndConsoleView = pTabItem->GetTabView();
	it = m_mapViews.find(hwndConsoleView);

	if (it == m_mapViews.end()) return;

	RemoveTab(hwndConsoleView);
	it->second->DestroyWindow();
	m_mapViews.erase(it);

	if (m_mapViews.size() == 0) PostMessage(WM_CLOSE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::UpdateTabsMenu(CMenuHandle mainMenu, CMenu& tabsMenu) {

	if (!tabsMenu.IsNull()) tabsMenu.DestroyMenu();
	tabsMenu.CreateMenu();

	// build tabs menu
	TabDataVector&			tabDataVector	= g_settingsHandler->GetTabSettings().tabDataVector;
	TabDataVector::iterator	it				= tabDataVector.begin();
	DWORD					dwId			= ID_NEW_TAB_1;

	for (it; it != tabDataVector.end(); ++it, ++dwId) {

		CMenuItemInfo	subMenuItem;
		ICONINFO		iconInfo;
		BITMAP			bmp;

		::GetIconInfo(tabDataVector[dwId-ID_NEW_TAB_1]->tabIcon, &iconInfo);
		::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);

		subMenuItem.fMask		= MIIM_STRING | MIIM_ID;
/*
		subMenuItem.fMask		= MIIM_BITMAP | MIIM_ID | MIIM_TYPE;
		subMenuItem.fType		= MFT_BITMAP;
*/
		subMenuItem.wID			= dwId;
//		subMenuItem.hbmpItem	= iconInfo.hbmColor;
		subMenuItem.dwTypeData	= const_cast<wchar_t*>((*it)->strName.c_str());
		subMenuItem.cch			= static_cast<UINT>((*it)->strName.length());

		tabsMenu.InsertMenuItem(dwId-ID_NEW_TAB_1, TRUE, &subMenuItem);
		tabsMenu.SetMenuItemBitmaps(dwId, MF_BYCOMMAND, iconInfo.hbmColor, NULL);

	}

	// set tabs menu as popup submenu
	CMenuItemInfo	menuItem;

	menuItem.fMask		= MIIM_SUBMENU;
	menuItem.hSubMenu	= HMENU(tabsMenu);

	mainMenu.SetMenuItemInfo(ID_FILE_NEW_TAB, FALSE, &menuItem);

}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void MainFrame::UpdateWindowStyles() {

	WindowSettings& windowSettings = g_settingsHandler->GetAppearanceSettings().windowSettings;

	// adjust window styles
	DWORD	dwStyle		= GetWindowLong(GWL_STYLE);
	DWORD	dwExStyle	= GetWindowLong(GWL_EXSTYLE);

	if (!windowSettings.bShowCaption) dwStyle &= ~WS_CAPTION;
	if (!windowSettings.bResizable) dwStyle &= ~WS_THICKFRAME;
	if (!windowSettings.bTaskbarButton) {
		dwStyle		&= ~WS_MINIMIZEBOX;
		dwExStyle	|= WS_EX_TOOLWINDOW;
		dwExStyle	&= ~WS_EX_APPWINDOW;
	}

	SetWindowLong(GWL_STYLE, dwStyle);
	SetWindowLong(GWL_EXSTYLE, dwExStyle);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::ShowMenu(BOOL bShow) {

	CReBarCtrl rebar(m_hWndToolBar);
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST);	// menu is 1st added band
	rebar.ShowBand(nBandIndex, bShow);
	UISetCheck(ID_VIEW_MENU, bShow);

	UpdateLayout();
	AdjustWindowSize(false);

	return bShow;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::ShowToolbar(BOOL bShow) {

	CReBarCtrl rebar(m_hWndToolBar);
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bShow);
	UISetCheck(ID_VIEW_TOOLBAR, bShow);

	UpdateLayout();
	AdjustWindowSize(false);

	return bShow;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::ShowTabs(BOOL bShow) {

	if (bShow) {
		ShowTabControl();
	} else {
		HideTabControl();
	}

	UISetCheck(ID_VIEW_TABS, bShow);

	UpdateLayout();
	AdjustWindowSize(false);

	return bShow;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL MainFrame::ShowStatusbar(BOOL bShow) {

	::ShowWindow(m_hWndStatusBar, bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bShow);

	UpdateLayout();
	AdjustWindowSize(false);

	return bShow;
}

//////////////////////////////////////////////////////////////////////////////

#pragma once


//////////////////////////////////////////////////////////////////////////////

#define ID_NEW_TAB_FIRST		1000

// Timer that will force a call to ResizeWindow (called from WM_EXITSIZEMOVE handler
// when the Console window is resized using a mouse)
// External utilities that might resize Console window usually don't send WM_EXITSIZEMOVE
// message after resizing a window.
#define	TIMER_SIZING			42
#define	TIMER_SIZING_INTERVAL	100

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class TabView;
typedef map<HWND, std::shared_ptr<TabView> >	TabViewMap;

//////////////////////////////////////////////////////////////////////////////

class MainFrame 
	: public CTabbedFrameImpl<MainFrame>
	, public CUpdateUI<MainFrame>
	, public CMessageFilter
	, public CIdleHandler
{
	public:
		DECLARE_FRAME_WND_CLASS(L"Console_2_Main", IDR_MAINFRAME)

#ifdef _USE_AERO
		aero::CCommandBarCtrl m_CmdBar;

		void OnComposition()
    {
      this->SetTransparency();
    }
#else
		CCommandBarCtrl m_CmdBar;
#endif

		MainFrame
		(
			LPCTSTR lpstrCmdLine
		);

		static void ParseCommandLine
		(
			LPCTSTR lptstrCmdLine,
			wstring& strWindowTitle,
			vector<wstring>& startupTabs,
			vector<wstring>& startupDirs,
			vector<wstring>& startupCmds,
			int& nMultiStartSleep
		);

		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL OnIdle();

		BEGIN_UPDATE_UI_MAP(MainFrame)
			UPDATE_ELEMENT(ID_FILE_CLOSE_TAB, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_CLOSE_VIEW, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
			UPDATE_ELEMENT(ID_EDIT_SELECT_ALL, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_CLEAR_SELECTION, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
			UPDATE_ELEMENT(ID_VIEW_MENU, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_TABS, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_CONSOLE, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_FULLSCREEN, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)

			UPDATE_ELEMENT(1, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(2, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(3, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(4, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(5, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(6, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(7, UPDUI_STATUSBAR)
			UPDATE_ELEMENT(8, UPDUI_STATUSBAR)

		END_UPDATE_UI_MAP()

		BEGIN_MSG_MAP(MainFrame)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_ACTIVATEAPP, OnActivateApp)
			MESSAGE_HANDLER(WM_HOTKEY, OnHotKey)
			MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKeydown)
			MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
			MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_SIZING, OnSizing)
			MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseButtonUp)
			MESSAGE_HANDLER(WM_RBUTTONUP, OnMouseButtonUp)
			MESSAGE_HANDLER(WM_MBUTTONUP, OnMouseButtonUp)
			MESSAGE_HANDLER(WM_XBUTTONUP, OnMouseButtonUp)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
			MESSAGE_HANDLER(UM_CONSOLE_RESIZED, OnConsoleResized)
			MESSAGE_HANDLER(UM_CONSOLE_CLOSED, OnConsoleClosed)
			MESSAGE_HANDLER(UM_UPDATE_TITLES, OnUpdateTitles)
			MESSAGE_HANDLER(UM_SHOW_POPUP_MENU, OnShowPopupMenu)
			MESSAGE_HANDLER(UM_START_MOUSE_DRAG, OnStartMouseDrag)
			MESSAGE_HANDLER(m_uTaskbarRestart, OnTaskbarCreated)
			MESSAGE_HANDLER(UM_TRAY_NOTIFY, OnTrayNotify)
			MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)

			NOTIFY_CODE_HANDLER(CTCN_SELCHANGE, OnTabChanged)
			NOTIFY_CODE_HANDLER(CTCN_CLOSE, OnTabClose)
			NOTIFY_CODE_HANDLER(CTCN_MCLICK, OnTabMiddleClick);
#ifdef _USE_AERO
			NOTIFY_CODE_HANDLER(NM_CLICK, OnStartMouseDragExtendedFrameToClientArea)
			NOTIFY_CODE_HANDLER(NM_LDOWN, OnStartMouseDragExtendedFrameToClientArea)
			NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDBLClickExtendedFrameToClientArea)
#endif //_USE_AERO

			NOTIFY_CODE_HANDLER(RBN_HEIGHTCHANGE, OnRebarHeightChanged)
			NOTIFY_HANDLER(ATL_IDW_TOOLBAR, TBN_DROPDOWN, OnToolbarDropDown)

			CHAIN_MSG_MAP(CUpdateUI<MainFrame>)

			COMMAND_RANGE_HANDLER(ID_NEW_TAB_1, ID_NEW_TAB_1 + 99, OnFileNewTab)
			COMMAND_ID_HANDLER(ID_FILE_NEW_TAB, OnFileNewTab)
			COMMAND_RANGE_HANDLER(ID_SWITCH_TAB_1, ID_SWITCH_TAB_1 + 9, OnSwitchTab)
			COMMAND_ID_HANDLER(ID_NEXT_TAB, OnNextTab)
			COMMAND_ID_HANDLER(ID_PREV_TAB, OnPrevTab)
			COMMAND_ID_HANDLER(ID_NEXT_VIEW   , OnSwitchView)
			COMMAND_ID_HANDLER(ID_PREV_VIEW   , OnSwitchView)
			COMMAND_ID_HANDLER(ID_LEFT_VIEW   , OnSwitchView)
			COMMAND_ID_HANDLER(ID_RIGHT_VIEW  , OnSwitchView)
			COMMAND_ID_HANDLER(ID_TOP_VIEW    , OnSwitchView)
			COMMAND_ID_HANDLER(ID_BOTTOM_VIEW , OnSwitchView)
			COMMAND_ID_HANDLER(ID_CLOSE_VIEW  , OnCloseView)
			COMMAND_ID_HANDLER(ID_SPLIT_HORIZ , OnSplitHorizontally)
			COMMAND_ID_HANDLER(ID_SPLIT_VERT  , OnSplitVertically)
			COMMAND_ID_HANDLER(ID_GROUP_ALL   , OnGroupAll)
			COMMAND_ID_HANDLER(ID_UNGROUP_ALL , OnUngroupAll)
			COMMAND_ID_HANDLER(ID_GROUP_TAB   , OnGroupTab)
			COMMAND_ID_HANDLER(ID_UNGROUP_TAB , OnUngroupTab)
			COMMAND_ID_HANDLER(ID_FILE_CLOSE_TAB, OnFileCloseTab)
			COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
			COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
			COMMAND_ID_HANDLER(ID_EDIT_SELECT_ALL, OnEditSelectAll)
			COMMAND_ID_HANDLER(ID_EDIT_CLEAR_SELECTION, OnEditClearSelection)
			COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
			COMMAND_ID_HANDLER(ID_EDIT_STOP_SCROLLING, OnEditStopScrolling)
			COMMAND_ID_HANDLER(ID_EDIT_RENAME_TAB, OnEditRenameTab)
			COMMAND_ID_HANDLER(ID_EDIT_SETTINGS, OnEditSettings)
			COMMAND_ID_HANDLER(ID_VIEW_MENU, OnViewMenu)
			COMMAND_ID_HANDLER(ID_VIEW_MENU2, OnViewMenu)
			COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
			COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
			COMMAND_ID_HANDLER(ID_VIEW_TABS, OnViewTabs)
			COMMAND_ID_HANDLER(ID_VIEW_CONSOLE, OnViewConsole)
			COMMAND_ID_HANDLER(ID_HELP, OnHelp)
			COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
			COMMAND_ID_HANDLER(IDC_DUMP_BUFFER, OnDumpBuffer)
			COMMAND_ID_HANDLER(ID_VIEW_FULLSCREEN, OnFullScreen)
			COMMAND_ID_HANDLER(ID_VIEW_ZOOM_100, OnZoom)
			COMMAND_ID_HANDLER(ID_VIEW_ZOOM_INC, OnZoom)
			COMMAND_ID_HANDLER(ID_VIEW_ZOOM_DEC, OnZoom)

			CHAIN_MSG_MAP(CTabbedFrameImpl<MainFrame>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

//		Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnActivateApp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

		LRESULT OnHotKey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnSysKeydown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSizing(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
		LRESULT OnMouseButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnExitSizeMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

		LRESULT OnConsoleResized(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */);
		LRESULT OnConsoleClosed(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnUpdateTitles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnShowPopupMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnStartMouseDrag(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnTrayNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

		LRESULT OnTabChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnTabClose(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
		LRESULT OnTabMiddleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
#ifdef _USE_AERO
		LRESULT OnStartMouseDragExtendedFrameToClientArea(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
		LRESULT OnDBLClickExtendedFrameToClientArea(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
#endif //_USE_AERO
		LRESULT OnRebarHeightChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

		LRESULT OnToolbarDropDown(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

		LRESULT OnFileNewTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSwitchTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFileCloseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnNextTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnPrevTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnSwitchView(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCloseView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSplitHorizontally(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSplitVertically(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnGroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUngroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnGroupTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUngroupTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditClearSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditStopScrolling(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditRenameTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnViewMenu(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewConsole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFullScreen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnZoom(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnDumpBuffer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	public:

		void AdjustWindowRect(CRect& rect);
		void AdjustWindowSize(ADJUSTSIZE as);
		void CloseTab(HWND hwndTabView);
		void SetActiveConsole(HWND hwndTabView, HWND hwndConsoleView);
		void PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam);
		void PasteToConsoles();
		void SendTextToConsoles(const wchar_t* pszText);
		bool GetAppActiveStatus(void) const { return this->m_bAppActive; }

	private:

		void ActivateApp(void);
		bool CreateNewConsole(DWORD dwTabIndex, const wstring& strCmdLineInitialDir = wstring(L""), const wstring& strCmdLineInitialCmd = wstring(L""));
		void CloseTab(CTabViewTabItem* pTabItem);

		void UpdateTabTitle(std::shared_ptr<TabView> tabView);
		void UpdateTabTitle(HWND hwndTabView, CString& strTabTitle);
		void UpdateTabsMenu(CMenuHandle mainMenu, CMenu& tabsMenu);
		void UpdateOpenedTabsMenu(CMenu& tabsMenu);
		void UpdateMenuHotKeys(void);
		void UpdateStatusBar();
		void SetWindowStyles(void);
		void DockWindow(DockPosition dockPosition);
		void SetZOrder(ZOrder zOrder);
		HWND GetDesktopWindow();

		void SetWindowIcons();

		void ShowMenu      (bool bShow);
		void ShowToolbar   (bool bShow);
		void ShowTabs      (bool bShow);
		void ShowStatusbar (bool bShow);
		void ShowFullScreen(bool bShow);

		void ResizeWindow();
		void SetMargins();
		void SetTransparency();
		void CreateAcceleratorTable();
		void RegisterGlobalHotkeys();
		void UnregisterGlobalHotkeys();
		void CreateStatusBar();
		BOOL SetTrayIcon(DWORD dwMessage);
		void ShowHideWindow();

		static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT /*lprcMonitor*/, LPARAM lpData);

	public:
		LRESULT CreateInitialTabs
		(
			vector<wstring> startupTabs,
			vector<wstring> startupCmds,
			vector<wstring> startupDirs,
			int nMultiStartSleep
		);
		LRESULT OnCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

		bool					m_bOnCreateDone;

	private:
		vector<wstring>	m_startupTabs;
		vector<wstring>	m_startupDirs;
		vector<wstring>	m_startupCmds;
		int						m_nMultiStartSleep;

		std::shared_ptr<TabView>	m_activeTabView;

		bool m_bMenuVisible;
		bool m_bMenuChecked;
		bool m_bToolbarVisible;
		bool m_bStatusBarVisible;
		bool m_bTabsVisible;
		bool m_bFullScreen;

		DockPosition	m_dockPosition;
		ZOrder			m_zOrder;
		CPoint			m_mousedragOffset;

		TabViewMap	m_tabs;
		Mutex			m_tabsMutex;

		CMenu			m_tabsMenu;

		CIcon			m_icon;
		CIcon			m_smallIcon;

		CString			m_strCmdLineWindowTitle;
		CString			m_strWindowTitle;

		DWORD			m_dwWindowWidth;
		DWORD			m_dwWindowHeight;
		DWORD			m_dwResizeWindowEdge;

		bool			m_bAppActive;
		bool			m_bRestoringWindow;
		CRect			m_rectRestoredWnd;
		CRect			m_rectWndNotFS;

		CToolBarCtrl	m_toolbar;
		CAccelerator	m_acceleratorTable;
		UINT			m_uTaskbarRestart;
		CMultiPaneStatusBarCtrl m_statusBar;
		CMenuHandle m_contextMenu;

		MARGINS m_Margins;

		int     m_nFullSreen1Bitmap;
		int     m_nFullSreen2Bitmap;
};

//////////////////////////////////////////////////////////////////////////////

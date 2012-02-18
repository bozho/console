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
typedef map<HWND, shared_ptr<TabView> >	TabViewMap;

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
#else
		CCommandBarCtrl m_CmdBar;
#endif

		MainFrame
		(
			LPCTSTR lpstrCmdLine
		);

		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL OnIdle();

		BEGIN_UPDATE_UI_MAP(MainFrame)
			UPDATE_ELEMENT(ID_FILE_CLOSE_TAB, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_CLEAR_SELECTION, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_MENU, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_TABS, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_CONSOLE, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(1, UPDUI_STATUSBAR)
		END_UPDATE_UI_MAP()

		BEGIN_MSG_MAP(MainFrame)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_ACTIVATEAPP, OnActivateApp)
			MESSAGE_HANDLER(WM_HOTKEY, OnHotKey)
			MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKeydown)
			MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
//			MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
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
			MESSAGE_HANDLER(UM_TRAY_NOTIFY, OnTrayNotify)
			
			NOTIFY_CODE_HANDLER(CTCN_SELCHANGE, OnTabChanged)
			NOTIFY_CODE_HANDLER(CTCN_CLOSE, OnTabClose)
			NOTIFY_CODE_HANDLER(CTCN_MCLICK, OnTabMiddleClick);
			NOTIFY_CODE_HANDLER(RBN_HEIGHTCHANGE, OnRebarHeightChanged)
			NOTIFY_HANDLER(ATL_IDW_TOOLBAR, TBN_DROPDOWN, OnToolbarDropDown)
			
			CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
			
			COMMAND_RANGE_HANDLER(ID_NEW_TAB_1, ID_NEW_TAB_1 + 99, OnFileNewTab)
			COMMAND_ID_HANDLER(ID_FILE_NEW_TAB, OnFileNewTab)
			COMMAND_RANGE_HANDLER(ID_SWITCH_TAB_1, ID_SWITCH_TAB_1 + 9, OnSwitchTab)
			COMMAND_ID_HANDLER(ID_NEXT_TAB, OnNextTab)
			COMMAND_ID_HANDLER(ID_PREV_TAB, OnPrevTab)
			COMMAND_ID_HANDLER(ID_NEXT_VIEW   , OnNextView)
			COMMAND_ID_HANDLER(ID_PREV_VIEW   , OnPrevView)
			COMMAND_ID_HANDLER(ID_CLOSE_VIEW  , OnCloseView)
			COMMAND_ID_HANDLER(ID_SPLIT_HORIZ , OnSplitHorizontally)
			COMMAND_ID_HANDLER(ID_SPLIT_VERT  , OnSplitVertically)
			COMMAND_ID_HANDLER(ID_GROUP_ALL   , OngroupAll)
			COMMAND_ID_HANDLER(ID_UNGROUP_ALL , OnUngroupAll)
			COMMAND_ID_HANDLER(ID_GROUP_TAB   , OnGroupTab)
			COMMAND_ID_HANDLER(ID_UNGROUP_TAB , OnUngroupTab)
			COMMAND_ID_HANDLER(ID_FILE_CLOSE_TAB, OnFileCloseTab)
			COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
			COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
			COMMAND_ID_HANDLER(ID_EDIT_CLEAR_SELECTION, OnEditClearSelection)
			COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
			COMMAND_ID_HANDLER(ID_EDIT_STOP_SCROLLING, OnEditStopScrolling)
			COMMAND_ID_HANDLER(ID_EDIT_RENAME_TAB, OnEditRenameTab)
			COMMAND_ID_HANDLER(ID_EDIT_SETTINGS, OnEditSettings)
			COMMAND_ID_HANDLER(ID_VIEW_MENU, OnViewMenu)
			COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
			COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
			COMMAND_ID_HANDLER(ID_VIEW_TABS, OnViewTabs)
			COMMAND_ID_HANDLER(ID_VIEW_CONSOLE, OnViewConsole)
			COMMAND_ID_HANDLER(ID_HELP, OnHelp)
			COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
			COMMAND_ID_HANDLER(IDC_DUMP_BUFFER, OnDumpBuffer)
			
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

		LRESULT OnTabChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnTabClose(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);
		LRESULT OnTabMiddleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);

		LRESULT OnRebarHeightChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

		LRESULT OnToolbarDropDown(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

		LRESULT OnFileNewTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSwitchTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFileCloseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnNextTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnPrevTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnNextView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnPrevView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCloseView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSplitHorizontally(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSplitVertically(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OngroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUngroupAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnGroupTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUngroupTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditClearSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditStopScrolling(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditRenameTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnViewMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewConsole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnDumpBuffer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	public:

		void AdjustWindowRect(CRect& rect);
		void SetSelectionSize(int iSelectionSize);
		void AdjustWindowSize(ADJUSTSIZE as);
		void CloseTab(HWND hwndTabView);
    void PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam);
    void PasteToConsoles();

	private:

		bool CreateNewConsole(DWORD dwTabIndex, const wstring& strStartupDir = wstring(L""), const wstring& strStartupCmd = wstring(L""));
		void CloseTab(CTabViewTabItem* pTabItem);

		void UpdateTabTitle(HWND hwndTabView, CString& strTabTitle);
		void UpdateTabsMenu(CMenuHandle mainMenu, CMenu& tabsMenu);
		void UpdateStatusBar();
		void SetWindowStyles();
		void DockWindow(DockPosition dockPosition);
		void SetZOrder(ZOrder zOrder);
		HWND GetDesktopWindow();

		void SetWindowIcons();

		void ShowMenu(BOOL bShow);
		void ShowToolbar(BOOL bShow);
		void ShowTabs(BOOL bShow);
		void ShowStatusbar(BOOL bShow);

		void ResizeWindow();
		void SetMargins();
		void SetTransparency();
		void CreateAcceleratorTable();
		void RegisterGlobalHotkeys();
		void UnregisterGlobalHotkeys();
		void CreateStatusBar();
		BOOL SetTrayIcon(DWORD dwMessage);

	public:

		LRESULT CreateInitialTabs
		(
			vector<wstring> startupTabs,
			vector<wstring> startupCmds,
			vector<wstring> startupDirs,
			int nMultiStartSleep
		);

		bool					m_bOnCreateDone;
	private:
		vector<wstring>	m_startupTabs;
		vector<wstring>	m_startupDirs;
		vector<wstring>	m_startupCmds;
		int						m_nMultiStartSleep;

		shared_ptr<TabView>	m_activeTabView;

		BOOL			m_bMenuVisible;
		BOOL			m_bToolbarVisible;
		BOOL			m_bStatusBarVisible;
		BOOL			m_bTabsVisible;

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

		int				m_iSelectionSize;

		DWORD			m_dwWindowWidth;
		DWORD			m_dwWindowHeight;
		DWORD			m_dwResizeWindowEdge;

		bool			m_bAppActive;
		bool			m_bRestoringWindow;
		CRect			m_rectRestoredWnd;

		CToolBarCtrl	m_toolbar;
		CAccelerator	m_acceleratorTable;
		CMultiPaneStatusBarCtrl m_statusBar;

		MARGINS m_Margins;

};

//////////////////////////////////////////////////////////////////////////////

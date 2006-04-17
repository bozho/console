#pragma once

//////////////////////////////////////////////////////////////////////////////

#define ID_NEW_TAB_FIRST		1000

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef map<HWND, shared_ptr<ConsoleView> >	ConsoleViewMap;

//////////////////////////////////////////////////////////////////////////////

class MainFrame 
	: public CTabbedFrameImpl<MainFrame>
	, public CUpdateUI<MainFrame>
	, public CMessageFilter
	, public CIdleHandler
{
	public:
		DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

		CCommandBarCtrl m_CmdBar;

		MainFrame();

		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL OnIdle();

		BEGIN_UPDATE_UI_MAP(MainFrame)
			UPDATE_ELEMENT(ID_VIEW_MENU, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_TABS, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
			UPDATE_ELEMENT(ID_VIEW_CONSOLE, UPDUI_MENUPOPUP)
		END_UPDATE_UI_MAP()

		BEGIN_MSG_MAP(MainFrame)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_ACTIVATEAPP, OnActivateApp)
			MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
			MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
			MESSAGE_HANDLER(UM_CONSOLE_RESIZED, OnConsoleResized)
			MESSAGE_HANDLER(UM_CONSOLE_CLOSED, OnConsoleClosed)
			MESSAGE_HANDLER(UM_UPDATE_TITLES, OnUpdateTitles)
			MESSAGE_HANDLER(UM_SHOW_POPUP_MENU, OnShowPopupMenu)
			MESSAGE_HANDLER(UM_START_MOUSE_DRAG, OnStartMouseDrag)
			MESSAGE_HANDLER(UM_TRAY_NOTIFY, OnTrayNotify)
			NOTIFY_CODE_HANDLER(CTCN_SELCHANGE, OnTabChanged)
			NOTIFY_CODE_HANDLER(CTCN_CLOSE, OnTabClose)
			NOTIFY_CODE_HANDLER(RBN_HEIGHTCHANGE, OnRebarHeightChanged)
			COMMAND_RANGE_HANDLER(ID_NEW_TAB_1, ID_NEW_TAB_1 + 99, OnFileNewTab)
			COMMAND_ID_HANDLER(ID_FILE_NEW_TAB, OnFileNewTab)
			COMMAND_RANGE_HANDLER(ID_SWITCH_TAB_1, ID_SWITCH_TAB_1 + 9, OnSwitchTab)
			COMMAND_ID_HANDLER(ID_FILE_CLOSE_TAB, OnFileCloseTab)
			COMMAND_ID_HANDLER(ID_NEXT_TAB, OnNextTab)
			COMMAND_ID_HANDLER(ID_PREV_TAB, OnPrevTab)
			COMMAND_ID_HANDLER(ID_FILE_CLOSE_TAB, OnFileCloseTab)
			COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
			COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
			COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
			COMMAND_ID_HANDLER(ID_EDIT_RENAME_TAB, OnEditRenameTab)
			COMMAND_ID_HANDLER(ID_EDIT_SETTINGS, OnEditSettings)
			COMMAND_ID_HANDLER(ID_VIEW_MENU, OnViewMenu)
			COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
			COMMAND_ID_HANDLER(ID_VIEW_TABS, OnViewTabs)
			COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
			COMMAND_ID_HANDLER(ID_VIEW_CONSOLE, OnViewConsole)
			COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
			CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
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

		LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

		LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
		LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnExitSizeMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnConsoleResized(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */);
		LRESULT OnConsoleClosed(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnUpdateTitles(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnShowPopupMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnStartMouseDrag(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnTrayNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

		LRESULT OnTabChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnTabClose(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */);

		LRESULT OnRebarHeightChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

		LRESULT OnFileNewTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSwitchTab(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFileCloseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnNextTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnPrevTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditRenameTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnEditSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnViewMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewConsole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	public:

//		shared_ptr<ConsoleView> GetActiveView();

		void AdjustWindowRect(CRect& rect);
		void AdjustAndResizeConsoleView(CRect& rectView);

	private:

		bool CreateNewConsole(DWORD dwTabIndex);
		void CloseTab(CTabViewTabItem* pTabItem);

		void UpdateTabsMenu(CMenuHandle mainMenu, CMenu& tabsMenu);
		void SetWindowStyles();
		void DockWindow(DockPosition dockPosition);
		void SetZOrder(ZOrder zOrder);

		void SetWindowIcons();

		void ShowMenu(BOOL bShow);
		void ShowToolbar(BOOL bShow);
		void ShowTabs(BOOL bShow);
		void ShowStatusbar(BOOL bShow);

		void AdjustWindowSize(bool bResizeConsole);
		void SetTransparency();
		void CreateAcceleratorTable();
		BOOL SetTrayIcon(DWORD dwMessage);

	private:

		shared_ptr<ConsoleView>	m_activeView;

		BOOL			m_bMenuVisible;
		BOOL			m_bToolbarVisible;
		BOOL			m_bStatusBarVisible;
		BOOL			m_bTabsVisible;

		DockPosition	m_dockPosition;
		ZOrder			m_zOrder;
		CPoint			m_mousedragOffset;

		ConsoleViewMap	m_mapViews;

		CMenu			m_tabsMenu;

		CIcon			m_icon;
		CIcon			m_smallIcon;

		CString			m_strWindowTitle;

		DWORD			m_dwWindowWidth;
		DWORD			m_dwWindowHeight;

		bool			m_bRestoringWindow;

		CAccelerator	m_acceleratorTable;

		CDC				m_dcOffscreen;
		CDC				m_dcText;

};

//////////////////////////////////////////////////////////////////////////////

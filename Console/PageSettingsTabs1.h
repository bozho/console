
#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define UM_TAB_TITLE_CHANGED	WM_USER + 0x2000

//////////////////////////////////////////////////////////////////////////////

class PageSettingsTabs1
	: public CDialogImpl<PageSettingsTabs1>
	, public CWinDataExchange<PageSettingsTabs1>
{
	public:

		enum { IDD = IDD_SETTINGS_TABS_1 };

		PageSettingsTabs1();

		BEGIN_DDX_MAP(PageSettingsTabs1)
			DDX_TEXT(IDC_TAB_TITLE, m_strTitle)
			DDX_TEXT(IDC_TAB_ICON, m_strIcon)
			DDX_CHECK(IDC_CHECK_DEFAULT_ICON, m_nUseDefaultIcon)
			DDX_TEXT(IDC_TAB_SHELL, m_strShell)
			DDX_TEXT(IDC_TAB_INIT_DIR, m_strInitialDir)
			DDX_CHECK(IDC_CHECK_RUN_AS_USER, m_bRunAsUser)
			DDX_TEXT(IDC_TAB_USER, m_strUser)
		END_DDX_MAP()

		BEGIN_MSG_MAP(PageSettingsTabs1)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)

			COMMAND_HANDLER(IDC_TAB_TITLE, EN_CHANGE, OnTabTitleChange)
			COMMAND_HANDLER(IDC_CURSOR_COLOR, BN_CLICKED, OnClickedCursorColor)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_ICON, OnBtnBrowseIcon)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_SHELL, OnClickedBtnBrowseShell)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_DIR, OnClickedBtnBrowseDir)
			COMMAND_ID_HANDLER(IDC_CHECK_DEFAULT_ICON, OnCheckboxClicked)
			COMMAND_ID_HANDLER(IDC_CHECK_RUN_AS_USER, OnCheckboxClicked)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnTabTitleChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedCursorColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseShell(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCheckboxClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		void EnableControls();

		void Load(shared_ptr<TabData>& tabData);
		void Save();

		const CString& GetTabTitle() const { return m_strTitle; }

	private:

		shared_ptr<TabData>	m_tabData;

		CComboBox		m_comboCursor;
		CStatic			m_staticCursorColor;

		CString			m_strTitle;
		CString			m_strIcon;
		int				m_nUseDefaultIcon;

		CString			m_strShell;
		CString			m_strInitialDir;
		bool			m_bRunAsUser;
		CString			m_strUser;
};

//////////////////////////////////////////////////////////////////////////////

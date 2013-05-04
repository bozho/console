#pragma once

#include "PageSettingsTab.h"
#include "Cursors.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define UM_TAB_TITLE_CHANGED	WM_USER + 0x2000
#define UM_TAB_ICON_CHANGED	WM_USER + 0x2001

//////////////////////////////////////////////////////////////////////////////

class PageSettingsTabs1
	: public CDialogImpl<PageSettingsTabs1>
	, public CWinDataExchange<PageSettingsTabs1>
	, public CursorCharDrawer
	, public PageSettingsTab
{
	public:

		enum { IDD = IDD_SETTINGS_TABS_1 };

		PageSettingsTabs1();

		BEGIN_DDX_MAP(PageSettingsTabs1)
			DDX_TEXT(IDC_TAB_TITLE, m_strTitle)
			DDX_TEXT(IDC_TAB_ICON, m_strIcon)
			DDX_CHECK(IDC_CHECK_DEFAULT_ICON, m_bUseDefaultIcon)
			DDX_TEXT(IDC_TAB_SHELL, m_strShell)
			DDX_TEXT(IDC_TAB_INIT_DIR, m_strInitialDir)
			DDX_CHECK(IDC_CHECK_RUN_AS_USER, m_bRunAsUser)
			DDX_TEXT(IDC_TAB_USER, m_strUser)
			DDX_CHECK(IDC_CHECK_NET_ONLY, m_bNetOnly)
		END_DDX_MAP()

		BEGIN_MSG_MAP(PageSettingsTabs1)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)

			COMMAND_HANDLER(IDC_TAB_TITLE, EN_CHANGE, OnTabTitleChange)
			COMMAND_HANDLER(IDC_TAB_ICON, EN_CHANGE, OnTabIconChange)
			COMMAND_HANDLER(IDC_TAB_SHELL, EN_CHANGE, OnTabShellChange)
			COMMAND_HANDLER(IDC_CURSOR_COLOR, BN_CLICKED, OnClickedCursorColor)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_ICON, OnBtnBrowseIcon)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_SHELL, OnClickedBtnBrowseShell)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_DIR, OnClickedBtnBrowseDir)
			COMMAND_ID_HANDLER(IDC_CHECK_DEFAULT_ICON, OnCheckboxClicked)
			COMMAND_ID_HANDLER(IDC_CHECK_RUN_AS_USER, OnCheckboxClicked)
      COMMAND_HANDLER(IDC_COMBO_CURSOR, CBN_SELCHANGE, OnCbnSelchangeComboCursor)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnTabTitleChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnTabIconChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnTabShellChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedCursorColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseShell(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCheckboxClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		void EnableControls();

		void Load(std::shared_ptr<TabData>& tabData);
		void Save();

		const CString& GetTabTitle() const { return m_strTitle; }
		const CString& GetTabIcon() const { return m_strIcon; }
		const CString& GetTabShell() const { return m_strShell; }
		const bool UseDefaultIcon() const { return m_bUseDefaultIcon; }

		virtual void RedrawCharOnCursor(CDC& dc);

private:
  void SetCursor(void);
  void DrawCursor(void);
  void DrawCursor(CDC& dc, const CRect& rectCursorAnim, COLORREF fg, COLORREF bg);

	private:
		std::shared_ptr<Cursor>	m_cursor;
		std::shared_ptr<TabData>	m_tabData;

    CStatic			m_staticCursorAnim;
		CComboBox		m_comboCursor;
		CStatic			m_staticCursorColor;

		CString			m_strTitle;
		CString			m_strIcon;
		bool			m_bUseDefaultIcon;

		CString			m_strShell;
		CString			m_strInitialDir;
		bool			m_bRunAsUser;
		CString			m_strUser;
		bool			m_bNetOnly;
public:
  LRESULT OnCbnSelchangeComboCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

//////////////////////////////////////////////////////////////////////////////

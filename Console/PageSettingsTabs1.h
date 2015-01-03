#pragma once

#include "PageSettingsTab.h"
#include "CFileNameEdit.h"

class PageSettingsTabs1;

class CFileNameAndLinkEdit: public CFileNameEdit
{
public:
	CFileNameAndLinkEdit(PageSettingsTabs1* p):p(p) {}
	virtual bool OnDropFile(CString& strFilename);

private:
	PageSettingsTabs1* p;
};

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
			DDX_RADIO(IDC_CHECK_RUN_AS_CURRENT_USER, m_nRunAs)
			DDX_TEXT(IDC_TAB_USER, m_strUser)
			DDX_CHECK(IDC_CHECK_NET_ONLY, m_bNetOnly)
		END_DDX_MAP()

		BEGIN_MSG_MAP(PageSettingsTabs1)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

			COMMAND_HANDLER(IDC_TAB_TITLE, EN_CHANGE, OnTabTitleChange)
			COMMAND_HANDLER(IDC_TAB_ICON, EN_CHANGE, OnTabIconChange)
			COMMAND_HANDLER(IDC_TAB_SHELL, EN_CHANGE, OnTabShellChange)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_ICON, OnBtnBrowseIcon)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_SHELL, OnClickedBtnBrowseShell)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_DIR, OnClickedBtnBrowseDir)
			COMMAND_ID_HANDLER(IDC_CHECK_DEFAULT_ICON, OnCheckboxClicked)
			COMMAND_ID_HANDLER(IDC_CHECK_RUN_AS_CURRENT_USER, OnCheckboxClicked)
			COMMAND_ID_HANDLER(IDC_CHECK_RUN_AS_ADMIN, OnCheckboxClicked)
			COMMAND_ID_HANDLER(IDC_CHECK_RUN_AS_USER, OnCheckboxClicked)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnTabTitleChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnTabIconChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnTabShellChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
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

		void ConvertShellLink(CString& strShell);

	private:
		std::shared_ptr<TabData>	m_tabData;

		CString			m_strTitle;
		CString			m_strIcon;
		bool			m_bUseDefaultIcon;

		CString			m_strShell;
		CString			m_strInitialDir;
		int				m_nRunAs;
		CString			m_strUser;
		bool			m_bNetOnly;

		CFileNameEdit        m_tabIconEdit;
		CFileNameAndLinkEdit m_tabShellEdit;
		CFileNameEdit        m_tabInitialDirEdit;
		CComboBox            m_comboPriority;
};

//////////////////////////////////////////////////////////////////////////////

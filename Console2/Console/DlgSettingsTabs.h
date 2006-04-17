
#pragma once

#include "DlgSettingsBase.h"
#include "HotkeyEdit.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsTabs 
	: public DlgSettingsBase
{
	public:

		DlgSettingsTabs(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsTabs)
			DDX_TEXT(IDC_TAB_TITLE, m_strTitle)
			DDX_TEXT(IDC_TAB_ICON, m_strIcon)
			DDX_TEXT(IDC_TAB_SHELL, m_strShell)
			DDX_TEXT(IDC_TAB_INIT_DIR, m_strInitialDir)
			DDX_RADIO(IDC_RADIO_BK_TYPE, m_nBkType)
			DDX_TEXT(IDC_BK_IMAGE, m_strBkImage)
			DDX_CHECK(IDC_CHECK_BK_RELATIVE, m_nRelative)
			DDX_CHECK(IDC_CHECK_BK_EXTEND, m_nExtend)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsTabs)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_ICON, OnBtnBrowseIcon)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_SHELL, OnClickedBtnBrowseShell)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_DIR, OnClickedBtnBrowseDir)
			COMMAND_HANDLER(IDC_CURSOR_COLOR, BN_CLICKED, OnClickedCursorColor)
			COMMAND_RANGE_CODE_HANDLER(IDC_RADIO_BK_TYPE, IDC_RADIO_BK_TYPE3, BN_CLICKED, OnClickedBkType)
			COMMAND_HANDLER(IDC_BK_COLOR, BN_CLICKED, OnClickedBkColor)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_BK, OnBtnBrowseImage)
			COMMAND_HANDLER(IDC_TINT_COLOR, BN_CLICKED, OnClickedTintColor)

			NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListItemChanged)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseShell(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedCursorColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnClickedBkType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBkColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnBtnBrowseImage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedTintColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);

		LRESULT OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	private:

		void SetTabData(TabData* pTabData);
		void UpdateSliderText();
		void EnableControls();

	private:

		TabSettings		m_tabSettings;

		CListViewCtrl	m_listCtrl;
		CComboBox		m_comboCursor;
		CComboBox		m_comboBkPosition;
		CTrackBarCtrl	m_sliderTintOpacity;
		CStatic			m_staticTintOpacity;


		CString			m_strTitle;
		CString			m_strIcon;

		CString			m_strShell;
		CString			m_strInitialDir;

		int				m_nBkType;
		CString			m_strBkImage;
		int				m_nRelative;
		int				m_nExtend;

/*
		HotKeys			m_hotKeys;

		CListViewCtrl	m_listCtrl;
		CEdit			m_editCommand;
		CHotkeyEdit		m_hotKeyEdit;
*/
};

//////////////////////////////////////////////////////////////////////////////

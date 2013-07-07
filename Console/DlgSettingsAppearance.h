
#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsAppearance 
	: public DlgSettingsBase
{
	public:

		DlgSettingsAppearance(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsAppearance)
			DDX_TEXT(IDC_WINDOW_TITLE, m_strWindowTitle)
			DDX_CHECK(IDC_CHECK_USE_TAB_TITLE, m_windowSettings.bUseTabTitles)
			DDX_CHECK(IDC_CHECK_USE_CONSOLE_TITLE, m_windowSettings.bUseConsoleTitle)
			DDX_CHECK(IDC_CHECK_SHOW_COMMAND, m_windowSettings.bShowCommand)
			DDX_CHECK(IDC_CHECK_SHOW_COMMAND_TABS, m_windowSettings.bShowCommandInTabs)
			DDX_CHECK(IDC_CHECK_TRIM_TAB_TITLES, m_bTrimTabTitles)
			DDX_UINT(IDC_TRIM_TAB_TITLES, m_windowSettings.dwTrimTabTitles)
			DDX_UINT(IDC_TRIM_TAB_TITLES_RIGHT, m_windowSettings.dwTrimTabTitlesRight)
			DDX_TEXT(IDC_WINDOW_ICON, m_strWindowIcon)
			DDX_CHECK(IDC_CHECK_USE_TAB_ICON, m_windowSettings.bUseTabIcon)
			DDX_CHECK(IDC_CHECK_POSITION, m_bUsePosition)
			DDX_INT(IDC_POS_X, m_nX)
			DDX_INT(IDC_POS_Y, m_nY)
			DDX_CHECK(IDC_CHECK_SAVE_POSITION, m_positionSettings.bSavePosition)
			DDX_CHECK(IDC_CHECK_SNAP, m_bSnapToEdges)
			DDX_INT(IDC_SNAP, m_positionSettings.nSnapDistance)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsAppearance)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_CHECK_USE_TAB_TITLE, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_USE_TAB_ICON, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_BTN_BROWSE_ICON, BN_CLICKED, OnClickedBtnBrowseIcon)
			COMMAND_HANDLER(IDC_CHECK_USE_CONSOLE_TITLE, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_TRIM_TAB_TITLES, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_POSITION, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_SNAP, BN_CLICKED, OnClickedCheckbox)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedCheckbox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

		void EnableControls();

	private:

		WindowSettings				m_windowSettings;
		PositionSettings			m_positionSettings;

		CString						m_strWindowTitle;
		bool						m_bTrimTabTitles;
		CString						m_strWindowIcon;

		bool						m_bUsePosition;
		int							m_nX;
		int							m_nY;
		bool						m_bSnapToEdges;

		CComboBox					m_comboDocking;
		CComboBox					m_comboZOrder;
};

//////////////////////////////////////////////////////////////////////////////

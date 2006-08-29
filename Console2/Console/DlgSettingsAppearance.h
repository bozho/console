
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
			DDX_CHECK(IDC_CHECK_USE_TAB_TITLE, m_nUseTabTitle)
			DDX_CHECK(IDC_CHECK_USE_CONSOLE_TITLE, m_nUseConsoleTitle)
			DDX_CHECK(IDC_CHECK_SHOW_COMMAND, m_nShowCommand)
			DDX_CHECK(IDC_CHECK_SHOW_COMMAND_TABS, m_nShowCommandTabs)
			DDX_CHECK(IDC_CHECK_TRIM_TAB_TITLES, m_nTrimTabTitles)
			DDX_UINT(IDC_TRIM_TAB_TITLES, m_windowSettings.dwTrimTabTitles)
			DDX_TEXT(IDC_WINDOW_ICON, m_strWindowIcon)
			DDX_CHECK(IDC_CHECK_USE_TAB_ICON, m_nUseTabIcon)
			DDX_TEXT(IDC_FONT, m_strFontName)
			DDX_UINT(IDC_FONT_SIZE, m_fontSettings.dwSize)
			DDX_CHECK(IDC_CHECK_BOLD, m_nFontBold)
			DDX_CHECK(IDC_CHECK_ITALIC, m_nFontItalic)
			DDX_CHECK(IDC_CHECK_USE_COLOR, m_nUseFontColor)
			DDX_CHECK(IDC_CHECK_POSITION, m_nUsePosition)
			DDX_INT(IDC_POS_X, m_nX)
			DDX_INT(IDC_POS_Y, m_nY)
			DDX_CHECK(IDC_CHECK_SAVE_POSITION, m_nSavePosition)
			DDX_CHECK(IDC_CHECK_SNAP, m_nSnapToEdges)
			DDX_INT(IDC_SNAP, m_positionSettings.nSnapDistance)
			DDX_RADIO(IDC_RADIO_DOCK_NONE, m_nDocking)
			DDX_RADIO(IDC_RADIO_Z_REGULAR, m_nZOrder)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsAppearance)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_CHECK_USE_TAB_TITLE, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_USE_TAB_ICON, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_BTN_BROWSE_ICON, BN_CLICKED, OnClickedBtnBrowseIcon)
			COMMAND_HANDLER(IDC_BTN_BROWSE_FONT, BN_CLICKED, OnClickedBtnBrowseFont)
			COMMAND_HANDLER(IDC_CHECK_USE_CONSOLE_TITLE, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_TRIM_TAB_TITLES, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_USE_COLOR, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_POSITION, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_CHECK_SNAP, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_FONT_COLOR, BN_CLICKED, OnClickedFontColor)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedCheckbox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedFontColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);

	private:

		void EnableControls();

	private:

		WindowSettings				m_windowSettings;
		FontSettings				m_fontSettings;
		PositionSettings			m_positionSettings;

		CString						m_strWindowTitle;
		int							m_nUseTabTitle;
		int							m_nUseConsoleTitle;
		int							m_nShowCommand;
		int							m_nShowCommandTabs;
		int							m_nTrimTabTitles;
		CString						m_strWindowIcon;
		int							m_nUseTabIcon;

		CString						m_strFontName;
		int							m_nFontBold;
		int							m_nFontItalic;
		int							m_nUseFontColor;

		int							m_nUsePosition;
		int							m_nX;
		int							m_nY;
		int							m_nSavePosition;
		int							m_nSnapToEdges;

		int							m_nDocking;
		int							m_nZOrder;
};

//////////////////////////////////////////////////////////////////////////////

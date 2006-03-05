
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
			DDX_TEXT(IDC_FONT, m_strFontName);
			DDX_UINT(IDC_FONT_SIZE, m_fontSettings.dwSize);
			DDX_CHECK(IDC_CHECK_BOLD, m_nFontBold);
			DDX_CHECK(IDC_CHECK_ITALIC, m_nFontItalic);
			DDX_CHECK(IDC_CHECK_USE_COLOR, m_nUseFontColor);
			DDX_CHECK(IDC_CHECK_SHOW_MENU, m_nShowMenu);
			DDX_CHECK(IDC_CHECK_SHOW_TOOLBAR, m_nShowToolbar);
			DDX_CHECK(IDC_CHECK_SHOW_TABS, m_nShowTabs);
			DDX_CHECK(IDC_CHECK_SHOW_STATUS, m_nShowStatusbar);
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsAppearance)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_BTN_BROWSE_FONT, BN_CLICKED, OnClickedBtnBrowseFont)
			COMMAND_HANDLER(IDC_CHECK_USE_COLOR, BN_CLICKED, OnClickedUseColor)
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
		LRESULT OnClickedBtnBrowseFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedUseColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedFontColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);

	private:

		void EnableControls();

	private:

		FontSettings				m_fontSettings;
		WindowSettings				m_windowSettings;

		CString						m_strFontName;
		int							m_nFontBold;
		int							m_nFontItalic;
		int							m_nUseFontColor;

		int							m_nShowMenu;
		int							m_nShowToolbar;
		int							m_nShowTabs;
		int							m_nShowStatusbar;
};

//////////////////////////////////////////////////////////////////////////////


#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsFont 
	: public DlgSettingsBase
{
	public:

		DlgSettingsFont(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsFont)
			DDX_TEXT(IDC_FONT, m_strFontName)
			DDX_UINT(IDC_FONT_SIZE, m_fontSettings.dwSize)
			DDX_UINT(IDC_FONT_EXTRAWIDTH, m_fontSettings.dwExtraWidth)
			DDX_CHECK(IDC_CHECK_BOLD, m_fontSettings.bBold)
			DDX_CHECK(IDC_CHECK_ITALIC, m_fontSettings.bItalic)
			DDX_CHECK(IDC_CHECK_USE_COLOR, m_fontSettings.bUseColor)
			DDX_CHECK(IDC_CHECK_BOLD_INTENSIFIED, m_fontSettings.bBoldIntensified)
			DDX_CHECK(IDC_CHECK_ITALIC_INTENSIFIED, m_fontSettings.bItalicIntensified)
			DDX_CHECK(IDC_CHECK_LIGATURE, m_fontSettings.bLigature)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsFont)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_BTN_BROWSE_FONT, BN_CLICKED, OnClickedBtnBrowseFont)
			COMMAND_HANDLER(IDC_CHECK_USE_COLOR, BN_CLICKED, OnClickedCheckbox)
			COMMAND_HANDLER(IDC_FONT_COLOR, BN_CLICKED, OnClickedFontColor)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnBrowseFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedCheckbox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedFontColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);

	private:

		void EnableControls();

	private:

		FontSettings				m_fontSettings;

		CString						m_strFontName;

		CComboBox					m_comboFontSmoothing;
};

//////////////////////////////////////////////////////////////////////////////

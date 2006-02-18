
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
			DDX_UINT(IDC_FONT_SIZE, m_appearanceSettings.fontSettings.dwSize);
			DDX_CHECK(IDC_CHECK_BOLD, m_nFontBold);
			DDX_CHECK(IDC_CHECK_ITALIC, m_nFontItalic);
			DDX_RADIO(IDC_TRANSPARENCY_TYPE, reinterpret_cast<int&>(m_appearanceSettings.transparencySettings.transType));
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsAppearance)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_BTN_BROWSE_FONT, BN_CLICKED, OnClickedBtnBrowseFont)
			COMMAND_HANDLER(IDC_KEY_COLOR, BN_CLICKED, OnClickedKeyColor)
			COMMAND_RANGE_CODE_HANDLER(IDC_TRANSPARENCY_TYPE, IDC_TRANSPARENCY_TYPE3, BN_CLICKED, OnClickedTransType)
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
		LRESULT OnClickedKeyColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnClickedTransType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

		void UpdateSliderText(HWND hwnd);
		void EnableTransparencyControls();

	private:

		AppearanceSettings			m_appearanceSettings;

		CTrackBarCtrl				m_sliderActiveAlpha;
		CTrackBarCtrl				m_sliderInactiveAlpha;

		CString						m_strFontName;
		int							m_nFontBold;
		int							m_nFontItalic;

		int							m_nTransparencyType;
};

//////////////////////////////////////////////////////////////////////////////

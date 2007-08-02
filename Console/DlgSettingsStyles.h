
#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsStyles 
	: public DlgSettingsBase
{
	public:

		DlgSettingsStyles(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsStyles)
			DDX_CHECK(IDC_CHECK_SHOW_MENU, m_nShowMenu)
			DDX_CHECK(IDC_CHECK_SHOW_TOOLBAR, m_nShowToolbar)
			DDX_CHECK(IDC_CHECK_SHOW_STATUS, m_nShowStatusbar)
			DDX_CHECK(IDC_CHECK_SHOW_TABS, m_nShowTabs)
			DDX_CHECK(IDC_CHECK_HIDE_SINGLE_TAB, m_nHideSingleTab)
			DDX_CHECK(IDC_CHECK_STYLE_CAPTION, m_nShowCaption)
			DDX_CHECK(IDC_CHECK_STYLE_RESIZABLE, m_nResizable)
			DDX_CHECK(IDC_CHECK_STYLE_BORDER, m_nBorder)
			DDX_CHECK(IDC_CHECK_STYLE_TASKBAR, m_nTaskbarButton)
			DDX_CHECK(IDC_CHECK_STYLE_TRAY, m_nTrayIcon)
			DDX_UINT(IDC_INSIDE_BORDER, m_stylesSettings.dwInsideBorder)
			DDX_RADIO(IDC_TRANSPARENCY_TYPE, reinterpret_cast<int&>(m_transparencySettings.transType))
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsStyles)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_CHECK_SHOW_TABS, BN_CLICKED, OnClickedShowTabs)
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
		LRESULT OnClickedShowTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnClickedKeyColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnClickedTransType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

		void UpdateSliderText(HWND hwnd);
		void EnableTabControls();
		void EnableTransparencyControls();

	private:

		ControlsSettings			m_controlsSettings;
		StylesSettings				m_stylesSettings;
		TransparencySettings		m_transparencySettings;

		CTrackBarCtrl				m_sliderActiveAlpha;
		CTrackBarCtrl				m_sliderInactiveAlpha;

		int							m_nShowMenu;
		int							m_nShowToolbar;
		int							m_nShowStatusbar;
		int							m_nShowTabs;
		int							m_nHideSingleTab;

		int							m_nShowCaption;
		int							m_nResizable;
		int							m_nTaskbarButton;
		int							m_nBorder;
		int							m_nTrayIcon;

		int							m_nTransparencyType;
};

//////////////////////////////////////////////////////////////////////////////

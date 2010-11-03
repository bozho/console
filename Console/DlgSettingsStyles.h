
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
			DDX_CHECK(IDC_CHECK_SHOW_MENU, m_controlsSettings.bShowMenu)
			DDX_CHECK(IDC_CHECK_SHOW_TOOLBAR, m_controlsSettings.bShowToolbar)
			DDX_CHECK(IDC_CHECK_SHOW_STATUS, m_controlsSettings.bShowStatusbar)
			DDX_CHECK(IDC_CHECK_SHOW_TABS, m_controlsSettings.bShowTabs)
			DDX_CHECK(IDC_CHECK_HIDE_SINGLE_TAB, m_controlsSettings.bHideSingleTab)
			DDX_CHECK(IDC_CHECK_TABS_ON_BOTTOM, m_controlsSettings.bTabsOnBottom)
			DDX_CHECK(IDC_CHECK_SHOW_SCROLLBARS, m_controlsSettings.bShowScrollbars)
			DDX_CHECK(IDC_CHECK_FLAT_SCROLLBARS, m_controlsSettings.bFlatScrollbars)
			DDX_CHECK(IDC_CHECK_STYLE_CAPTION, m_stylesSettings.bCaption)
			DDX_CHECK(IDC_CHECK_STYLE_RESIZABLE, m_stylesSettings.bResizable)
			DDX_CHECK(IDC_CHECK_STYLE_BORDER, m_stylesSettings.bBorder)
			DDX_CHECK(IDC_CHECK_STYLE_TASKBAR, m_stylesSettings.bTaskbarButton)
			DDX_CHECK(IDC_CHECK_STYLE_TRAY, m_stylesSettings.bTrayIcon)
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
			COMMAND_HANDLER(IDC_SELECTION_COLOR, BN_CLICKED, OnClickedSelColor)
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
		LRESULT OnClickedSelColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
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

		int							m_nTransparencyType;
};

//////////////////////////////////////////////////////////////////////////////

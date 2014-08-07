
#pragma once

#include "PageSettingsTab.h"
#include "Cursors.h"

class PageSettingsTabsColors
	: public CDialogImpl<PageSettingsTabsColors>
	, public CWinDataExchange<PageSettingsTabsColors>
	, public CursorCharDrawer
	, public PageSettingsTab
{
	public:

		enum { IDD = IDD_SETTINGS_TABS_COLORS };

		PageSettingsTabsColors(ConsoleSettings &consoleSettings);

		BEGIN_DDX_MAP(PageSettingsTabsColors)
			DDX_CHECK(IDC_BTN_INHERIT_COLORS, m_tabData->bInheritedColors)
			DDX_CHECK(IDC_BTN_INHERIT_CURSOR, m_tabData->bInheritedCursor)
		END_DDX_MAP()

		BEGIN_MSG_MAP(PageSettingsTabsColors)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
#ifdef _USE_AERO
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
#endif //_USE_AERO

			COMMAND_HANDLER(IDC_BTN_RESET_COLORS, BN_CLICKED, OnClickedBtnResetColors)
			COMMAND_HANDLER(IDC_BTN_IMPORT_COLORS, BN_CLICKED, OnClickedBtnImportColors)
			COMMAND_HANDLER(IDC_BTN_INHERIT_COLORS, BN_CLICKED, OnClickedBtnInheritColors)
			COMMAND_RANGE_CODE_HANDLER(IDC_CLR_00, IDC_CLR_15, BN_CLICKED, OnClickedClrBtn)
			COMMAND_HANDLER(IDC_CURSOR_COLOR, BN_CLICKED, OnClickedCursorColor)
			COMMAND_HANDLER(IDC_COMBO_CURSOR, CBN_SELCHANGE, OnCbnSelchangeComboCursor)
			COMMAND_HANDLER(IDC_BTN_INHERIT_CURSOR, BN_CLICKED, OnClickedBtnInheritCursor)
			COMMAND_HANDLER(IDC_BTN_SET_AS_DEFAULT_COLORS, BN_CLICKED, OnClickedBtnSetAsDefaultColors)
			COMMAND_HANDLER(IDC_BTN_SET_AS_DEFAULT_CURSOR, BN_CLICKED, OnClickedBtnSetAsDefaultCursor)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

#ifdef _USE_AERO
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
#endif //_USE_AERO

		LRESULT OnClickedBtnResetColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnImportColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnInheritColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedClrBtn(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnClickedCursorColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnCbnSelchangeComboCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnInheritCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnSetAsDefaultColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnSetAsDefaultCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		void Load(shared_ptr<TabData>& tabData);
		void Save();

	private:
		void SetCursor(void);
		void DrawCursor(void);
		void DrawCursor(CDC& dc, const CRect& rectCursorAnim, COLORREF fg, COLORREF bg);
		virtual void RedrawCharOnCursor(CDC& dc);

		void EnableControls();
		void Load();

	private:

		ConsoleSettings	&m_consoleSettings;
		shared_ptr<TabData>	m_tabData;
		std::shared_ptr<Cursor>	m_cursor;

		CStatic			m_staticCursorAnim;
		CComboBox		m_comboCursor;
		CStatic			m_staticCursorColor;

#ifdef _USE_AERO
		CTrackBarCtrl   m_sliderBGTextOpacity;
		CStatic         m_staticBGTextOpacity;

		void UpdateSliderText();
#endif //_USE_AERO
};

//////////////////////////////////////////////////////////////////////////////

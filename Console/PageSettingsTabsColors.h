
#pragma once

#include "PageSettingsTab.h"

class PageSettingsTabsColors
	: public CDialogImpl<PageSettingsTabsColors>
	, public CWinDataExchange<PageSettingsTabsColors>
	, public PageSettingsTab
{
	public:

		enum { IDD = IDD_SETTINGS_TABS_COLORS };

		PageSettingsTabsColors(ConsoleSettings &consoleSettings);

		BEGIN_DDX_MAP(PageSettingsTabsColors)
			DDX_CHECK(IDC_BTN_INHERIT_COLORS, m_tabData->bInheritedColors)
		END_DDX_MAP()

		BEGIN_MSG_MAP(PageSettingsTabs1)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)

			COMMAND_HANDLER(IDC_BTN_RESET_COLORS, BN_CLICKED, OnClickedBtnResetColors)
			COMMAND_HANDLER(IDC_BTN_INHERIT_COLORS, BN_CLICKED, OnClickedBtnInheritColors)
			COMMAND_RANGE_CODE_HANDLER(IDC_CLR_00, IDC_CLR_15, BN_CLICKED, OnClickedClrBtn)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnClickedBtnResetColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnInheritColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedClrBtn(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

		LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

		void Load(shared_ptr<TabData>& tabData);
		void Save();

	private:

		ConsoleSettings	&m_consoleSettings;
		shared_ptr<TabData>	m_tabData;
};

//////////////////////////////////////////////////////////////////////////////

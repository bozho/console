
#pragma once

#include "PageSettingsTab.h"

class PageSettingsTabsEnv
	: public CDialogImpl<PageSettingsTabsEnv>
	, public CWinDataExchange<PageSettingsTabsEnv>
	, public PageSettingsTab
{
	public:

		enum { IDD = IDD_SETTINGS_TABS_ENV };

		PageSettingsTabsEnv();

		BEGIN_DDX_MAP(PageSettingsTabsEnv)
		END_DDX_MAP()

		BEGIN_MSG_MAP(PageSettingsTabsEnv)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

			COMMAND_ID_HANDLER(IDC_BTN_ENV_ADD,    OnClickedBtnEnvAdd)
			COMMAND_ID_HANDLER(IDC_BTN_ENV_DELETE, OnClickedBtnEnvDelete)
			COMMAND_ID_HANDLER(IDC_BTN_ENV_EDIT,   OnClickedBtnEnvEdit)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnClickedBtnEnvAdd   (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnEnvDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnEnvEdit  (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		void Load(shared_ptr<TabData>& tabData);
		void Save();

	private:

		shared_ptr<TabData> m_tabData;

		CListViewCtrl       m_listCtrl;
		CEdit               m_editVariable;
		CEdit               m_editValue;
};

//////////////////////////////////////////////////////////////////////////////

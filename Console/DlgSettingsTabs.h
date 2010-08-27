
#pragma once

#include "DlgSettingsBase.h"
#include "PageSettingsTabs1.h"
#include "PageSettingsTabs2.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsTabs 
	: public DlgSettingsBase
{
	public:

		DlgSettingsTabs(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsTabs)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsTabs)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(UM_TAB_TITLE_CHANGED, OnTabTitleChanged)

			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_ID_HANDLER(IDC_BTN_ADD, OnAdd)
			COMMAND_ID_HANDLER(IDC_BTN_DELETE, OnDelete)
			COMMAND_ID_HANDLER(IDC_BTN_UP, OnUp)
			COMMAND_ID_HANDLER(IDC_BTN_DOWN, OnDown)

			NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListItemChanged)
			NOTIFY_CODE_HANDLER(TCN_SELCHANGE, OnTabItemChanged)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnTabTitleChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
		LRESULT OnTabItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	private:

		void MoveListItem(int nItem, int nDirection);

	private:

		TabSettings		m_tabSettings;

		CListViewCtrl	m_listCtrl;

		CTabCtrl			m_tabCtrl;
		PageSettingsTabs1	m_page1;
		PageSettingsTabs2	m_page2;
};

//////////////////////////////////////////////////////////////////////////////

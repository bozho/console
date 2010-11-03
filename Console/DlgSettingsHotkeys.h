
#pragma once

#include "DlgSettingsBase.h"
#include "HotkeyEdit.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsHotkeys 
	: public DlgSettingsBase
{
	public:

		DlgSettingsHotkeys(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsHotkeys)
			DDX_CHECK(IDC_CHECK_USE_SCROLL_LOCK, m_hotKeys.bUseScrollLock)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsHotkeys)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListItemChanged)
			COMMAND_ID_HANDLER(IDC_BTN_ASSIGN, OnBtnAssign)
			COMMAND_ID_HANDLER(IDC_BTN_CLEAR, OnBtnClear)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

		LRESULT OnBtnAssign(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBtnClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

		HotKeys			m_hotKeys;

		CListViewCtrl	m_listCtrl;
		CEdit			m_editCommand;
		CHotkeyEdit		m_hotKeyEdit;
};

//////////////////////////////////////////////////////////////////////////////

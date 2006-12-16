
#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsBehavior 
	: public DlgSettingsBase
{
	public:

		DlgSettingsBehavior(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsBehavior)
			DDX_CHECK(IDC_CHECK_COPY_ON_SELECT, m_nCopyOnSelect)
			DDX_CHECK(IDC_CHECK_NO_WRAP, m_nNoWrap)
			DDX_CHECK(IDC_CHECK_TRIM_SPACES, m_nTrimSpaces)
			DDX_RADIO(IDC_PAGE_SCROLL, m_nScrollPageType)
			DDX_UINT(IDC_SCROLL_PAGE_ROWS, m_behaviorSettings.scrollSettings.dwPageScrollRows)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsBehavior)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_RANGE_CODE_HANDLER(IDC_PAGE_SCROLL, IDC_PAGE_SCROLL2, BN_CLICKED, OnClickedScrollType)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedScrollType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

		void EnableScrollControls();


	private:

		BehaviorSettings	m_behaviorSettings;

		int					m_nCopyOnSelect;
		int					m_nNoWrap;
		int					m_nTrimSpaces;

		int					m_nScrollPageType;
};

//////////////////////////////////////////////////////////////////////////////

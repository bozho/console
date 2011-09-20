
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
			DDX_CHECK(IDC_CHECK_COPY_ON_SELECT, m_behaviorSettings.copyPasteSettings.bCopyOnSelect)
			DDX_CHECK(IDC_CHECK_CLEAR_ON_COPY, m_behaviorSettings.copyPasteSettings.bClearOnCopy)
			DDX_CHECK(IDC_CHECK_SENSITIVE_COPY, m_behaviorSettings.copyPasteSettings.bSensitiveCopy)
			DDX_CHECK(IDC_CHECK_NO_WRAP, m_behaviorSettings.copyPasteSettings.bNoWrap)
			DDX_CHECK(IDC_CHECK_TRIM_SPACES, m_behaviorSettings.copyPasteSettings.bTrimSpaces)
			DDX_RADIO(IDC_RADIO_COPY_NEWLINE_CHAR, m_nCopyNewlineChar)
			DDX_RADIO(IDC_PAGE_SCROLL, m_nScrollPageType)
			DDX_UINT(IDC_SCROLL_PAGE_ROWS, m_behaviorSettings.scrollSettings.dwPageScrollRows)
			DDX_CHECK(IDC_CHECK_FLASH_TAB, m_bFlashInactiveTab)
			DDX_UINT(IDC_TAB_FLASHES, m_behaviorSettings.tabHighlightSettings.dwFlashes)
			DDX_CHECK(IDC_CHECK_LEAVE_HIGHLIGHTED, m_behaviorSettings.tabHighlightSettings.bStayHighlighted)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsBehavior)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_RANGE_CODE_HANDLER(IDC_PAGE_SCROLL, IDC_PAGE_SCROLL2, BN_CLICKED, OnClickedScrollType)
			COMMAND_HANDLER(IDC_CHECK_FLASH_TAB, BN_CLICKED, OnClickedFlashTab)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedScrollType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedFlashTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

		void EnableScrollControls();
		void EnableFlashTabControls();


	private:

		BehaviorSettings	m_behaviorSettings;

		int					m_nCopyNewlineChar;

		int					m_nScrollPageType;

		bool				m_bFlashInactiveTab;
};

//////////////////////////////////////////////////////////////////////////////

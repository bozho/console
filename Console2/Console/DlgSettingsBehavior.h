
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
			DDX_CHECK(IDC_CHECK_DRAG_ON, m_nMouseDrag)
			DDX_CHECK(IDC_CHECK_INVERSE_SHIFT, m_nInverseShift)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsBehavior)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

	private:

		BehaviorSettings	m_behaviorSettings;

		int					m_nCopyOnSelect;
		int					m_nNoWrap;
		int					m_nTrimSpaces;

		int					m_nMouseDrag;
		int					m_nInverseShift;
};

//////////////////////////////////////////////////////////////////////////////

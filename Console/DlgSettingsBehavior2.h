
#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsBehavior2
	: public DlgSettingsBase
{
	public:

		DlgSettingsBehavior2(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsBehavior2)
			DDX_CHECK(IDC_CHECK_FOCUS_FOLLOW_MOUSE, m_behaviorSettings2.focusSettings.bFollowMouse)
			DDX_CHECK(IDC_CHECK_MULTI_INSTANCES,    m_behaviorSettings2.instanceSettings.bAllowMultipleInstances)
			DDX_CHECK(IDC_CHECK_OPEN_CURRENT_DIR,   m_behaviorSettings2.cloneSettings.bUseCurrentDirectory)
			DDX_CHECK(IDC_CHECK_USE_CREDPROV,       m_behaviorSettings2.runAsUserSettings.bUseCredentialProviders)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsBehavior2)
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

		BehaviorSettings2	m_behaviorSettings2;

};

//////////////////////////////////////////////////////////////////////////////

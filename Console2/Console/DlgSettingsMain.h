
#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef map<HTREEITEM, shared_ptr<DlgSettingsBase> >	SettingsDlgsMap;

//////////////////////////////////////////////////////////////////////////////

class DlgSettingsMain 
	: public CDialogImpl<DlgSettingsMain>
{
	public:
		enum { IDD = IDD_SETTINGS_MAIN };

		DlgSettingsMain();

		BEGIN_MSG_MAP(DlgSettingsMain)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeSelChanged)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnTreeSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	private:

		void CreateSettingsTree();
		HTREEITEM AddDialogToTree(const wstring& strName, const shared_ptr<DlgSettingsBase>& newDlg, CRect& rect, HTREEITEM htiParent = NULL);

	private:

		wstring						m_strSettingsFileName;

		CTreeViewCtrl				m_treeCtrl;
		CButton						m_checkUserDataDir;

		SettingsDlgsMap				m_settingsDlgMap;
		
		CComPtr<IXMLDOMDocument>	m_pSettingsDocument;
		CComPtr<IXMLDOMElement>		m_pSettingsRoot;
};

//////////////////////////////////////////////////////////////////////////////

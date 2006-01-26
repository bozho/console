
#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgRenameTab 
	: public CDialogImpl<DlgRenameTab>
	, public CWinDataExchange<DlgRenameTab> {

	public:
		enum { IDD = IDD_RENAME_TAB };

		DlgRenameTab(const CString& strTabName);

		BEGIN_DDX_MAP(DlgRenameTab)
			DDX_TEXT(IDC_TAB_NAME, m_strTabName)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgRenameTab)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnOK)
			COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	public:

		CString	m_strTabName;
};

//////////////////////////////////////////////////////////////////////////////

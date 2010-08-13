
#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgRenameTab 
#ifdef _USE_AERO
  : public aero::CDialogImpl<DlgRenameTab>
#else
	: public CDialogImpl<DlgRenameTab>
#endif
	, public CWinDataExchange<DlgRenameTab>
{

	public:
		enum { IDD = IDD_RENAME_TAB };

		DlgRenameTab(const CString& strTabName);

		BEGIN_DDX_MAP(DlgRenameTab)
			DDX_TEXT(IDC_TAB_NAME, m_strTabName)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgRenameTab)
#ifdef _USE_AERO
      CHAIN_MSG_MAP(aero::CDialogImpl<DlgRenameTab>)
#endif
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

	public:

		CString	m_strTabName;
};

//////////////////////////////////////////////////////////////////////////////

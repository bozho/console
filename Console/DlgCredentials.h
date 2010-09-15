
#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgCredentials 
	: public CDialogImpl<DlgCredentials>
	, public CWinDataExchange<DlgCredentials>
{
	public:
		enum { IDD = IDD_CREDENTIALS };

		DlgCredentials(const CString& user);
		~DlgCredentials();

		BEGIN_DDX_MAP(DlgCredentials)
			DDX_TEXT(IDC_USER, m_user)
			DDX_TEXT(IDC_PASSWORD, m_password)
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgCredentials)
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

		const CString& GetUser() const { return m_user; }
		const CString& GetPassword() const { return m_password; }

	private:

		CString	m_user;
		CString	m_password;
};

//////////////////////////////////////////////////////////////////////////////

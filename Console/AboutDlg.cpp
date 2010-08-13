// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CWindow	staticMessage(GetDlgItem(IDC_STATIC_VERSION));
	CString	strMsg;

	strMsg.Format(L"\nConsole %i.%02i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
	staticMessage.SetWindowText(strMsg);

	CenterWindow(GetParent());

#ifdef _USE_AERO
  AERO_CONTROL(CButton, m_Ok, IDOK)
  AERO_CONTROL(CStatic, m_Version, IDC_STATIC_VERSION)
  AERO_CONTROL(CStatic, m_Text, IDC_APPTEXT)
#endif

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

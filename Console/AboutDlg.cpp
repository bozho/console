// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
#ifdef _USE_AERO
  AERO_CONTROL(CButton, m_Ok, IDOK)
  this->OpenThemeData(VSCLASS_WINDOW);
#else
  CString strMsg;
  strMsg.Format(L"\nConsoleZ %i.%02i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);

  CWindow staticMessage(GetDlgItem(IDC_STATIC_VERSION));
  staticMessage.SetWindowText(strMsg);
#endif
	CenterWindow(GetParent());

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

#include "stdafx.h"
#include "resource.h"

#include "Console.h"
#include "DlgSettingsConsole.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsConsole::DlgSettingsConsole(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
, m_strShell(L"")
, m_strInitialDir(L"")
{
	IDD = IDD_SETTINGS_CONSOLE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsConsole::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_consoleSettings.Load(m_pOptionsRoot);
	m_strShell		= m_consoleSettings.strShell.c_str();
	m_strInitialDir	= m_consoleSettings.strInitialDir.c_str();

	CUpDownCtrl	spin;
	UDACCEL udAccel;

	spin.Attach(GetDlgItem(IDC_SPIN_CHANGE_REFRESH));
	spin.SetRange(5, 200);
	udAccel.nSec = 0;
	udAccel.nInc = 5;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_REFRESH));
	spin.SetRange(50, 2000);
	udAccel.nSec = 0;
	udAccel.nInc = 10;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_ROWS));
	spin.SetRange(10, 200);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_BUFFER_ROWS));
	spin.SetRange(10, 9999);
	udAccel.nSec = 0;
	udAccel.nInc = 10;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_COLUMNS));
	spin.SetRange(10, 200);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_BUFFER_COLUMNS));
	spin.SetRange(10, 200);
	udAccel.nSec = 0;
	udAccel.nInc = 5;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsConsole::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CWindow		staticCtl(reinterpret_cast<HWND>(lParam));
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));
	DWORD		i;

	for (i = IDC_DEF_00; i <= IDC_DEF_15; ++i)
	{
		if (staticCtl.m_hWnd == GetDlgItem(i))
		{
			CBrush	brush(::CreateSolidBrush(m_consoleSettings.defaultConsoleColors[i-IDC_DEF_00]));
			CRect	rect;

			staticCtl.GetClientRect(&rect);
			dc.FillRect(&rect, brush);
			return 0;
		}
	}

	for (i = IDC_CLR_00; i <= IDC_CLR_15; ++i)
	{
		if (staticCtl.m_hWnd == GetDlgItem(i))
		{
			CBrush	brush(::CreateSolidBrush(m_consoleSettings.consoleColors[i-IDC_CLR_00]));
			CRect	rect;

			staticCtl.GetClientRect(&rect);
			dc.FillRect(&rect, brush);
			return 0;
		}
	}

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsConsole::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		if (!DoDataExchange(DDX_SAVE)) return -1;

		m_consoleSettings.strShell		= m_strShell;
		m_consoleSettings.strInitialDir	= m_strInitialDir;

		// set immediate settings
		ConsoleSettings& consoleSettings = g_settingsHandler->GetConsoleSettings();

		consoleSettings = m_consoleSettings;
		m_consoleSettings.Save(m_pOptionsRoot);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsConsole::OnClickedBtnBrowseShell(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog fileDialog(
					TRUE, 
					NULL, 
					NULL, 
					OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST, 
					L"Executable Files (*.exe)\0*.exe\0Batch Files (*.bat;*.cmd)\0*.bat;*.cmd\0All Files (*.*)\0*.*\0\0");

	if (fileDialog.DoModal() == IDOK)
	{
		m_strShell = fileDialog.m_szFileName;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsConsole::OnClickedBtnBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFolderDialog folderDialog(m_hWnd, L"Choose initial directory");

	if (folderDialog.DoModal() == IDOK)
	{
		m_strInitialDir = folderDialog.m_szFolderPath;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsConsole::OnClickedBtnResetColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::CopyMemory(m_consoleSettings.consoleColors, m_consoleSettings.defaultConsoleColors, sizeof(m_consoleSettings.defaultConsoleColors));

	DoDataExchange(DDX_LOAD);
	Invalidate();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsConsole::OnClickedClrBtn(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CColorDialog	dlg(m_consoleSettings.consoleColors[wID-IDC_CLR_00], CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		m_consoleSettings.consoleColors[wID-IDC_CLR_00] = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsConsole::OnDataValidateError(UINT nCtrlID, BOOL bSave, _XData& data)
{
	CString message;

	switch (nCtrlID)
	{
		case IDC_BUFFER_ROWS :
		{
			message.LoadString(MSG_SETTINGS_INVALID_BUFFER_ROWS);
			break;
		}

		case IDC_BUFFER_COLUMNS :
		{
			message.LoadString(MSG_SETTINGS_INVALID_BUFFER_COLUMNS);
			break;
		}

		case IDC_ROWS :
		{
			message.LoadString(MSG_SETTINGS_INVALID_ROWS);
			break;
		}

		case IDC_COLUMNS :
		{
			message.LoadString(MSG_SETTINGS_INVALID_COLUMNS);
			break;
		}

		default: break;
	}

	if (message.GetLength() > 0) ::MessageBox(this->GetParent(), message, L"Error", MB_OK|MB_ICONERROR);

	DlgSettingsBase::OnDataValidateError(nCtrlID, bSave, data);
}

//////////////////////////////////////////////////////////////////////////////

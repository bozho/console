#include "stdafx.h"
#include "resource.h"

#include "Console.h"
#include "DlgSettingsConsole.h"
#include "XmlHelper.h"

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
	spin.SetRange(MIN_WINDOW_ROWS, MAX_WINDOW_ROWS);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_BUFFER_ROWS));
	spin.SetRange(MIN_BUFFER_ROWS, MAX_BUFFER_ROWS);
	udAccel.nSec = 0;
	udAccel.nInc = 10;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_COLUMNS));
	spin.SetRange(MIN_WINDOW_COLUMNS, MAX_WINDOW_COLUMNS);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_BUFFER_COLUMNS));
	spin.SetRange(MIN_BUFFER_COLUMNS, MAX_BUFFER_COLUMNS);
	udAccel.nSec = 0;
	udAccel.nInc = 5;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	m_ShellEdit.SubclassWindow(GetDlgItem(IDC_SHELL));
	m_InitialDirEdit.SubclassWindow(GetDlgItem(IDC_INIT_DIR));

#ifdef _USE_AERO
	m_staticBGTextOpacity.Attach(GetDlgItem(IDC_BGTEXT_OPACITY_VAL));
	m_sliderBGTextOpacity.Attach(GetDlgItem(IDC_BGTEXT_OPACITY));
	m_sliderBGTextOpacity.SetRange(0, 255);
	m_sliderBGTextOpacity.SetTicFreq(5);
	m_sliderBGTextOpacity.SetPageSize(5);
	m_sliderBGTextOpacity.SetPos(m_consoleSettings.backgroundTextOpacity);
	UpdateSliderText();
#endif //_USE_AERO

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

#ifdef _USE_AERO
		m_consoleSettings.backgroundTextOpacity = static_cast<BYTE>(m_sliderBGTextOpacity.GetPos());
#endif //_USE_AERO

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
#ifdef _USE_AERO
	m_sliderBGTextOpacity.SetPos(255);
	UpdateSliderText();
#endif //_USE_AERO

	DoDataExchange(DDX_LOAD);
	Invalidate();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsConsole::OnClickedBtnImportColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  CFileDialog fileDialog(
    TRUE,
    NULL,
    NULL,
    OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST,
    L"Config Files (*.xml)\0*.xml\0All Files (*.*)\0*.*\0\0");

  if (fileDialog.DoModal() == IDOK)
  {
    CComPtr<IXMLDOMDocument> pSettingsDocument;
    CComPtr<IXMLDOMElement>  pSettingsRoot;
    if(FAILED(XmlHelper::OpenXmlDocument(
      fileDialog.m_szFileName,
      pSettingsDocument,
      pSettingsRoot))) return 0;

    CComPtr<IXMLDOMElement>	pConsoleElement;
    if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"console"), pConsoleElement))) return false;

    COLORREF colors[16];
    if(!XmlHelper::LoadColors(pConsoleElement, colors)) return 0;

    ::CopyMemory(m_consoleSettings.consoleColors, colors, sizeof(m_consoleSettings.defaultConsoleColors));

    DoDataExchange(DDX_LOAD);
    Invalidate();
  }

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
			message.Format(MSG_SETTINGS_INVALID_BUFFER_ROWS, MIN_BUFFER_ROWS, MAX_BUFFER_ROWS);
			break;
		}

		case IDC_BUFFER_COLUMNS :
		{
			message.Format(MSG_SETTINGS_INVALID_BUFFER_COLUMNS, MIN_BUFFER_COLUMNS, MAX_BUFFER_COLUMNS);
			break;
		}

		case IDC_ROWS :
		{
			message.Format(MSG_SETTINGS_INVALID_ROWS, MIN_WINDOW_ROWS, MAX_WINDOW_ROWS);
			break;
		}

		case IDC_COLUMNS :
		{
			message.Format(MSG_SETTINGS_INVALID_COLUMNS, MIN_WINDOW_COLUMNS, MAX_WINDOW_COLUMNS);
			break;
		}

		default: break;
	}

	if (message.GetLength() > 0) ::MessageBox(this->GetParent(), message, L"Error", MB_OK|MB_ICONERROR);

	DlgSettingsBase::OnDataValidateError(nCtrlID, bSave, data);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

#ifdef _USE_AERO

LRESULT DlgSettingsConsole::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	UpdateSliderText();
	return 0;
}

void DlgSettingsConsole::UpdateSliderText()
{
	CString strStaticText;
	strStaticText.Format(L"%i", m_sliderBGTextOpacity.GetPos());

	m_staticBGTextOpacity.SetWindowText(strStaticText);
}

#endif //_USE_AERO

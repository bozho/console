#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsFullScreen.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsFullScreen::DlgSettingsFullScreen(CComPtr<IXMLDOMElement>& pOptionsRoot)
  : DlgSettingsBase(pOptionsRoot)
{
  IDD = IDD_SETTINGS_FULLSCREEN;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsFullScreen::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  m_comboFullScreenMonitor.Attach(GetDlgItem(IDC_COMBO_FULLSCREEN_MONITOR));
	Helpers::LoadCombo(m_comboFullScreenMonitor, IDC_COMBO_FULLSCREEN_MONITOR);

  m_fullScreenSettings.Load(m_pOptionsRoot);

  ::EnumDisplayMonitors(NULL, NULL, DlgSettingsFullScreen::MonitorEnumProc, reinterpret_cast<LPARAM>(this));
  if( m_fullScreenSettings.dwFullScreenMonitor > static_cast<DWORD>(m_comboFullScreenMonitor.GetCount()) )
    m_comboFullScreenMonitor.SetCurSel(0);
  else
    m_comboFullScreenMonitor.SetCurSel(m_fullScreenSettings.dwFullScreenMonitor);

  DoDataExchange(DDX_LOAD);

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsFullScreen::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if (wID == IDOK)
  {
    DoDataExchange(DDX_SAVE);
    m_fullScreenSettings.dwFullScreenMonitor = m_comboFullScreenMonitor.GetCurSel();

    FullScreenSettings& fullScreenSettings= g_settingsHandler->GetAppearanceSettings().fullScreenSettings;
    fullScreenSettings = m_fullScreenSettings;

    m_fullScreenSettings.Save(m_pOptionsRoot);
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DlgSettingsFullScreen::MonitorEnumProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT /*lprcMonitor*/, LPARAM lpData)
{
  DlgSettingsFullScreen* p = reinterpret_cast<DlgSettingsFullScreen*>(lpData);

  MONITORINFOEX miex;
  miex.cbSize = sizeof(miex);
  ::GetMonitorInfo(hMonitor, &miex);

  DISPLAY_DEVICE dd;
  dd.cb = sizeof(dd);
  ::EnumDisplayDevices(miex.szDevice, 0, &dd, EDD_GET_DEVICE_INTERFACE_NAME);

  std::wstring strMonitor = miex.szDevice;
  strMonitor += L" - ";
  if(hMonitor == ::MonitorFromWindow(p->m_hWnd, MONITOR_DEFAULTTONULL))
	{
		strMonitor += L"[";
		strMonitor += Helpers::LoadStringW(MSG_SETTINGS_FULLSCREEN_CURRENT);
		strMonitor += L"] - ";
	}
  strMonitor += dd.DeviceString;
  p->m_comboFullScreenMonitor.AddString(strMonitor.c_str());

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

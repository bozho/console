#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsAppearance.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsAppearance::DlgSettingsAppearance(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
, m_bTrimTabTitles(false)
, m_bUsePosition(false)
, m_nX(0)
, m_nY(0)
, m_bUseSize(false)
, m_nW(0)
, m_nH(0)
, m_bSnapToEdges(false)
{
	IDD = IDD_SETTINGS_APPEARANCE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ExecuteDlgInit(IDD);

	m_comboDocking.Attach(GetDlgItem(IDC_COMBO_DOCKING));
	m_comboZOrder.Attach(GetDlgItem(IDC_COMBO_ZORDER));

	m_windowSettings.Load(m_pOptionsRoot);
	m_positionSettings.Load(m_pOptionsRoot);

	m_strWindowTitle = m_windowSettings.strTitle.c_str();
	m_bTrimTabTitles = (m_windowSettings.dwTrimTabTitles > 0);
	m_strWindowIcon  = m_windowSettings.strIcon.c_str();

	m_bUsePosition   = ((m_positionSettings.nX == -1) && (m_positionSettings.nY == -1)) ? false : true;
	m_nX             = m_bUsePosition ? m_positionSettings.nX : 0;
	m_nY             = m_bUsePosition ? m_positionSettings.nY : 0;

	m_bUseSize       = ((m_positionSettings.nW == -1) && (m_positionSettings.nH == -1)) ? false : true;
	m_nW             = m_bUseSize ? m_positionSettings.nW : 0;
	m_nH             = m_bUseSize ? m_positionSettings.nH : 0;

	m_bSnapToEdges   = (m_positionSettings.nSnapDistance != -1);
	if (!m_bSnapToEdges) m_positionSettings.nSnapDistance = 0;

	m_comboDocking.SetCurSel(static_cast<int>(m_positionSettings.dockPosition) + 1);
	m_comboZOrder.SetCurSel(static_cast<int>(m_positionSettings.zOrder));

	CUpDownCtrl	spin;
	UDACCEL udAccel;

	spin.Attach(GetDlgItem(IDC_SPIN_TRIM_TAB_TITLES));
	spin.SetRange(1, 100);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_TRIM_TAB_TITLES_RIGHT));
	spin.SetRange(1, 100);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_X));
	spin.SetRange(-2048, 2048);
	udAccel.nSec = 0;
	udAccel.nInc = 5;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_Y));
	spin.SetRange(-2048, 2048);
	udAccel.nSec = 0;
	udAccel.nInc = 5;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_W));
	spin.SetRange(-2048, 2048);
	udAccel.nSec = 0;
	udAccel.nInc = 5;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_H));
	spin.SetRange(-2048, 2048);
	udAccel.nSec = 0;
	udAccel.nInc = 5;
	spin.SetAccel(1, &udAccel);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_SNAP));
	spin.SetRange(0, 20);
	spin.Detach();

	EnableControls();

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		m_windowSettings.strTitle			= m_strWindowTitle;
		if (!m_bTrimTabTitles) m_windowSettings.dwTrimTabTitles = 0;
		m_windowSettings.strIcon			= m_strWindowIcon;

		if (m_bUsePosition)
		{
			m_positionSettings.nX = m_nX;
			m_positionSettings.nY = m_nY;

			if (m_positionSettings.nX == -1) m_positionSettings.nX = 0;
			if (m_positionSettings.nY == -1) m_positionSettings.nY = 0;

		}
		else
		{
			m_positionSettings.nX = -1;
			m_positionSettings.nY = -1;
		}

		if (m_bUseSize)
		{
			m_positionSettings.nW = m_nW;
			m_positionSettings.nH = m_nH;

			if (m_positionSettings.nW == -1) m_positionSettings.nW = 0;
			if (m_positionSettings.nH == -1) m_positionSettings.nH = 0;

		}
		else
		{
			m_positionSettings.nW = -1;
			m_positionSettings.nH = -1;
		}

		if (!m_bSnapToEdges)
		{
			m_positionSettings.nSnapDistance = -1;
		}

		m_positionSettings.dockPosition	= static_cast<DockPosition>(m_comboDocking.GetCurSel() - 1);
		m_positionSettings.zOrder		= static_cast<ZOrder>(m_comboZOrder.GetCurSel());

		WindowSettings&		windowSettings	= g_settingsHandler->GetAppearanceSettings().windowSettings;
		PositionSettings&	positionSettings= g_settingsHandler->GetAppearanceSettings().positionSettings;

		windowSettings	= m_windowSettings;
		positionSettings= m_positionSettings;

		m_windowSettings.Save(m_pOptionsRoot);
		m_positionSettings.Save(m_pOptionsRoot);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnClickedBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);

	CFileDialog fileDialog(
					TRUE, 
					NULL, 
					NULL, 
					OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST, 
					L"Icon Files (*.ico)\0*.ico\0\0");

	if (fileDialog.DoModal() == IDOK)
	{
		m_strWindowIcon = fileDialog.m_szFileName;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnClickedCheckbox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsAppearance::EnableControls()
{
	GetDlgItem(IDC_WINDOW_TITLE).EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_SHOW_COMMAND).EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_SHOW_COMMAND_TABS).EnableWindow(FALSE);
	GetDlgItem(IDC_TRIM_TAB_TITLES).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_TRIM_TAB_TITLES).EnableWindow(FALSE);
	GetDlgItem(IDC_TRIM_TAB_TITLES_RIGHT).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_TRIM_TAB_TITLES_RIGHT).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_TRIM_CHARS).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_TRIM_CHARS_RIGHT).EnableWindow(FALSE);
	GetDlgItem(IDC_WINDOW_ICON).EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_BROWSE_ICON).EnableWindow(FALSE);
	GetDlgItem(IDC_POS_X).EnableWindow(FALSE);
	GetDlgItem(IDC_POS_Y).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_X).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_Y).EnableWindow(FALSE);
	GetDlgItem(IDC_POS_W).EnableWindow(FALSE);
	GetDlgItem(IDC_POS_H).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_W).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_H).EnableWindow(FALSE);
	GetDlgItem(IDC_SNAP).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_SNAP).EnableWindow(FALSE);

	if (!m_windowSettings.bUseTabTitles)
	{
		GetDlgItem(IDC_WINDOW_TITLE).EnableWindow();
	}

	if (!m_windowSettings.bUseConsoleTitle)
	{
		GetDlgItem(IDC_CHECK_SHOW_COMMAND_TABS).EnableWindow();
	}

	if (!m_windowSettings.bUseTabTitles && !m_windowSettings.bUseConsoleTitle)
	{
		GetDlgItem(IDC_CHECK_SHOW_COMMAND).EnableWindow();
	}

	if (m_bTrimTabTitles)
	{
		GetDlgItem(IDC_TRIM_TAB_TITLES).EnableWindow();
		GetDlgItem(IDC_SPIN_TRIM_TAB_TITLES).EnableWindow();
		GetDlgItem(IDC_TRIM_TAB_TITLES_RIGHT).EnableWindow();
		GetDlgItem(IDC_SPIN_TRIM_TAB_TITLES_RIGHT).EnableWindow();
		GetDlgItem(IDC_STATIC_TRIM_CHARS).EnableWindow();
		GetDlgItem(IDC_STATIC_TRIM_CHARS_RIGHT).EnableWindow();
	}
	
	if (!m_windowSettings.bUseTabIcon)
	{
		GetDlgItem(IDC_WINDOW_ICON).EnableWindow();
		GetDlgItem(IDC_BTN_BROWSE_ICON).EnableWindow();
	}

	if (m_bUsePosition)
	{
		GetDlgItem(IDC_POS_X).EnableWindow();
		GetDlgItem(IDC_POS_Y).EnableWindow();
		GetDlgItem(IDC_SPIN_X).EnableWindow();
		GetDlgItem(IDC_SPIN_Y).EnableWindow();
	}

	if (m_bUseSize)
	{
		GetDlgItem(IDC_POS_W).EnableWindow();
		GetDlgItem(IDC_POS_H).EnableWindow();
		GetDlgItem(IDC_SPIN_W).EnableWindow();
		GetDlgItem(IDC_SPIN_H).EnableWindow();
	}

	if (m_bSnapToEdges)
	{
		GetDlgItem(IDC_SNAP).EnableWindow();
		GetDlgItem(IDC_SPIN_SNAP).EnableWindow();
	}
}

//////////////////////////////////////////////////////////////////////////////

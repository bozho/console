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

	m_comboFontSmoothing.Attach(GetDlgItem(IDC_COMBO_SMOOTHING));
	m_comboDocking.Attach(GetDlgItem(IDC_COMBO_DOCKING));
	m_comboZOrder.Attach(GetDlgItem(IDC_COMBO_ZORDER));

	m_windowSettings.Load(m_pOptionsRoot);
	m_fontSettings.Load(m_pOptionsRoot);
	m_positionSettings.Load(m_pOptionsRoot);

	m_strWindowTitle	= m_windowSettings.strTitle.c_str();
	m_bTrimTabTitles	= (m_windowSettings.dwTrimTabTitles > 0);
	m_strWindowIcon		= m_windowSettings.strIcon.c_str();

	m_strFontName	= m_fontSettings.strName.c_str();

	m_comboFontSmoothing.SetCurSel(static_cast<int>(m_fontSettings.fontSmoothing));

	m_bUsePosition	= ((m_positionSettings.nX == -1) && (m_positionSettings.nY == -1)) ? 0 : 1;
	m_nX			= ((m_positionSettings.nX == -1) && (m_positionSettings.nY == -1)) ? 0 : m_positionSettings.nX;
	m_nY			= ((m_positionSettings.nX == -1) && (m_positionSettings.nY == -1)) ? 0 : m_positionSettings.nY;

	m_bSnapToEdges	= (m_positionSettings.nSnapDistance != -1);
	if (!m_bSnapToEdges) m_positionSettings.nSnapDistance = 0;

	m_comboDocking.SetCurSel(static_cast<int>(m_positionSettings.dockPosition) + 1);
	m_comboZOrder.SetCurSel(static_cast<int>(m_positionSettings.zOrder));

	CUpDownCtrl	spin;
	UDACCEL udAccel;

	spin.Attach(GetDlgItem(IDC_SPIN_TRIM_TAB_TITLES));
	spin.SetRange(1, 100);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_FONT_SIZE));
	spin.SetRange(5, 36);
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

	spin.Attach(GetDlgItem(IDC_SPIN_SNAP));
	spin.SetRange(0, 20);
	spin.Detach();

	EnableControls();

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CWindow		staticCtl(reinterpret_cast<HWND>(lParam));
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));

	if (staticCtl.m_hWnd == GetDlgItem(IDC_FONT_COLOR))
	{
		CBrush	brush(::CreateSolidBrush(m_fontSettings.crFontColor));
		CRect	rect;

		staticCtl.GetClientRect(&rect);
		dc.FillRect(&rect, brush);
		return 0;
	}

	bHandled = FALSE;
	return 0;
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

		if (m_fontSettings.dwSize > 36) m_fontSettings.dwSize = 36;

		m_fontSettings.strName			= m_strFontName;

		m_fontSettings.fontSmoothing	= static_cast<FontSmoothing>(m_comboFontSmoothing.GetCurSel());

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

		if (!m_bSnapToEdges)
		{
			m_positionSettings.nSnapDistance = -1;
		}

		m_positionSettings.dockPosition	= static_cast<DockPosition>(m_comboDocking.GetCurSel() - 1);
		m_positionSettings.zOrder		= static_cast<ZOrder>(m_comboZOrder.GetCurSel());

		WindowSettings&		windowSettings	= g_settingsHandler->GetAppearanceSettings().windowSettings;
		FontSettings&		fontSettings	= g_settingsHandler->GetAppearanceSettings().fontSettings;
		PositionSettings&	positionSettings= g_settingsHandler->GetAppearanceSettings().positionSettings;

		windowSettings	= m_windowSettings;
		fontSettings	= m_fontSettings;
		positionSettings= m_positionSettings;

		m_windowSettings.Save(m_pOptionsRoot);
		m_fontSettings.Save(m_pOptionsRoot);
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

LRESULT DlgSettingsAppearance::OnClickedBtnBrowseFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);

	LOGFONT	lf;
	::ZeroMemory(&lf, sizeof(LOGFONT));

	wcsncpy_s(lf.lfFaceName, _countof(lf.lfFaceName), LPCTSTR(m_strFontName), 32);
	lf.lfHeight	= -MulDiv(m_fontSettings.dwSize, GetDeviceCaps(::GetDC(NULL), LOGPIXELSY), 72);
	lf.lfWeight	= m_fontSettings.bBold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic	= m_fontSettings.bItalic ? 1 : 0;

	CFontDialog	fontDlg(&lf, CF_FIXEDPITCHONLY|CF_SCREENFONTS);


	if (fontDlg.DoModal() == IDOK)
	{
		m_strFontName							= fontDlg.GetFaceName();
		m_fontSettings.dwSize= static_cast<DWORD>(static_cast<double>(-fontDlg.m_lf.lfHeight*72)/static_cast<double>(GetDeviceCaps(::GetDC(NULL), LOGPIXELSY)) + 0.5);
		m_fontSettings.bBold					= fontDlg.IsBold() ? true : false;
		m_fontSettings.bItalic					= fontDlg.IsItalic() ? true : false;

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

LRESULT DlgSettingsAppearance::OnClickedFontColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CColorDialog	dlg(m_fontSettings.crFontColor, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		m_fontSettings.crFontColor = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();
	}

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
	GetDlgItem(IDC_FONT_COLOR).EnableWindow(FALSE);
	GetDlgItem(IDC_POS_X).EnableWindow(FALSE);
	GetDlgItem(IDC_POS_Y).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_X).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_Y).EnableWindow(FALSE);
	GetDlgItem(IDC_SNAP).EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_SNAP).EnableWindow(FALSE);

	if (!m_windowSettings.bUseTabTitles) GetDlgItem(IDC_WINDOW_TITLE).EnableWindow();

	if (!m_windowSettings.bUseConsoleTitle)
	{
		GetDlgItem(IDC_CHECK_SHOW_COMMAND).EnableWindow();
		GetDlgItem(IDC_CHECK_SHOW_COMMAND_TABS).EnableWindow();
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

	if (m_fontSettings.bUseColor)
	{
		GetDlgItem(IDC_FONT_COLOR).EnableWindow();
	}

	if (m_bUsePosition)
	{
		GetDlgItem(IDC_POS_X).EnableWindow();
		GetDlgItem(IDC_POS_Y).EnableWindow();
		GetDlgItem(IDC_SPIN_X).EnableWindow();
		GetDlgItem(IDC_SPIN_Y).EnableWindow();
	}

	if (m_bSnapToEdges)
	{
		GetDlgItem(IDC_SNAP).EnableWindow();
		GetDlgItem(IDC_SPIN_SNAP).EnableWindow();
	}
}

//////////////////////////////////////////////////////////////////////////////

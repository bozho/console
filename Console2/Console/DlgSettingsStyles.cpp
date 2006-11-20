#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsStyles.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsStyles::DlgSettingsStyles(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
{
	IDD = IDD_SETTINGS_STYLES;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsStyles::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_controlsSettings.Load(m_pOptionsRoot);
	m_stylesSettings.Load(m_pOptionsRoot);
	m_transparencySettings.Load(m_pOptionsRoot);

	m_nShowMenu		= m_controlsSettings.bShowMenu ? 1 : 0;
	m_nShowToolbar	= m_controlsSettings.bShowToolbar ? 1 : 0;
	m_nShowStatusbar= m_controlsSettings.bShowStatusbar ? 1 : 0;
	m_nShowTabs		= m_controlsSettings.bShowTabs ? 1 : 0;
	m_nHideSingleTab= m_controlsSettings.bHideSingleTab ? 1 : 0;

	m_nShowCaption	= m_stylesSettings.bCaption ? 1 : 0;
	m_nResizable	= m_stylesSettings.bResizable ? 1 : 0;
	m_nTaskbarButton= m_stylesSettings.bTaskbarButton ? 1 : 0;
	m_nBorder		= m_stylesSettings.bBorder ? 1 : 0;
	m_nTrayIcon		= m_stylesSettings.bTrayIcon ? 1 : 0;
	
	CUpDownCtrl	spin;

	spin.Attach(GetDlgItem(IDC_SPIN_INSIDE_BORDER));
	spin.SetRange(0, 10);
	spin.Detach();

	m_sliderActiveAlpha.Attach(GetDlgItem(IDC_ACTIVE_ALPHA));
	m_sliderActiveAlpha.SetRange(0, 255);
	m_sliderActiveAlpha.SetTicFreq(5);
	m_sliderActiveAlpha.SetPageSize(5);

	m_sliderInactiveAlpha.Attach(GetDlgItem(IDC_INACTIVE_ALPHA));
	m_sliderInactiveAlpha.SetRange(0, 255);
	m_sliderInactiveAlpha.SetTicFreq(5);
	m_sliderInactiveAlpha.SetPageSize(5);

	m_sliderActiveAlpha.SetPos(255 - m_transparencySettings.byActiveAlpha);
	m_sliderInactiveAlpha.SetPos(255 - m_transparencySettings.byInactiveAlpha);

	UpdateSliderText(m_sliderActiveAlpha.m_hWnd);
	UpdateSliderText(m_sliderInactiveAlpha.m_hWnd);

	EnableTabControls();
	EnableTransparencyControls();

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsStyles::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CWindow		staticCtl(reinterpret_cast<HWND>(lParam));
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));

	if (staticCtl.m_hWnd == GetDlgItem(IDC_KEY_COLOR))
	{
		CBrush	brush(::CreateSolidBrush(m_transparencySettings.crColorKey));
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

LRESULT DlgSettingsStyles::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	UpdateSliderText(reinterpret_cast<HWND>(lParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsStyles::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		if (m_stylesSettings.dwInsideBorder > 10) m_stylesSettings.dwInsideBorder = 10;

		m_controlsSettings.bShowMenu		= (m_nShowMenu > 0);
		m_controlsSettings.bShowToolbar		= (m_nShowToolbar > 0);
		m_controlsSettings.bShowStatusbar	= (m_nShowStatusbar > 0);
		m_controlsSettings.bShowTabs		= (m_nShowTabs > 0);
		m_controlsSettings.bHideSingleTab	= (m_nHideSingleTab > 0);

		m_stylesSettings.bCaption		= (m_nShowCaption > 0);
		m_stylesSettings.bResizable		= (m_nResizable > 0);
		m_stylesSettings.bTaskbarButton	= (m_nTaskbarButton > 0);
		m_stylesSettings.bBorder		= (m_nBorder > 0);
		m_stylesSettings.bTrayIcon		= (m_nTrayIcon > 0);

		m_transparencySettings.byActiveAlpha	= static_cast<BYTE>(255 - m_sliderActiveAlpha.GetPos());
		m_transparencySettings.byInactiveAlpha	= static_cast<BYTE>(255 - m_sliderInactiveAlpha.GetPos());

		ControlsSettings&			controlsSettings	= g_settingsHandler->GetAppearanceSettings().controlsSettings;
		StylesSettings&				stylesSettings		= g_settingsHandler->GetAppearanceSettings().stylesSettings;
		TransparencySettings&		transparencySettings= g_settingsHandler->GetAppearanceSettings().transparencySettings;

		controlsSettings	= m_controlsSettings;
		stylesSettings		= m_stylesSettings;
		transparencySettings= m_transparencySettings;

		m_controlsSettings.Save(m_pOptionsRoot);
		m_stylesSettings.Save(m_pOptionsRoot);
		m_transparencySettings.Save(m_pOptionsRoot);
	}

	DestroyWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsStyles::OnClickedShowTabs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableTabControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsStyles::OnClickedKeyColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CColorDialog	dlg(m_transparencySettings.crColorKey, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		m_transparencySettings.crColorKey = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsStyles::OnClickedTransType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableTransparencyControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsStyles::UpdateSliderText(HWND hwndSlider)
{
	CTrackBarCtrl	trackBar;
	CWindow			wndStaticCtrl;

	if (hwndSlider == m_sliderActiveAlpha.m_hWnd)
	{
		trackBar.Attach(hwndSlider);
		wndStaticCtrl.Attach(GetDlgItem(IDC_STATIC_ACTIVE_ALPHA));
	}
	else if (hwndSlider == m_sliderInactiveAlpha.m_hWnd)
	{
		trackBar.Attach(hwndSlider);
		wndStaticCtrl.Attach(GetDlgItem(IDC_STATIC_INACTIVE_ALPHA));
	}
	else
	{
		return;
	}

	CString strStaticText;
	strStaticText.Format(L"%i", trackBar.GetPos());

	wndStaticCtrl.SetWindowText(strStaticText);

	wndStaticCtrl.Detach();
	trackBar.Detach();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsStyles::EnableTabControls()
{
	GetDlgItem(IDC_CHECK_HIDE_SINGLE_TAB).EnableWindow(FALSE);

	if (m_nShowTabs > 0)
	{
		GetDlgItem(IDC_CHECK_HIDE_SINGLE_TAB).EnableWindow();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsStyles::EnableTransparencyControls()
{
	GetDlgItem(IDC_STATIC_ACTIVE_WINDOW).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_INACTIVE_WINDOW).EnableWindow(FALSE);
	GetDlgItem(IDC_ACTIVE_ALPHA).EnableWindow(FALSE);
	GetDlgItem(IDC_INACTIVE_ALPHA).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_ACTIVE_ALPHA).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_INACTIVE_ALPHA).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_KEY_COLOR).EnableWindow(FALSE);
	GetDlgItem(IDC_KEY_COLOR).EnableWindow(FALSE);

	if (m_transparencySettings.transType == transAlpha)
	{
		GetDlgItem(IDC_STATIC_ACTIVE_WINDOW).EnableWindow();
		GetDlgItem(IDC_STATIC_INACTIVE_WINDOW).EnableWindow();
		GetDlgItem(IDC_ACTIVE_ALPHA).EnableWindow();
		GetDlgItem(IDC_INACTIVE_ALPHA).EnableWindow();
		GetDlgItem(IDC_STATIC_ACTIVE_ALPHA).EnableWindow();
		GetDlgItem(IDC_STATIC_INACTIVE_ALPHA).EnableWindow();

	}
	else if (m_transparencySettings.transType == transColorKey)
	{
		GetDlgItem(IDC_STATIC_KEY_COLOR).EnableWindow();
		GetDlgItem(IDC_KEY_COLOR).EnableWindow();
	}
}

//////////////////////////////////////////////////////////////////////////////

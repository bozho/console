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

LRESULT DlgSettingsAppearance::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	m_appearanceSettings.Load(m_pOptionsRoot);

	m_strFontName	= m_appearanceSettings.fontSettings.strName.c_str();
	m_nFontBold		= m_appearanceSettings.fontSettings.bBold ? 1 : 0;
	m_nFontItalic	= m_appearanceSettings.fontSettings.bItalic ? 1 : 0;

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_SPIN_FONT_SIZE));
	spin.SetRange(5, 42);
	spin.Detach();

	m_sliderActiveAlpha.Attach(GetDlgItem(IDC_ACTIVE_ALPHA));
	m_sliderActiveAlpha.SetRange(0, 255);
	m_sliderActiveAlpha.SetTicFreq(5);
	m_sliderActiveAlpha.SetPageSize(5);

	m_sliderInactiveAlpha.Attach(GetDlgItem(IDC_INACTIVE_ALPHA));
	m_sliderInactiveAlpha.SetRange(0, 255);
	m_sliderInactiveAlpha.SetTicFreq(5);
	m_sliderInactiveAlpha.SetPageSize(5);

	m_sliderActiveAlpha.SetPos(static_cast<int>(255 - m_appearanceSettings.transparencySettings.byActiveAlpha));
	m_sliderInactiveAlpha.SetPos(static_cast<int>(255 - m_appearanceSettings.transparencySettings.byInactiveAlpha));

	UpdateSliderText(m_sliderActiveAlpha.m_hWnd);
	UpdateSliderText(m_sliderInactiveAlpha.m_hWnd);

	EnableTransparencyControls();

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {


	CWindow		staticCtl(reinterpret_cast<HWND>(lParam));
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));

	if (staticCtl.m_hWnd == GetDlgItem(IDC_KEY_COLOR)) {

		CBrush	brush(::CreateSolidBrush(m_appearanceSettings.transparencySettings.crColorKey));
		RECT	rect;

		staticCtl.GetClientRect(&rect);
		dc.FillRect(&rect, brush);
		return 0;
	}

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {

	UpdateSliderText(reinterpret_cast<HWND>(lParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	if (wID == IDOK) {
		DoDataExchange(DDX_SAVE);

		m_appearanceSettings.fontSettings.strName	= m_strFontName;
		m_appearanceSettings.fontSettings.bBold		= (m_nFontBold > 0);
		m_appearanceSettings.fontSettings.bItalic	= (m_nFontItalic > 0);

		m_appearanceSettings.transparencySettings.byActiveAlpha		= static_cast<BYTE>(255 - m_sliderActiveAlpha.GetPos());
		m_appearanceSettings.transparencySettings.byInactiveAlpha	= static_cast<BYTE>(255 - m_sliderInactiveAlpha.GetPos());

		AppearanceSettings&		appearanceSettings	= g_settingsHandler->GetAppearanceSettings();

		appearanceSettings.fontSettings.strName	= m_appearanceSettings.fontSettings.strName;
		appearanceSettings.fontSettings.dwSize	= m_appearanceSettings.fontSettings.dwSize;
		appearanceSettings.fontSettings.bBold	= m_appearanceSettings.fontSettings.bBold;
		appearanceSettings.fontSettings.bItalic	= m_appearanceSettings.fontSettings.bItalic;

		appearanceSettings.transparencySettings.transType		= m_appearanceSettings.transparencySettings.transType;
		appearanceSettings.transparencySettings.byActiveAlpha	= m_appearanceSettings.transparencySettings.byActiveAlpha;
		appearanceSettings.transparencySettings.byInactiveAlpha	= m_appearanceSettings.transparencySettings.byInactiveAlpha;
		appearanceSettings.transparencySettings.crColorKey		= m_appearanceSettings.transparencySettings.crColorKey;

		m_appearanceSettings.Save(m_pOptionsRoot);
	}

	DestroyWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnClickedBtnBrowseFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	DoDataExchange(DDX_SAVE);

	LOGFONT	lf;
	::ZeroMemory(&lf, sizeof(LOGFONT));

	wcsncpy(lf.lfFaceName, LPCTSTR(m_strFontName), 32);
	lf.lfHeight	= -MulDiv(m_appearanceSettings.fontSettings.dwSize, GetDeviceCaps(::GetDC(NULL), LOGPIXELSY), 72);
	lf.lfWeight	= (m_nFontBold > 0) ? FW_BOLD : FW_NORMAL;
	lf.lfItalic	= static_cast<BYTE>(m_nFontItalic);

	CFontDialog	fontDlg(&lf);


	if (fontDlg.DoModal() == IDOK) {
		m_strFontName							= fontDlg.GetFaceName();// fontDlg.m_lf.lfFaceName;
		m_appearanceSettings.fontSettings.dwSize= static_cast<DWORD>(static_cast<double>(-fontDlg.m_lf.lfHeight*72)/static_cast<double>(GetDeviceCaps(::GetDC(NULL), LOGPIXELSY)) + 0.5);
		m_nFontBold								= fontDlg.IsBold() ? 1 : 0; //(fontDlg.m_lf.lfWeight == FW_BOLD) ? 1 : 0;
		m_nFontItalic							= fontDlg.IsItalic() ? 1 : 0; // fontDlg.m_lf.lfItalic;

		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnClickedKeyColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {

	CColorDialog	dlg(m_appearanceSettings.transparencySettings.crColorKey, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK) {
		// update color
		m_appearanceSettings.transparencySettings.crColorKey = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnClickedTransType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	DoDataExchange(DDX_SAVE);
	EnableTransparencyControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsAppearance::UpdateSliderText(HWND hwndSlider) {

	CTrackBarCtrl	trackBar;
	CWindow			wndStaticCtrl;

	if (hwndSlider == m_sliderActiveAlpha.m_hWnd) {
		trackBar.Attach(hwndSlider);
		wndStaticCtrl.Attach(GetDlgItem(IDC_STATIC_ACTIVE_ALPHA));
	} else if (hwndSlider == m_sliderInactiveAlpha.m_hWnd) {
		trackBar.Attach(hwndSlider);
		wndStaticCtrl.Attach(GetDlgItem(IDC_STATIC_INACTIVE_ALPHA));
	} else {
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

void DlgSettingsAppearance::EnableTransparencyControls() {

	::EnableWindow(GetDlgItem(IDC_STATIC_ACTIVE_WINDOW), FALSE);
	::EnableWindow(GetDlgItem(IDC_STATIC_INACTIVE_WINDOW), FALSE);
	::EnableWindow(GetDlgItem(IDC_ACTIVE_ALPHA), FALSE);
	::EnableWindow(GetDlgItem(IDC_INACTIVE_ALPHA), FALSE);
	::EnableWindow(GetDlgItem(IDC_STATIC_ACTIVE_ALPHA), FALSE);
	::EnableWindow(GetDlgItem(IDC_STATIC_INACTIVE_ALPHA), FALSE);
	::EnableWindow(GetDlgItem(IDC_STATIC_KEY_COLOR), FALSE);
	::EnableWindow(GetDlgItem(IDC_KEY_COLOR), FALSE);

	if (m_appearanceSettings.transparencySettings.transType == transAlpha) {

		::EnableWindow(GetDlgItem(IDC_STATIC_ACTIVE_WINDOW), TRUE);
		::EnableWindow(GetDlgItem(IDC_STATIC_INACTIVE_WINDOW), TRUE);
		::EnableWindow(GetDlgItem(IDC_ACTIVE_ALPHA), TRUE);
		::EnableWindow(GetDlgItem(IDC_INACTIVE_ALPHA), TRUE);
		::EnableWindow(GetDlgItem(IDC_STATIC_ACTIVE_ALPHA), TRUE);
		::EnableWindow(GetDlgItem(IDC_STATIC_INACTIVE_ALPHA), TRUE);

	} else if (m_appearanceSettings.transparencySettings.transType == transColorKey) {

		::EnableWindow(GetDlgItem(IDC_STATIC_KEY_COLOR), TRUE);
		::EnableWindow(GetDlgItem(IDC_KEY_COLOR), TRUE);
	}
}

//////////////////////////////////////////////////////////////////////////////

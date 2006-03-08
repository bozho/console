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

	m_fontSettings.Load(m_pOptionsRoot);
	m_windowSettings.Load(m_pOptionsRoot);

	m_strFontName	= m_fontSettings.strName.c_str();
	m_nFontBold		= m_fontSettings.bBold ? 1 : 0;
	m_nFontItalic	= m_fontSettings.bItalic ? 1 : 0;
	m_nUseFontColor	= m_fontSettings.bUseColor ? 1 : 0;

	m_nShowMenu		= m_windowSettings.bShowMenu ? 1 : 0;
	m_nShowToolbar	= m_windowSettings.bShowToolbar ? 1 : 0;
	m_nShowTabs		= m_windowSettings.bShowTabs ? 1 : 0;
	m_nShowStatusbar= m_windowSettings.bShowStatusbar ? 1 : 0;

	m_nShowCaption	= m_windowSettings.bCaption ? 1 : 0;
	m_nResizable	= m_windowSettings.bResizable ? 1 : 0;
	m_nTaskbarButton= m_windowSettings.bTaskbarButton ? 1 : 0;

	EnableControls();

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {


	CWindow		staticCtl(reinterpret_cast<HWND>(lParam));
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));

	if (staticCtl.m_hWnd == GetDlgItem(IDC_FONT_COLOR)) {

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

LRESULT DlgSettingsAppearance::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	if (wID == IDOK) {
		DoDataExchange(DDX_SAVE);

		m_fontSettings.strName			= m_strFontName;
		m_fontSettings.bBold			= (m_nFontBold > 0);
		m_fontSettings.bItalic			= (m_nFontItalic > 0);
		m_fontSettings.bUseColor		= (m_nUseFontColor > 0);

		m_windowSettings.bShowMenu		= (m_nShowMenu > 0);
		m_windowSettings.bShowToolbar	= (m_nShowToolbar > 0);
		m_windowSettings.bShowTabs		= (m_nShowTabs > 0);
		m_windowSettings.bShowStatusbar	= (m_nShowStatusbar > 0);

		m_windowSettings.bCaption		= (m_nShowCaption > 0);
		m_windowSettings.bResizable		= (m_nResizable > 0);
		m_windowSettings.bTaskbarButton	= (m_nTaskbarButton > 0);

		FontSettings&		fontSettings	= g_settingsHandler->GetAppearanceSettings().fontSettings;
		WindowSettings&		windowSettings	= g_settingsHandler->GetAppearanceSettings().windowSettings;

		fontSettings.strName	= m_fontSettings.strName;
		fontSettings.dwSize		= m_fontSettings.dwSize;
		fontSettings.bBold		= m_fontSettings.bBold;
		fontSettings.bItalic	= m_fontSettings.bItalic;
		fontSettings.bUseColor	= m_fontSettings.bUseColor;
		fontSettings.crFontColor= m_fontSettings.crFontColor;

		windowSettings.bShowMenu		= m_windowSettings.bShowMenu;
		windowSettings.bShowToolbar		= m_windowSettings.bShowToolbar;
		windowSettings.bShowTabs		= m_windowSettings.bShowTabs;
		windowSettings.bShowStatusbar	= m_windowSettings.bShowStatusbar;

		windowSettings.bCaption			= m_windowSettings.bCaption;
		windowSettings.bResizable		= m_windowSettings.bResizable;
		windowSettings.bTaskbarButton	= m_windowSettings.bTaskbarButton;

		m_fontSettings.Save(m_pOptionsRoot);
		m_windowSettings.Save(m_pOptionsRoot);
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
	lf.lfHeight	= -MulDiv(m_fontSettings.dwSize, GetDeviceCaps(::GetDC(NULL), LOGPIXELSY), 72);
	lf.lfWeight	= (m_nFontBold > 0) ? FW_BOLD : FW_NORMAL;
	lf.lfItalic	= static_cast<BYTE>(m_nFontItalic);

	CFontDialog	fontDlg(&lf);


	if (fontDlg.DoModal() == IDOK) {
		m_strFontName							= fontDlg.GetFaceName();// fontDlg.m_lf.lfFaceName;
		m_fontSettings.dwSize= static_cast<DWORD>(static_cast<double>(-fontDlg.m_lf.lfHeight*72)/static_cast<double>(GetDeviceCaps(::GetDC(NULL), LOGPIXELSY)) + 0.5);
		m_nFontBold								= fontDlg.IsBold() ? 1 : 0; //(fontDlg.m_lf.lfWeight == FW_BOLD) ? 1 : 0;
		m_nFontItalic							= fontDlg.IsItalic() ? 1 : 0; // fontDlg.m_lf.lfItalic;

		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnClickedUseColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	DoDataExchange(DDX_SAVE);
	EnableControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsAppearance::OnClickedFontColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {

	CColorDialog	dlg(m_fontSettings.crFontColor, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK) {
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

void DlgSettingsAppearance::EnableControls() {

	::EnableWindow(GetDlgItem(IDC_FONT_COLOR), FALSE);

	if (m_nUseFontColor > 0) {

		::EnableWindow(GetDlgItem(IDC_FONT_COLOR), TRUE);
	}
}

//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsFont.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsFont::DlgSettingsFont(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
{
	IDD = IDD_SETTINGS_FONT;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsFont::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_comboFontSmoothing.Attach(GetDlgItem(IDC_COMBO_SMOOTHING));
	Helpers::LoadCombo(m_comboFontSmoothing, IDC_COMBO_SMOOTHING);

	m_fontSettings.Load(m_pOptionsRoot);

	m_strFontName	= m_fontSettings.strName.c_str();

	m_comboFontSmoothing.SetCurSel(static_cast<int>(m_fontSettings.fontSmoothing));

	CUpDownCtrl	spin;

	spin.Attach(GetDlgItem(IDC_SPIN_FONT_SIZE));
	spin.SetRange(5, 36);
	spin.Detach();

	spin.Attach(GetDlgItem(IDC_SPIN_FONT_EXTRAWIDTH));
	spin.SetRange(0, 30);
	spin.Detach();

	EnableControls();

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsFont::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT DlgSettingsFont::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		if (m_fontSettings.dwSize > 36) m_fontSettings.dwSize = 36;
		if (m_fontSettings.dwExtraWidth > 30) m_fontSettings.dwExtraWidth = 30;

		m_fontSettings.strName			= m_strFontName;

		m_fontSettings.fontSmoothing	= static_cast<FontSmoothing>(m_comboFontSmoothing.GetCurSel());

		FontSettings&		fontSettings	= g_settingsHandler->GetAppearanceSettings().fontSettings;
		fontSettings	= m_fontSettings;

		m_fontSettings.Save(m_pOptionsRoot);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsFont::OnClickedBtnBrowseFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT DlgSettingsFont::OnClickedCheckbox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsFont::OnClickedFontColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
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

void DlgSettingsFont::EnableControls()
{
	GetDlgItem(IDC_FONT_COLOR).EnableWindow(FALSE);

	if (m_fontSettings.bUseColor)
	{
		GetDlgItem(IDC_FONT_COLOR).EnableWindow();
	}
}

//////////////////////////////////////////////////////////////////////////////

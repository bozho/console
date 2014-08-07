#include "stdafx.h"
#include "resource.h"

#include "PageSettingsTabs2.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

PageSettingsTabs2::PageSettingsTabs2(ConsoleSettings &consoleSettings)
: m_tabData()
, m_consoleSettings(consoleSettings)
, m_strBkImage(L"")
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (CTheme().IsThemingSupported()) ::EnableThemeDialogTexture(m_hWnd, ETDT_USETABTEXTURE);

	m_comboBkPosition.Attach(GetDlgItem(IDC_COMBO_BK_POS));
	m_comboBkPosition.AddString(L"Center");
	m_comboBkPosition.AddString(L"Stretch");
	m_comboBkPosition.AddString(L"Tile");
	m_comboBkPosition.AddString(L"Fit");
	m_comboBkPosition.AddString(L"Fill");

	m_staticTintOpacity.Attach(GetDlgItem(IDC_TINT_OPACITY_VAL));
	m_sliderTintOpacity.Attach(GetDlgItem(IDC_TINT_OPACITY));
	m_sliderTintOpacity.SetRange(0, 255);
	m_sliderTintOpacity.SetTicFreq(5);
	m_sliderTintOpacity.SetPageSize(5);

	m_staticBkColor.Attach(GetDlgItem(IDC_BK_COLOR));
	m_staticTintColor.Attach(GetDlgItem(IDC_TINT_COLOR));

	m_bkImageEdit.SubclassWindow(GetDlgItem(IDC_BK_IMAGE));

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs2::Load()
{
	m_tabData->SetBackground(m_consoleSettings.backgroundImageType, m_consoleSettings.crBackgroundColor, m_consoleSettings.imageData, false);

	m_strBkImage = m_tabData->imageData.strFilename.c_str();

	m_comboBkPosition.SetCurSel(static_cast<int>(m_tabData->imageData.imagePosition));

	m_sliderTintOpacity.SetPos(m_tabData->imageData.byTintOpacity);

	m_staticBkColor.Invalidate();
	m_staticTintColor.Invalidate();

	UpdateSliderText();

	DoDataExchange(DDX_LOAD);

	EnableControls();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND		hWnd = reinterpret_cast<HWND>(lParam); 
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));

	if (hWnd == m_staticBkColor.m_hWnd)
	{
		CBrush	brush(::CreateSolidBrush(m_tabData->crBackgroundColor));
		CRect	rect;

		m_staticBkColor.GetClientRect(&rect);
		dc.FillRect(&rect, brush);
		return 0;
	}
	else if (hWnd == m_staticTintColor.m_hWnd)
	{
		CBrush	brush(::CreateSolidBrush(m_tabData->imageData.crTint));
		CRect	rect;

		m_staticTintColor.GetClientRect(&rect);
		dc.FillRect(&rect, brush);
		return 0;
	}

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_tabData->imageData.byTintOpacity  = static_cast<BYTE>(m_sliderTintOpacity.GetPos());
	m_tabData->bInheritedBackground = false;
	DoDataExchange(DDX_LOAD);

	UpdateSliderText();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnClickedBkType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Save();
	m_tabData->bInheritedBackground = false;
	DoDataExchange(DDX_LOAD);

	EnableControls();

	return 1;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnClickedBkColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CColorDialog	dlg(m_tabData->crBackgroundColor, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		m_tabData->crBackgroundColor = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();

		m_tabData->bInheritedBackground = false;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnBtnBrowseImage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog fileDialog(
					TRUE, 
					NULL, 
					NULL, 
					OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST, 
					L"Image Files (*.jpg)\0*.jpg\0All Files (*.*)\0*.*\0\0");

	if (fileDialog.DoModal() == IDOK)
	{
		m_strBkImage = fileDialog.m_szFileName;
		m_tabData->bInheritedBackground = false;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnClickedTintColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CColorDialog	dlg(m_tabData->imageData.crTint, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		m_tabData->imageData.crTint = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();

		m_tabData->bInheritedBackground = false;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnCbnSelchangeComboBkPos(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tabData->imageData.imagePosition  = static_cast<ImagePosition>(m_comboBkPosition.GetCurSel());
	m_tabData->bInheritedBackground = false;
	DoDataExchange(DDX_LOAD);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnChangeBkImage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	auto strBkImage = m_strBkImage;
	Save();

	if( m_tabData->bInheritedBackground )
	{
		if( strBkImage != m_strBkImage )
		{
			m_tabData->bInheritedBackground = false;
			DoDataExchange(DDX_LOAD);
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnClickedBtnInheritBackground(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Save();

	if (m_tabData->bInheritedBackground)
	{
		Load();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs2::OnClickedBtnSetAsDefaultBackground(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Save();
	m_consoleSettings.backgroundImageType = m_tabData->backgroundImageType;
	m_consoleSettings.crBackgroundColor   = m_tabData->crBackgroundColor;
	m_consoleSettings.imageData           = m_tabData->imageData;

	m_tabData->bInheritedBackground       = true;
	DoDataExchange(DDX_LOAD);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs2::UpdateSliderText()
{
	CString strStaticText;
	strStaticText.Format(L"%i", m_sliderTintOpacity.GetPos());

	m_staticTintOpacity.SetWindowText(strStaticText);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs2::EnableControls()
{
	GetDlgItem(IDC_STATIC_BK_COLOR).EnableWindow(FALSE);
	GetDlgItem(IDC_BK_COLOR).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_BK_IMAGE).EnableWindow(FALSE);
	GetDlgItem(IDC_BK_IMAGE).EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_BROWSE_BK).EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_BK_RELATIVE).EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_BK_EXTEND).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_BK_POS).EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_BK_POS).EnableWindow(FALSE);

	GetDlgItem(IDC_STATIC_TINT_COLOR).EnableWindow(FALSE);
	GetDlgItem(IDC_TINT_COLOR).EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_TINT_OPACITY).EnableWindow(FALSE);
	GetDlgItem(IDC_TINT_OPACITY).EnableWindow(FALSE);
	GetDlgItem(IDC_TINT_OPACITY_VAL).EnableWindow(FALSE);

	if (m_tabData->backgroundImageType == bktypeNone)
	{
		GetDlgItem(IDC_STATIC_BK_COLOR).EnableWindow();
		GetDlgItem(IDC_BK_COLOR).EnableWindow();
	}
	else if (m_tabData->backgroundImageType == bktypeImage)
	{
		GetDlgItem(IDC_STATIC_BK_IMAGE).EnableWindow();
		GetDlgItem(IDC_BK_IMAGE).EnableWindow();
		GetDlgItem(IDC_BTN_BROWSE_BK).EnableWindow();
		GetDlgItem(IDC_CHECK_BK_RELATIVE).EnableWindow();
		GetDlgItem(IDC_CHECK_BK_EXTEND).EnableWindow();
		GetDlgItem(IDC_STATIC_BK_POS).EnableWindow();
		GetDlgItem(IDC_COMBO_BK_POS).EnableWindow();
	}

	if (m_tabData->backgroundImageType != bktypeNone)
	{
		GetDlgItem(IDC_STATIC_TINT_COLOR).EnableWindow();
		GetDlgItem(IDC_TINT_COLOR).EnableWindow();
		GetDlgItem(IDC_STATIC_TINT_OPACITY).EnableWindow();
		GetDlgItem(IDC_TINT_OPACITY).EnableWindow();
		GetDlgItem(IDC_TINT_OPACITY_VAL).EnableWindow();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs2::Load(std::shared_ptr<TabData>& tabData)
{
	m_tabData = tabData;

	Load();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs2::Save()
{
	DoDataExchange(DDX_SAVE);

	m_tabData->imageData.strFilename    = m_strBkImage;
	m_tabData->imageData.imagePosition  = static_cast<ImagePosition>(m_comboBkPosition.GetCurSel());
	m_tabData->imageData.byTintOpacity  = static_cast<BYTE>(m_sliderTintOpacity.GetPos());
}

//////////////////////////////////////////////////////////////////////////////

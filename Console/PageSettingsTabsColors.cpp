#include "stdafx.h"
#include "resource.h"

#include "PageSettingsTabsColors.h"
#include "XmlHelper.h"

extern std::shared_ptr<SettingsHandler>	g_settingsHandler;

PageSettingsTabsColors::PageSettingsTabsColors(ConsoleSettings &consoleSettings)
: m_tabData()
, m_consoleSettings(consoleSettings)
{
}

LRESULT PageSettingsTabsColors::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ExecuteDlgInit(IDD);

	if (CTheme().IsThemingSupported()) ::EnableThemeDialogTexture(m_hWnd, ETDT_USETABTEXTURE);

	m_staticCursorAnim.Attach(GetDlgItem(IDC_CURSOR_ANIM));
	m_comboCursor.Attach(GetDlgItem(IDC_COMBO_CURSOR));
	m_staticCursorColor.Attach(GetDlgItem(IDC_CURSOR_COLOR));

#ifdef _USE_AERO
	m_staticBGTextOpacity.Attach(GetDlgItem(IDC_BGTEXT_OPACITY_VAL));
	m_sliderBGTextOpacity.Attach(GetDlgItem(IDC_BGTEXT_OPACITY));
	m_sliderBGTextOpacity.SetRange(0, 255);
	m_sliderBGTextOpacity.SetTicFreq(5);
	m_sliderBGTextOpacity.SetPageSize(5);
	m_sliderBGTextOpacity.SetPos(m_consoleSettings.backgroundTextOpacity);
	UpdateSliderText();
#endif //_USE_AERO

	return TRUE;
}

LRESULT PageSettingsTabsColors::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CWindow		staticCtl(reinterpret_cast<HWND>(lParam));
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));
	DWORD		i;

	if (staticCtl.m_hWnd == m_staticCursorColor.m_hWnd)
	{
		CBrush	brush(::CreateSolidBrush(m_tabData->crCursorColor));
		CRect	rect;

		m_staticCursorColor.GetClientRect(&rect);
		dc.FillRect(&rect, brush);

		SetCursor();
		return 0;
	}

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
			CBrush	brush(::CreateSolidBrush(m_tabData->consoleColors[i-IDC_CLR_00]));
			CRect	rect;

			staticCtl.GetClientRect(&rect);
			dc.FillRect(&rect, brush);
			return 0;
		}
	}

	bHandled = FALSE;
	return 0;
}

LRESULT PageSettingsTabsColors::OnClickedClrBtn(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CColorDialog	dlg(m_tabData->consoleColors[wID-IDC_CLR_00], CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		m_tabData->consoleColors[wID-IDC_CLR_00] = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();

		m_tabData->bInheritedColors = false;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

LRESULT PageSettingsTabsColors::OnClickedBtnResetColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tabData->SetColors(m_consoleSettings.defaultConsoleColors, 255, true);
	m_tabData->bInheritedColors = false;

	Load();
	return 0;
}

LRESULT PageSettingsTabsColors::OnClickedBtnImportColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
    BYTE     opacity = 255;
    if(!XmlHelper::LoadColors(pConsoleElement, colors, opacity)) return 0;

    m_tabData->SetColors(colors, opacity, true);
    m_tabData->bInheritedColors = false;

    Load();
  }

  return 0;
}

LRESULT PageSettingsTabsColors::OnClickedBtnInheritColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);

	if (m_tabData->bInheritedColors)
	{
		Load();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabsColors::OnClickedBtnInheritCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);

	if (m_tabData->bInheritedCursor)
	{
		Load();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabsColors::OnClickedBtnSetAsDefaultColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::CopyMemory(m_consoleSettings.consoleColors, m_tabData->consoleColors, sizeof(m_consoleSettings.consoleColors));
	m_consoleSettings.backgroundTextOpacity = m_tabData->backgroundTextOpacity;

	m_tabData->bInheritedColors = true;
	DoDataExchange(DDX_LOAD);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabsColors::OnClickedBtnSetAsDefaultCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_consoleSettings.crCursorColor = m_tabData->crCursorColor;
	m_consoleSettings.dwCursorStyle = m_tabData->dwCursorStyle;

	m_tabData->bInheritedCursor = true;
	DoDataExchange(DDX_LOAD);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabsColors::EnableControls()
{
	m_staticCursorColor.ShowWindow(static_cast<CursorStyle>(m_comboCursor.GetCurSel()) != cstyleConsole? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_COLOR).EnableWindow(static_cast<CursorStyle>(m_comboCursor.GetCurSel()) != cstyleConsole);
}

void PageSettingsTabsColors::Load()
{
	m_tabData->SetColors(m_consoleSettings.consoleColors, m_consoleSettings.backgroundTextOpacity, false);

	for(int nID = IDC_CLR_00; nID <= IDC_CLR_15; ++nID)
		GetDlgItem(nID).Invalidate();

	m_tabData->SetCursor(m_consoleSettings.dwCursorStyle, m_consoleSettings.crCursorColor, false);
	m_comboCursor.SetCurSel(m_tabData->dwCursorStyle);
	m_staticCursorColor.Invalidate();
	SetCursor();

#ifdef _USE_AERO
	m_sliderBGTextOpacity.SetPos(m_tabData->backgroundTextOpacity);
	UpdateSliderText();
#endif

	DoDataExchange(DDX_LOAD);

	EnableControls();
}

void PageSettingsTabsColors::Load(shared_ptr<TabData>& tabData)
{
	m_tabData = tabData;

	Load();
}

void PageSettingsTabsColors::Save()
{
	DoDataExchange(DDX_SAVE);

#ifdef _USE_AERO
	m_tabData->backgroundTextOpacity = static_cast<BYTE>(m_sliderBGTextOpacity.GetPos());
#endif //_USE_AERO
	m_tabData->dwCursorStyle = m_comboCursor.GetCurSel();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabsColors::OnCbnSelchangeComboCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tabData->dwCursorStyle = m_comboCursor.GetCurSel();
	m_tabData->bInheritedCursor = false;
	DoDataExchange(DDX_LOAD);

	SetCursor();
	EnableControls();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabsColors::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if ((wParam == CURSOR_TIMER) && (m_cursor.get() != NULL))
  {
    DrawCursor();
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabsColors::OnClickedCursorColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CColorDialog	dlg(m_tabData->crCursorColor, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		m_tabData->crCursorColor = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();

		m_tabData->bInheritedCursor = false;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabsColors::SetCursor(void)
{
  CRect rectCursorAnim;
  m_staticCursorAnim.GetClientRect(&rectCursorAnim);
  CClientDC dc(m_staticCursorAnim.m_hWnd);
  CBrush brush(::CreateSolidBrush(RGB(0,0,0)));
  dc.FillRect(rectCursorAnim, brush);

  rectCursorAnim.right  -= 12;
  rectCursorAnim.right  /= 2;
  rectCursorAnim.bottom -= 8;

  m_cursor.reset();
  m_cursor = CursorFactory::CreateCursor(
    m_hWnd,
    true,
    static_cast<CursorStyle>(m_comboCursor.GetCurSel()),
    dc,
    rectCursorAnim,
    m_tabData->crCursorColor,
    this,
    true);

  DrawCursor();
}

void PageSettingsTabsColors::RedrawCharOnCursor(CDC& dc)
{
  CRect rectCursorAnim;
  m_staticCursorAnim.GetClientRect(&rectCursorAnim);

  rectCursorAnim.right  -= 12;
  rectCursorAnim.right  /= 2;
  rectCursorAnim.bottom -= 8;

  rectCursorAnim.OffsetRect(4, 4);

  DrawCursor(
    dc,
    rectCursorAnim,
    g_settingsHandler->GetConsoleSettings().consoleColors[0],
    m_tabData->crCursorColor);
}

void PageSettingsTabsColors::DrawCursor()
{
  m_staticCursorAnim.RedrawWindow();

  CClientDC dc(m_staticCursorAnim.m_hWnd);
  CBrush brush(::CreateSolidBrush(RGB(0,0,0)));

  m_cursor->PrepareNext();

  CRect rectCursorAnim;
  m_staticCursorAnim.GetClientRect(&rectCursorAnim);
  dc.FillRect(rectCursorAnim, brush);

  rectCursorAnim.right  -= 12;
  rectCursorAnim.right  /= 2;
  rectCursorAnim.bottom -= 8;

  rectCursorAnim.OffsetRect(4, 4);

  DrawCursor(
    dc,
    rectCursorAnim,
    g_settingsHandler->GetConsoleSettings().consoleColors[7],
    g_settingsHandler->GetConsoleSettings().consoleColors[0]);

  m_cursor->Draw(true, 40);
  m_cursor->BitBlt(dc, rectCursorAnim.left, rectCursorAnim.top);

  rectCursorAnim.OffsetRect(rectCursorAnim.Width() + 4, 0);

  DrawCursor(
    dc,
    rectCursorAnim,
    g_settingsHandler->GetConsoleSettings().consoleColors[7],
    g_settingsHandler->GetConsoleSettings().consoleColors[0]);

  m_cursor->Draw(false, 40);
  m_cursor->BitBlt(dc, rectCursorAnim.left, rectCursorAnim.top);
}

void PageSettingsTabsColors::DrawCursor(CDC& dc, const CRect& rectCursorAnim, COLORREF fg, COLORREF bg)
{
  CBrush brush(::CreateSolidBrush(bg));
  CPen pen(::CreatePen(PS_SOLID, 2, fg));

  dc.FillRect(rectCursorAnim, brush);

  CRect rectChar(rectCursorAnim);
  rectChar.DeflateRect(3, 3);
  dc.SelectPen(pen);
  dc.MoveTo(rectChar.left , rectChar.top   );
  dc.LineTo(rectChar.right, rectChar.bottom);
  dc.MoveTo(rectChar.right, rectChar.top   );
  dc.LineTo(rectChar.left , rectChar.bottom);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

#ifdef _USE_AERO

LRESULT PageSettingsTabsColors::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_tabData->backgroundTextOpacity = static_cast<BYTE>(m_sliderBGTextOpacity.GetPos());
	m_tabData->bInheritedColors = false;
	DoDataExchange(DDX_LOAD);

	UpdateSliderText();
	return 0;
}

void PageSettingsTabsColors::UpdateSliderText()
{
	CString strStaticText;
	strStaticText.Format(L"%i", m_sliderBGTextOpacity.GetPos());

	m_staticBGTextOpacity.SetWindowText(strStaticText);
}

#endif //_USE_AERO

#include "stdafx.h"
#include "resource.h"

#include "PageSettingsTabsColors.h"
#include "XmlHelper.h"

PageSettingsTabsColors::PageSettingsTabsColors(ConsoleSettings &consoleSettings)
: m_tabData()
, m_consoleSettings(consoleSettings)
{
}

LRESULT PageSettingsTabsColors::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ExecuteDlgInit(IDD);

	if (CTheme().IsThemingSupported()) ::EnableThemeDialogTexture(m_hWnd, ETDT_USETABTEXTURE);

	return TRUE;
}

LRESULT PageSettingsTabsColors::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	m_tabData->SetColors(m_consoleSettings.consoleColors, false);

	bHandled = FALSE;
	return 0;
}

LRESULT PageSettingsTabsColors::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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
	m_tabData->SetColors(m_consoleSettings.defaultConsoleColors, true);
	m_tabData->bInheritedColors = false;

	DoDataExchange(DDX_LOAD);
	Invalidate();
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
    if(!XmlHelper::LoadColors(pConsoleElement, colors)) return 0;

    m_tabData->SetColors(colors, true);
    m_tabData->bInheritedColors = false;

    DoDataExchange(DDX_LOAD);
    Invalidate();
  }

  return 0;
}

LRESULT PageSettingsTabsColors::OnClickedBtnInheritColors(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);

	if (m_tabData->bInheritedColors)
	{
		Invalidate();
	}

	return 0;
}

void PageSettingsTabsColors::Load(shared_ptr<TabData>& tabData)
{
	m_tabData = tabData;

	Invalidate();
	DoDataExchange(DDX_LOAD);
}

void PageSettingsTabsColors::Save()
{
	DoDataExchange(DDX_SAVE);
}

#include "stdafx.h"
#include "resource.h"

#include "Cursors.h"
#include "PageSettingsTabs1.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

PageSettingsTabs1::PageSettingsTabs1()
: m_tabData()
, m_strTitle(L"")
, m_strIcon(L"")
, m_nUseDefaultIcon(0)
, m_strShell(L"")
, m_strInitialDir(L"")
, m_bRunAsUser(false)
, m_strUser(L"")
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ExecuteDlgInit(IDD);

	if (CTheme().IsThemingSupported()) ::EnableThemeDialogTexture(m_hWnd, ETDT_USETABTEXTURE);

	m_staticCursorAnim.Attach(GetDlgItem(IDC_CURSOR_ANIM));
	m_comboCursor.Attach(GetDlgItem(IDC_COMBO_CURSOR));
	m_staticCursorColor.Attach(GetDlgItem(IDC_CURSOR_COLOR));

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND		hWnd = reinterpret_cast<HWND>(lParam); 
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));

	if (hWnd == m_staticCursorColor.m_hWnd)
	{
		CBrush	brush(::CreateSolidBrush(m_tabData->crCursorColor));
		CRect	rect;

		m_staticCursorColor.GetClientRect(&rect);
		dc.FillRect(&rect, brush);

		SetCursor();
		return 0;
	}

/*
    ::SetBkMode(dc, TRANSPARENT);

    // return the brush
    return (LRESULT)(::GetStockObject(HOLLOW_BRUSH));
*/

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnTabTitleChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CWindow(GetDlgItem(IDC_TAB_TITLE)).GetWindowText(m_strTitle);
	GetParent().PostMessage(UM_TAB_TITLE_CHANGED, 0, 0);
	return 0;
}

LRESULT PageSettingsTabs1::OnTabIconChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CWindow(GetDlgItem(IDC_TAB_ICON)).GetWindowText(m_strIcon);
	GetParent().PostMessage(UM_TAB_ICON_CHANGED, 0, 0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnClickedCursorColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CColorDialog	dlg(m_tabData->crCursorColor, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		m_tabData->crCursorColor = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
		m_strIcon = fileDialog.m_szFileName;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnClickedBtnBrowseShell(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog fileDialog(
					TRUE, 
					NULL, 
					NULL, 
					OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST, 
					L"Executable Files (*.exe)\0*.exe\0Batch Files (*.bat;*.cmd)\0*.bat;*.cmd\0All Files (*.*)\0*.*\0\0");

	if (fileDialog.DoModal() == IDOK)
	{
		m_strShell = fileDialog.m_szFileName;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnClickedBtnBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFolderDialog folderDialog(m_hWnd, L"Choose initial directory");

	if (folderDialog.DoModal() == IDOK)
	{
		m_strInitialDir = folderDialog.m_szFolderPath;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnCheckboxClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableControls();

	GetParent().PostMessage(UM_TAB_ICON_CHANGED, 0, 0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs1::EnableControls()
{
	BOOL bEnableEditIconCtrls = TRUE;

	if (m_nUseDefaultIcon > 0) bEnableEditIconCtrls = FALSE;

	GetDlgItem(IDC_TAB_ICON).EnableWindow(bEnableEditIconCtrls);
	GetDlgItem(IDC_BTN_BROWSE_ICON).EnableWindow(bEnableEditIconCtrls);

	GetDlgItem(IDC_TAB_USER).EnableWindow(m_bRunAsUser ? TRUE : FALSE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs1::Load(shared_ptr<TabData>& tabData)
{
	m_tabData			= tabData;

	m_strTitle			= m_tabData->strTitle.c_str();
	m_strIcon			= m_tabData->strIcon.c_str();
	m_nUseDefaultIcon	= m_tabData->bUseDefaultIcon ? 1 : 0;

	m_strShell			= m_tabData->strShell.c_str();
	m_strInitialDir		= m_tabData->strInitialDir.c_str();
	m_bRunAsUser		= m_tabData->bRunAsUser;
	m_strUser			= m_tabData->strUser.c_str();

	m_comboCursor.SetCurSel(m_tabData->dwCursorStyle);

	m_staticCursorColor.Invalidate();

	DoDataExchange(DDX_LOAD);

	EnableControls();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs1::Save()
{
	DoDataExchange(DDX_SAVE);

	m_tabData->strTitle			= m_strTitle;
	m_tabData->strIcon			= m_strIcon;
	m_tabData->bUseDefaultIcon	= (m_nUseDefaultIcon > 0);

	m_tabData->strShell			= m_strShell;
	m_tabData->strInitialDir	= m_strInitialDir;
	m_tabData->bRunAsUser		= m_bRunAsUser;
	m_tabData->strUser			= m_strUser;

	m_tabData->dwCursorStyle	= m_comboCursor.GetCurSel();
}

//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnCbnSelchangeComboCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  SetCursor();

  return 0;
}

LRESULT PageSettingsTabs1::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if ((wParam == CURSOR_TIMER) && (m_cursor.get() != NULL))
  {
    DrawCursor();
  }

  return 0;
}

void PageSettingsTabs1::SetCursor(void)
{
  CRect rectCursorAnim;
  m_staticCursorAnim.GetClientRect(&rectCursorAnim);
  CClientDC dc(m_staticCursorAnim.m_hWnd);
  CBrush brush(::CreateSolidBrush(RGB(0,0,0)));
  dc.FillRect(rectCursorAnim, brush);

  rectCursorAnim.right  -= 8;
  rectCursorAnim.bottom -= 8;

  m_cursor.reset();
  m_cursor = CursorFactory::CreateCursor(
    m_hWnd,
    true,
    static_cast<CursorStyle>(m_comboCursor.GetCurSel()),
    dc,
    rectCursorAnim,
    m_tabData->crCursorColor);

  DrawCursor();
}

void PageSettingsTabs1::DrawCursor(void)
{
  m_staticCursorAnim.RedrawWindow();

  CClientDC dc(m_staticCursorAnim.m_hWnd);
  CBrush brush(::CreateSolidBrush(RGB(0,0,0)));
  CPen pen(::CreatePen(PS_SOLID, 2, RGB(255,255,255)));
  CRect rectCursorAnim;
  m_staticCursorAnim.GetClientRect(&rectCursorAnim);

  dc.FillRect(rectCursorAnim, brush);
  dc.SelectPen(pen);
  dc.MoveTo(rectCursorAnim.left  + 7, rectCursorAnim.top    + 7);
  dc.LineTo(rectCursorAnim.right - 7, rectCursorAnim.bottom - 7);
  dc.MoveTo(rectCursorAnim.right - 7, rectCursorAnim.top    + 7);
  dc.LineTo(rectCursorAnim.left  + 7, rectCursorAnim.bottom - 7);

  m_cursor->PrepareNext();
  m_cursor->Draw(true);
  m_cursor->BitBlt(dc, 4, 4);
}
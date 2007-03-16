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
, m_strShell(L"")
, m_strInitialDir(L"")
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (CTheme().IsThemingSupported()) ::EnableThemeDialogTexture(m_hWnd, ETDT_USETABTEXTURE);

	m_comboCursor.Attach(GetDlgItem(IDC_COMBO_CURSOR));

//	for (int i = 0; i < sizeof(Cursor::s_cursorNames)/sizeof(wchar_t*); ++i)
	// TODO: fix
	for (int i = 0; i < 12; ++i)
	{
		m_comboCursor.AddString(Cursor::s_cursorNames[i]);
	}

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
//	GetParent().SendMessage(UM_TAB_TITLE_CHANGED, 0, 0);
	GetParent().PostMessage(UM_TAB_TITLE_CHANGED, 0, 0);
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


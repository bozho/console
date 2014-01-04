#include "stdafx.h"
#include "resource.h"

#include "PageSettingsTabs1.h"

extern std::shared_ptr<SettingsHandler>	g_settingsHandler;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

PageSettingsTabs1::PageSettingsTabs1()
: m_tabData()
, m_strTitle(L"")
, m_strIcon(L"")
, m_bUseDefaultIcon(false)
, m_strShell(L"")
, m_strInitialDir(L"")
, m_nRunAs(0)
, m_strUser(L"")
, m_bNetOnly(false)
, m_bRunAsAdmin(false)
, m_tabShellEdit(this)
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
	m_tabIconEdit.SubclassWindow(GetDlgItem(IDC_TAB_ICON));
	m_tabShellEdit.SubclassWindow(GetDlgItem(IDC_TAB_SHELL));
	m_tabInitialDirEdit.SubclassWindow(GetDlgItem(IDC_TAB_INIT_DIR));

	DoDataExchange(DDX_LOAD);
	return TRUE;
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

LRESULT PageSettingsTabs1::OnTabShellChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if( m_bUseDefaultIcon )
	{
		CWindow(GetDlgItem(IDC_TAB_SHELL)).GetWindowText(m_strShell);
		GetParent().PostMessage(UM_TAB_ICON_CHANGED, 0, 0);
	}
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

	int index = 0;
	WCHAR path[MAX_PATH];
	::GetModuleFileName(NULL, path, ARRAYSIZE(path));

	if( PickIconDlg(NULL, path, ARRAYSIZE(path), &index) == 1 )
	{
		if( index != 0 )
			m_strIcon.Format(L"%s,%d", path, index);
		else
			m_strIcon = path;
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
					OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|OFN_NODEREFERENCELINKS, 
					L"Executable Files (*.exe)\0*.exe\0Shell Links (*.lnk)\0*.lnk\0Batch Files (*.bat;*.cmd)\0*.bat;*.cmd\0All Files (*.*)\0*.*\0\0");

	if (fileDialog.DoModal() == IDOK)
	{
		CString strShell(fileDialog.m_szFileName);

		this->ConvertShellLink(strShell);
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
	GetDlgItem(IDC_TAB_ICON).EnableWindow(m_bUseDefaultIcon == false);
	GetDlgItem(IDC_BTN_BROWSE_ICON).EnableWindow(m_bUseDefaultIcon == false);

	GetDlgItem(IDC_TAB_USER).EnableWindow(m_nRunAs == 1);
	GetDlgItem(IDC_CHECK_NET_ONLY).EnableWindow(m_nRunAs == 1);
	GetDlgItem(IDC_CHECK_RUN_AS_ADMIN).EnableWindow(m_nRunAs == 0);

	m_staticCursorColor.ShowWindow(static_cast<CursorStyle>(m_comboCursor.GetCurSel()) != cstyleConsole? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_COLOR).EnableWindow(static_cast<CursorStyle>(m_comboCursor.GetCurSel()) != cstyleConsole);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabs1::Load(std::shared_ptr<TabData>& tabData)
{
	m_tabData         = tabData;

	m_strTitle        = m_tabData->strTitle.c_str();
	m_strIcon         = m_tabData->strIcon.c_str();
	m_bUseDefaultIcon = m_tabData->bUseDefaultIcon;

	m_strShell        = m_tabData->strShell.c_str();
	m_strInitialDir   = m_tabData->strInitialDir.c_str();
	m_nRunAs          = m_tabData->bRunAsUser? 1 : 0;
	m_strUser         = m_tabData->strUser.c_str();
	m_bNetOnly        = m_tabData->bNetOnly;
	m_bRunAsAdmin     = m_tabData->bRunAsAdministrator;

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

	m_tabData->strTitle            = m_strTitle;
	m_tabData->strIcon             = m_strIcon;
	m_tabData->bUseDefaultIcon     = m_bUseDefaultIcon;

	m_tabData->strShell            = m_strShell;
	m_tabData->strInitialDir       = m_strInitialDir;
	m_tabData->bRunAsUser          = m_nRunAs == 1;
	m_tabData->strUser             = m_strUser;
	m_tabData->bNetOnly            = m_bNetOnly;
	m_tabData->bRunAsAdministrator = m_bRunAsAdmin;

	m_tabData->dwCursorStyle	= m_comboCursor.GetCurSel();
}

//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabs1::OnCbnSelchangeComboCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SetCursor();
	EnableControls();

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

void PageSettingsTabs1::RedrawCharOnCursor(CDC& dc)
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

void PageSettingsTabs1::DrawCursor()
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

void PageSettingsTabs1::DrawCursor(CDC& dc, const CRect& rectCursorAnim, COLORREF fg, COLORREF bg)
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

void PageSettingsTabs1::ConvertShellLink(CString& strShell)
{
	m_strShell = strShell;

	if( m_strShell.Right(4).CompareNoCase(L".lnk") == 0 )
	{
		CComPtr<IShellLink> shellLink;
		if( SUCCEEDED(shellLink.CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER)) )
		{
			CComPtr<IPersistFile> persistFile;
			if( SUCCEEDED(shellLink.QueryInterface(&persistFile)) )
			{
				if( SUCCEEDED(persistFile->Load(m_strShell, STGM_READ)) &&
					  SUCCEEDED(shellLink->Resolve(m_hWnd, 0)) )
				{
					WCHAR szBuffer[MAX_PATH];
					WCHAR szBuffer2[2048];
					WIN32_FIND_DATA wfd;

					if( SUCCEEDED(shellLink->GetPath(szBuffer, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH)) &&
						  SUCCEEDED(shellLink->GetArguments(szBuffer2, ARRAYSIZE(szBuffer2))) )
					{
						m_strShell = szBuffer;

						if( m_strShell.Find(L' ') != -1 )
						{
							m_strShell.Insert(0, L'"');
							m_strShell.Insert(m_strShell.GetLength(), L'"');
						}

						m_strShell += L" ";
						m_strShell += szBuffer2;

					}

					if( SUCCEEDED(shellLink->GetWorkingDirectory(szBuffer, MAX_PATH)) )
					{
						m_strInitialDir = szBuffer;
					}

					int iIcon = 0;
					if( SUCCEEDED(shellLink->GetIconLocation(szBuffer, MAX_PATH, &iIcon)) )
					{
						m_strIcon = szBuffer;
						if( iIcon )
						{
							swprintf_s<MAX_PATH>(szBuffer, L",%i", iIcon);
							m_strIcon += szBuffer;
						}
						m_bUseDefaultIcon = m_strIcon.IsEmpty();
					}

					CComPtr<IShellLinkDataList> dataList;
					DWORD dwFlags;
					if( SUCCEEDED(shellLink.QueryInterface(&dataList)) &&
						  SUCCEEDED(dataList->GetFlags(&dwFlags)) )
					{
						m_bRunAsAdmin = (dwFlags & SLDF_RUNAS_USER) == SLDF_RUNAS_USER;
					}
				}
			}
		}
	}
	else
	{
		if( m_strShell.Find(L' ') != -1 )
		{
			m_strShell.Insert(0, L'"');
			m_strShell.Insert(m_strShell.GetLength(), L'"');
		}
	}

	DoDataExchange(DDX_LOAD);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CFileNameAndLinkEdit::OnDropFile(CString& strFilename)
{
	p->ConvertShellLink(strFilename);
	return false;
}
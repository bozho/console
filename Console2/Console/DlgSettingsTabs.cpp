#include "stdafx.h"
#include "resource.h"

#include "Cursors.h"
#include "DlgSettingsTabs.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsTabs::DlgSettingsTabs(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
, m_strTitle(L"")
, m_strIcon(L"")
, m_strShell(L"")
, m_strInitialDir(L"")
, m_nBkType(0)
, m_strBkImage(L"")
, m_nRelative(0)
, m_nExtend(0)
{
	IDD = IDD_SETTINGS_TABS;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_tabSettings.Load(m_pOptionsRoot);

	m_listCtrl.Attach(GetDlgItem(IDC_LIST_TABS));

	m_listCtrl.SetExtendedListViewStyle(m_listCtrl.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT);
	m_listCtrl.InsertColumn(0, L"Tab name");
	m_listCtrl.SetColumnWidth(0, 230);

	TabDataVector::iterator	it = m_tabSettings.tabDataVector.begin();
	for (; it != m_tabSettings.tabDataVector.end(); ++it)
	{
		int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), (*it)->strTitle.c_str());
		m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(it->get()));
	}

	m_comboCursor.Attach(GetDlgItem(IDC_COMBO_CURSOR));

//	for (int i = 0; i < sizeof(Cursor::s_cursorNames)/sizeof(wchar_t*); ++i)
	// TODO: fix
	for (int i = 0; i < 12; ++i)
	{
		m_comboCursor.AddString(Cursor::s_cursorNames[i]);
	}

	m_comboBkPosition.Attach(GetDlgItem(IDC_COMBO_BK_POS));
	m_comboBkPosition.AddString(L"Center");
	m_comboBkPosition.AddString(L"Stretch");
	m_comboBkPosition.AddString(L"Tile");

	m_staticTintOpacity.Attach(GetDlgItem(IDC_TINT_OPACITY_VAL));
	m_sliderTintOpacity.Attach(GetDlgItem(IDC_TINT_OPACITY));
	m_sliderTintOpacity.SetRange(0, 255);
	m_sliderTintOpacity.SetTicFreq(5);
	m_sliderTintOpacity.SetPageSize(5);


	m_listCtrl.SelectItem(0);

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CWindow		staticCtl(reinterpret_cast<HWND>(lParam));
	CDCHandle	dc(reinterpret_cast<HDC>(wParam));

	TabData*	pTabData = reinterpret_cast<TabData*>(m_listCtrl.GetItemData(m_listCtrl.GetSelectedIndex()));

	if (staticCtl.m_hWnd == GetDlgItem(IDC_CURSOR_COLOR))
	{
		CBrush	brush(::CreateSolidBrush(pTabData->crCursorColor));
		CRect	rect;

		staticCtl.GetClientRect(&rect);
		dc.FillRect(&rect, brush);
		return 0;
	}
	else if (staticCtl.m_hWnd == GetDlgItem(IDC_BK_COLOR))
	{
		CBrush	brush(::CreateSolidBrush(pTabData->crBackgroundColor));
		CRect	rect;

		staticCtl.GetClientRect(&rect);
		dc.FillRect(&rect, brush);
		return 0;
	}
	else if (staticCtl.m_hWnd == GetDlgItem(IDC_TINT_COLOR))
	{
		CBrush	brush(::CreateSolidBrush(pTabData->imageData.crTint));
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

LRESULT DlgSettingsTabs::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	UpdateSliderText();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		TabData* pTabData = reinterpret_cast<TabData*>(m_listCtrl.GetItemData(m_listCtrl.GetSelectedIndex()));

		DoDataExchange(DDX_SAVE);
		SetTabData(pTabData);

		m_tabSettings.Save(m_pOptionsRoot);

/*
		m_hotKeys.Save(m_pOptionsRoot);

		HotKeys& hotKeys = g_settingsHandler->GetHotKeys();

		hotKeys.mapHotKeys.clear();
		hotKeys.mapHotKeys.insert(m_hotKeys.mapHotKeys.begin(), m_hotKeys.mapHotKeys.end());
*/
	}

	DestroyWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnBtnBrowseIcon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT DlgSettingsTabs::OnClickedBtnBrowseShell(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog fileDialog(
					TRUE, 
					NULL, 
					NULL, 
					OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST, 
					L"Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0\0");

	if (fileDialog.DoModal() == IDOK)
	{
		m_strShell = fileDialog.m_szFileName;
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnClickedBtnBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT DlgSettingsTabs::OnClickedCursorColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	TabData*	pTabData = reinterpret_cast<TabData*>(m_listCtrl.GetItemData(m_listCtrl.GetSelectedIndex()));

	CColorDialog	dlg(pTabData->crCursorColor, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		pTabData->crCursorColor = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnClickedBkType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(DDX_SAVE);
	EnableControls();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnClickedBkColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	TabData*	pTabData = reinterpret_cast<TabData*>(m_listCtrl.GetItemData(m_listCtrl.GetSelectedIndex()));

	CColorDialog	dlg(pTabData->crBackgroundColor, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		pTabData->crBackgroundColor = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnBtnBrowseImage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
		DoDataExchange(DDX_LOAD);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnClickedTintColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	TabData*	pTabData = reinterpret_cast<TabData*>(m_listCtrl.GetItemData(m_listCtrl.GetSelectedIndex()));

	CColorDialog	dlg(pTabData->imageData.crTint, CC_FULLOPEN);

	if (dlg.DoModal() == IDOK)
	{
		// update color
		pTabData->imageData.crTint = dlg.GetColor();
		CWindow(hWndCtl).Invalidate();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMLISTVIEW*	pnmv		= reinterpret_cast<NMLISTVIEW*>(pnmh);

	if (pnmv->iItem < 0) return 0;

	TabData* pTabData		= reinterpret_cast<TabData*>(pnmv->lParam);

	if (pnmv->uNewState & LVIS_SELECTED)
	{
		// selecting new item
		m_strTitle		= pTabData->strTitle.c_str();
		m_strIcon		= pTabData->strIcon.c_str();

		m_strShell		= pTabData->strShell.c_str();
		m_strInitialDir	= pTabData->strInitialDir.c_str();

		m_nBkType		= static_cast<int>(pTabData->backgroundImageType);
		m_strBkImage	= pTabData->imageData.strFilename.c_str();
		m_nRelative		= pTabData->imageData.bRelative ? 1 : 0;
		m_nExtend		= pTabData->imageData.bExtend ? 1 : 0;

		m_comboCursor.SetCurSel(pTabData->dwCursorStyle);
		m_comboBkPosition.SetCurSel(static_cast<int>(pTabData->imageData.imagePosition));

		m_sliderTintOpacity.SetPos(pTabData->imageData.byTintOpacity);
		UpdateSliderText();
	
		DoDataExchange(DDX_LOAD);
		EnableControls();
	}
	else if (pnmv->uOldState & LVIS_SELECTED)
	{
		// deselecting item
		DoDataExchange(DDX_SAVE);
		SetTabData(pTabData);
		m_listCtrl.SetItemText(pnmv->iItem, 0, m_strTitle);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsTabs::SetTabData(TabData* pTabData)
{
	pTabData->strTitle					= m_strTitle;
	pTabData->strIcon					= m_strIcon;

	pTabData->strShell					= m_strShell;
	pTabData->strInitialDir				= m_strInitialDir;

	pTabData->backgroundImageType		= static_cast<BackgroundImageType>(m_nBkType);
	pTabData->imageData.strFilename		= m_strBkImage;
	pTabData->imageData.bRelative		= m_nRelative > 0;
	pTabData->imageData.bExtend			= m_nExtend > 0;

	pTabData->dwCursorStyle				= m_comboCursor.GetCurSel();
	pTabData->imageData.imagePosition	= static_cast<ImagePosition>(m_comboBkPosition.GetCurSel());

	pTabData->imageData.byTintOpacity	= static_cast<BYTE>(m_sliderTintOpacity.GetPos());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsTabs::UpdateSliderText()
{
	CString strStaticText;
	strStaticText.Format(L"%i", m_sliderTintOpacity.GetPos());

	m_staticTintOpacity.SetWindowText(strStaticText);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsTabs::EnableControls()
{
	::EnableWindow(GetDlgItem(IDC_STATIC_BK_COLOR), FALSE);
	::EnableWindow(GetDlgItem(IDC_BK_COLOR), FALSE);
	::EnableWindow(GetDlgItem(IDC_STATIC_BK_IMAGE), FALSE);
	::EnableWindow(GetDlgItem(IDC_BK_IMAGE), FALSE);
	::EnableWindow(GetDlgItem(IDC_BTN_BROWSE_BK), FALSE);
	::EnableWindow(GetDlgItem(IDC_CHECK_BK_RELATIVE), FALSE);
	::EnableWindow(GetDlgItem(IDC_CHECK_BK_EXTEND), FALSE);
	::EnableWindow(GetDlgItem(IDC_STATIC_BK_POS), FALSE);
	::EnableWindow(GetDlgItem(IDC_COMBO_BK_POS), FALSE);

	::EnableWindow(GetDlgItem(IDC_STATIC_TINT_COLOR), FALSE);
	::EnableWindow(GetDlgItem(IDC_TINT_COLOR), FALSE);
	::EnableWindow(GetDlgItem(IDC_STATIC_TINT_OPACITY), FALSE);
	::EnableWindow(GetDlgItem(IDC_TINT_OPACITY), FALSE);
	::EnableWindow(GetDlgItem(IDC_TINT_OPACITY_VAL), FALSE);

	if (m_nBkType == static_cast<int>(bktypeNone))
	{
		::EnableWindow(GetDlgItem(IDC_STATIC_BK_COLOR), TRUE);
		::EnableWindow(GetDlgItem(IDC_BK_COLOR), TRUE);
	}
	else if (m_nBkType == static_cast<int>(bktypeImage))
	{
		::EnableWindow(GetDlgItem(IDC_STATIC_BK_IMAGE), TRUE);
		::EnableWindow(GetDlgItem(IDC_BK_IMAGE), TRUE);
		::EnableWindow(GetDlgItem(IDC_BTN_BROWSE_BK), TRUE);
		::EnableWindow(GetDlgItem(IDC_CHECK_BK_RELATIVE), TRUE);
		::EnableWindow(GetDlgItem(IDC_CHECK_BK_EXTEND), TRUE);
		::EnableWindow(GetDlgItem(IDC_STATIC_BK_POS), TRUE);
		::EnableWindow(GetDlgItem(IDC_COMBO_BK_POS), TRUE);
	}

	if (m_nBkType != static_cast<int>(bktypeNone))
	{
		::EnableWindow(GetDlgItem(IDC_STATIC_TINT_COLOR), TRUE);
		::EnableWindow(GetDlgItem(IDC_TINT_COLOR), TRUE);
		::EnableWindow(GetDlgItem(IDC_STATIC_TINT_OPACITY), TRUE);
		::EnableWindow(GetDlgItem(IDC_TINT_OPACITY), TRUE);
		::EnableWindow(GetDlgItem(IDC_TINT_OPACITY_VAL), TRUE);
	}
}

//////////////////////////////////////////////////////////////////////////////


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
, m_page1()
, m_page2()
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
	m_listCtrl.SetColumnWidth(0, 188);

	TabDataVector::iterator	it = m_tabSettings.tabDataVector.begin();
	for (; it != m_tabSettings.tabDataVector.end(); ++it)
	{
		int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), (*it)->strTitle.c_str());
		m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(it->get()));
	}

	m_tabCtrl.Attach(GetDlgItem(IDC_TABS));

	m_tabCtrl.InsertItem(0, L"Main");
	m_tabCtrl.InsertItem(1, L"Background");

	CRect	rect;
	m_tabCtrl.GetWindowRect(&rect);
	ScreenToClient(&rect);

	rect.DeflateRect(10, 25, 10, 10);

	m_page1.Create(m_hWnd, rect);
	m_page1.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
	m_page1.ShowWindow(SW_SHOW);

	m_page2.Create(m_hWnd, rect);
	m_page2.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);

	m_tabCtrl.SetCurSel(0);
	m_listCtrl.SelectItem(0);

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnTabTitleChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_listCtrl.SetItemText(m_listCtrl.GetSelectedIndex(), 0, m_page1.m_strTitle);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		TabData* pTabData = reinterpret_cast<TabData*>(m_listCtrl.GetItemData(m_listCtrl.GetSelectedIndex()));

		m_page1.DoDataExchange(DDX_SAVE);
		m_page2.DoDataExchange(DDX_SAVE);
		DoDataExchange(DDX_SAVE);
		SetTabData(pTabData);

		m_tabSettings.Save(m_pOptionsRoot);

		TabSettings& tabSettings = g_settingsHandler->GetTabSettings();

		tabSettings.tabDataVector.clear();
		tabSettings.tabDataVector.insert(
									tabSettings.tabDataVector.begin(), 
									m_tabSettings.tabDataVector.begin(), 
									m_tabSettings.tabDataVector.end());
	}

	DestroyWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ConsoleSettings& consoleSettings = g_settingsHandler->GetConsoleSettings();

	shared_ptr<TabData>	tabData(new TabData(consoleSettings.strShell, consoleSettings.strInitialDir));

	m_tabSettings.tabDataVector.push_back(tabData);

	int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), tabData->strTitle.c_str());
	m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(tabData.get()));

	m_listCtrl.SelectItem(nItem);
	GetDlgItem(IDC_BTN_DELETE).EnableWindow();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nItem = m_listCtrl.GetSelectedIndex();

	if (nItem > 0) MoveListItem(nItem, -1);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nItem = m_listCtrl.GetSelectedIndex();

	if (nItem < m_listCtrl.GetItemCount() - 1) MoveListItem(nItem, 1);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (::MessageBox(m_hWnd, L"Delete selected tab?", L"Console", MB_YESNO|MB_ICONQUESTION) != IDYES) return 0;

	int nItem = m_listCtrl.GetSelectedIndex();

	m_listCtrl.DeleteItem(nItem);
	m_tabSettings.tabDataVector.erase(m_tabSettings.tabDataVector.begin() + nItem);

	if (nItem > 0) --nItem;

	m_listCtrl.SelectItem(nItem);
	if (m_listCtrl.GetItemCount() < 2) GetDlgItem(IDC_BTN_DELETE).EnableWindow(FALSE);

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
		m_page1.m_tabData		= m_tabSettings.tabDataVector[m_listCtrl.GetSelectedIndex()];
		m_page2.m_tabData		= m_tabSettings.tabDataVector[m_listCtrl.GetSelectedIndex()];

		m_page1.m_strTitle		= pTabData->strTitle.c_str();
		m_page1.m_strIcon		= pTabData->strIcon.c_str();

		m_page1.m_strShell		= pTabData->strShell.c_str();
		m_page1.m_strInitialDir	= pTabData->strInitialDir.c_str();

		m_page2.m_nBkType		= static_cast<int>(pTabData->backgroundImageType);
		m_page2.m_strBkImage	= pTabData->imageData.strFilename.c_str();
		m_page2.m_nRelative		= pTabData->imageData.bRelative ? 1 : 0;
		m_page2.m_nExtend		= pTabData->imageData.bExtend ? 1 : 0;

		m_page1.m_comboCursor.SetCurSel(pTabData->dwCursorStyle);

		m_page2.m_comboBkPosition.SetCurSel(static_cast<int>(pTabData->imageData.imagePosition));

		m_page2.m_sliderTintOpacity.SetPos(pTabData->imageData.byTintOpacity);
		m_page2.UpdateSliderText();
	
		m_page2.EnableControls();

		m_page1.m_staticCursorColor.Invalidate();

		m_page2.m_staticBkColor.Invalidate();
		m_page2.m_staticTintColor.Invalidate();

		m_page1.DoDataExchange(DDX_LOAD);
		m_page2.DoDataExchange(DDX_LOAD);
	}
	else if (pnmv->uOldState & LVIS_SELECTED)
	{
		// deselecting item
		m_page1.DoDataExchange(DDX_SAVE);
		m_page2.DoDataExchange(DDX_SAVE);
		SetTabData(pTabData);
		m_listCtrl.SetItemText(pnmv->iItem, 0, m_page1.m_strTitle);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnTabItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
	int	nItem = m_tabCtrl.GetCurSel();

	if (nItem == 0)
	{
		m_page1.ShowWindow(SW_SHOW);
		m_page2.ShowWindow(SW_HIDE);
	}
	else if (nItem == 1)
	{
		m_page1.ShowWindow(SW_HIDE);
		m_page2.ShowWindow(SW_SHOW);
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
	pTabData->strTitle					= m_page1.m_strTitle;
	pTabData->strIcon					= m_page1.m_strIcon;

	pTabData->strShell					= m_page1.m_strShell;
	pTabData->strInitialDir				= m_page1.m_strInitialDir;

	pTabData->backgroundImageType		= static_cast<BackgroundImageType>(m_page2.m_nBkType);

	if (pTabData->backgroundImageType != bktypeNone)
	{
		pTabData->crBackgroundColor = RGB(0, 0, 0);
	}

	pTabData->imageData.strFilename		= m_page2.m_strBkImage;
	pTabData->imageData.bRelative		= m_page2.m_nRelative > 0;
	pTabData->imageData.bExtend			= m_page2.m_nExtend > 0;

	pTabData->dwCursorStyle				= m_page1.m_comboCursor.GetCurSel();

	pTabData->imageData.imagePosition	= static_cast<ImagePosition>(m_page2.m_comboBkPosition.GetCurSel());
	pTabData->imageData.byTintOpacity	= static_cast<BYTE>(m_page2.m_sliderTintOpacity.GetPos());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsTabs::MoveListItem(int nItem, int nDirection)
{
	shared_ptr<TabData>	tmpData(*(m_tabSettings.tabDataVector.begin() + nItem));

	m_listCtrl.DeleteItem(nItem);
	int nNewItem = m_listCtrl.InsertItem(nItem + nDirection, tmpData->strTitle.c_str());

	m_tabSettings.tabDataVector.erase(m_tabSettings.tabDataVector.begin() + nItem);
	m_tabSettings.tabDataVector.insert(m_tabSettings.tabDataVector.begin() + nItem + nDirection, tmpData);

	m_listCtrl.SetItemData(nNewItem, reinterpret_cast<DWORD_PTR>(tmpData.get()));
	m_listCtrl.SelectItem(nNewItem);
}

//////////////////////////////////////////////////////////////////////////////


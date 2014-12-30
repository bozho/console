#include "stdafx.h"
#include "resource.h"

#include "Cursors.h"
#include "DlgSettingsTabs.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsTabs::DlgSettingsTabs(CComPtr<IXMLDOMElement>& pOptionsRoot, ConsoleSettings &consoleSettings)
: DlgSettingsBase(pOptionsRoot)
, m_page1()
, m_page2(consoleSettings)
, m_page3(consoleSettings)
, m_page4()
{
	IDD = IDD_SETTINGS_TABS;

	m_pages[0] = &m_page1;
	m_pages[1] = &m_page2;
	m_pages[2] = &m_page3;
	m_pages[3] = &m_page4;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_tabSettings.Load(m_pOptionsRoot);
  m_ImageList.Create(
    ::GetSystemMetrics(SM_CXSMICON),
    ::GetSystemMetrics(SM_CYSMICON),
    ILC_COLOR32 | ILC_MASK, 4, 4);
	m_listCtrl.Attach(GetDlgItem(IDC_LIST_TABS));

	m_listCtrl.SetExtendedListViewStyle(m_listCtrl.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT);
	m_listCtrl.InsertColumn(0, L"Tab name");
	m_listCtrl.SetColumnWidth(0, 188);
	m_listCtrl.SetImageList(m_ImageList, LVSIL_SMALL);
	//m_listCtrl.SetIconSpacing(16, 16);

	TabDataVector::iterator	it = m_tabSettings.tabDataVector.begin();
	for (; it != m_tabSettings.tabDataVector.end(); ++it)
	{
		CIcon tabSmallIcon(Helpers::LoadTabIcon(false, (*it)->bUseDefaultIcon, (*it)->strIcon, (*it)->strShell));
		int nIcon = tabSmallIcon.m_hIcon? m_ImageList.AddIcon(tabSmallIcon.m_hIcon) : -1;
		int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), (*it)->strTitle.c_str(), nIcon);
		m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(it->get()));
	}

	if (m_listCtrl.GetItemCount() < 2) GetDlgItem(IDC_BTN_DELETE).EnableWindow(FALSE);

	m_tabCtrl.Attach(GetDlgItem(IDC_TABS));

	m_tabCtrl.InsertItem(0, L"Main");
	m_tabCtrl.InsertItem(1, L"Background");
	m_tabCtrl.InsertItem(2, L"Colors && Cursor");
	m_tabCtrl.InsertItem(3, L"Environment");

	CRect	rect;
	m_tabCtrl.GetWindowRect(&rect);
	ScreenToClient(&rect);

	rect.DeflateRect(10, 25, 10, 10);

	m_page1.Create(m_hWnd, rect);
	m_page1.SetWindowPos(HWND_TOP, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	m_page2.Create(m_hWnd, rect);
	m_page2.SetWindowPos(HWND_TOP, rect.left, rect.top, rect.Width(), rect.Height(), 0);

	m_page3.Create(m_hWnd, rect);
	m_page3.SetWindowPos(HWND_TOP, rect.left, rect.top, rect.Width(), rect.Height(), 0);

	m_page4.Create(m_hWnd, rect);
	m_page4.SetWindowPos(HWND_TOP, rect.left, rect.top, rect.Width(), rect.Height(), 0);

	m_tabCtrl.SetCurSel(0);
	m_listCtrl.SelectItem(0);

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnTabTitleChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_listCtrl.SetItemText(m_listCtrl.GetSelectedIndex(), 0, m_page1.GetTabTitle());
	return 0;
}

LRESULT DlgSettingsTabs::OnTabIconChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  bool    bUseDefaultIcon = m_page1.UseDefaultIcon() ? true : false;
  wstring strIcon         = m_page1.GetTabIcon();
  wstring strShell        = m_page1.GetTabShell();

  CIcon tabSmallIcon(Helpers::LoadTabIcon(false, bUseDefaultIcon, strIcon, strShell));
  int nIcon = tabSmallIcon.m_hIcon? m_ImageList.AddIcon(tabSmallIcon.m_hIcon) : -1;
  // list control is not refreshed when an empty icon is set ...
  // so the text is updated too !
  m_listCtrl.SetItem(m_listCtrl.GetSelectedIndex(), 0, LVIF_TEXT|LVIF_IMAGE, m_page1.GetTabTitle(), nIcon, 0, 0, 0);

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		for each (PageSettingsTab *page in m_pages)
		{
			page->Save();
		}

		DoDataExchange(DDX_SAVE);

		m_tabSettings.Save(m_pOptionsRoot);

		TabSettings& tabSettings = g_settingsHandler->GetTabSettings();

		tabSettings.tabDataVector.clear();
		tabSettings.tabDataVector.insert(
									tabSettings.tabDataVector.begin(), 
									m_tabSettings.tabDataVector.begin(), 
									m_tabSettings.tabDataVector.end());
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ConsoleSettings& consoleSettings = g_settingsHandler->GetConsoleSettings();

	std::shared_ptr<TabData>	tabData(new TabData(consoleSettings.strShell, consoleSettings.strInitialDir));

	m_tabSettings.tabDataVector.push_back(tabData);

	int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), tabData->strTitle.c_str(), -1);
	m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(tabData.get()));

	m_listCtrl.SelectItem(nItem);
	GetDlgItem(IDC_BTN_DELETE).EnableWindow();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnClone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nItem = m_listCtrl.GetSelectedIndex();

	m_page1.Save();
	m_page2.Save();

	std::shared_ptr<TabData>	tabData(new TabData(*(m_tabSettings.tabDataVector[nItem].get())));
	tabData->strTitle += L" (2)";

	m_tabSettings.tabDataVector.insert(m_tabSettings.tabDataVector.begin() + nItem + 1, tabData);
	int nItemClone = m_listCtrl.InsertItem(nItem + 1, tabData->strTitle.c_str(), -1);
	m_listCtrl.SetItemData(nItemClone, reinterpret_cast<DWORD_PTR>(tabData.get()));

	m_listCtrl.SelectItem(nItemClone);
	GetDlgItem(IDC_BTN_DELETE).EnableWindow(TRUE);

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
	if (::MessageBox(m_hWnd, L"Delete selected tab?", L"ConsoleZ", MB_YESNO|MB_ICONQUESTION) != IDYES) return 0;

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

	if (pnmv->uNewState & LVIS_SELECTED)
	{
		// selecting new item
		for each (PageSettingsTab *page in m_pages)
		{
			page->Load(m_tabSettings.tabDataVector[m_listCtrl.GetSelectedIndex()]);
		}
	}
	else if (pnmv->uOldState & LVIS_SELECTED)
	{
		// deselecting item
		for each (PageSettingsTab *page in m_pages)
		{
			page->Save();
		}

		m_listCtrl.SetItemText(pnmv->iItem, 0, m_page1.GetTabTitle());
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnTabItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
	int	nItem = m_tabCtrl.GetCurSel();

	for(int i = 0; i < _countof(m_pages); i++)
	{
		m_pages[i]->Show(nItem == i ? SW_SHOW : SW_HIDE);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsTabs::MoveListItem(int nItem, int nDirection)
{
	std::shared_ptr<TabData>	tmpData(*(m_tabSettings.tabDataVector.begin() + nItem));

	m_listCtrl.DeleteItem(nItem);
	int nNewItem = m_listCtrl.InsertItem(nItem + nDirection, tmpData->strTitle.c_str());

	m_tabSettings.tabDataVector.erase(m_tabSettings.tabDataVector.begin() + nItem);
	m_tabSettings.tabDataVector.insert(m_tabSettings.tabDataVector.begin() + nItem + nDirection, tmpData);

	m_listCtrl.SetItemData(nNewItem, reinterpret_cast<DWORD_PTR>(tmpData.get()));
	m_listCtrl.SelectItem(nNewItem);
}

//////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "resource.h"

#include "XmlHelper.h"

#include "DlgSettingsConsole.h"
#include "DlgSettingsAppearance.h"
#include "DlgSettingsStyles.h"
#include "DlgSettingsBehavior.h"
#include "DlgSettingsHotkeys.h"
#include "DlgSettingsMouse.h"
#include "DlgSettingsTabs.h"
#include "DlgSettingsMain.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsMain::DlgSettingsMain()
: m_strSettingsFileName(L"")
, m_treeCtrl()
, m_settingsDlgMap()
, m_pSettingsDocument()
, m_pSettingsRoot()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsMain::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HRESULT hr = S_OK;

	hr = XmlHelper::OpenXmlDocument(
						g_settingsHandler->GetSettingsFileName(), 
						m_pSettingsDocument, 
						m_pSettingsRoot);

	if (FAILED(hr)) return FALSE;

	m_treeCtrl.Attach(GetDlgItem(IDC_TREE_SECTIONS));
	m_checkUserDataDir.Attach(GetDlgItem(IDC_CHECK_USER_DATA_DIR));

	m_checkUserDataDir.SetCheck((g_settingsHandler->GetSettingsDirType() == SettingsHandler::dirTypeUser) ? 1 : 0);
	if (g_settingsHandler->GetSettingsDirType() == SettingsHandler::dirTypeCustom) m_checkUserDataDir.EnableWindow(FALSE);

	CreateSettingsTree();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsMain::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SettingsDlgsMap::iterator it = m_settingsDlgMap.begin();
	for (; it != m_settingsDlgMap.end(); ++it)
	{
		(it->second)->SendMessage(WM_COMMAND, wID, 0);
	}

	if (wID == IDOK)
	{
		if (m_checkUserDataDir.IsWindowEnabled())
		{
			g_settingsHandler->SetUserDataDir((m_checkUserDataDir.GetCheck() == 1) ? SettingsHandler::dirTypeUser : SettingsHandler::dirTypeExe);
		}
		m_pSettingsDocument->save(CComVariant(g_settingsHandler->GetSettingsFileName().c_str()));
	}

	EndDialog(wID);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsMain::OnTreeSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMTREEVIEW* pnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);

	SettingsDlgsMap::iterator itOld = m_settingsDlgMap.find(pnmtv->itemOld.hItem);
	SettingsDlgsMap::iterator itNew = m_settingsDlgMap.find(pnmtv->itemNew.hItem);

	if (itOld != m_settingsDlgMap.end())
	{
		(itOld->second)->ShowWindow(SW_HIDE);
	}
	else
	{
		CWindow	wndPlaceholder(GetDlgItem(IDC_CHILD_PLACEHOLDER));

		wndPlaceholder.ShowWindow(SW_HIDE);
	}

	if (itNew != m_settingsDlgMap.end())
	{
		(itNew->second)->ShowWindow(SW_SHOW);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsMain::CreateSettingsTree()
{
	CRect	rect;
	CWindow	wndPlaceholder(GetDlgItem(IDC_CHILD_PLACEHOLDER));

	wndPlaceholder.GetWindowRect(&rect);
	ScreenToClient(&rect);


	// create console settings dialog
	shared_ptr<DlgSettingsBase>	dlgConsole(dynamic_cast<DlgSettingsBase*>(new DlgSettingsConsole(m_pSettingsRoot)));
	AddDialogToTree(L"Console", dlgConsole, rect);

	// create appearance settings dialog
	shared_ptr<DlgSettingsBase>	dlgAppearance(dynamic_cast<DlgSettingsBase*>(new DlgSettingsAppearance(m_pSettingsRoot)));
	HTREEITEM htiAppearance = AddDialogToTree(L"Appearance", dlgAppearance, rect);

	// create styles settings dialog
	shared_ptr<DlgSettingsBase>	dlgStyles(dynamic_cast<DlgSettingsBase*>(new DlgSettingsStyles(m_pSettingsRoot)));
	AddDialogToTree(L"More...", dlgStyles, rect, htiAppearance);

	// create behavior settings dialog
	shared_ptr<DlgSettingsBase>	dlgBehavior(dynamic_cast<DlgSettingsBase*>(new DlgSettingsBehavior(m_pSettingsRoot)));
	AddDialogToTree(L"Behavior", dlgBehavior, rect);

	// create hotkeys settings dialog
	shared_ptr<DlgSettingsBase>	dlgHotKeys(dynamic_cast<DlgSettingsBase*>(new DlgSettingsHotkeys(m_pSettingsRoot)));
	HTREEITEM htiHotkeys = AddDialogToTree(L"Hotkeys", dlgHotKeys, rect);

	// create mouse commands settings dialog
	shared_ptr<DlgSettingsBase>	dlgMouseCmds(dynamic_cast<DlgSettingsBase*>(new DlgSettingsMouse(m_pSettingsRoot)));
	AddDialogToTree(L"Mouse", dlgMouseCmds, rect, htiHotkeys);

	// create tabs settings dialog
	shared_ptr<DlgSettingsBase>	dlgTabs(dynamic_cast<DlgSettingsBase*>(new DlgSettingsTabs(m_pSettingsRoot)));
	AddDialogToTree(L"Tabs", dlgTabs, rect);

	m_treeCtrl.Expand(htiAppearance);
	m_treeCtrl.Expand(htiHotkeys);
	m_treeCtrl.SelectItem(m_treeCtrl.GetRootItem());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HTREEITEM DlgSettingsMain::AddDialogToTree(const wstring& strName, const shared_ptr<DlgSettingsBase>& newDlg, CRect& rect, HTREEITEM htiParent /*= NULL*/)
{
	newDlg->Create(m_hWnd, rect);
	newDlg->SetWindowPos(HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);

	HTREEITEM hItem = m_treeCtrl.InsertItem(strName.c_str(), htiParent, NULL);

	if (hItem != NULL) m_settingsDlgMap.insert(SettingsDlgsMap::value_type(hItem, newDlg));

	return hItem;
}

//////////////////////////////////////////////////////////////////////////////

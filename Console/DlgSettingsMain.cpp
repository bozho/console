#include "stdafx.h"
#include "resource.h"

#include "XmlHelper.h"

#include "DlgSettingsConsole.h"
#include "DlgSettingsAppearance.h"
#include "DlgSettingsStyles.h"
#include "DlgSettingsTransparency.h"
#include "DlgSettingsFont.h"
#include "DlgSettingsFullScreen.h"
#include "DlgSettingsBehavior.h"
#include "DlgSettingsBehavior2.h"
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
	g_settingsHandler->SaveSettings();

	HRESULT hr = XmlHelper::OpenXmlDocument(
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
	if (wID == IDOK)
	{
		if (m_checkUserDataDir.IsWindowEnabled())
		{
			g_settingsHandler->SetUserDataDir((m_checkUserDataDir.GetCheck() == 1) ? SettingsHandler::dirTypeUser : SettingsHandler::dirTypeExe);
		}

		HRESULT hr = m_pSettingsDocument->save(CComVariant(g_settingsHandler->GetSettingsFileName().c_str()));
		while (FAILED(hr))
		{
			std::wstring error = boost::str(boost::wformat(Helpers::LoadStringW(MSG_SETTINGS_CONFIG_FILE_WRITE_ERROR)) % g_settingsHandler->GetSettingsFileName().c_str());
			if( g_settingsHandler->GetSettingsDirType() == SettingsHandler::dirTypeExe )
			{
				error += L"\n\n";
				error += Helpers::LoadStringW(MSG_SETTINGS_CONFIG_FILE_WRITE_ERROR_EXEDIR);
			}

			switch( MessageBox(error.c_str(), Helpers::LoadStringW(IDS_CAPTION_ERROR).c_str(), MB_CANCELTRYCONTINUE | MB_ICONERROR) )
			{
			case IDCANCEL:
				return 0;
				break;

			case IDTRYAGAIN:
				hr = m_pSettingsDocument->save(CComVariant(g_settingsHandler->GetSettingsFileName().c_str()));
				break;

			case IDCONTINUE:
				hr = S_OK;
				break;
			}
		}

		if( g_settingsHandler->GetSettingsDirType() == SettingsHandler::dirTypeExe )
		{
			// rename the file located in %APPDATA%\console
			g_settingsHandler->SetUserDataDir(SettingsHandler::dirTypeUser);
			MoveFile(
				g_settingsHandler->GetSettingsFileName().c_str(),
				(g_settingsHandler->GetSettingsFileName() + L".bak").c_str());

			g_settingsHandler->SetUserDataDir(SettingsHandler::dirTypeExe);
		}
	}

	SettingsDlgsMap::iterator it = m_settingsDlgMap.begin();
	for (; it != m_settingsDlgMap.end(); ++it)
	{
		if ((it->second)->SendMessage(WM_COMMAND, wID, 0) != 0)
		{
			m_treeCtrl.Select(it->first, TVGN_CARET);
			return -1;
		}
	}

	for (it = m_settingsDlgMap.begin(); it != m_settingsDlgMap.end(); ++it)
	{
		it->second->DestroyWindow();
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
	std::shared_ptr<DlgSettingsBase>	dlgConsole(new DlgSettingsConsole(m_pSettingsRoot));
	AddDialogToTree(IDS_SETTINGS_CONSOLE, dlgConsole, rect);

	// create appearance settings dialog
	std::shared_ptr<DlgSettingsBase>	dlgAppearance(new DlgSettingsAppearance(m_pSettingsRoot));
	HTREEITEM htiAppearance = AddDialogToTree(IDS_SETTINGS_APPEARANCE, dlgAppearance, rect);

	// create styles settings dialog
	std::shared_ptr<DlgSettingsBase>	dlgStyles(new DlgSettingsStyles(m_pSettingsRoot));
	AddDialogToTree(IDS_SETTINGS_STYLES, dlgStyles, rect, htiAppearance);

	// create window transparency dialog
	std::shared_ptr<DlgSettingsBase>	dlgTransparency(new DlgSettingsTransparency(m_pSettingsRoot));
	AddDialogToTree(IDS_SETTINGS_TRANSPARENCY, dlgTransparency, rect, htiAppearance);

	// create styles settings dialog
	std::shared_ptr<DlgSettingsBase>	dlgFont(new DlgSettingsFont(m_pSettingsRoot));
	AddDialogToTree(IDS_SETTINGS_FONT, dlgFont, rect, htiAppearance);

	// create full screen settings dialog
	std::shared_ptr<DlgSettingsBase>	dlgFullScreen(new DlgSettingsFullScreen(m_pSettingsRoot));
	AddDialogToTree(IDS_SETTINGS_FULLSCREEN, dlgFullScreen, rect, htiAppearance);

	// create behavior settings dialog
	std::shared_ptr<DlgSettingsBase>	dlgBehavior(new DlgSettingsBehavior(m_pSettingsRoot));
	HTREEITEM htiBehavior = AddDialogToTree(IDS_SETTINGS_BEHAVIOR, dlgBehavior, rect);

	std::shared_ptr<DlgSettingsBase>	dlgBehavior2(new DlgSettingsBehavior2(m_pSettingsRoot));
	AddDialogToTree(IDS_SETTINGS_MORE, dlgBehavior2, rect, htiBehavior);

	// create hotkeys settings dialog
	std::shared_ptr<DlgSettingsBase>	dlgHotKeys(new DlgSettingsHotkeys(m_pSettingsRoot));
	HTREEITEM htiHotkeys = AddDialogToTree(IDS_SETTINGS_HOTKEYS, dlgHotKeys, rect);

	// create mouse commands settings dialog
	std::shared_ptr<DlgSettingsBase>	dlgMouseCmds(new DlgSettingsMouse(m_pSettingsRoot));
	AddDialogToTree(IDS_SETTINGS_MOUSE, dlgMouseCmds, rect, htiHotkeys);

	// create tabs settings dialog
	shared_ptr<DlgSettingsBase>	dlgTabs(new DlgSettingsTabs(m_pSettingsRoot, dynamic_cast<DlgSettingsConsole*>(dlgConsole.get())->m_consoleSettings));
	AddDialogToTree(IDS_SETTINGS_TABS, dlgTabs, rect);

	m_treeCtrl.Expand(htiAppearance);
	m_treeCtrl.Expand(htiBehavior);
	m_treeCtrl.Expand(htiHotkeys);
	m_treeCtrl.SelectItem(m_treeCtrl.GetRootItem());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HTREEITEM DlgSettingsMain::AddDialogToTree(const UINT descriptionID, const std::shared_ptr<DlgSettingsBase>& newDlg, CRect& rect, HTREEITEM htiParent /*= NULL*/)
{
	newDlg->Create(m_hWnd, rect);
	newDlg->SetWindowPos(HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);

	HTREEITEM hItem = m_treeCtrl.InsertItem(Helpers::LoadString(descriptionID).c_str(), htiParent, NULL);

	if (hItem != NULL) m_settingsDlgMap.insert(SettingsDlgsMap::value_type(hItem, newDlg));

	return hItem;
}

//////////////////////////////////////////////////////////////////////////////

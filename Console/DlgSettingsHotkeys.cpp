#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsHotkeys.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsHotkeys::DlgSettingsHotkeys(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
{
	IDD = IDD_SETTINGS_HOTKEYS;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_hotKeys.Load(m_pOptionsRoot);

	m_listCtrl.Attach(GetDlgItem(IDC_LIST_HOTKEYS));
	m_editCommand.Attach(GetDlgItem(IDC_EDIT_COMMAND));
	m_hotKeyEdit.SubclassWindow(GetDlgItem(IDC_EDIT_HOTKEY));

	m_listCtrl.SetExtendedListViewStyle(m_listCtrl.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT);

	m_listCtrl.InsertColumn(0, Helpers::LoadStringW(IDS_SETTINGS_COMMAND).c_str());
	m_listCtrl.InsertColumn(1, Helpers::LoadStringW(IDS_SETTINGS_HOTKEY).c_str());

	m_listCtrl.SetColumnWidth(0, 170);
	m_listCtrl.SetColumnWidth(1, 218);

	HotKeys::CommandsSequence::iterator	it = m_hotKeys.commands.begin();
	for (; it != m_hotKeys.commands.end(); ++it)
	{
		WORD	wModifiers = 0;
		CString	strKeyName;

		if ((*it)->accelHotkey.fVirt & FCONTROL) wModifiers |= HOTKEYF_CONTROL;
		if ((*it)->accelHotkey.fVirt & FSHIFT)   wModifiers |= HOTKEYF_SHIFT;
		if ((*it)->accelHotkey.fVirt & FALT)     wModifiers |= HOTKEYF_ALT;
		if ((*it)->bExtended)                    wModifiers |= HOTKEYF_EXT;
		if ((*it)->bWin)                         wModifiers |= FAKE_HOTKEYF_WIN;

		strKeyName = m_hotKeyEdit.GetHotKeyName((*it)->accelHotkey.key, wModifiers);

		int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), (*it)->strDescription.c_str());
		m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(it->get()));
		m_listCtrl.SetItemText(nItem, 1, strKeyName);
	}

	m_listCtrl.SelectItem(0);

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMLISTVIEW*				pnmv		= reinterpret_cast<NMLISTVIEW*>(pnmh);
	HotKeys::CommandData*	pCommandData= reinterpret_cast<HotKeys::CommandData*>(pnmv->lParam);

	WORD wModifiers = 0;

	if (pCommandData->accelHotkey.fVirt & FCONTROL) wModifiers |= HOTKEYF_CONTROL;
	if (pCommandData->accelHotkey.fVirt & FSHIFT)   wModifiers |= HOTKEYF_SHIFT;
	if (pCommandData->accelHotkey.fVirt & FALT)     wModifiers |= HOTKEYF_ALT;
	if (pCommandData->bExtended)                    wModifiers |= HOTKEYF_EXT;
	if (pCommandData->bWin)                         wModifiers |= FAKE_HOTKEYF_WIN;

	if( pCommandData->wCommandID >= ID_EXTERNAL_COMMAND_1 && pCommandData->wCommandID < (ID_EXTERNAL_COMMAND_1 + EXTERNAL_COMMANDS_COUNT) )
	{
		m_editCommand.SetWindowText(m_hotKeys.externalCommands[pCommandData->wCommandID - ID_EXTERNAL_COMMAND_1].c_str());
		m_editCommand.EnableWindow(TRUE);
	}
	else
	{
		CString strItemText;

		m_listCtrl.GetItemText(pnmv->iItem, 0, strItemText);
		m_editCommand.SetWindowText(strItemText);
		m_editCommand.EnableWindow(FALSE);
	}
	m_hotKeyEdit.SetHotKey(pCommandData->accelHotkey.key, wModifiers);
	m_hotKeyEdit.UseGlobalKeys(pCommandData->bGlobal);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnBtnAssign(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	UINT	uiVirtualKeyCode= 0;
	WORD	wModifiers		= 0;
	LVITEM	selectedItem;

	::ZeroMemory(&selectedItem, sizeof(LVITEM));
	selectedItem.mask = LVIF_PARAM;

	m_hotKeyEdit.GetHotKey(uiVirtualKeyCode, wModifiers);
	if (!m_listCtrl.GetSelectedItem(&selectedItem)) return 0;

	HotKeys::CommandData* pCommandData = reinterpret_cast<HotKeys::CommandData*>(selectedItem.lParam);

	if( pCommandData->wCommandID >= ID_EXTERNAL_COMMAND_1 && pCommandData->wCommandID < (ID_EXTERNAL_COMMAND_1 + EXTERNAL_COMMANDS_COUNT) )
	{
		CString strItemText;
		m_editCommand.GetWindowText(strItemText);
		m_hotKeys.externalCommands[pCommandData->wCommandID - ID_EXTERNAL_COMMAND_1] = strItemText;
	}

	BYTE fVirt     = FVIRTKEY;
	bool bExtended = false;
	bool bWin      = false;

	if (wModifiers & HOTKEYF_CONTROL)  fVirt |= FCONTROL;
	if (wModifiers & HOTKEYF_SHIFT)    fVirt |= FSHIFT;
	if (wModifiers & HOTKEYF_ALT)      fVirt |= FALT;
	if (wModifiers & HOTKEYF_EXT)      bExtended = true;
	if (wModifiers & FAKE_HOTKEYF_WIN) bWin      = true;

	if( uiVirtualKeyCode )
	{
		for(auto it = m_hotKeys.commands.begin(); it != m_hotKeys.commands.end(); ++it)
		{
			if( (*it)->wCommandID != pCommandData->wCommandID )
			{
				if( (*it)->accelHotkey.key != static_cast<WORD>(uiVirtualKeyCode) ) continue;
				if( (*it)->accelHotkey.fVirt != fVirt ) continue;
				if( (*it)->bExtended != bExtended ) continue;
				if( (*it)->bWin != bWin ) continue;

				std::wstring error = boost::str(boost::wformat(Helpers::LoadStringW(MSG_SETTINGS_HOTKEY_ALREADY_ASSIGNED)) % (*it)->strDescription.c_str());
				MessageBox(error.c_str(), Helpers::LoadString(IDS_CAPTION_ERROR).c_str(), MB_OK);

				return 0;
			}
		}
	}

	pCommandData->accelHotkey.cmd   = pCommandData->wCommandID;
	pCommandData->accelHotkey.key   = static_cast<WORD>(uiVirtualKeyCode);
	pCommandData->accelHotkey.fVirt = fVirt;
	pCommandData->bExtended         = bExtended;
	pCommandData->bWin              = bWin;

	m_listCtrl.SetItemText(m_listCtrl.GetSelectedIndex(), 1, m_hotKeyEdit.GetHotKeyName());

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnBtnClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_hotKeyEdit.SetHotKey(0, 0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		HotKeys& hotKeys = g_settingsHandler->GetHotKeys();

		hotKeys = m_hotKeys;
		hotKeys.Save(m_pOptionsRoot);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


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

	m_listCtrl.InsertColumn(0, L"Command");
	m_listCtrl.InsertColumn(1, L"Hotkey");

	m_listCtrl.SetColumnWidth(0, 100);
	m_listCtrl.SetColumnWidth(1, 218);

	HotKeys::CommandsVector::iterator	it = m_hotKeys.vecCommands.begin();
	for (; it != m_hotKeys.vecCommands.end(); ++it)
	{
		HotKeys::HotKeysMap::iterator itHotKey = m_hotKeys.mapHotKeys.find((*it)->wCommandID);

		WORD wModifiers = 0;

		if (itHotKey->second->accelHotkey.fVirt & FCONTROL)	wModifiers |= HOTKEYF_CONTROL;
		if (itHotKey->second->accelHotkey.fVirt & FSHIFT)	wModifiers |= HOTKEYF_SHIFT;
		if (itHotKey->second->accelHotkey.fVirt & FALT)		wModifiers |= HOTKEYF_ALT;
		if (itHotKey->second->bExtended)					wModifiers |= HOTKEYF_EXT;

		CString strKeyName(m_hotKeyEdit.GetHotKeyName(itHotKey->second->accelHotkey.key, wModifiers));

		int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), (*it)->strDescription.c_str());
		m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(itHotKey->second.get()));
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
	HotKeys::HotkeyData*	pHotkeyData	= reinterpret_cast<HotKeys::HotkeyData*>(pnmv->lParam);

	WORD wModifiers = 0;

	if (pHotkeyData->accelHotkey.fVirt & FCONTROL)	wModifiers |= HOTKEYF_CONTROL;
	if (pHotkeyData->accelHotkey.fVirt & FSHIFT)	wModifiers |= HOTKEYF_SHIFT;
	if (pHotkeyData->accelHotkey.fVirt & FALT)		wModifiers |= HOTKEYF_ALT;
	if (pHotkeyData->bExtended)						wModifiers |= HOTKEYF_EXT;

	CString strItemText;

	m_listCtrl.GetItemText(pnmv->iItem, 0, strItemText);
	m_editCommand.SetWindowText(strItemText);
	m_hotKeyEdit.SetHotKey(pHotkeyData->accelHotkey.key, wModifiers);

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

	HotKeys::HotkeyData* pHotkeyData = reinterpret_cast<HotKeys::HotkeyData*>(selectedItem.lParam);

	pHotkeyData->accelHotkey.key	= static_cast<WORD>(uiVirtualKeyCode);
	pHotkeyData->accelHotkey.fVirt	= FVIRTKEY;
	pHotkeyData->bExtended			= false;

	if (wModifiers & HOTKEYF_CONTROL)	pHotkeyData->accelHotkey.fVirt |= FCONTROL;
	if (wModifiers & HOTKEYF_SHIFT)		pHotkeyData->accelHotkey.fVirt |= FSHIFT;
	if (wModifiers & HOTKEYF_ALT)		pHotkeyData->accelHotkey.fVirt |= FALT;
	if (wModifiers & HOTKEYF_EXT)		pHotkeyData->bExtended = true;

	m_listCtrl.SetItemText(m_listCtrl.GetSelectedIndex(), 1, m_hotKeyEdit.GetHotKeyName());

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		m_hotKeys.Save(m_pOptionsRoot);

		HotKeys& hotKeys = g_settingsHandler->GetHotKeys();

		hotKeys.mapHotKeys.clear();
		hotKeys.mapHotKeys.insert(m_hotKeys.mapHotKeys.begin(), m_hotKeys.mapHotKeys.end());
	}

	DestroyWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


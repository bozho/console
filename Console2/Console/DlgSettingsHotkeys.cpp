#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsHotkeys.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*

//////////////////////////////////////////////////////////////////////////////

CString CHotKeyCtrlEx::GetHotKeyName() const {

	WORD	wVirtualKeyCode;
	WORD	wModifiers;

	GetHotKey(wVirtualKeyCode, wModifiers);

	return GetHotKeyName(wVirtualKeyCode, wModifiers);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CString CHotKeyCtrlEx::GetHotKeyName(WORD wVirtualKeyCode, WORD wModifiers) {

	CString	strKeyName(L"");

	if ((wVirtualKeyCode == 0) && (wModifiers == 0)) return CString(L"None");

	if (wModifiers & HOTKEYF_CONTROL) {
		strKeyName += GetKeyName(VK_CONTROL, FALSE);
		strKeyName += L"+";
	}

	if (wModifiers & HOTKEYF_SHIFT) {
		strKeyName += GetKeyName(VK_SHIFT, FALSE);
		strKeyName += L"+";
	}

	if (wModifiers & HOTKEYF_ALT) {
		strKeyName += GetKeyName(VK_MENU, FALSE);
		strKeyName += L"+";
	}

	strKeyName += GetKeyName(wVirtualKeyCode, wModifiers & HOTKEYF_EXT);

	return strKeyName;
}

//////////////////////////////////////////////////////////////////////////////

*/

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*

//////////////////////////////////////////////////////////////////////////////

CString CHotKeyCtrlEx::GetKeyName(UINT uiVk, BOOL bExtendedKey) {

	LONG lScanCode = ::MapVirtualKey(uiVk, 0) << 16;

	// if it's an extended key, add extended flag (bit 24)
	if (bExtendedKey) lScanCode |= 0x01000000L;

	CString strKeyName(L"");

	::GetKeyNameText(lScanCode, strKeyName.GetBufferSetLength(255), 255);
	strKeyName.ReleaseBuffer();

	return strKeyName;
}

//////////////////////////////////////////////////////////////////////////////

*/

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

LRESULT DlgSettingsHotkeys::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	m_hotKeys.Load(m_pOptionsRoot);

	m_listCtrl.Attach(GetDlgItem(IDC_LIST_HOTKEYS));
	m_hotKeyEdit.SubclassWindow(GetDlgItem(IDC_EDIT_HOTKEY));

	m_listCtrl.SetExtendedListViewStyle(m_listCtrl.GetExtendedListViewStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	m_listCtrl.InsertColumn(0, L"Command");
	m_listCtrl.InsertColumn(1, L"Hotkey");

	m_listCtrl.SetColumnWidth(0, 100);
	m_listCtrl.SetColumnWidth(1, 230);

	HotKeys::CommandsVector::iterator	it = m_hotKeys.vecCommands.begin();
	for (; it != m_hotKeys.vecCommands.end(); ++it) {

		HotKeys::HotKeysMap::iterator itHotKey = m_hotKeys.mapHotKeys.find((*it)->wCommandID);

		WORD wModifiers = 0;

		if (itHotKey->second->accelHotkey.fVirt & FCONTROL)	wModifiers |= HOTKEYF_CONTROL;
		if (itHotKey->second->accelHotkey.fVirt & FSHIFT)	wModifiers |= HOTKEYF_SHIFT;
		if (itHotKey->second->accelHotkey.fVirt & FALT)		wModifiers |= HOTKEYF_ALT;
		if (itHotKey->second->bExtended)					wModifiers |= HOTKEYF_EXT;

		CString strKeyName(m_hotKeyEdit.GetHotKeyName(itHotKey->second->accelHotkey.key, wModifiers));

		m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), (*it)->strDescription.c_str());
		m_listCtrl.SetItemData(m_listCtrl.GetItemCount() - 1, reinterpret_cast<DWORD_PTR>(itHotKey->second.get()));
		m_listCtrl.SetItemText(m_listCtrl.GetItemCount() - 1, 1, strKeyName);
	}

	m_listCtrl.SelectItem(0);

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {

	NMLISTVIEW*				pnmv = reinterpret_cast<NMLISTVIEW*>(pnmh);
	HotKeys::HotkeyData*	pHotkeyData = reinterpret_cast<HotKeys::HotkeyData*>(pnmv->lParam);

	WORD wModifiers = 0;

	if (pHotkeyData->accelHotkey.fVirt & FCONTROL)	wModifiers |= HOTKEYF_CONTROL;
	if (pHotkeyData->accelHotkey.fVirt & FSHIFT)	wModifiers |= HOTKEYF_SHIFT;
	if (pHotkeyData->accelHotkey.fVirt & FALT)		wModifiers |= HOTKEYF_ALT;
	if (pHotkeyData->bExtended)						wModifiers |= HOTKEYF_EXT;

	m_hotKeyEdit.SetHotKey(pHotkeyData->accelHotkey.key, wModifiers);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnBtnAssign(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	UINT	uiVirtualKeyCode;
	WORD	wModifiers;

	m_hotKeyEdit.GetHotKey(uiVirtualKeyCode, wModifiers);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsHotkeys::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	if (wID == IDOK) {
		DoDataExchange(DDX_SAVE);

//		m_hotKeys.Save(m_pOptionsRoot);
	}

	DestroyWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


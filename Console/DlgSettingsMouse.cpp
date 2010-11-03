#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsMouse.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsMouse::DlgSettingsMouse(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
{
	IDD = IDD_SETTINGS_MOUSE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsMouse::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ExecuteDlgInit(IDD);

	m_mouseSettings.Load(m_pOptionsRoot);

	m_listCtrl.Attach(GetDlgItem(IDC_LIST_MOUSE_COMMANDS));
	m_editCommand.Attach(GetDlgItem(IDC_EDIT_COMMAND));
	m_comboButtons.Attach(GetDlgItem(IDC_COMBO_BUTTONS));

	m_btnCtrl.Attach(GetDlgItem(IDC_CHECK_CTRL));
	m_btnShift.Attach(GetDlgItem(IDC_CHECK_SHIFT));
	m_btnAlt.Attach(GetDlgItem(IDC_CHECK_ALT));

	m_listCtrl.SetExtendedListViewStyle(m_listCtrl.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT);

	m_listCtrl.InsertColumn(0, L"Command");
	m_listCtrl.InsertColumn(1, L"Mouse action");

	m_listCtrl.SetColumnWidth(0, 170);
	m_listCtrl.SetColumnWidth(1, 218);

	MouseSettings::CommandsSequence::iterator it = m_mouseSettings.commands.begin();
	for (; it != m_mouseSettings.commands.end(); ++it)
	{
		int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), (*it)->strDescription.c_str());
		m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(it->get()));

		CString strAction;

		m_comboButtons.GetLBText(static_cast<int>((*it)->action.button), strAction);
		
		if ((*it)->action.modifiers & MouseSettings::mkCtrl) strAction += L" + Ctrl";
		if ((*it)->action.modifiers & MouseSettings::mkShift) strAction += L" + Shift";
		if ((*it)->action.modifiers & MouseSettings::mkAlt) strAction += L" + Alt";
		m_listCtrl.SetItemText(nItem, 1, strAction);
	}

	m_comboButtons.SetCurSel(0);
	m_listCtrl.SelectItem(0);

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsMouse::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMLISTVIEW*					pnmv		= reinterpret_cast<NMLISTVIEW*>(pnmh);
	MouseSettings::CommandData*	pCommandData= reinterpret_cast<MouseSettings::CommandData*>(pnmv->lParam);

	CString strItemText;

	m_listCtrl.GetItemText(pnmv->iItem, 0, strItemText);
	m_editCommand.SetWindowText(strItemText);

	m_comboButtons.SetCurSel(static_cast<int>(pCommandData->action.button));

	m_btnCtrl.SetCheck((pCommandData->action.modifiers & MouseSettings::mkCtrl) ? 1 : 0);
	m_btnShift.SetCheck((pCommandData->action.modifiers & MouseSettings::mkShift) ? 1 : 0);
	m_btnAlt.SetCheck((pCommandData->action.modifiers & MouseSettings::mkAlt) ? 1 : 0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsMouse::OnBtnAssign(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LVITEM	selectedItem;

	::ZeroMemory(&selectedItem, sizeof(LVITEM));
	selectedItem.mask = LVIF_PARAM;

	if (!m_listCtrl.GetSelectedItem(&selectedItem)) return 0;

	MouseSettings::CommandData*	pCommandData= reinterpret_cast<MouseSettings::CommandData*>(selectedItem.lParam);

	pCommandData->action.modifiers = MouseSettings::mkNone;

	pCommandData->action.button = static_cast<MouseSettings::Button>(m_comboButtons.GetCurSel());

	if (m_btnCtrl.GetCheck())	pCommandData->action.modifiers |= MouseSettings::mkCtrl;
	if (m_btnShift.GetCheck())	pCommandData->action.modifiers |= MouseSettings::mkShift;
	if (m_btnAlt.GetCheck())	pCommandData->action.modifiers |= MouseSettings::mkAlt;

	CString strAction;

	m_comboButtons.GetLBText(static_cast<int>(pCommandData->action.button), strAction);
	
	if (pCommandData->action.modifiers & MouseSettings::mkCtrl)	strAction += L" + Ctrl";
	if (pCommandData->action.modifiers & MouseSettings::mkShift)strAction += L" + Shift";
	if (pCommandData->action.modifiers & MouseSettings::mkAlt)	strAction += L" + Alt";
	m_listCtrl.SetItemText(m_listCtrl.GetSelectedIndex(), 1, strAction);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsMouse::OnBtnClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_comboButtons.SetCurSel(0);
	m_btnCtrl.SetCheck(0);
	m_btnShift.SetCheck(0);
	m_btnAlt.SetCheck(0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsMouse::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		MouseSettings& mouseSettings = g_settingsHandler->GetMouseSettings();

		mouseSettings = m_mouseSettings;
		mouseSettings.Save(m_pOptionsRoot);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


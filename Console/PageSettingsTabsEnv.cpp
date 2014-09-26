#include "stdafx.h"
#include "resource.h"

#include "PageSettingsTabsEnv.h"
#include "XmlHelper.h"

extern std::shared_ptr<SettingsHandler>	g_settingsHandler;

PageSettingsTabsEnv::PageSettingsTabsEnv()
: m_tabData()
{
}

LRESULT PageSettingsTabsEnv::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ExecuteDlgInit(IDD);

	if (CTheme().IsThemingSupported()) ::EnableThemeDialogTexture(m_hWnd, ETDT_USETABTEXTURE);

	m_listCtrl.Attach(GetDlgItem(IDC_LIST_ENV));
	m_listCtrl.SetExtendedListViewStyle(m_listCtrl.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_listCtrl.InsertColumn(0, L"Variable");
	m_listCtrl.InsertColumn(1, L"Options");
	m_listCtrl.InsertColumn(2, L"Value");

	m_listCtrl.SetColumnWidth(0, 100);
	m_listCtrl.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_listCtrl.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);

	m_editVariable.Attach(GetDlgItem(IDC_ENV_VARIABLE));
	m_editValue.Attach(GetDlgItem(IDC_ENV_VALUE));
	m_checkOverride.Attach(GetDlgItem(IDC_CHECK_ENV_OVERRIDE));
	m_checkAppend.Attach(GetDlgItem(IDC_CHECK_ENV_APPEND));

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PageSettingsTabsEnv::Load(shared_ptr<TabData>& tabData)
{
	m_tabData = tabData;

	DoDataExchange(DDX_LOAD);

	m_listCtrl.DeleteAllItems();
	for(size_t i = 0; i < m_tabData->environmentVariables.size(); ++i)
	{
		int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), m_tabData->environmentVariables[i]->strEnvVariable.c_str());
		m_listCtrl.SetCheckState(nItem, m_tabData->environmentVariables[i]->bEnvChecked);

		std::wstring options;
		if( m_tabData->environmentVariables[i]->bOverride ) options += L'O';
		if( m_tabData->environmentVariables[i]->bAppend ) options += L'A';
		m_listCtrl.SetItemText(nItem, 1, options.c_str());

		m_listCtrl.SetItemText(nItem, 2, m_tabData->environmentVariables[i]->strEnvValue.c_str());
	}
}

void PageSettingsTabsEnv::Save()
{
	DoDataExchange(DDX_SAVE);

	m_tabData->environmentVariables.clear();

	for(int nItem = 0; nItem < m_listCtrl.GetItemCount(); ++nItem)
	{
		CString strVariable;
		m_listCtrl.GetItemText(nItem, 0, strVariable);
		CString strOptions;
		m_listCtrl.GetItemText(nItem, 1, strOptions);
		CString strValue;
		m_listCtrl.GetItemText(nItem, 2, strValue);

		std::shared_ptr<VarEnv> varenv (new VarEnv);
		varenv->strEnvVariable = strVariable.GetString();
		varenv->strEnvValue = strValue.GetString();
		varenv->bEnvChecked = m_listCtrl.GetCheckState(nItem)? true : false;
		int idx = 0;
		if( idx < strOptions.GetLength() && strOptions.GetAt(idx) == L'O' )
		{
			varenv->bOverride = true;
			idx ++;
		}
		if( idx < strOptions.GetLength() && strOptions.GetAt(idx) == L'A' )
		{
			varenv->bAppend = true;
			idx ++;
		}
		m_tabData->environmentVariables.push_back(varenv);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT PageSettingsTabsEnv::OnClickedBtnEnvAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString strVariable;
	m_editVariable.GetWindowText(strVariable);

	CString strValue;
	m_editValue.GetWindowText(strValue);

	LVFINDINFO findInfo;
	findInfo.flags = LVFI_STRING;
	findInfo.psz = strVariable.GetString();
	int nItem = m_listCtrl.FindItem(&findInfo, -1);

	if( nItem == -1 )
	{
		nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), strVariable);
		m_listCtrl.SetCheckState(nItem, TRUE);
	}

	std::wstring options;
	if( m_checkOverride.GetCheck() ) options += L'O';
	if( m_checkAppend.GetCheck() ) options += L'A';
	m_listCtrl.SetItemText(nItem, 1, options.c_str());

	m_listCtrl.SetItemText(nItem, 2, strValue);

	return 0;
}

LRESULT PageSettingsTabsEnv::OnClickedBtnEnvDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nItem = m_listCtrl.GetSelectedIndex();
	if( nItem < 0 || nItem >= m_listCtrl.GetItemCount() ) return 0;

	m_listCtrl.DeleteItem(nItem);

	return 0;
}

LRESULT PageSettingsTabsEnv::OnClickedBtnEnvEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nItem = m_listCtrl.GetSelectedIndex();
	if( nItem < 0 || nItem >= m_listCtrl.GetItemCount() ) return 0;

	CString strVariable;
	m_listCtrl.GetItemText(nItem, 0, strVariable);
	CString strOptions;
	m_listCtrl.GetItemText(nItem, 1, strOptions);
	CString strValue;
	m_listCtrl.GetItemText(nItem, 2, strValue);

	m_editVariable.SetWindowText(strVariable);
	m_editValue.SetWindowText(strValue);

	bool bOverride = false;
	bool bAppend = false;
	int idx = 0;
	if( idx < strOptions.GetLength() && strOptions.GetAt(idx) == L'O' )
	{
		bOverride = true;
		idx ++;
	}
	if( idx < strOptions.GetLength() && strOptions.GetAt(idx) == L'A' )
	{
		bAppend = true;
		idx ++;
	}

	m_checkOverride.SetCheck(bOverride);
	m_checkAppend.SetCheck(bAppend);

	return 0;
}

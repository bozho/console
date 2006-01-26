#include "stdafx.h"
#include "resource.h"

#include "SettingsHandler.h"

//////////////////////////////////////////////////////////////////////////////

extern shared_ptr<ImageHandler>		g_imageHandler;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HotKeys::HotKeys()
: vecHotKeys()
{
	mapCommands.insert(CommandsMap::value_type(L"newtab1",		ID_NEW_TAB_1));
	mapCommands.insert(CommandsMap::value_type(L"newtab2",		ID_NEW_TAB_1 + 1));
	mapCommands.insert(CommandsMap::value_type(L"newtab3",		ID_NEW_TAB_1 + 2));
	mapCommands.insert(CommandsMap::value_type(L"newtab4",		ID_NEW_TAB_1 + 3));
	mapCommands.insert(CommandsMap::value_type(L"newtab5",		ID_NEW_TAB_1 + 4));
	mapCommands.insert(CommandsMap::value_type(L"newtab6",		ID_NEW_TAB_1 + 5));
	mapCommands.insert(CommandsMap::value_type(L"newtab7",		ID_NEW_TAB_1 + 6));
	mapCommands.insert(CommandsMap::value_type(L"newtab8",		ID_NEW_TAB_1 + 7));
	mapCommands.insert(CommandsMap::value_type(L"newtab9",		ID_NEW_TAB_1 + 8));
	mapCommands.insert(CommandsMap::value_type(L"newtab10",		ID_NEW_TAB_1 + 9));

	mapCommands.insert(CommandsMap::value_type(L"switchtab1",	ID_SWITCH_TAB_1));
	mapCommands.insert(CommandsMap::value_type(L"switchtab2",	ID_SWITCH_TAB_1 + 1));
	mapCommands.insert(CommandsMap::value_type(L"switchtab3",	ID_SWITCH_TAB_1 + 2));
	mapCommands.insert(CommandsMap::value_type(L"switchtab4",	ID_SWITCH_TAB_1 + 3));
	mapCommands.insert(CommandsMap::value_type(L"switchtab5",	ID_SWITCH_TAB_1 + 4));
	mapCommands.insert(CommandsMap::value_type(L"switchtab6",	ID_SWITCH_TAB_1 + 5));
	mapCommands.insert(CommandsMap::value_type(L"switchtab7",	ID_SWITCH_TAB_1 + 6));
	mapCommands.insert(CommandsMap::value_type(L"switchtab8",	ID_SWITCH_TAB_1 + 7));
	mapCommands.insert(CommandsMap::value_type(L"switchtab9",	ID_SWITCH_TAB_1 + 8));
	mapCommands.insert(CommandsMap::value_type(L"switchtab10",	ID_SWITCH_TAB_1 + 9));

	mapCommands.insert(CommandsMap::value_type(L"nexttab",		ID_NEXT_TAB));
	mapCommands.insert(CommandsMap::value_type(L"prevtab",		ID_PREV_TAB));

	mapCommands.insert(CommandsMap::value_type(L"closetab",		ID_FILE_CLOSE_TAB));

	mapCommands.insert(CommandsMap::value_type(L"copy",			ID_COPY));
	mapCommands.insert(CommandsMap::value_type(L"paste",		ID_PASTE));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

SettingsHandler::SettingsHandler()
{
}

SettingsHandler::~SettingsHandler()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool SettingsHandler::LoadOptions(const wstring& strOptionsFileName) {

	HRESULT hr					= S_OK;
	VARIANT_BOOL bLoadSuccess	= false;
	
	hr = m_pOptionsDocument.CoCreateInstance(__uuidof(DOMDocument));
	if (FAILED(hr) || (m_pOptionsDocument.p == NULL)) return false;

	if (strOptionsFileName.length() > 0) {
		hr = m_pOptionsDocument->load(CComVariant(strOptionsFileName.c_str()), &bLoadSuccess);
	}

	if (FAILED(hr) || (!bLoadSuccess)) {

		wchar_t szModuleFileName[MAX_PATH + 1];
		::ZeroMemory(szModuleFileName, (MAX_PATH+1)*sizeof(wchar_t));
		::GetModuleFileName(NULL, szModuleFileName, MAX_PATH);

		wstring strModuleFileName(szModuleFileName);
		wstring strDefaultOptionsFileName(strModuleFileName.substr(0, strModuleFileName.rfind(L'\\')));

		strDefaultOptionsFileName += L"\\console.xml";

		hr = m_pOptionsDocument->load(CComVariant(strDefaultOptionsFileName.c_str()), &bLoadSuccess);
		if (FAILED(hr) || (!bLoadSuccess)) return false;
	}

	// load settings' sections
	LoadConsoleSettings();
	LoadAppearanceSettings();
	LoadHotKeys();
	LoadTabSettings();

	return true;

}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadConsoleSettings() {

	CComPtr<IXMLDOMElement>	pConsoleElement;

	if (FAILED(GetDomElement(NULL, CComBSTR(L"settings/console"), pConsoleElement))) return;

	GetAttribute(pConsoleElement, CComBSTR(L"shell"), m_consoleSettings.strShell, wstring(L""));
	GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), m_consoleSettings.strInitialDir, wstring(L""));
	GetAttribute(pConsoleElement, CComBSTR(L"refresh"), m_consoleSettings.dwRefreshInterval, 100);
	GetAttribute(pConsoleElement, CComBSTR(L"change_refresh"), m_consoleSettings.dwChangeRefreshInterval, 10);
	GetAttribute(pConsoleElement, CComBSTR(L"rows"), m_consoleSettings.dwRows, 25);
	GetAttribute(pConsoleElement, CComBSTR(L"columns"), m_consoleSettings.dwColumns, 80);
	GetAttribute(pConsoleElement, CComBSTR(L"buffer_rows"), m_consoleSettings.dwBufferRows, m_consoleSettings.dwRows);
	GetAttribute(pConsoleElement, CComBSTR(L"buffer_columns"), m_consoleSettings.dwBufferColumns, m_consoleSettings.dwColumns);

	for (DWORD i = 0; i < 16; ++i) {

		CComPtr<IXMLDOMElement>	pFontColorElement;

		if (FAILED(GetDomElement(NULL, CComBSTR(str(wformat(L"settings/console/colors/color[%1%]") % i).c_str()), pFontColorElement))) continue;

		DWORD id;

		GetAttribute(pFontColorElement, CComBSTR(L"id"), id, i);
		GetRGBAttribute(pFontColorElement, m_consoleSettings.consoleColors[id], m_consoleSettings.consoleColors[i]);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadAppearanceSettings() {

	LoadFontSettings();
	LoadTransparencySettings();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadHotKeys() {

	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pHotKeyNodes;

	hr = m_pOptionsDocument->selectNodes(CComBSTR(L"settings/hotkeys/hotkey"), &pHotKeyNodes);
	if (FAILED(hr)) return;

	long	lListLength;
	pHotKeyNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i) {

		CComPtr<IXMLDOMNode>	pHotKeyNode;
		CComPtr<IXMLDOMElement>	pHotKeyElement;

		pHotKeyNodes->get_item(i, &pHotKeyNode);
		if (FAILED(pHotKeyNode.QueryInterface(&pHotKeyElement))) continue;

		wstring	strCommand(L"");
		bool	bShift;
		bool	bCtrl;
		bool	bAlt;
		DWORD	dwKeyCode;

		GetAttribute(pHotKeyElement, CComBSTR(L"command"), strCommand, wstring(L""));

		CommandsMap::iterator it = m_hotKeys.mapCommands.find(strCommand);
		if (it == m_hotKeys.mapCommands.end()) continue;

		GetAttribute(pHotKeyElement, CComBSTR(L"shift"), bShift, false);
		GetAttribute(pHotKeyElement, CComBSTR(L"ctrl"), bCtrl, false);
		GetAttribute(pHotKeyElement, CComBSTR(L"alt"), bAlt, false);
		GetAttribute(pHotKeyElement, CComBSTR(L"code"), dwKeyCode, 0);

		ACCEL	newAccel;
		::ZeroMemory(&newAccel, sizeof(ACCEL));

		if (bShift)	newAccel.fVirt |= FSHIFT;
		if (bCtrl)	newAccel.fVirt |= FCONTROL;
		if (bAlt)	newAccel.fVirt |= FALT;

		newAccel.fVirt	|= FVIRTKEY;
		newAccel.key	= static_cast<WORD>(dwKeyCode);
		newAccel.cmd	= it->second;

		m_hotKeys.vecHotKeys.push_back(newAccel);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadTabSettings() {

	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pTabNodes;

	hr = m_pOptionsDocument->selectNodes(CComBSTR(L"settings/tabs/tab"), &pTabNodes);
	if (FAILED(hr)) return;

	long	lListLength;
	pTabNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i) {

		CComPtr<IXMLDOMNode>	pTabNode;
		CComPtr<IXMLDOMElement>	pTabElement;

		pTabNodes->get_item(i, &pTabNode);
		if (FAILED(pTabNode.QueryInterface(&pTabElement))) continue;

		shared_ptr<TabSettings>	tabSettings(new TabSettings);
		CComPtr<IXMLDOMElement>	pConsoleElement;
		CComPtr<IXMLDOMElement>	pCursorElement;
		CComPtr<IXMLDOMElement>	pBackgroundElement;
		wstring					strIconFile(L"");

		GetAttribute(pTabElement, CComBSTR(L"name"), tabSettings->strName, L"Console");
		GetAttribute(pTabElement, CComBSTR(L"icon"), strIconFile, L"");

		m_tabSettings.push_back(tabSettings);

		// load icon
		if (strIconFile.length() > 0) {
			tabSettings->tabIcon.reset(new CIcon(static_cast<HICON>(::LoadImage(
															NULL, 
															strIconFile.c_str(), 
															IMAGE_ICON, 
															16, 
															16, 
															LR_DEFAULTCOLOR|LR_LOADFROMFILE))));
		} else {
			tabSettings->tabIcon.reset(new CIcon(static_cast<HICON>(::LoadImage(
															::GetModuleHandle(NULL), 
															MAKEINTRESOURCE(IDR_MAINFRAME), 
															IMAGE_ICON, 
															16, 
															16, 
															LR_DEFAULTCOLOR))));
		}

		if (SUCCEEDED(GetDomElement(pTabNode, CComBSTR(L"console"), pConsoleElement))) {

			GetAttribute(pConsoleElement, CComBSTR(L"shell"), tabSettings->strShell, m_consoleSettings.strShell);
			GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), tabSettings->strInitialDir, m_consoleSettings.strInitialDir);
		}

		if (SUCCEEDED(GetDomElement(pTabNode, CComBSTR(L"cursor"), pCursorElement))) {

			GetAttribute(pCursorElement, CComBSTR(L"style"), tabSettings->dwCursorStyle, 0);
			GetRGBAttribute(pCursorElement, tabSettings->crCursorColor, RGB(255, 255, 255));
		}

		if (SUCCEEDED(GetDomElement(pTabNode, CComBSTR(L"background"), pBackgroundElement))) {

			GetAttribute(pBackgroundElement, CComBSTR(L"image"), tabSettings->bImageBackground, false);
			GetRGBAttribute(pBackgroundElement, tabSettings->crBackgroundColor, RGB(0, 0, 0));

			if (tabSettings->bImageBackground) {

				// load image settings and let ImageHandler return appropriate bitmap
				CComPtr<IXMLDOMElement>	pImageElement;
				CComPtr<IXMLDOMElement>	pTintElement;

				wstring		strFilename(L"");
				bool		bRelative	= false;
				bool		bResize		= false;
				bool		bExtend		= false;
				COLORREF	crTint		= RGB(0, 0, 0);
				BYTE		byTintOpacity = 0;

				if (FAILED(GetDomElement(pTabNode, CComBSTR(L"background/image"), pImageElement))) return;

				GetAttribute(pImageElement, CComBSTR(L"file"), strFilename, wstring(L""));
				GetAttribute(pImageElement, CComBSTR(L"relative"), bRelative, false);
				GetAttribute(pImageElement, CComBSTR(L"resize"), bResize, false);
				GetAttribute(pImageElement, CComBSTR(L"extend"), bExtend, false);

				if (SUCCEEDED(GetDomElement(pTabNode, CComBSTR(L"background/image/tint"), pTintElement))) {
					GetRGBAttribute(pTintElement, crTint, RGB(0, 0, 0));
					GetAttribute(pTintElement, CComBSTR(L"opacity"), byTintOpacity, 0);
				}

				tabSettings->tabBackground = g_imageHandler->GetImageData(strFilename, bRelative, bResize, bExtend, crTint, byTintOpacity);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadFontSettings() {

	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(GetDomElement(NULL, CComBSTR(L"settings/appearance/font"), pFontElement))) return;

	GetAttribute(pFontElement, CComBSTR(L"name"), m_appearanceSettings.fontSettings.strName, wstring(L"Courier New"));
	GetAttribute(pFontElement, CComBSTR(L"size"), m_appearanceSettings.fontSettings.dwSize, 10);
	GetAttribute(pFontElement, CComBSTR(L"bold"), m_appearanceSettings.fontSettings.bBold, false);
	GetAttribute(pFontElement, CComBSTR(L"italic"), m_appearanceSettings.fontSettings.bItalic, false);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadTransparencySettings() {

	CComPtr<IXMLDOMElement>	pTransElement;

	if (FAILED(GetDomElement(NULL, CComBSTR(L"settings/appearance/transparency"), pTransElement))) return;

	GetAttribute(pTransElement, CComBSTR(L"style"), (DWORD&)m_appearanceSettings.transparencySettings.transStyle, static_cast<DWORD>(transNone));
	GetAttribute(pTransElement, CComBSTR(L"active_alpha"), m_appearanceSettings.transparencySettings.byActiveAlpha, 255);
	GetAttribute(pTransElement, CComBSTR(L"inactive_alpha"), m_appearanceSettings.transparencySettings.byInactiveAlpha, 255);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HRESULT SettingsHandler::GetDomElement(const CComPtr<IXMLDOMNode>& pRootNode, const CComBSTR& bstrPath, CComPtr<IXMLDOMElement>& pElement) {

	HRESULT					hr = S_OK;
	CComPtr<IXMLDOMNode>	pNode;
	
	if (pRootNode.p == NULL) {
		hr = m_pOptionsDocument->selectSingleNode(bstrPath, &pNode);
	} else {
		hr = pRootNode->selectSingleNode(bstrPath, &pNode);
	}

	if (hr != S_OK) return E_FAIL;

	return pNode.QueryInterface(&pElement);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD& dwValue, DWORD dwDefaultValue) {

	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK) {
		dwValue = _wtol(varValue.bstrVal);
	} else {
		dwValue = dwDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, BYTE& byValue, BYTE byDefaultValue) {

	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK) {
		byValue = static_cast<BYTE>(_wtoi(varValue.bstrVal));
	} else {
		byValue = byDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool& bValue, bool bDefaultValue) {

	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK) {
		bValue = (_wtol(varValue.bstrVal) > 0);
	} else {
		bValue = bDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, wstring& strValue, const wstring& strDefaultValue) {

	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK) {
		strValue = varValue.bstrVal;
	} else {
		strValue = strDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::GetRGBAttribute(const CComPtr<IXMLDOMElement>& pElement, COLORREF& crValue, COLORREF crDefaultValue) {

	DWORD r;
	DWORD g;
	DWORD b;

	GetAttribute(pElement, CComBSTR(L"r"), r, GetRValue(crDefaultValue));
	GetAttribute(pElement, CComBSTR(L"g"), g, GetGValue(crDefaultValue));
	GetAttribute(pElement, CComBSTR(L"b"), b, GetBValue(crDefaultValue));

	crValue = RGB(r, g, b);
}

//////////////////////////////////////////////////////////////////////////////


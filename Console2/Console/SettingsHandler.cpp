#include "stdafx.h"
#include "SettingsHandler.h"

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
	LoadFontSettings();
	LoadHotKeys();
	LoadTabSettings();

	return true;

}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

shared_ptr<TabSettings> SettingsHandler::GetTabSettings(const wstring& strId) {

	shared_ptr<TabSettings> tabSettings;

	TabSettingsMap::iterator it = m_tabSettings.find(strId);

	if (it != m_tabSettings.end()) tabSettings = it->second;
	
	return tabSettings;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadConsoleSettings() {

	CComPtr<IXMLDOMElement>	pConsoleElement;

	if (FAILED(GetDomElement(NULL, CComBSTR(L"console"), pConsoleElement))) return;

	GetAttribute(pConsoleElement, CComBSTR(L"refresh"), m_consoleSettings.dwRefreshInterval, 100);
	GetAttribute(pConsoleElement, CComBSTR(L"change_refresh"), m_consoleSettings.dwChangeRefreshInterval, 10);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadFontSettings() {

	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(GetDomElement(NULL, CComBSTR(L"console/font"), pFontElement))) return;

	GetAttribute(pFontElement, CComBSTR(L"name"), m_fontSettings.strName, wstring(L"Courier New"));
	GetAttribute(pFontElement, CComBSTR(L"size"), m_fontSettings.dwSize, 10);
	GetAttribute(pFontElement, CComBSTR(L"bold"), m_fontSettings.bBold, false);
	GetAttribute(pFontElement, CComBSTR(L"italic"), m_fontSettings.bItalic, false);

	for (DWORD i = 0; i < 16; ++i) {

		CComPtr<IXMLDOMElement>	pFontColorElement;

		if (FAILED(GetDomElement(NULL, CComBSTR(str(wformat(L"console/font/colors/color[%1%]") % i).c_str()), pFontColorElement))) continue;

		DWORD id;

		GetAttribute(pFontColorElement, CComBSTR(L"id"), id, i);
		GetRGBAttribute(pFontColorElement, m_fontSettings.consoleColors[id], m_fontSettings.consoleColors[i]);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadHotKeys() {

	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pHotKeyNodes;

	hr = m_pOptionsDocument->selectNodes(CComBSTR(L"console/hotkeys/hotkey"), &pHotKeyNodes);
	if (FAILED(hr)) return;

	WORD	wHotKeyCommand = ID_HOTKEY_FIRST;
	long	lListLength;
	pHotKeyNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i) {

		CComPtr<IXMLDOMNode>	pHotKeyNode;
		CComPtr<IXMLDOMElement>	pHotKeyElement;

		pHotKeyNodes->get_item(i, &pHotKeyNode);
		if (FAILED(pHotKeyNode.QueryInterface(&pHotKeyElement))) continue;

		bool	bShift;
		bool	bCtrl;
		bool	bAlt;
		bool	bVirtKey;
		DWORD	dwKeyCode;

		GetAttribute(pHotKeyElement, CComBSTR(L"shift"), bShift, false);
		GetAttribute(pHotKeyElement, CComBSTR(L"ctrl"), bCtrl, false);
		GetAttribute(pHotKeyElement, CComBSTR(L"alt"), bAlt, false);
		GetAttribute(pHotKeyElement, CComBSTR(L"virtkey"), bVirtKey, false);
		GetAttribute(pHotKeyElement, CComBSTR(L"code"), dwKeyCode, 0);

		shared_ptr<HotKey>	newHotKey(new HotKey);
		if (bShift)		newHotKey->hotKey.fVirt |= FSHIFT;
		if (bCtrl)		newHotKey->hotKey.fVirt |= FCONTROL;
		if (bAlt)		newHotKey->hotKey.fVirt |= FALT;
		if (bVirtKey)	newHotKey->hotKey.fVirt |= FVIRTKEY;

		newHotKey->hotKey.key	= static_cast<WORD>(dwKeyCode);
		newHotKey->hotKey.cmd	= wHotKeyCommand++;

		GetAttribute(pHotKeyElement, CComBSTR(L"command"), newHotKey->strCommand, wstring(L""));

		m_hotKeys.push_back(newHotKey);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::LoadTabSettings() {

	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pTabNodes;

	hr = m_pOptionsDocument->selectNodes(CComBSTR(L"console/tabs/tab"), &pTabNodes);
	if (FAILED(hr)) return;

	long	lListLength;
	pTabNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i) {

		CComPtr<IXMLDOMNode>	pTabNode;
		CComPtr<IXMLDOMElement>	pTabElement;

		pTabNodes->get_item(i, &pTabNode);
		if (FAILED(pTabNode.QueryInterface(&pTabElement))) continue;

		wstring strId;
		GetAttribute(pTabElement, CComBSTR(L"id"), strId, L"default");

		shared_ptr<TabSettings>	tabSettings(new TabSettings);
		CComPtr<IXMLDOMElement>	pCursorElement;

		if (SUCCEEDED(GetDomElement(pTabNode, CComBSTR(L"cursor"), pCursorElement))) {

			GetAttribute(pCursorElement, CComBSTR(L"style"), tabSettings->dwStyle, 0);
			GetRGBAttribute(pCursorElement, tabSettings->crColor, RGB(255, 255, 255));
		}

		m_tabSettings.insert(TabSettingsMap::value_type(strId, tabSettings));
	}
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
	if (FAILED(hr)) return hr;

	return pNode.QueryInterface(&pElement);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD& dwValue, DWORD dwDefaultValue) {

	CComVariant	varValue;
	HRESULT		hr = S_OK;

	hr = pElement->getAttribute(bstrName, &varValue);

	if (SUCCEEDED(hr)) {
		dwValue = _wtol(varValue.bstrVal);
	} else {
		dwValue = dwDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool& bValue, bool bDefaultValue) {

	CComVariant	varValue;
	HRESULT		hr = S_OK;

	hr = pElement->getAttribute(bstrName, &varValue);

	if (SUCCEEDED(hr)) {
		bValue = (_wtol(varValue.bstrVal) > 0);
	} else {
		bValue = bDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, wstring& strValue, const wstring& strDefaultValue) {

	CComVariant	varValue;
	HRESULT		hr = S_OK;

	hr = pElement->getAttribute(bstrName, &varValue);

	if (SUCCEEDED(hr)) {
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


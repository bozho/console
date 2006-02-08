#include "stdafx.h"
#include "resource.h"

#include "XmlHelper.h"
#include "SettingsHandler.h"

//////////////////////////////////////////////////////////////////////////////

extern shared_ptr<ImageHandler>		g_imageHandler;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ConsoleSettings::ConsoleSettings()
: strShell(L"")
, strInitialDir(L"")
, dwRefreshInterval(100)
, dwChangeRefreshInterval(10)
, dwRows(25)
, dwColumns(80)
, dwBufferRows(200)
, dwBufferColumns(80)
{
	defaultConsoleColors[0]	= 0x000000;
	defaultConsoleColors[1]	= 0x800000;
	defaultConsoleColors[2]	= 0x008000;
	defaultConsoleColors[3]	= 0x808000;
	defaultConsoleColors[4]	= 0x000080;
	defaultConsoleColors[5]	= 0x800080;
	defaultConsoleColors[6]	= 0x008080;
	defaultConsoleColors[7]	= 0xC0C0C0;
	defaultConsoleColors[8]	= 0x808080;
	defaultConsoleColors[9]	= 0xFF0000;
	defaultConsoleColors[10]= 0x00FF00;
	defaultConsoleColors[11]= 0xFFFF00;
	defaultConsoleColors[12]= 0x0000FF;
	defaultConsoleColors[13]= 0xFF00FF;
	defaultConsoleColors[14]= 0x00FFFF;
	defaultConsoleColors[15]= 0xFFFFFF;

	::CopyMemory(consoleColors, defaultConsoleColors, sizeof(COLORREF)*16);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	CComPtr<IXMLDOMElement>	pConsoleElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"console"), pConsoleElement))) return false;

	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"shell"), strShell, wstring(L""));
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), strInitialDir, wstring(L""));
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"refresh"), dwRefreshInterval, 100);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"change_refresh"), dwChangeRefreshInterval, 10);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"rows"), dwRows, 25);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"columns"), dwColumns, 80);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"buffer_rows"), dwBufferRows, 0);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"buffer_columns"), dwBufferColumns, 0);

	for (DWORD i = 0; i < 16; ++i) {

		CComPtr<IXMLDOMElement>	pFontColorElement;

		if (FAILED(XmlHelper::GetDomElement(pConsoleElement, CComBSTR(str(wformat(L"colors/color[%1%]") % i).c_str()), pFontColorElement))) continue;

		DWORD id;

		XmlHelper::GetAttribute(pFontColorElement, CComBSTR(L"id"), id, i);
		XmlHelper::GetRGBAttribute(pFontColorElement, consoleColors[id], consoleColors[i]);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	CComPtr<IXMLDOMElement>	pConsoleElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"console"), pConsoleElement))) return false;

	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"shell"), strShell);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"init_dir"), strInitialDir);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"refresh"), dwRefreshInterval);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"change_refresh"), dwChangeRefreshInterval);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"rows"), dwRows);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"columns"), dwColumns);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"buffer_rows"), dwBufferRows);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"buffer_columns"), dwBufferColumns);

	for (DWORD i = 0; i < 16; ++i) {

		CComPtr<IXMLDOMElement>	pFontColorElement;

		if (FAILED(XmlHelper::GetDomElement(pConsoleElement, CComBSTR(str(wformat(L"colors/color[%1%]") % i).c_str()), pFontColorElement))) continue;

		XmlHelper::SetAttribute(pFontColorElement, CComBSTR(L"id"), i);
		XmlHelper::SetRGBAttribute(pFontColorElement, consoleColors[i]);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

FontSettings::FontSettings()
: strName(L"Courier New")
, dwSize(10)
, bBold(false)
, bItalic(false)
{
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FontSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/font"), pFontElement))) return false;

	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"name"), strName, wstring(L"Courier New"));
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"size"), dwSize, 10);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"bold"), bBold, false);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"italic"), bItalic, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FontSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/font"), pFontElement))) return false;

	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"name"), strName);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"size"), dwSize);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"bold"), bBold);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"italic"), bItalic);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TransparencySettings::TransparencySettings()
: transType(transNone)
, byActiveAlpha(255)
, byInactiveAlpha(255)
, crColorKey(RGB(0, 0, 0))
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TransparencySettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	CComPtr<IXMLDOMElement>	pTransElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/transparency"), pTransElement))) return false;

	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"type"), reinterpret_cast<DWORD&>(transType), static_cast<DWORD>(transNone));
	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"active_alpha"), byActiveAlpha, 255);
	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"inactive_alpha"), byInactiveAlpha, 255);
	XmlHelper::GetRGBAttribute(pTransElement, crColorKey, RGB(0, 0, 0));

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TransparencySettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	CComPtr<IXMLDOMElement>	pTransElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/transparency"), pTransElement))) return false;

	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"type"), reinterpret_cast<DWORD&>(transType));
	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"active_alpha"), byActiveAlpha);
	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"inactive_alpha"), byInactiveAlpha);
	XmlHelper::SetRGBAttribute(pTransElement, crColorKey);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

AppearanceSettings::AppearanceSettings()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool AppearanceSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	fontSettings.Load(pOptionsRoot);
	transparencySettings.Load(pOptionsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool AppearanceSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	fontSettings.Save(pOptionsRoot);
	transparencySettings.Save(pOptionsRoot);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


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
	mapCommands.insert(CommandsMap::value_type(L"renametab",	ID_EDIT_RENAME_TAB));

	mapCommands.insert(CommandsMap::value_type(L"copy",			ID_COPY));
	mapCommands.insert(CommandsMap::value_type(L"paste",		ID_PASTE));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool HotKeys::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pHotKeyNodes;

	hr = pOptionsRoot->selectNodes(CComBSTR(L"hotkeys/hotkey"), &pHotKeyNodes);
	if (FAILED(hr)) return false;

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

		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"command"), strCommand, wstring(L""));

		CommandsMap::iterator it = mapCommands.find(strCommand);
		if (it == mapCommands.end()) continue;

		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"shift"), bShift, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"ctrl"), bCtrl, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"alt"), bAlt, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"code"), dwKeyCode, 0);

		ACCEL	newAccel;
		::ZeroMemory(&newAccel, sizeof(ACCEL));

		if (bShift)	newAccel.fVirt |= FSHIFT;
		if (bCtrl)	newAccel.fVirt |= FCONTROL;
		if (bAlt)	newAccel.fVirt |= FALT;

		newAccel.fVirt	|= FVIRTKEY;
		newAccel.key	= static_cast<WORD>(dwKeyCode);
		newAccel.cmd	= it->second;

		vecHotKeys.push_back(newAccel);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool HotKeys::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TabSettings::TabSettings(const ConsoleSettings& conSettings)
: consoleSettings(conSettings)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pTabNodes;

	hr = pOptionsRoot->selectNodes(CComBSTR(L"tabs/tab"), &pTabNodes);
	if (FAILED(hr)) return false;

	long	lListLength;
	pTabNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i) {

		CComPtr<IXMLDOMNode>	pTabNode;
		CComPtr<IXMLDOMElement>	pTabElement;

		pTabNodes->get_item(i, &pTabNode);
		if (FAILED(pTabNode.QueryInterface(&pTabElement))) continue;

		shared_ptr<TabData>	tabData(new TabData(consoleSettings));
		CComPtr<IXMLDOMElement>	pConsoleElement;
		CComPtr<IXMLDOMElement>	pCursorElement;
		CComPtr<IXMLDOMElement>	pBackgroundElement;
		wstring					strIconFile(L"");

		XmlHelper::GetAttribute(pTabElement, CComBSTR(L"name"), tabData->strName, L"Console");
		XmlHelper::GetAttribute(pTabElement, CComBSTR(L"icon"), strIconFile, L"");

		tabDataVector.push_back(tabData);

		// load icon
		if (strIconFile.length() > 0) {

			tabData->tabIcon = static_cast<HICON>(::LoadImage(
														NULL, 
														strIconFile.c_str(), 
														IMAGE_ICON, 
														16, 
														16, 
														LR_DEFAULTCOLOR|LR_LOADFROMFILE));
		} else {

			tabData->tabIcon = static_cast<HICON>(::LoadImage(
														::GetModuleHandle(NULL), 
														MAKEINTRESOURCE(IDR_MAINFRAME), 
														IMAGE_ICON, 
														16, 
														16, 
														LR_DEFAULTCOLOR));
		}

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"console"), pConsoleElement))) {

			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"shell"), tabData->strShell, consoleSettings.strShell);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), tabData->strInitialDir, consoleSettings.strInitialDir);
		}

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"cursor"), pCursorElement))) {

			XmlHelper::GetAttribute(pCursorElement, CComBSTR(L"style"), tabData->dwCursorStyle, 0);
			XmlHelper::GetRGBAttribute(pCursorElement, tabData->crCursorColor, RGB(255, 255, 255));
		}

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background"), pBackgroundElement))) {

			XmlHelper::GetAttribute(pBackgroundElement, CComBSTR(L"image"), tabData->bImageBackground, false);
			XmlHelper::GetRGBAttribute(pBackgroundElement, tabData->crBackgroundColor, RGB(0, 0, 0));

			if (tabData->bImageBackground) {

				// load image settings and let ImageHandler return appropriate bitmap
				CComPtr<IXMLDOMElement>	pImageElement;
				CComPtr<IXMLDOMElement>	pTintElement;

				wstring		strFilename(L"");
				bool		bRelative	= false;
				bool		bResize		= false;
				bool		bExtend		= false;
				COLORREF	crTint		= RGB(0, 0, 0);
				BYTE		byTintOpacity = 0;

				if (FAILED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background/image"), pImageElement))) return false;

				XmlHelper::GetAttribute(pImageElement, CComBSTR(L"file"), strFilename, wstring(L""));
				XmlHelper::GetAttribute(pImageElement, CComBSTR(L"relative"), bRelative, false);
				XmlHelper::GetAttribute(pImageElement, CComBSTR(L"resize"), bResize, false);
				XmlHelper::GetAttribute(pImageElement, CComBSTR(L"extend"), bExtend, false);

				if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background/image/tint"), pTintElement))) {
					XmlHelper::GetRGBAttribute(pTintElement, crTint, RGB(0, 0, 0));
					XmlHelper::GetAttribute(pTintElement, CComBSTR(L"opacity"), byTintOpacity, 0);
				}

				tabData->tabBackground = g_imageHandler->GetImageData(
															strFilename, 
															bRelative, 
															bResize, 
															bExtend, 
															crTint, 
															byTintOpacity);
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot) {

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

SettingsHandler::SettingsHandler()
: m_pOptionsDocument()
, m_pOptionsRoot()
, m_tabSettings(m_consoleSettings)
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

bool SettingsHandler::LoadSettings(const wstring& strOptionsFileName) {

	HRESULT hr = S_OK;

	hr = XmlHelper::OpenXmlDocument(strOptionsFileName, L"console.xml", m_pOptionsDocument, m_pOptionsRoot);
	if (FAILED(hr)) return false;

	// load settings' sections
	m_consoleSettings.Load(m_pOptionsRoot);
	m_appearanceSettings.Load(m_pOptionsRoot);
	m_hotKeys.Load(m_pOptionsRoot);
	m_tabSettings.Load(m_pOptionsRoot);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


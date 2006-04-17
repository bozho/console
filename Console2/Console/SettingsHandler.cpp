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

bool ConsoleSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
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

	for (DWORD i = 0; i < 16; ++i)
	{
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

bool ConsoleSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
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

	for (DWORD i = 0; i < 16; ++i)
	{
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
, bUseColor(false)
, crFontColor(0)
{
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FontSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/font"), pFontElement))) return false;

	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"name"), strName, wstring(L"Courier New"));
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"size"), dwSize, 10);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"bold"), bBold, false);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"italic"), bItalic, false);

	CComPtr<IXMLDOMElement>	pColorElement;

	if (FAILED(XmlHelper::GetDomElement(pFontElement, CComBSTR(L"color"), pColorElement))) return false;

	XmlHelper::GetAttribute(pColorElement, CComBSTR(L"use"), bUseColor, false);
	XmlHelper::GetRGBAttribute(pColorElement, crFontColor, RGB(0, 0, 0));

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool FontSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/font"), pFontElement))) return false;

	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"name"), strName);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"size"), dwSize);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"bold"), bBold);
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"italic"), bItalic);

	CComPtr<IXMLDOMElement>	pColorElement;

	if (FAILED(XmlHelper::GetDomElement(pFontElement, CComBSTR(L"color"), pColorElement))) return false;

	XmlHelper::SetAttribute(pColorElement, CComBSTR(L"use"), bUseColor);
	XmlHelper::SetRGBAttribute(pColorElement, crFontColor);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

WindowSettings::WindowSettings()
: strTitle(L"Console")
, strIcon(L"")
, bUseTabIcon(false)
, bShowCommand(true)
, bShowCommandInTabs(true)
, bUseTabTitles(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool WindowSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pWindowElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/window"), pWindowElement))) return false;

	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"title"), strTitle, wstring(L"Console"));
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"icon"), strIcon, wstring(L""));
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"use_tab_icon"), bUseTabIcon, false);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"show_cmd"), bShowCommand, true);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"show_cmd_tabs"), bShowCommandInTabs, true);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"use_tab_title"), bUseTabTitles, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool WindowSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pWindowElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/window"), pWindowElement))) return false;

	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"title"), strTitle);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"icon"), strIcon);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"use_tab_icon"), bUseTabIcon);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"show_cmd"), bShowCommand);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"show_cmd_tabs"), bShowCommandInTabs);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"use_tab_title"), bUseTabTitles);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ControlsSettings::ControlsSettings()
: bShowMenu(true)
, bShowToolbar(true)
, bShowTabs(true)
, bShowStatusbar(true)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ControlsSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pCtrlsElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/controls"), pCtrlsElement))) return false;

	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_menu"), bShowMenu, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_toolbar"), bShowToolbar, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_tabs"), bShowTabs, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_statusbar"), bShowStatusbar, true);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ControlsSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pCtrlsElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/controls"), pCtrlsElement))) return false;

	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_menu"), bShowMenu);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_toolbar"), bShowToolbar);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_tabs"), bShowTabs);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_statusbar"), bShowStatusbar);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

StylesSettings::StylesSettings()
: bCaption(true)
, bResizable(true)
, bTaskbarButton(true)
, bBorder(true)
, dwInsideBoder(2)
, bTrayIcon(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool StylesSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pStylesElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/styles"), pStylesElement))) return false;

	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"caption"), bCaption, true);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"resizable"), bResizable, true);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"taskbar_button"), bTaskbarButton, true);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"border"), bBorder, true);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"inside_border"), dwInsideBoder, 2);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"tray_icon"), bTrayIcon, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool StylesSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pStylesElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/styles"), pStylesElement))) return false;

	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"caption"), bCaption);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"resizable"), bResizable);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"taskbar_button"), bTaskbarButton);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"border"), bBorder);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"inside_border"), dwInsideBoder);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"tray_icon"), bTrayIcon);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

PositionSettings::PositionSettings()
: nX(-1)
, nY(-1)
, zOrder(zorderNormal)
, dockPosition(dockNone)
, nSnapDistance(-1)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool PositionSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pPositionElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/position"), pPositionElement))) return false;

	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"x"), nX, -1);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"y"), nY, -1);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"z_order"), reinterpret_cast<int&>(zOrder), static_cast<int>(zorderNormal));
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"dock"), reinterpret_cast<int&>(dockPosition), static_cast<int>(dockNone));
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"snap"), nSnapDistance, -1);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool PositionSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pPositionElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"appearance/position"), pPositionElement))) return false;

	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"x"), nX);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"y"), nY);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"z_order"), static_cast<int>(zOrder));
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"dock"), static_cast<int>(dockPosition));
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"snap"), nSnapDistance);

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

bool TransparencySettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
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

bool TransparencySettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
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

bool AppearanceSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	fontSettings.Load(pOptionsRoot);
	windowSettings.Load(pOptionsRoot);
	controlsSettings.Load(pOptionsRoot);
	stylesSettings.Load(pOptionsRoot);
	positionSettings.Load(pOptionsRoot);
	transparencySettings.Load(pOptionsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool AppearanceSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	fontSettings.Save(pOptionsRoot);
	windowSettings.Save(pOptionsRoot);
	controlsSettings.Save(pOptionsRoot);
	stylesSettings.Save(pOptionsRoot);
	positionSettings.Save(pOptionsRoot);
	transparencySettings.Save(pOptionsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CopyPasteSettings::CopyPasteSettings()
: bCopyOnSelect(false)
, bNoWrap(false)
, bTrimSpaces(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CopyPasteSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pCopyPasteElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"behavior/copy_paste"), pCopyPasteElement))) return false;

	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"copy_on_select"), bCopyOnSelect, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"no_wrap"), bNoWrap, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"trim_spaces"), bTrimSpaces, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CopyPasteSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pCopyPasteElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"behavior/copy_paste"), pCopyPasteElement))) return false;

	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"copy_on_select"), bCopyOnSelect);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"no_wrap"), bNoWrap);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"trim_spaces"), bTrimSpaces);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

MouseDragSettings::MouseDragSettings()
: bMouseDrag(false)
, bInverseShift(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MouseDragSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pMouseDragElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"behavior/mouse_drag"), pMouseDragElement))) return false;

	XmlHelper::GetAttribute(pMouseDragElement, CComBSTR(L"on"), bMouseDrag, false);
	XmlHelper::GetAttribute(pMouseDragElement, CComBSTR(L"inverse_shift"), bInverseShift, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MouseDragSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	CComPtr<IXMLDOMElement>	pMouseDragElement;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"behavior/mouse_drag"), pMouseDragElement))) return false;

	XmlHelper::SetAttribute(pMouseDragElement, CComBSTR(L"on"), bMouseDrag);
	XmlHelper::SetAttribute(pMouseDragElement, CComBSTR(L"inverse_shift"), bInverseShift);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BehaviorSettings::BehaviorSettings()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool BehaviorSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	copyPasteSettings.Load(pOptionsRoot);
	mouseDragSettings.Load(pOptionsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool BehaviorSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	copyPasteSettings.Save(pOptionsRoot);
	mouseDragSettings.Save(pOptionsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HotKeys::HotKeys()
: mapHotKeys()
{
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab1",		L"New Tab 1",			ID_NEW_TAB_1)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab2",		L"New Tab 2",			ID_NEW_TAB_1 + 1)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab3",		L"New Tab 3",			ID_NEW_TAB_1 + 2)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab4",		L"New Tab 4",			ID_NEW_TAB_1 + 3)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab5",		L"New Tab 5",			ID_NEW_TAB_1 + 4)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab6",		L"New Tab 6",			ID_NEW_TAB_1 + 5)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab7",		L"New Tab 7",			ID_NEW_TAB_1 + 6)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab8",		L"New Tab 8",			ID_NEW_TAB_1 + 7)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab9",		L"New Tab 9",			ID_NEW_TAB_1 + 8)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab10",		L"New Tab 10",			ID_NEW_TAB_1 + 9)));
                                                                                      
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab1",	L"Switch to tab 1",		ID_SWITCH_TAB_1)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab2",	L"Switch to tab 2",		ID_SWITCH_TAB_1 + 1)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab3",	L"Switch to tab 3",		ID_SWITCH_TAB_1 + 2)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab4",	L"Switch to tab 4",		ID_SWITCH_TAB_1 + 3)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab5",	L"Switch to tab 5",		ID_SWITCH_TAB_1 + 4)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab6",	L"Switch to tab 6",		ID_SWITCH_TAB_1 + 5)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab7",	L"Switch to tab 7",		ID_SWITCH_TAB_1 + 6)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab8",	L"Switch to tab 8",		ID_SWITCH_TAB_1 + 7)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab9",	L"Switch to tab 9",		ID_SWITCH_TAB_1 + 8)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab10",	L"Switch to tab 10",	ID_SWITCH_TAB_1 + 9)));

	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"nexttab",		L"Next tab",			ID_NEXT_TAB)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"prevtab",		L"Previous tab",		ID_PREV_TAB)));

	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"closetab",		L"Close tab",			ID_FILE_CLOSE_TAB)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"renametab",		L"Rename tab",			ID_EDIT_RENAME_TAB)));

	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"copy",			L"Copy",				ID_EDIT_COPY)));
	vecCommands.push_back(shared_ptr<CommandData>(new CommandData(L"paste",			L"Paste",				ID_EDIT_PASTE)));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool HotKeys::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pHotKeyNodes;

	hr = pOptionsRoot->selectNodes(CComBSTR(L"hotkeys/hotkey"), &pHotKeyNodes);
	if (FAILED(hr)) return false;

	long	lListLength;
	pHotKeyNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i)
	{
		CComPtr<IXMLDOMNode>	pHotKeyNode;
		CComPtr<IXMLDOMElement>	pHotKeyElement;

		pHotKeyNodes->get_item(i, &pHotKeyNode);
		if (FAILED(pHotKeyNode.QueryInterface(&pHotKeyElement))) continue;

		wstring	strCommand(L"");
		bool	bShift;
		bool	bCtrl;
		bool	bAlt;
		bool	bExtended;
		DWORD	dwKeyCode;

		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"command"), strCommand, wstring(L""));

		CommandsVector::iterator it = vecCommands.begin();
		for (; (it != vecCommands.end()) && (strCommand != (*it)->strCommand); ++it);
		if (it == vecCommands.end()) continue;

		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"shift"), bShift, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"ctrl"), bCtrl, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"alt"), bAlt, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"extended"), bExtended, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"code"), dwKeyCode, 0);

		ACCEL	newAccel;
		::ZeroMemory(&newAccel, sizeof(ACCEL));

		if (bShift)	newAccel.fVirt |= FSHIFT;
		if (bCtrl)	newAccel.fVirt |= FCONTROL;
		if (bAlt)	newAccel.fVirt |= FALT;

		newAccel.fVirt	|= FVIRTKEY;
		newAccel.key	= static_cast<WORD>(dwKeyCode);
		newAccel.cmd	= (*it)->wCommandID;

		shared_ptr<HotkeyData>	hotKeyData(new HotkeyData((*it)->wCommandID, newAccel, bExtended));

		mapHotKeys.insert(HotKeysMap::value_type(hotKeyData->dwCommandID, hotKeyData));
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool HotKeys::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMElement>		pHotkeysElement;
	CComPtr<IXMLDOMNodeList>	pHotKeyNodes;

	if (FAILED(XmlHelper::GetDomElement(pOptionsRoot, CComBSTR(L"hotkeys"), pHotkeysElement))) return false;

	if (SUCCEEDED(pHotkeysElement->selectNodes(CComBSTR(L"hotkey"), &pHotKeyNodes)))
	{
		long	lListLength;
		pHotKeyNodes->get_length(&lListLength);

		for (long i = 0; i < lListLength; ++i)
		{
			CComPtr<IXMLDOMNode>	pHotKeyNode;
			CComPtr<IXMLDOMNode>	pRemovedHotKeyNode;
			if (FAILED(pHotKeyNodes->get_item(i, &pHotKeyNode))) continue; 

			hr = pHotkeysElement->removeChild(pHotKeyNode, &pRemovedHotKeyNode);
		}

		CComPtr<IXMLDOMDocument>	pSettingsDoc;
		CommandsVector::iterator	itCommand;
		CommandsVector::iterator	itLastCommand = vecCommands.end() - 1;

		pHotkeysElement->get_ownerDocument(&pSettingsDoc);

		for (itCommand = vecCommands.begin(); itCommand != vecCommands.end(); ++itCommand)
		{
			CComPtr<IXMLDOMElement>	pNewHotkeyElement;
			CComPtr<IXMLDOMNode>	pNewHotkeyOut;
			HotKeysMap::iterator	itHotkey = mapHotKeys.find((*itCommand)->wCommandID);
			CComVariant				varAttrVal;

			if (itHotkey == mapHotKeys.end()) continue;

			pSettingsDoc->createElement(CComBSTR(L"hotkey"), &pNewHotkeyElement);

			varAttrVal = (itHotkey->second->accelHotkey.fVirt & FCONTROL) ? L"1" : L"0";
            pNewHotkeyElement->setAttribute(CComBSTR(L"ctrl"), varAttrVal);

			varAttrVal = (itHotkey->second->accelHotkey.fVirt & FSHIFT) ? L"1" : L"0";
			pNewHotkeyElement->setAttribute(CComBSTR(L"shift"), varAttrVal);

			varAttrVal = (itHotkey->second->accelHotkey.fVirt & FALT) ? L"1" : L"0";
			pNewHotkeyElement->setAttribute(CComBSTR(L"alt"), varAttrVal);

			varAttrVal = (itHotkey->second->bExtended) ? L"1" : L"0";
			pNewHotkeyElement->setAttribute(CComBSTR(L"extended"), varAttrVal);

			pNewHotkeyElement->setAttribute(CComBSTR(L"code"), CComVariant(itHotkey->second->accelHotkey.key));

			pNewHotkeyElement->setAttribute(CComBSTR(L"command"), CComVariant((*itCommand)->strCommand.c_str()));

			pHotkeysElement->appendChild(pNewHotkeyElement, &pNewHotkeyOut);

			// this is just for pretty printing
			CComPtr<IXMLDOMText>	pDomText;
			CComPtr<IXMLDOMNode>	pDomTextOut;

			if (itCommand == itLastCommand)
			{
				pSettingsDoc->createTextNode(CComBSTR(L"\n\t"), &pDomText);
			}
			else
			{
				pSettingsDoc->createTextNode(CComBSTR(L"\n\t\t"), &pDomText);
			}

			pHotkeysElement->appendChild(pDomText, &pDomTextOut);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TabSettings::TabSettings()
: strDefaultShell(L"")
, strDefaultInitialDir(L"")
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabSettings::Load(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pTabNodes;

	hr = pOptionsRoot->selectNodes(CComBSTR(L"tabs/tab"), &pTabNodes);
	if (FAILED(hr)) return false;

	long	lListLength;
	pTabNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i)
	{
		CComPtr<IXMLDOMNode>	pTabNode;
		CComPtr<IXMLDOMElement>	pTabElement;

		pTabNodes->get_item(i, &pTabNode);
		if (FAILED(pTabNode.QueryInterface(&pTabElement))) continue;

		shared_ptr<TabData>	tabData(new TabData(strDefaultShell, strDefaultInitialDir));
		CComPtr<IXMLDOMElement>	pConsoleElement;
		CComPtr<IXMLDOMElement>	pCursorElement;
		CComPtr<IXMLDOMElement>	pBackgroundElement;

		XmlHelper::GetAttribute(pTabElement, CComBSTR(L"title"), tabData->strTitle, L"Console");
		XmlHelper::GetAttribute(pTabElement, CComBSTR(L"icon"), tabData->strIcon, L"");

		tabDataVector.push_back(tabData);

		// load icon
		if (tabData->strIcon.length() > 0)
		{
			tabData->tabIcon = static_cast<HICON>(::LoadImage(
															NULL, 
															tabData->strIcon.c_str(), 
															IMAGE_ICON, 
															0, 
															0, 
															LR_DEFAULTCOLOR|LR_LOADFROMFILE|LR_DEFAULTSIZE));

			tabData->tabSmallIcon = static_cast<HICON>(::LoadImage(
															NULL, 
															tabData->strIcon.c_str(), 
															IMAGE_ICON, 
															16, 
															16, 
															LR_DEFAULTCOLOR|LR_LOADFROMFILE));
		}
		else
		{
			tabData->tabIcon = static_cast<HICON>(::LoadImage(
															::GetModuleHandle(NULL), 
															MAKEINTRESOURCE(IDR_MAINFRAME), 
															IMAGE_ICON, 
															0, 
															0, 
															LR_DEFAULTCOLOR|LR_DEFAULTSIZE));

			tabData->tabSmallIcon = static_cast<HICON>(::LoadImage(
															::GetModuleHandle(NULL), 
															MAKEINTRESOURCE(IDR_MAINFRAME), 
															IMAGE_ICON, 
															16, 
															16, 
															LR_DEFAULTCOLOR));
		}

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"console"), pConsoleElement)))
		{
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"shell"), tabData->strShell, strDefaultShell);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), tabData->strInitialDir, strDefaultInitialDir);
		}

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"cursor"), pCursorElement)))
		{
			XmlHelper::GetAttribute(pCursorElement, CComBSTR(L"style"), tabData->dwCursorStyle, 0);
			XmlHelper::GetRGBAttribute(pCursorElement, tabData->crCursorColor, RGB(255, 255, 255));
		}

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background"), pBackgroundElement)))
		{
			DWORD dwBackgroundImageType = 0;

			XmlHelper::GetAttribute(pBackgroundElement, CComBSTR(L"type"), dwBackgroundImageType, 0);
			XmlHelper::GetRGBAttribute(pBackgroundElement, tabData->crBackgroundColor, RGB(0, 0, 0));

			tabData->backgroundImageType = static_cast<BackgroundImageType>(dwBackgroundImageType);

			if (tabData->backgroundImageType != bktypeNone)
			{
				// load image settings and let ImageHandler return appropriate bitmap
				CComPtr<IXMLDOMElement>	pImageElement;
				CComPtr<IXMLDOMElement>	pTintElement;

				if (FAILED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background/image"), pImageElement))) return false;

				if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"background/image/tint"), pTintElement)))
				{
					XmlHelper::GetRGBAttribute(pTintElement, tabData->imageData.crTint, RGB(0, 0, 0));
					XmlHelper::GetAttribute(pTintElement, CComBSTR(L"opacity"), tabData->imageData.byTintOpacity, 0);
				}

				if (tabData->backgroundImageType == bktypeImage)
				{
					DWORD dwImagePosition = 0;

					XmlHelper::GetAttribute(pImageElement, CComBSTR(L"file"), tabData->imageData.strFilename, wstring(L""));
					XmlHelper::GetAttribute(pImageElement, CComBSTR(L"relative"), tabData->imageData.bRelative, false);
					XmlHelper::GetAttribute(pImageElement, CComBSTR(L"extend"), tabData->imageData.bExtend, false);
					XmlHelper::GetAttribute(pImageElement, CComBSTR(L"position"), dwImagePosition, 0);

					tabData->imageData.imagePosition = static_cast<ImagePosition>(dwImagePosition);
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabSettings::Save(const CComPtr<IXMLDOMElement>& pOptionsRoot)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void TabSettings::SetDefaults(const wstring& defaultShell, const wstring& defaultInitialDir)
{
	strDefaultShell		= defaultShell;
	strDefaultInitialDir= defaultInitialDir;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

SettingsHandler::SettingsHandler()
: m_pOptionsDocument()
, m_pOptionsRoot()
, m_strSettingsFileName(L"")
, m_tabSettings()
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

bool SettingsHandler::LoadSettings(const wstring& strSettingsFileName)
{
	HRESULT hr = S_OK;

	m_strSettingsFileName = strSettingsFileName;

	hr = XmlHelper::OpenXmlDocument(m_strSettingsFileName, L"console.xml", m_pOptionsDocument, m_pOptionsRoot);
	if (FAILED(hr)) return false;

	// load settings' sections
	m_consoleSettings.Load(m_pOptionsRoot);
	m_appearanceSettings.Load(m_pOptionsRoot);
	m_behaviorSettings.Load(m_pOptionsRoot);
	m_hotKeys.Load(m_pOptionsRoot);

	m_tabSettings.SetDefaults(m_consoleSettings.strShell, m_consoleSettings.strInitialDir);
	m_tabSettings.Load(m_pOptionsRoot);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


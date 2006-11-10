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

void SettingsBase::AddTextNode(CComPtr<IXMLDOMDocument>& pDoc, CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrText)
{
	CComPtr<IXMLDOMText>	pDomText;
	CComPtr<IXMLDOMNode>	pDomTextOut;

	pDoc->createTextNode(bstrText, &pDomText);
	pElement->appendChild(pDomText, &pDomTextOut);
}

//////////////////////////////////////////////////////////////////////////////


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
, bSaveSize(false)
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

bool ConsoleSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pConsoleElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"console"), pConsoleElement))) return false;

	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"shell"), strShell, wstring(L""));
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), strInitialDir, wstring(L""));
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"refresh"), dwRefreshInterval, 100);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"change_refresh"), dwChangeRefreshInterval, 10);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"rows"), dwRows, 25);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"columns"), dwColumns, 80);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"buffer_rows"), dwBufferRows, 0);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"buffer_columns"), dwBufferColumns, 0);
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"save_size"), bSaveSize, false);

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

bool ConsoleSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pConsoleElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"console"), pConsoleElement))) return false;

	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"shell"), strShell);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"init_dir"), strInitialDir);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"refresh"), dwRefreshInterval);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"change_refresh"), dwChangeRefreshInterval);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"rows"), dwRows);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"columns"), dwColumns);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"buffer_rows"), dwBufferRows);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"buffer_columns"), dwBufferColumns);
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"save_size"), bSaveSize);

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

ConsoleSettings& ConsoleSettings::operator=(const ConsoleSettings& other)
{
	strShell				= other.strShell;
	strInitialDir			= other.strInitialDir;

	dwRefreshInterval		= other.dwRefreshInterval;
	dwChangeRefreshInterval	= other.dwChangeRefreshInterval;
	dwRows					= other.dwRows;
	dwColumns				= other.dwColumns;
	dwBufferRows			= other.dwBufferRows;
	dwBufferColumns			= other.dwBufferColumns;
	bSaveSize				= other.bSaveSize;

	::CopyMemory(consoleColors, other.consoleColors, sizeof(COLORREF)*16);

	return *this;
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

bool FontSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/font"), pFontElement))) return false;

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

bool FontSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pFontElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/font"), pFontElement))) return false;

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

FontSettings& FontSettings::operator=(const FontSettings& other)
{
	strName		= other.strName;
	dwSize		= other.dwSize;
	bBold		= other.bBold;
	bItalic		= other.bItalic;

	bUseColor	= other.bUseColor;
	crFontColor	= other.crFontColor;

	return *this;
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
, bUseConsoleTitle(false)
, bShowCommand(true)
, bShowCommandInTabs(true)
, bUseTabTitles(false)
, dwTrimTabTitles(0)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool WindowSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pWindowElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/window"), pWindowElement))) return false;

	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"title"), strTitle, wstring(L"Console"));
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"icon"), strIcon, wstring(L""));
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"use_tab_icon"), bUseTabIcon, false);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"use_console_title"), bUseConsoleTitle, false);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"show_cmd"), bShowCommand, true);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"show_cmd_tabs"), bShowCommandInTabs, true);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"use_tab_title"), bUseTabTitles, false);
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"trim_tab_titles"), dwTrimTabTitles, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool WindowSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pWindowElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/window"), pWindowElement))) return false;

	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"title"), strTitle);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"icon"), strIcon);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"use_tab_icon"), bUseTabIcon);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"use_console_title"), bUseConsoleTitle);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"show_cmd"), bShowCommand);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"show_cmd_tabs"), bShowCommandInTabs);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"use_tab_title"), bUseTabTitles);
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"trim_tab_titles"), dwTrimTabTitles);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

WindowSettings& WindowSettings::operator=(const WindowSettings& other)
{
	strTitle			= other.strTitle;
	strIcon				= other.strIcon;
	bUseTabIcon			= other.bUseTabIcon;
	bUseConsoleTitle	= other.bUseConsoleTitle;
	bShowCommand		= other.bShowCommand;
	bShowCommandInTabs	= other.bShowCommandInTabs;
	bUseTabTitles		= other.bUseTabTitles;
	dwTrimTabTitles		= other.dwTrimTabTitles;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ControlsSettings::ControlsSettings()
: bShowMenu(true)
, bShowToolbar(true)
, bShowStatusbar(true)
, bShowTabs(true)
, bHideSingleTab(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ControlsSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCtrlsElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/controls"), pCtrlsElement))) return false;

	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_menu"), bShowMenu, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_toolbar"), bShowToolbar, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_statusbar"), bShowStatusbar, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_tabs"), bShowTabs, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"hide_single_tab"), bHideSingleTab, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ControlsSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCtrlsElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/controls"), pCtrlsElement))) return false;

	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_menu"), bShowMenu);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_toolbar"), bShowToolbar);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_statusbar"), bShowStatusbar);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_tabs"), bShowTabs);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"hide_single_tab"), bHideSingleTab);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ControlsSettings& ControlsSettings::operator=(const ControlsSettings& other)
{
	bShowMenu		= other.bShowMenu;
	bShowToolbar	= other.bShowToolbar;
	bShowStatusbar	= other.bShowStatusbar;
	bShowTabs		= other.bShowTabs;
	bHideSingleTab	= other.bHideSingleTab;

	return *this;
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

bool StylesSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pStylesElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/styles"), pStylesElement))) return false;

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

bool StylesSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pStylesElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/styles"), pStylesElement))) return false;

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

StylesSettings& StylesSettings::operator=(const StylesSettings& other)
{
	bCaption		= other.bCaption;
	bResizable		= other.bResizable;
	bTaskbarButton	= other.bTaskbarButton;
	bBorder			= other.bBorder;
	dwInsideBoder	= other.dwInsideBoder;
	bTrayIcon		= other.bTrayIcon;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

PositionSettings::PositionSettings()
: nX(-1)
, nY(-1)
, bSavePosition(false)
, zOrder(zorderNormal)
, dockPosition(dockNone)
, nSnapDistance(-1)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool PositionSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pPositionElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/position"), pPositionElement))) return false;

	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"x"), nX, -1);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"y"), nY, -1);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"save_position"), bSavePosition, false);
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"z_order"), reinterpret_cast<int&>(zOrder), static_cast<int>(zorderNormal));
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"dock"), reinterpret_cast<int&>(dockPosition), static_cast<int>(dockNone));
	XmlHelper::GetAttribute(pPositionElement, CComBSTR(L"snap"), nSnapDistance, -1);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool PositionSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pPositionElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/position"), pPositionElement))) return false;

	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"x"), nX);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"y"), nY);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"save_position"), bSavePosition);
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"z_order"), static_cast<int>(zOrder));
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"dock"), static_cast<int>(dockPosition));
	XmlHelper::SetAttribute(pPositionElement, CComBSTR(L"snap"), nSnapDistance);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

PositionSettings& PositionSettings::operator=(const PositionSettings& other)
{
	nX				= other.nX;
	nY				= other.nY;
	bSavePosition	= other.bSavePosition;
	zOrder			= other.zOrder;
	dockPosition	= other.dockPosition;
	nSnapDistance	= other.nSnapDistance;

	return *this;
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

bool TransparencySettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pTransElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/transparency"), pTransElement))) return false;

	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"type"), reinterpret_cast<DWORD&>(transType), static_cast<DWORD>(transNone));
	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"active_alpha"), byActiveAlpha, 255);
	XmlHelper::GetAttribute(pTransElement, CComBSTR(L"inactive_alpha"), byInactiveAlpha, 255);
	XmlHelper::GetRGBAttribute(pTransElement, crColorKey, RGB(0, 0, 0));

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TransparencySettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pTransElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"appearance/transparency"), pTransElement))) return false;

	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"type"), reinterpret_cast<DWORD&>(transType));
	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"active_alpha"), byActiveAlpha);
	XmlHelper::SetAttribute(pTransElement, CComBSTR(L"inactive_alpha"), byInactiveAlpha);
	XmlHelper::SetRGBAttribute(pTransElement, crColorKey);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TransparencySettings& TransparencySettings::operator=(const TransparencySettings& other)
{
	transType		= other.transType;
	byActiveAlpha	= other.byActiveAlpha;
	byInactiveAlpha	= other.byInactiveAlpha;
	crColorKey		= other.crColorKey;

	return *this;
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

bool AppearanceSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	fontSettings.Load(pSettingsRoot);
	windowSettings.Load(pSettingsRoot);
	controlsSettings.Load(pSettingsRoot);
	stylesSettings.Load(pSettingsRoot);
	positionSettings.Load(pSettingsRoot);
	transparencySettings.Load(pSettingsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool AppearanceSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	fontSettings.Save(pSettingsRoot);
	windowSettings.Save(pSettingsRoot);
	controlsSettings.Save(pSettingsRoot);
	stylesSettings.Save(pSettingsRoot);
	positionSettings.Save(pSettingsRoot);
	transparencySettings.Save(pSettingsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

AppearanceSettings& AppearanceSettings::operator=(const AppearanceSettings& other)
{
	fontSettings		= other.fontSettings;
	windowSettings		= other.windowSettings;
	controlsSettings	= other.controlsSettings;
	stylesSettings		= other.stylesSettings;
	positionSettings	= other.positionSettings;
	transparencySettings= other.transparencySettings;

	return *this;
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

bool CopyPasteSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCopyPasteElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/copy_paste"), pCopyPasteElement))) return false;

	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"copy_on_select"), bCopyOnSelect, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"no_wrap"), bNoWrap, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"trim_spaces"), bTrimSpaces, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CopyPasteSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCopyPasteElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/copy_paste"), pCopyPasteElement))) return false;

	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"copy_on_select"), bCopyOnSelect);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"no_wrap"), bNoWrap);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"trim_spaces"), bTrimSpaces);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CopyPasteSettings& CopyPasteSettings::operator=(const CopyPasteSettings& other)
{
	bCopyOnSelect	= other.bCopyOnSelect;
	bNoWrap			= other.bNoWrap;
	bTrimSpaces		= other.bTrimSpaces;

	return *this;
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

bool MouseDragSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pMouseDragElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/mouse_drag"), pMouseDragElement))) return false;

	XmlHelper::GetAttribute(pMouseDragElement, CComBSTR(L"on"), bMouseDrag, false);
	XmlHelper::GetAttribute(pMouseDragElement, CComBSTR(L"inverse_shift"), bInverseShift, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MouseDragSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pMouseDragElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/mouse_drag"), pMouseDragElement))) return false;

	XmlHelper::SetAttribute(pMouseDragElement, CComBSTR(L"on"), bMouseDrag);
	XmlHelper::SetAttribute(pMouseDragElement, CComBSTR(L"inverse_shift"), bInverseShift);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

MouseDragSettings& MouseDragSettings::operator=(const MouseDragSettings& other)
{
	bMouseDrag		= other.bMouseDrag;
	bInverseShift	= other.bInverseShift;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ScrollSettings::ScrollSettings()
: dwPageScrollRows(0)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ScrollSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pScrollElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/scroll"), pScrollElement))) return false;

	XmlHelper::GetAttribute(pScrollElement, CComBSTR(L"page_scroll_rows"), dwPageScrollRows, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ScrollSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pScrollElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/scroll"), pScrollElement))) return false;

	XmlHelper::SetAttribute(pScrollElement, CComBSTR(L"page_scroll_rows"), dwPageScrollRows);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ScrollSettings& ScrollSettings::operator=(const ScrollSettings& other)
{
	dwPageScrollRows= other.dwPageScrollRows;

	return *this;
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

bool BehaviorSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	copyPasteSettings.Load(pSettingsRoot);
	mouseDragSettings.Load(pSettingsRoot);
	scrollSettings.Load(pSettingsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool BehaviorSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	copyPasteSettings.Save(pSettingsRoot);
	mouseDragSettings.Save(pSettingsRoot);
	scrollSettings.Save(pSettingsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BehaviorSettings& BehaviorSettings::operator=(const BehaviorSettings& other)
{
	copyPasteSettings	= other.copyPasteSettings;
	mouseDragSettings	= other.mouseDragSettings;
	scrollSettings		= other.scrollSettings;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HotKeys::HotKeys()
: bUseScrollLock(false)
{
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"help",			ID_HELP,			L"Help")));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"exit",			ID_APP_EXIT,		L"Exit Console")));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab1",		ID_NEW_TAB_1,		L"New Tab 1")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab2",		ID_NEW_TAB_1 + 1,	L"New Tab 2")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab3",		ID_NEW_TAB_1 + 2,	L"New Tab 3")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab4",		ID_NEW_TAB_1 + 3,	L"New Tab 4")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab5",		ID_NEW_TAB_1 + 4,	L"New Tab 5")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab6",		ID_NEW_TAB_1 + 5,	L"New Tab 6")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab7",		ID_NEW_TAB_1 + 6,	L"New Tab 7")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab8",		ID_NEW_TAB_1 + 7,	L"New Tab 8")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab9",		ID_NEW_TAB_1 + 8,	L"New Tab 9")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab10",		ID_NEW_TAB_1 + 9,	L"New Tab 10")));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab1",	ID_SWITCH_TAB_1,	L"Switch to tab 1")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab2",	ID_SWITCH_TAB_1 + 1,L"Switch to tab 2")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab3",	ID_SWITCH_TAB_1 + 2,L"Switch to tab 3")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab4",	ID_SWITCH_TAB_1 + 3,L"Switch to tab 4")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab5",	ID_SWITCH_TAB_1 + 4,L"Switch to tab 5")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab6",	ID_SWITCH_TAB_1 + 5,L"Switch to tab 6")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab7",	ID_SWITCH_TAB_1 + 6,L"Switch to tab 7")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab8",	ID_SWITCH_TAB_1 + 7,L"Switch to tab 8")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab9",	ID_SWITCH_TAB_1 + 8,L"Switch to tab 9")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab10",	ID_SWITCH_TAB_1 + 9,L"Switch to tab 10")));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"nexttab",		ID_NEXT_TAB,		L"Next tab")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"prevtab",		ID_PREV_TAB,		L"Previous tab")));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"closetab",		ID_FILE_CLOSE_TAB,	L"Close tab")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"renametab",	ID_EDIT_RENAME_TAB,	L"Rename tab")));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"copy",			ID_EDIT_COPY,		L"Copy")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"paste",		ID_EDIT_PASTE,		L"Paste")));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollrowup",		ID_SCROLL_UP,			L"Scroll buffer one row up")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollrowdown",	ID_SCROLL_DOWN,			L"Scroll buffer one row down")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollpageup",		ID_SCROLL_PAGE_UP,		L"Scroll buffer one page up")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollpagedown",	ID_SCROLL_PAGE_DOWN,	L"Scroll buffer one page down")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollcolleft",	ID_SCROLL_LEFT,			L"Scroll buffer one column left")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollcolright",	ID_SCROLL_RIGHT,		L"Scroll buffer one column right")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollpageleft",	ID_SCROLL_PAGE_LEFT,	L"Scroll buffer one page left")));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollpageright",	ID_SCROLL_PAGE_RIGHT,	L"Scroll buffer one page right")));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"dumpbuffer",	IDC_DUMP_BUFFER,	L"Dump screen buffer")));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool HotKeys::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	HRESULT						hr = S_OK;

	CComPtr<IXMLDOMElement>		pHotkeysElement;
	CComPtr<IXMLDOMNodeList>	pHotKeyNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"hotkeys"), pHotkeysElement))) return false;

	XmlHelper::GetAttribute(pHotkeysElement, CComBSTR(L"use_scroll_lock"), bUseScrollLock, false);

	hr = pHotkeysElement->selectNodes(CComBSTR(L"hotkey"), &pHotKeyNodes);
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

		CommandNameIndex::iterator it = commands.get<command>().find(strCommand);
		if (it == commands.get<command>().end()) continue;

		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"shift"), bShift, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"ctrl"), bCtrl, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"alt"), bAlt, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"extended"), bExtended, false);
		XmlHelper::GetAttribute(pHotKeyElement, CComBSTR(L"code"), dwKeyCode, 0);

		if (bShift)	(*it)->accelHotkey.fVirt |= FSHIFT;
		if (bCtrl)	(*it)->accelHotkey.fVirt |= FCONTROL;
		if (bAlt)	(*it)->accelHotkey.fVirt |= FALT;

		(*it)->accelHotkey.fVirt|= FVIRTKEY;
		(*it)->accelHotkey.key	= static_cast<WORD>(dwKeyCode);
		(*it)->accelHotkey.cmd	= (*it)->wCommandID;
		(*it)->bExtended		= bExtended;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool HotKeys::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMElement>		pHotkeysElement;
	CComPtr<IXMLDOMNodeList>	pHotKeyChildNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"hotkeys"), pHotkeysElement))) return false;

	XmlHelper::SetAttribute(pHotkeysElement, CComBSTR(L"use_scroll_lock"), bUseScrollLock);

	if (FAILED(pHotkeysElement->get_childNodes(&pHotKeyChildNodes))) return false;

	long	lListLength;
	pHotKeyChildNodes->get_length(&lListLength);

	for (long i = lListLength - 1; i >= 0; --i)
	{
		CComPtr<IXMLDOMNode>	pHotKeyChildNode;
		CComPtr<IXMLDOMNode>	pRemovedHotKeyNode;
		if (FAILED(pHotKeyChildNodes->get_item(i, &pHotKeyChildNode))) continue;

		hr = pHotkeysElement->removeChild(pHotKeyChildNode, &pRemovedHotKeyNode);
	}

	CComPtr<IXMLDOMDocument>	pSettingsDoc;

	CommandsSequence::iterator	itCommand;
	CommandsSequence::iterator	itLastCommand = commands.end();
	--itLastCommand;

	pHotkeysElement->get_ownerDocument(&pSettingsDoc);

	for (itCommand = commands.begin(); itCommand != commands.end(); ++itCommand)
	{
		CComPtr<IXMLDOMElement>	pNewHotkeyElement;
		CComPtr<IXMLDOMNode>	pNewHotkeyOut;
		CComVariant				varAttrVal;

		pSettingsDoc->createElement(CComBSTR(L"hotkey"), &pNewHotkeyElement);

		varAttrVal = ((*itCommand)->accelHotkey.fVirt & FCONTROL) ? L"1" : L"0";
        pNewHotkeyElement->setAttribute(CComBSTR(L"ctrl"), varAttrVal);

		varAttrVal = ((*itCommand)->accelHotkey.fVirt & FSHIFT) ? L"1" : L"0";
		pNewHotkeyElement->setAttribute(CComBSTR(L"shift"), varAttrVal);

		varAttrVal = ((*itCommand)->accelHotkey.fVirt & FALT) ? L"1" : L"0";
		pNewHotkeyElement->setAttribute(CComBSTR(L"alt"), varAttrVal);

		varAttrVal = ((*itCommand)->bExtended) ? L"1" : L"0";
		pNewHotkeyElement->setAttribute(CComBSTR(L"extended"), varAttrVal);

		pNewHotkeyElement->setAttribute(CComBSTR(L"code"), CComVariant((*itCommand)->accelHotkey.key));

		pNewHotkeyElement->setAttribute(CComBSTR(L"command"), CComVariant((*itCommand)->strCommand.c_str()));

		pHotkeysElement->appendChild(pNewHotkeyElement, &pNewHotkeyOut);

		// this is just for pretty printing
		if (itCommand == itLastCommand)
		{
			SettingsBase::AddTextNode(pSettingsDoc, pHotkeysElement, CComBSTR(L"\n\t"));
		}
		else
		{
			SettingsBase::AddTextNode(pSettingsDoc, pHotkeysElement, CComBSTR(L"\n\t\t"));
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

bool TabSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMNodeList>	pTabNodes;

	hr = pSettingsRoot->selectNodes(CComBSTR(L"tabs/tab"), &pTabNodes);
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

bool TabSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMElement>		pTabsElement;
	CComPtr<IXMLDOMNodeList>	pTabChildNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"tabs"), pTabsElement))) return false;

	if (FAILED(pTabsElement->get_childNodes(&pTabChildNodes))) return false;

	long	lListLength;
	pTabChildNodes->get_length(&lListLength);

	for (long i = lListLength - 1; i >= 0; --i)
	{
		CComPtr<IXMLDOMNode>	pTabChildNode;
		CComPtr<IXMLDOMNode>	pRemovedTabNode;
		if (FAILED(pTabChildNodes->get_item(i, &pTabChildNode))) continue;

		hr = pTabsElement->removeChild(pTabChildNode, &pRemovedTabNode);
	}

	CComPtr<IXMLDOMDocument>	pSettingsDoc;
	TabDataVector::iterator		itTab;
	TabDataVector::iterator		itLastTab = tabDataVector.end() - 1;

	pTabsElement->get_ownerDocument(&pSettingsDoc);

	for (itTab = tabDataVector.begin(); itTab != tabDataVector.end(); ++itTab)
	{
		CComPtr<IXMLDOMElement>	pNewTabElement;
		CComPtr<IXMLDOMNode>	pNewTabOut;

		CComVariant				varAttrVal;

		pSettingsDoc->createElement(CComBSTR(L"tab"), &pNewTabElement);

		// set tab attributes
		if ((*itTab)->strTitle.length() > 0)
		{
			pNewTabElement->setAttribute(CComBSTR(L"title"), CComVariant((*itTab)->strTitle.c_str()));
		}

		if ((*itTab)->strIcon.length() > 0)
		{
			pNewTabElement->setAttribute(CComBSTR(L"icon"), CComVariant((*itTab)->strIcon.c_str()));
		}

		// add <console> tag
		CComPtr<IXMLDOMElement>	pNewConsoleElement;
		CComPtr<IXMLDOMNode>	pNewConsoleOut;

		pSettingsDoc->createElement(CComBSTR(L"console"), &pNewConsoleElement);

		pNewConsoleElement->setAttribute(CComBSTR(L"shell"), CComVariant((*itTab)->strShell.c_str()));
		pNewConsoleElement->setAttribute(CComBSTR(L"init_dir"), CComVariant((*itTab)->strInitialDir.c_str()));

		SettingsBase::AddTextNode(pSettingsDoc, pNewTabElement, CComBSTR(L"\n\t\t\t"));
		pNewTabElement->appendChild(pNewConsoleElement, &pNewConsoleOut);

		// add <cursor> tag
		CComPtr<IXMLDOMElement>	pNewCursorElement;
		CComPtr<IXMLDOMNode>	pNewCursorOut;

		pSettingsDoc->createElement(CComBSTR(L"cursor"), &pNewCursorElement);

		pNewCursorElement->setAttribute(CComBSTR(L"style"), CComVariant((*itTab)->dwCursorStyle));
		pNewCursorElement->setAttribute(CComBSTR(L"r"), CComVariant(GetRValue((*itTab)->crCursorColor)));
		pNewCursorElement->setAttribute(CComBSTR(L"g"), CComVariant(GetGValue((*itTab)->crCursorColor)));
		pNewCursorElement->setAttribute(CComBSTR(L"b"), CComVariant(GetBValue((*itTab)->crCursorColor)));

		SettingsBase::AddTextNode(pSettingsDoc, pNewTabElement, CComBSTR(L"\n\t\t\t"));
		pNewTabElement->appendChild(pNewCursorElement, &pNewCursorOut);

		// add <background> tag
		CComPtr<IXMLDOMElement>	pNewBkElement;
		CComPtr<IXMLDOMNode>	pNewBkOut;

		pSettingsDoc->createElement(CComBSTR(L"background"), &pNewBkElement);

		pNewBkElement->setAttribute(CComBSTR(L"type"), CComVariant((*itTab)->backgroundImageType));
		pNewBkElement->setAttribute(CComBSTR(L"r"), CComVariant(GetRValue((*itTab)->crBackgroundColor)));
		pNewBkElement->setAttribute(CComBSTR(L"g"), CComVariant(GetGValue((*itTab)->crBackgroundColor)));
		pNewBkElement->setAttribute(CComBSTR(L"b"), CComVariant(GetBValue((*itTab)->crBackgroundColor)));

		// add <image> tag
		CComPtr<IXMLDOMElement>	pNewImageElement;
		CComPtr<IXMLDOMNode>	pNewImageOut;

		pSettingsDoc->createElement(CComBSTR(L"image"), &pNewImageElement);

		if ((*itTab)->backgroundImageType == bktypeImage)
		{
			pNewImageElement->setAttribute(CComBSTR(L"file"), CComVariant((*itTab)->imageData.strFilename.c_str()));
			pNewImageElement->setAttribute(CComBSTR(L"relative"), CComVariant((*itTab)->imageData.bRelative ? 1 : 0));
			pNewImageElement->setAttribute(CComBSTR(L"extend"), CComVariant((*itTab)->imageData.bExtend ? 1 : 0));
			pNewImageElement->setAttribute(CComBSTR(L"position"), CComVariant(static_cast<DWORD>((*itTab)->imageData.imagePosition)));
		}
		else
		{
			pNewImageElement->setAttribute(CComBSTR(L"file"), CComVariant(L""));
			pNewImageElement->setAttribute(CComBSTR(L"relative"), CComVariant(0));
			pNewImageElement->setAttribute(CComBSTR(L"extend"), CComVariant(0));
			pNewImageElement->setAttribute(CComBSTR(L"position"), CComVariant(0));
		}

		// add <tint> tag
		CComPtr<IXMLDOMElement>	pNewTintElement;
		CComPtr<IXMLDOMNode>	pNewTintOut;

		pSettingsDoc->createElement(CComBSTR(L"tint"), &pNewTintElement);

		pNewTintElement->setAttribute(CComBSTR(L"opacity"), CComVariant((*itTab)->imageData.byTintOpacity));
		pNewTintElement->setAttribute(CComBSTR(L"r"), CComVariant(GetRValue((*itTab)->imageData.crTint)));
		pNewTintElement->setAttribute(CComBSTR(L"g"), CComVariant(GetGValue((*itTab)->imageData.crTint)));
		pNewTintElement->setAttribute(CComBSTR(L"b"), CComVariant(GetBValue((*itTab)->imageData.crTint)));


		SettingsBase::AddTextNode(pSettingsDoc, pNewImageElement, CComBSTR(L"\n\t\t\t\t\t"));
		pNewImageElement->appendChild(pNewTintElement, &pNewTintOut);
		SettingsBase::AddTextNode(pSettingsDoc, pNewImageElement, CComBSTR(L"\n\t\t\t\t"));
		SettingsBase::AddTextNode(pSettingsDoc, pNewBkElement, CComBSTR(L"\n\t\t\t\t"));
		pNewBkElement->appendChild(pNewImageElement, &pNewImageOut);
		SettingsBase::AddTextNode(pSettingsDoc, pNewBkElement, CComBSTR(L"\n\t\t\t"));
		SettingsBase::AddTextNode(pSettingsDoc, pNewTabElement, CComBSTR(L"\n\t\t\t"));
		pNewTabElement->appendChild(pNewBkElement, &pNewBkOut);
		SettingsBase::AddTextNode(pSettingsDoc, pNewTabElement, CComBSTR(L"\n\t\t"));

		pTabsElement->appendChild(pNewTabElement, &pNewTabOut);

		// this is just for pretty printing
		if (itTab == itLastTab)
		{
			SettingsBase::AddTextNode(pSettingsDoc, pTabsElement, CComBSTR(L"\n\t"));
		}
		else
		{
			SettingsBase::AddTextNode(pSettingsDoc, pTabsElement, CComBSTR(L"\n\t\t"));
		}
	}

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
: m_pSettingsDocument()
, m_pSettingsRoot()
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

	hr = XmlHelper::OpenXmlDocument(m_strSettingsFileName, L"console.xml", m_pSettingsDocument, m_pSettingsRoot);
	if (FAILED(hr)) return false;

	// load settings' sections
	m_consoleSettings.Load(m_pSettingsRoot);
	m_appearanceSettings.Load(m_pSettingsRoot);
	m_behaviorSettings.Load(m_pSettingsRoot);
	m_hotKeys.Load(m_pSettingsRoot);

	m_tabSettings.SetDefaults(m_consoleSettings.strShell, m_consoleSettings.strInitialDir);
	m_tabSettings.Load(m_pSettingsRoot);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool SettingsHandler::SaveSettings()
{
	m_consoleSettings.Save(m_pSettingsRoot);
	m_appearanceSettings.Save(m_pSettingsRoot);
	m_behaviorSettings.Save(m_pSettingsRoot);
	m_hotKeys.Save(m_pSettingsRoot);
	m_tabSettings.Save(m_pSettingsRoot);

	HRESULT hr = m_pSettingsDocument->save(CComVariant(m_strSettingsFileName.c_str()));

	return SUCCEEDED(hr) ? true : false;
}

//////////////////////////////////////////////////////////////////////////////



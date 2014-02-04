#include "stdafx.h"
#include "resource.h"

#include "XmlHelper.h"
#include "SettingsHandler.h"

using namespace boost::algorithm;

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
, bStartHidden(false)
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
	XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"start_hidden"), bStartHidden, false);
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
	XmlHelper::SetAttribute(pConsoleElement, CComBSTR(L"start_hidden"), bStartHidden);
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
	bStartHidden			= other.bStartHidden;
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
, fontSmoothing(fontSmoothDefault)
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

	int nFontSmoothing;

	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"name"), strName, wstring(L"Courier New"));
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"size"), dwSize, 10);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"bold"), bBold, false);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"italic"), bItalic, false);
	XmlHelper::GetAttribute(pFontElement, CComBSTR(L"smoothing"), nFontSmoothing, 0);

	fontSmoothing = static_cast<FontSmoothing>(nFontSmoothing);

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
	XmlHelper::SetAttribute(pFontElement, CComBSTR(L"smoothing"), static_cast<int>(fontSmoothing));

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
	strName			= other.strName;
	dwSize			= other.dwSize;
	bBold			= other.bBold;
	bItalic			= other.bItalic;
	fontSmoothing	= other.fontSmoothing;

	bUseColor		= other.bUseColor;
	crFontColor		= other.crFontColor;

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
, dwTrimTabTitlesRight(0)
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
	XmlHelper::GetAttribute(pWindowElement, CComBSTR(L"trim_tab_titles_right"), dwTrimTabTitlesRight, 0);

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
	XmlHelper::SetAttribute(pWindowElement, CComBSTR(L"trim_tab_titles_right"), dwTrimTabTitlesRight);

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
	dwTrimTabTitlesRight= other.dwTrimTabTitlesRight;

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
, bTabsOnBottom(false)
, bShowScrollbars(true)
, bFlatScrollbars(false)
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
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"tabs_on_bottom"), bTabsOnBottom, false);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"show_scrollbars"), bShowScrollbars, true);
	XmlHelper::GetAttribute(pCtrlsElement, CComBSTR(L"flat_scrollbars"), bFlatScrollbars, false);

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
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"tabs_on_bottom"), bTabsOnBottom);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"show_scrollbars"), bShowScrollbars);
	XmlHelper::SetAttribute(pCtrlsElement, CComBSTR(L"flat_scrollbars"), bFlatScrollbars);

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
	bTabsOnBottom	= other.bTabsOnBottom;
	bShowScrollbars	= other.bShowScrollbars;
	bFlatScrollbars	= other.bFlatScrollbars;

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
, dwInsideBorder(2)
, bTrayIcon(false)
, crSelectionColor(RGB(255, 255, 255))
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
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"inside_border"), dwInsideBorder, 2);
	XmlHelper::GetAttribute(pStylesElement, CComBSTR(L"tray_icon"), bTrayIcon, false);

	CComPtr<IXMLDOMElement>	pSelColorElement;

	if (FAILED(XmlHelper::GetDomElement(pStylesElement, CComBSTR(L"selection_color"), pSelColorElement))) return false;

	XmlHelper::GetRGBAttribute(pSelColorElement, crSelectionColor, RGB(255, 255, 255));

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
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"inside_border"), dwInsideBorder);
	XmlHelper::SetAttribute(pStylesElement, CComBSTR(L"tray_icon"), bTrayIcon);

	CComPtr<IXMLDOMElement>	pSelColorElement;

	if (FAILED(XmlHelper::GetDomElement(pStylesElement, CComBSTR(L"selection_color"), pSelColorElement))) return false;

	XmlHelper::SetRGBAttribute(pSelColorElement, crSelectionColor);

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
	dwInsideBorder	= other.dwInsideBorder;
	bTrayIcon		= other.bTrayIcon;
	crSelectionColor= other.crSelectionColor;

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


BYTE TransparencySettings::minAlpha = 5;

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

	if (byActiveAlpha < minAlpha) byActiveAlpha = minAlpha;
	if (byInactiveAlpha < minAlpha) byInactiveAlpha = minAlpha;

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
, bClearOnCopy(true)
, bNoWrap(false)
, bTrimSpaces(false)
, copyNewlineChar(newlineCRLF)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CopyPasteSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCopyPasteElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/copy_paste"), pCopyPasteElement))) return false;

	int nNewlineChar;

	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"copy_on_select"), bCopyOnSelect, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"clear_on_copy"), bClearOnCopy, true);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"sensitive_copy"), bSensitiveCopy, true);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"no_wrap"), bNoWrap, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"trim_spaces"), bTrimSpaces, false);
	XmlHelper::GetAttribute(pCopyPasteElement, CComBSTR(L"copy_newline_char"), nNewlineChar, 0);

	copyNewlineChar = static_cast<CopyNewlineChar>(nNewlineChar);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool CopyPasteSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pCopyPasteElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/copy_paste"), pCopyPasteElement))) return false;

	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"copy_on_select"), bCopyOnSelect);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"clear_on_copy"), bClearOnCopy);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"sensitive_copy"), bSensitiveCopy);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"no_wrap"), bNoWrap);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"trim_spaces"), bTrimSpaces);
	XmlHelper::SetAttribute(pCopyPasteElement, CComBSTR(L"copy_newline_char"), static_cast<int>(copyNewlineChar));

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

CopyPasteSettings& CopyPasteSettings::operator=(const CopyPasteSettings& other)
{
	bCopyOnSelect	= other.bCopyOnSelect;
	bClearOnCopy	= other.bClearOnCopy;
	bSensitiveCopy	= other.bSensitiveCopy;
	bNoWrap			= other.bNoWrap;
	bTrimSpaces		= other.bTrimSpaces;
	copyNewlineChar	= other.copyNewlineChar;

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

TabHighlightSettings::TabHighlightSettings()
: dwFlashes(0)
, bStayHighlighted(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabHighlightSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pTabElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/tab_highlight"), pTabElement))) return false;

	XmlHelper::GetAttribute(pTabElement, CComBSTR(L"flashes"), dwFlashes, 0);
	XmlHelper::GetAttribute(pTabElement, CComBSTR(L"stay_highligted"), bStayHighlighted, false);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool TabHighlightSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pTabElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/tab_highlight"), pTabElement))) return false;

	XmlHelper::SetAttribute(pTabElement, CComBSTR(L"flashes"), dwFlashes);
	XmlHelper::SetAttribute(pTabElement, CComBSTR(L"stay_highligted"), bStayHighlighted);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

TabHighlightSettings& TabHighlightSettings::operator=(const TabHighlightSettings& other)
{
	dwFlashes		= other.dwFlashes;
	bStayHighlighted= other.bStayHighlighted;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

AnimateSettings::AnimateSettings()
: dwType(animTypeNone)
, dwHorzDirection(animDirNone)
, dwVertDirection(animDirNone)
, dwTime(0)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool AnimateSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pAnimateElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/animate"), pAnimateElement))) return false;

	XmlHelper::GetAttribute(pAnimateElement, CComBSTR(L"type"), dwType, animTypeNone);
	XmlHelper::GetAttribute(pAnimateElement, CComBSTR(L"horz_direction"), dwHorzDirection, animDirNone);
	XmlHelper::GetAttribute(pAnimateElement, CComBSTR(L"vert_direction"), dwVertDirection, animDirNone);
	XmlHelper::GetAttribute(pAnimateElement, CComBSTR(L"time"), dwTime, 200);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool AnimateSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	CComPtr<IXMLDOMElement>	pAnimateElement;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"behavior/animate"), pAnimateElement))) return false;

	XmlHelper::SetAttribute(pAnimateElement, CComBSTR(L"type"), dwType);
	XmlHelper::SetAttribute(pAnimateElement, CComBSTR(L"horz_direction"), dwHorzDirection);
	XmlHelper::SetAttribute(pAnimateElement, CComBSTR(L"vert_direction"), dwVertDirection);
	XmlHelper::SetAttribute(pAnimateElement, CComBSTR(L"time"), dwTime);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

AnimateSettings& AnimateSettings::operator=(const AnimateSettings& other)
{
	dwType			= other.dwType;
	dwHorzDirection	= other.dwHorzDirection;
	dwVertDirection	= other.dwVertDirection;
	dwTime			= other.dwTime;

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
	scrollSettings.Load(pSettingsRoot);
	tabHighlightSettings.Load(pSettingsRoot);
//	animateSettings.Load(pSettingsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool BehaviorSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	copyPasteSettings.Save(pSettingsRoot);
	scrollSettings.Save(pSettingsRoot);
	tabHighlightSettings.Save(pSettingsRoot);
//	animateSettings.Save(pSettingsRoot);
	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BehaviorSettings& BehaviorSettings::operator=(const BehaviorSettings& other)
{
	copyPasteSettings	= other.copyPasteSettings;
	scrollSettings		= other.scrollSettings;
	tabHighlightSettings= other.tabHighlightSettings;
//	animateSettings		= other.animateSettings;

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
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"settings",		ID_EDIT_SETTINGS,	IDS_COMMAND_SETTINGS_DIALOG)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"help",			ID_HELP,			IDS_COMMAND_HELP)));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"exit",			ID_APP_EXIT,		IDS_COMMAND_EXIT_CONSOLE)));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab1",		ID_NEW_TAB_1,		IDS_COMMAND_NEWTAB1)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab2",		ID_NEW_TAB_1 + 1,	IDS_COMMAND_NEWTAB2)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab3",		ID_NEW_TAB_1 + 2,	IDS_COMMAND_NEWTAB3)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab4",		ID_NEW_TAB_1 + 3,	IDS_COMMAND_NEWTAB4)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab5",		ID_NEW_TAB_1 + 4,	IDS_COMMAND_NEWTAB5)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab6",		ID_NEW_TAB_1 + 5,	IDS_COMMAND_NEWTAB6)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab7",		ID_NEW_TAB_1 + 6,	IDS_COMMAND_NEWTAB7)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab8",		ID_NEW_TAB_1 + 7,	IDS_COMMAND_NEWTAB8)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab9",		ID_NEW_TAB_1 + 8,	IDS_COMMAND_NEWTAB9)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"newtab10",		ID_NEW_TAB_1 + 9,	IDS_COMMAND_NEWTAB10)));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab1",	ID_SWITCH_TAB_1,	IDS_COMMAND_TAB1)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab2",	ID_SWITCH_TAB_1 + 1,IDS_COMMAND_TAB2)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab3",	ID_SWITCH_TAB_1 + 2,IDS_COMMAND_TAB3)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab4",	ID_SWITCH_TAB_1 + 3,IDS_COMMAND_TAB4)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab5",	ID_SWITCH_TAB_1 + 4,IDS_COMMAND_TAB5)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab6",	ID_SWITCH_TAB_1 + 5,IDS_COMMAND_TAB6)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab7",	ID_SWITCH_TAB_1 + 6,IDS_COMMAND_TAB7)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab8",	ID_SWITCH_TAB_1 + 7,IDS_COMMAND_TAB8)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab9",	ID_SWITCH_TAB_1 + 8,IDS_COMMAND_TAB9)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"switchtab10",	ID_SWITCH_TAB_1 + 9,IDS_COMMAND_TAB10)));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"nexttab",		ID_NEXT_TAB,		IDS_COMMAND_NEXT_TAB)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"prevtab",		ID_PREV_TAB,		IDS_COMMAND_PREVIOUS_TAB)));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"closetab",		ID_FILE_CLOSE_TAB,	IDS_COMMAND_CLOSE_TAB)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"renametab",	ID_EDIT_RENAME_TAB,	IDS_COMMAND_RENAME_TAB)));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"copy",			ID_EDIT_COPY,				IDS_COMMAND_COPY_SELECTION)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"clear_selection",ID_EDIT_CLEAR_SELECTION,	IDS_COMMAND_CLEAR_SELECTION)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"paste",		ID_EDIT_PASTE,				IDS_COMMAND_PASTE)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"stopscroll",	ID_EDIT_STOP_SCROLLING,		IDS_COMMAND_STOP_SCROLLING)));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollrowup",		ID_SCROLL_UP,			IDS_COMMAND_SCROLL_UP)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollrowdown",	ID_SCROLL_DOWN,			IDS_COMMAND_SCROLL_DOWN)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollpageup",		ID_SCROLL_PAGE_UP,		IDS_COMMAND_SCROLL_UP_PAGE)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollpagedown",	ID_SCROLL_PAGE_DOWN,	IDS_COMMAND_SCROLL_DOWN_PAGE)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollcolleft",	ID_SCROLL_LEFT,			IDS_COMMAND_SCROLL_LEFT)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollcolright",	ID_SCROLL_RIGHT,		IDS_COMMAND_SCROLL_RIGHT)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollpageleft",	ID_SCROLL_PAGE_LEFT,	IDS_COMMAND_SCROLL_LEFT_PAGE)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"scrollpageright",	ID_SCROLL_PAGE_RIGHT,	IDS_COMMAND_SCROLL_RIGHT_PAGE)));

	commands.push_back(shared_ptr<CommandData>(new CommandData(L"dumpbuffer",	IDC_DUMP_BUFFER,	IDS_COMMAND_DUMP_BUFFER)));

	// global commands
	commands.push_back(shared_ptr<CommandData>(new CommandData(L"activate",	IDC_GLOBAL_ACTIVATE,	IDS_COMMAND_ACTIVATE_CONSOLE, true)));
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
		bool					bAttrVal;

		pSettingsDoc->createElement(CComBSTR(L"hotkey"), &pNewHotkeyElement);

		bAttrVal = ((*itCommand)->accelHotkey.fVirt & FCONTROL) ? true : false;
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"ctrl"), bAttrVal);

		bAttrVal = ((*itCommand)->accelHotkey.fVirt & FSHIFT) ? true : false;
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"shift"), bAttrVal);

		bAttrVal = ((*itCommand)->accelHotkey.fVirt & FALT) ? true : false;
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"alt"), bAttrVal);

		bAttrVal = ((*itCommand)->bExtended) ? true : false;
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"extended"), bAttrVal);

		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"code"), (*itCommand)->accelHotkey.key);
		XmlHelper::SetAttribute(pNewHotkeyElement, CComBSTR(L"command"), (*itCommand)->strCommand);

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

HotKeys& HotKeys::operator=(const HotKeys& other)
{
	bUseScrollLock = other.bUseScrollLock;

	commands.clear();
	commands.insert(commands.begin(), other.commands.begin(), other.commands.end());

	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

MouseSettings::MouseSettings()
: commands()
{
	commands.push_back(shared_ptr<CommandData>(new CommandData(cmdCopy,		L"copy",	IDS_COMMAND_MOUSE_COPY)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(cmdSelect,	L"select",	IDS_COMMAND_MOUSE_SELECT)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(cmdPaste,	L"paste",	IDS_COMMAND_MOUSE_PASTE)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(cmdDrag,		L"drag",	IDS_COMMAND_MOUSE_DRAG)));
	commands.push_back(shared_ptr<CommandData>(new CommandData(cmdMenu,		L"menu",	IDS_COMMAND_MOUSE_MENU)));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MouseSettings::Load(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	HRESULT						hr = S_OK;

	CComPtr<IXMLDOMElement>		pActionsElement;
	CComPtr<IXMLDOMNodeList>	pActionNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"mouse/actions"), pActionsElement))) return false;

	hr = pActionsElement->selectNodes(CComBSTR(L"action"), &pActionNodes);
	if (FAILED(hr)) return false;

	long	lListLength;
	pActionNodes->get_length(&lListLength);

	for (long i = 0; i < lListLength; ++i)
	{
		CComPtr<IXMLDOMNode>	pActionNode;
		CComPtr<IXMLDOMElement>	pActionElement;

		pActionNodes->get_item(i, &pActionNode);
		if (FAILED(pActionNode.QueryInterface(&pActionElement))) continue;

		wstring	strName;
		DWORD	dwButton;
		bool	bUseCtrl;
		bool	bUseShift;
		bool	bUseAlt;
		
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"name"), strName, L"");
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"button"), dwButton, 0);
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"ctrl"), bUseCtrl, false);
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"shift"), bUseShift, false);
		XmlHelper::GetAttribute(pActionElement, CComBSTR(L"alt"), bUseAlt, false);

		typedef Commands::index<commandName>::type		CommandNameIndex;

		CommandNameIndex::iterator it = commands.get<commandName>().find(strName);
		if (it == commands.get<commandName>().end()) continue;

		(*it)->action.clickType = clickSingle;
		(*it)->action.button	= static_cast<Button>(dwButton);
		if (bUseCtrl)	(*it)->action.modifiers |= mkCtrl;
		if (bUseShift)	(*it)->action.modifiers |= mkShift;
		if (bUseAlt)	(*it)->action.modifiers |= mkAlt;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool MouseSettings::Save(const CComPtr<IXMLDOMElement>& pSettingsRoot)
{
	HRESULT						hr = S_OK;
	CComPtr<IXMLDOMElement>		pMouseActionsElement;
	CComPtr<IXMLDOMNodeList>	pMouseActionsChildNodes;

	if (FAILED(XmlHelper::GetDomElement(pSettingsRoot, CComBSTR(L"mouse/actions"), pMouseActionsElement))) return false;

	if (FAILED(pMouseActionsElement->get_childNodes(&pMouseActionsChildNodes))) return false;

	long	lListLength;
	pMouseActionsChildNodes->get_length(&lListLength);

	for (long i = lListLength - 1; i >= 0; --i)
	{
		CComPtr<IXMLDOMNode>	pMouseActionsChildNode;
		CComPtr<IXMLDOMNode>	pRemovedMouseActionsNode;
		if (FAILED(pMouseActionsChildNodes->get_item(i, &pMouseActionsChildNode))) continue;

		hr = pMouseActionsElement->removeChild(pMouseActionsChildNode, &pRemovedMouseActionsNode);
	}

	CComPtr<IXMLDOMDocument>	pSettingsDoc;

	CommandsSequence::iterator	itCommand;
	CommandsSequence::iterator	itLastCommand = commands.end();
	--itLastCommand;

	pMouseActionsElement->get_ownerDocument(&pSettingsDoc);

	for (itCommand = commands.begin(); itCommand != commands.end(); ++itCommand)
	{
		CComPtr<IXMLDOMElement>	pNewMouseActionsElement;
		CComPtr<IXMLDOMNode>	pNewMouseActionsOut;
		bool					bVal;

		pSettingsDoc->createElement(CComBSTR(L"action"), &pNewMouseActionsElement);

		bVal = ((*itCommand)->action.modifiers & mkCtrl) ? true : false;
		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"ctrl"), bVal);

		bVal = ((*itCommand)->action.modifiers & mkShift) ? true : false;
		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"shift"), bVal);

		bVal = ((*itCommand)->action.modifiers & mkAlt) ? true : false;
		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"alt"), bVal);

		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"button"), static_cast<int>((*itCommand)->action.button));
		XmlHelper::SetAttribute(pNewMouseActionsElement, CComBSTR(L"name"), (*itCommand)->strCommand);

		pMouseActionsElement->appendChild(pNewMouseActionsElement, &pNewMouseActionsOut);

		// this is just for pretty printing
		if (itCommand == itLastCommand)
		{
			SettingsBase::AddTextNode(pSettingsDoc, pMouseActionsElement, CComBSTR(L"\n\t\t"));
		}
		else
		{
			SettingsBase::AddTextNode(pSettingsDoc, pMouseActionsElement, CComBSTR(L"\n\t\t\t"));
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

MouseSettings& MouseSettings::operator=(const MouseSettings& other)
{
	commands.clear();
	commands.insert(commands.begin(), other.commands.begin(), other.commands.end());

	return *this;
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
		XmlHelper::GetAttribute(pTabElement, CComBSTR(L"use_default_icon"), tabData->bUseDefaultIcon, false);

		tabDataVector.push_back(tabData);

		if (SUCCEEDED(XmlHelper::GetDomElement(pTabElement, CComBSTR(L"console"), pConsoleElement)))
		{
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"shell"), tabData->strShell, strDefaultShell);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"init_dir"), tabData->strInitialDir, strDefaultInitialDir);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"run_as_user"), tabData->bRunAsUser, false);
			XmlHelper::GetAttribute(pConsoleElement, CComBSTR(L"user"), tabData->strUser, L"");
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
			tabData->backgroundImageType = static_cast<BackgroundImageType>(dwBackgroundImageType);

			if (tabData->backgroundImageType == bktypeNone)
			{
				XmlHelper::GetRGBAttribute(pBackgroundElement, tabData->crBackgroundColor, RGB(0, 0, 0));
			}
			else
			{
				tabData->crBackgroundColor = RGB(0, 0, 0);

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

		pSettingsDoc->createElement(CComBSTR(L"tab"), &pNewTabElement);

		// set tab attributes
		if ((*itTab)->strTitle.length() > 0)
		{
			XmlHelper::SetAttribute(pNewTabElement, CComBSTR(L"title"), (*itTab)->strTitle);
		}

		if ((*itTab)->strIcon.length() > 0)
		{
			XmlHelper::SetAttribute(pNewTabElement, CComBSTR(L"icon"), (*itTab)->strIcon);
		}

		XmlHelper::SetAttribute(pNewTabElement, CComBSTR(L"use_default_icon"), (*itTab)->bUseDefaultIcon);

		// add <console> tag
		CComPtr<IXMLDOMElement>	pNewConsoleElement;
		CComPtr<IXMLDOMNode>	pNewConsoleOut;

		pSettingsDoc->createElement(CComBSTR(L"console"), &pNewConsoleElement);

		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"shell"), (*itTab)->strShell);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"init_dir"), (*itTab)->strInitialDir);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"run_as_user"), (*itTab)->bRunAsUser);
		XmlHelper::SetAttribute(pNewConsoleElement, CComBSTR(L"user"), (*itTab)->strUser);

		SettingsBase::AddTextNode(pSettingsDoc, pNewTabElement, CComBSTR(L"\n\t\t\t"));
		pNewTabElement->appendChild(pNewConsoleElement, &pNewConsoleOut);

		// add <cursor> tag
		CComPtr<IXMLDOMElement>	pNewCursorElement;
		CComPtr<IXMLDOMNode>	pNewCursorOut;

		pSettingsDoc->createElement(CComBSTR(L"cursor"), &pNewCursorElement);

		XmlHelper::SetAttribute(pNewCursorElement, CComBSTR(L"style"), (*itTab)->dwCursorStyle);
		XmlHelper::SetAttribute(pNewCursorElement, CComBSTR(L"r"), GetRValue((*itTab)->crCursorColor));
		XmlHelper::SetAttribute(pNewCursorElement, CComBSTR(L"g"), GetGValue((*itTab)->crCursorColor));
		XmlHelper::SetAttribute(pNewCursorElement, CComBSTR(L"b"), GetBValue((*itTab)->crCursorColor));


		SettingsBase::AddTextNode(pSettingsDoc, pNewTabElement, CComBSTR(L"\n\t\t\t"));
		pNewTabElement->appendChild(pNewCursorElement, &pNewCursorOut);

		// add <background> tag
		CComPtr<IXMLDOMElement>	pNewBkElement;
		CComPtr<IXMLDOMNode>	pNewBkOut;

		pSettingsDoc->createElement(CComBSTR(L"background"), &pNewBkElement);

		XmlHelper::SetAttribute(pNewBkElement, CComBSTR(L"type"), (*itTab)->backgroundImageType);
		XmlHelper::SetAttribute(pNewBkElement, CComBSTR(L"r"), GetRValue((*itTab)->crBackgroundColor));
		XmlHelper::SetAttribute(pNewBkElement, CComBSTR(L"g"), GetGValue((*itTab)->crBackgroundColor));
		XmlHelper::SetAttribute(pNewBkElement, CComBSTR(L"b"), GetBValue((*itTab)->crBackgroundColor));


		// add <image> tag
		CComPtr<IXMLDOMElement>	pNewImageElement;
		CComPtr<IXMLDOMNode>	pNewImageOut;

		pSettingsDoc->createElement(CComBSTR(L"image"), &pNewImageElement);

		if ((*itTab)->backgroundImageType == bktypeImage)
		{
			XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"file"), (*itTab)->imageData.strFilename);
			XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"relative"), (*itTab)->imageData.bRelative ? true : false);
			XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"extend"), (*itTab)->imageData.bExtend ? true : false);
			XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"position"), static_cast<DWORD>((*itTab)->imageData.imagePosition));
		}
		else
		{
			XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"file"), wstring(L""));
			XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"relative"), false);
			XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"extend"), false);
			XmlHelper::SetAttribute(pNewImageElement, CComBSTR(L"position"), 0);
		}

		// add <tint> tag
		CComPtr<IXMLDOMElement>	pNewTintElement;
		CComPtr<IXMLDOMNode>	pNewTintOut;

		pSettingsDoc->createElement(CComBSTR(L"tint"), &pNewTintElement);

		XmlHelper::SetAttribute(pNewTintElement, CComBSTR(L"opacity"), (*itTab)->imageData.byTintOpacity);
		XmlHelper::SetAttribute(pNewTintElement, CComBSTR(L"r"), GetRValue((*itTab)->imageData.crTint));
		XmlHelper::SetAttribute(pNewTintElement, CComBSTR(L"g"), GetGValue((*itTab)->imageData.crTint));
		XmlHelper::SetAttribute(pNewTintElement, CComBSTR(L"b"), GetBValue((*itTab)->imageData.crTint));


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
, m_strSettingsPath(L"")
, m_strSettingsFileName(L"")
, m_settingsDirType(dirTypeExe)
, m_consoleSettings()
, m_appearanceSettings()
, m_behaviorSettings()
, m_hotKeys()
, m_mouseSettings()
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

	size_t pos = strSettingsFileName.rfind(L'\\');

	if (pos == wstring::npos)
	{
		// no path, first try with user's APPDATA dir

		wchar_t wszAppData[32767];
		::ZeroMemory(wszAppData, sizeof(wszAppData));
		::GetEnvironmentVariable(L"APPDATA", wszAppData, _countof(wszAppData));

		m_strSettingsFileName = strSettingsFileName;

		if (wszAppData == NULL)
		{
			hr = E_FAIL;
		}
		else
		{
			m_strSettingsPath	= wstring(wszAppData) + wstring(L"\\Console\\");
			m_settingsDirType	= dirTypeUser;

			hr = XmlHelper::OpenXmlDocument(
								GetSettingsFileName(), 
								m_pSettingsDocument, 
								m_pSettingsRoot);
		}

		if (FAILED(hr))
		{
			m_strSettingsPath	= Helpers::GetModulePath(NULL);
			m_settingsDirType	= dirTypeExe;

			hr = XmlHelper::OpenXmlDocument(
								GetSettingsFileName(), 
								m_pSettingsDocument, 
								m_pSettingsRoot);

			if (FAILED(hr)) return false;
		}
	}
	else
	{
		// settings file name with a path
		m_strSettingsPath		= strSettingsFileName.substr(0, pos+1);
		m_strSettingsFileName	= strSettingsFileName.substr(pos+1);

		wchar_t wszAppData[32767];
		::ZeroMemory(wszAppData, sizeof(wszAppData));
		::GetEnvironmentVariable(L"APPDATA", wszAppData, _countof(wszAppData));

		if (equals(m_strSettingsPath, wstring(wszAppData) + wstring(L"\\Console\\"), is_iequal()))
		{
			m_settingsDirType = dirTypeUser;
		}
		else if (equals(m_strSettingsPath, Helpers::GetModulePath(NULL), is_iequal()))
		{
			m_settingsDirType = dirTypeExe;
		}
		else
		{
			m_settingsDirType = dirTypeCustom;
		}

		hr = XmlHelper::OpenXmlDocument(
							strSettingsFileName, 
							m_pSettingsDocument, 
							m_pSettingsRoot);

		if (FAILED(hr)) return false;
	}

	// load settings' sections
	m_consoleSettings.Load(m_pSettingsRoot);
	m_appearanceSettings.Load(m_pSettingsRoot);
	m_behaviorSettings.Load(m_pSettingsRoot);
	m_hotKeys.Load(m_pSettingsRoot);
	m_mouseSettings.Load(m_pSettingsRoot);

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
	m_mouseSettings.Save(m_pSettingsRoot);
	m_tabSettings.Save(m_pSettingsRoot);

	HRESULT hr = m_pSettingsDocument->save(CComVariant(GetSettingsFileName().c_str()));

	return SUCCEEDED(hr) ? true : false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SettingsHandler::SetUserDataDir(SettingsDirType settingsDirType)
{
	if (settingsDirType == dirTypeExe)
	{
		m_strSettingsPath = Helpers::GetModulePath(NULL);
	}
	else if (settingsDirType == dirTypeUser)
	{
		wchar_t wszAppData[32767];
		::ZeroMemory(wszAppData, sizeof(wszAppData));
		::GetEnvironmentVariable(L"APPDATA", wszAppData, _countof(wszAppData));

		m_strSettingsPath = wstring(wszAppData) + wstring(L"\\Console\\");
		::CreateDirectory(m_strSettingsPath.c_str(), NULL);
	}

	m_settingsDirType = settingsDirType;
}

//////////////////////////////////////////////////////////////////////////////

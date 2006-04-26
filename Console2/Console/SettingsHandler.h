#pragma once

#include "resource.h"

#include <msxml.h>

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct SettingsBase
{
	virtual bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot) = 0;
	virtual bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot) = 0;

	static void AddTextNode(CComPtr<IXMLDOMDocument>& pDoc, CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrText);
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleSettings : public SettingsBase
{
	ConsoleSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	wstring		strShell;
	wstring		strInitialDir;

	DWORD		dwRefreshInterval;
	DWORD		dwChangeRefreshInterval;
	DWORD		dwRows;
	DWORD		dwColumns;
	DWORD		dwBufferRows;
	DWORD		dwBufferColumns;

	COLORREF	defaultConsoleColors[16];
	COLORREF	consoleColors[16];
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct FontSettings : public SettingsBase
{
	FontSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	wstring		strName;
	DWORD		dwSize;
	bool		bBold;
	bool		bItalic;

	bool		bUseColor;
	COLORREF	crFontColor;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct WindowSettings : public SettingsBase
{
	WindowSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	wstring			strTitle;
	wstring			strIcon;
	bool			bUseTabIcon;
	bool			bUseConsoleTitle;
	bool			bShowCommand;
	bool			bShowCommandInTabs;
	bool			bUseTabTitles;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ControlsSettings
{
	ControlsSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	bool			bShowMenu;
	bool			bShowToolbar;
	bool			bShowTabs;
	bool			bShowStatusbar;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct StylesSettings : public SettingsBase
{
	StylesSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	bool			bCaption;
	bool			bResizable;
	bool			bTaskbarButton;
	bool			bBorder;
	DWORD			dwInsideBoder;
	bool			bTrayIcon;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum DockPosition
{
	dockNone	= -1,
	dockTL		= 0,
	dockTR		= 1,
	dockBL		= 2,
	dockBR		= 3,
};

//////////////////////////////////////////////////////////////////////////////

enum ZOrder
{
	zorderNormal	= 0,
	zorderOnTop		= 1,
	zorderOnBottom	= 2
};

//////////////////////////////////////////////////////////////////////////////

struct PositionSettings : public SettingsBase
{
	PositionSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	int				nX;
	int				nY;
	ZOrder			zOrder;
	DockPosition	dockPosition;
	int				nSnapDistance;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum TransparencyType
{
	transNone		= 0,
	transAlpha		= 1,
	transColorKey	= 2
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct TransparencySettings : public SettingsBase
{
	TransparencySettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	TransparencyType	transType;
	BYTE				byActiveAlpha;
	BYTE				byInactiveAlpha;
	COLORREF			crColorKey;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct AppearanceSettings : public SettingsBase
{
	AppearanceSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	FontSettings			fontSettings;
	WindowSettings			windowSettings;
	ControlsSettings		controlsSettings;
	StylesSettings			stylesSettings;
	PositionSettings		positionSettings;
	TransparencySettings	transparencySettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct CopyPasteSettings : public SettingsBase
{
	CopyPasteSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	bool	bCopyOnSelect;
	bool	bNoWrap;
	bool	bTrimSpaces;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct MouseDragSettings : public SettingsBase
{
	MouseDragSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	bool	bMouseDrag;
	bool	bInverseShift;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct BehaviorSettings : public SettingsBase
{
	BehaviorSettings ();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	CopyPasteSettings	copyPasteSettings;
	MouseDragSettings	mouseDragSettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct HotKeys : public SettingsBase
{
	HotKeys();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	struct CommandData
	{
		CommandData(const wchar_t* pszCommand, const wchar_t* pszDescription, WORD commandID)
		: strCommand(pszCommand)
		, strDescription(pszDescription)
		, wCommandID(commandID)
		{
		}
		
		wstring	strCommand;
		wstring	strDescription;
		WORD	wCommandID;
	};

	struct HotkeyData
	{
		HotkeyData(DWORD commandID, ACCEL accel, bool extended)
		: dwCommandID(commandID)
		, accelHotkey(accel)
		, bExtended(extended)
		{
		}

		DWORD	dwCommandID;
		ACCEL	accelHotkey;
		bool	bExtended;
	};

	typedef vector<shared_ptr<CommandData> >		CommandsVector;
	typedef map<DWORD, shared_ptr<HotkeyData> >		HotKeysMap;

	HotKeysMap		mapHotKeys;
	CommandsVector	vecCommands;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum BackgroundImageType
{
	bktypeNone		= 0,
	bktypeImage		= 1,
	bktypeDesktop	= 2,
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct TabData
{
	TabData(const wstring& shell, const wstring& initialDir)
	: strTitle(L"Console")
	, strIcon(L"")
	, strShell(shell)
	, strInitialDir(initialDir)
	, dwCursorStyle(0)
	, crCursorColor(RGB(255, 255, 255))
	, backgroundImageType(bktypeNone)
	, crBackgroundColor(RGB(0, 0, 0))
	, imageData()
	{
	}

	// custom shell settings
	wstring							strTitle;
	wstring							strIcon;

	wstring							strShell;
	wstring							strInitialDir;

	DWORD							dwCursorStyle;
	COLORREF						crCursorColor;

	BackgroundImageType				backgroundImageType;
	COLORREF						crBackgroundColor;

	ImageData						imageData;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

typedef vector<shared_ptr<TabData> >	TabDataVector;

struct TabSettings : public SettingsBase
{
	TabSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	void SetDefaults(const wstring& defaultShell, const wstring& defaultInitialDir);

	TabDataVector	tabDataVector;

private:

	wstring			strDefaultShell;
	wstring			strDefaultInitialDir;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class SettingsHandler
{
	public:
		SettingsHandler();
		~SettingsHandler();

	public:

		bool LoadSettings(const wstring& strSettingsFileName);

		wstring	GetSettingsFileName() const { return m_strSettingsFileName; }

		ConsoleSettings& GetConsoleSettings() { return m_consoleSettings; }
		AppearanceSettings& GetAppearanceSettings() { return m_appearanceSettings; }
		BehaviorSettings& GetBehaviorSettings() { return m_behaviorSettings; }
		HotKeys& GetHotKeys() { return m_hotKeys; }
		TabSettings& GetTabSettings() { return m_tabSettings; }

	private:

		CComPtr<IXMLDOMDocument>	m_pOptionsDocument;
		CComPtr<IXMLDOMElement>		m_pOptionsRoot;

	private:

		wstring				m_strSettingsFileName;

		ConsoleSettings		m_consoleSettings;
		AppearanceSettings	m_appearanceSettings;
		BehaviorSettings	m_behaviorSettings;
		HotKeys				m_hotKeys;

		TabSettings			m_tabSettings;
};

//////////////////////////////////////////////////////////////////////////////

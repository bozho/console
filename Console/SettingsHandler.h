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
	virtual bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot) = 0;
	virtual bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot) = 0;

	static void AddTextNode(CComPtr<IXMLDOMDocument>& pDoc, CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrText);
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleSettings : public SettingsBase
{
	ConsoleSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	ConsoleSettings& operator=(const ConsoleSettings& other);

	wstring		strShell;
	wstring		strInitialDir;

	DWORD		dwRefreshInterval;
	DWORD		dwChangeRefreshInterval;
	DWORD		dwRows;
	DWORD		dwColumns;
	DWORD		dwBufferRows;
	DWORD		dwBufferColumns;

	bool		bStartHidden;
	bool		bSaveSize;

	COLORREF	defaultConsoleColors[16];
	COLORREF	consoleColors[16];
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

enum FontSmoothing
{
	fontSmoothDefault	= 0,
	fontSmoothNone		= 1,
	fontSmoothCleartype	= 2
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct FontSettings : public SettingsBase
{
	FontSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	FontSettings& operator=(const FontSettings& other);

	wstring			strName;
	DWORD			dwSize;
	bool			bBold;
	bool			bItalic;
	FontSmoothing	fontSmoothing;

	bool			bUseColor;
	COLORREF		crFontColor;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct WindowSettings : public SettingsBase
{
	WindowSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	WindowSettings& operator=(const WindowSettings& other);

	wstring			strTitle;
	wstring			strIcon;
	bool			bUseTabIcon;
	bool			bUseConsoleTitle;
	bool			bShowCommand;
	bool			bShowCommandInTabs;
	bool			bUseTabTitles;
	DWORD			dwTrimTabTitles;
	DWORD			dwTrimTabTitlesRight;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ControlsSettings
{
	ControlsSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	ControlsSettings& operator=(const ControlsSettings& other);

	bool			bShowMenu;
	bool			bShowToolbar;
	bool			bShowStatusbar;
	bool			bShowTabs;
	bool			bHideSingleTab;
	bool			bTabsOnBottom;
	bool			bShowScrollbars;
	bool			bFlatScrollbars;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct StylesSettings : public SettingsBase
{
	StylesSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	StylesSettings& operator=(const StylesSettings& other);

	bool			bCaption;
	bool			bResizable;
	bool			bTaskbarButton;
	bool			bBorder;
	DWORD			dwInsideBorder;
	bool			bTrayIcon;
	COLORREF		crSelectionColor;
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


//////////////////////////////////////////////////////////////////////////////

enum ZOrder
{
	zorderNormal	= 0,
	zorderOnTop		= 1,
	zorderOnBottom	= 2,
	zorderDesktop	= 3
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct PositionSettings : public SettingsBase
{
	PositionSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	PositionSettings& operator=(const PositionSettings& other);

	int				nX;
	int				nY;
	bool			bSavePosition;
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

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	TransparencySettings& operator=(const TransparencySettings& other);

	TransparencyType	transType;
	BYTE				byActiveAlpha;
	BYTE				byInactiveAlpha;
	COLORREF			crColorKey;

	static BYTE			minAlpha;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct AppearanceSettings : public SettingsBase
{
	AppearanceSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	AppearanceSettings& operator=(const AppearanceSettings& other);

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

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	CopyPasteSettings& operator=(const CopyPasteSettings& other);

	bool			bCopyOnSelect;
	bool			bClearOnCopy;
	bool			bSensitiveCopy;
	bool			bNoWrap;
	bool			bTrimSpaces;

	CopyNewlineChar	copyNewlineChar;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ScrollSettings : public SettingsBase
{
	ScrollSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	ScrollSettings& operator=(const ScrollSettings& other);

	DWORD	dwPageScrollRows;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct TabHighlightSettings : public SettingsBase
{
	TabHighlightSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	TabHighlightSettings& operator=(const TabHighlightSettings& other);

	DWORD	dwFlashes;
	bool	bStayHighlighted;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum AnimationDirection
{
	animDirNone		= 0,
	animDirPositive	= 1,
	animDirNegative	= 2
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum AnimationType
{
	animTypeNone	= 0,
	animTypeSlide	= 1,
	animTypeZoom	= 2,
	animTypeBlend	= 3
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct AnimateSettings : public SettingsBase
{
	AnimateSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	AnimateSettings& operator=(const AnimateSettings& other);

	DWORD	dwType;
	DWORD	dwHorzDirection;
	DWORD	dwVertDirection;
	DWORD	dwTime;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct BehaviorSettings : public SettingsBase
{
	BehaviorSettings ();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	BehaviorSettings& operator=(const BehaviorSettings& other);

	CopyPasteSettings		copyPasteSettings;
	ScrollSettings			scrollSettings;
	TabHighlightSettings	tabHighlightSettings;
//	AnimateSettings			animateSettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct HotKeys : public SettingsBase
{
	HotKeys();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	HotKeys& operator=(const HotKeys& other);

	struct CommandData
	{
		CommandData(const wstring& command, WORD commandID, const wstring& description, bool global = false)
		: strCommand(command)
		, wCommandID(commandID)
		, strDescription(description)
		, bExtended(false)
		, bGlobal(global)
		{
			::ZeroMemory(&accelHotkey, sizeof(ACCEL));
		}

		CommandData(const wstring& command, WORD commandID, const UINT descriptionID, bool global = false)
			: strCommand(command)
			, wCommandID(commandID)
			, bExtended(false)
			, bGlobal(global)
		{
			::ZeroMemory(&accelHotkey, sizeof(ACCEL));

			CAtlString description;
			description.LoadString (descriptionID);
			strDescription = description;
		}

		wstring	strCommand;
		WORD	wCommandID;
		wstring	strDescription;
		ACCEL	accelHotkey;
		bool	bExtended;
		bool	bGlobal;
	};

	struct command{};
	struct commandID{};

	typedef multi_index_container<
				shared_ptr<CommandData>,
				indexed_by
				<
					sequenced<>,
					ordered_unique<tag<command>,	member<CommandData, wstring, &CommandData::strCommand> >,
					ordered_unique<tag<commandID>,	member<CommandData, WORD, &CommandData::wCommandID> >
				> >									Commands;

	typedef nth_index<Commands,0>::type				CommandsSequence;
	typedef Commands::index<command>::type			CommandNameIndex;
	typedef Commands::index<commandID>::type		CommandIDIndex;

	Commands	commands;

	bool		bUseScrollLock;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct MouseSettings : public SettingsBase
{
	enum Command
	{
		cmdNone		= 0,
		cmdCopy		= 1,
		cmdSelect	= 2,
		cmdPaste	= 3,
		cmdDrag		= 4,
		cmdMenu		= 5,
	};

	enum Button
	{
		btnNone		= 0,
		btnLeft		= 1,
		btnRight	= 2,
		btnMiddle	= 3,
		btn4th		= 4,
		btn5th		= 5,
	};

	enum ModifierKeys
	{
		mkNone	= 0x00,
		mkCtrl	= 0x01,
		mkShift	= 0x02,
		mkAlt	= 0x04
	};

	enum ClickType
	{
		clickNone	= 0,
		clickSingle	= 1,
		clickDouble	= 2
	};
	
	MouseSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	MouseSettings& operator=(const MouseSettings& other);

	struct Action
	{
		Action()
		: button(btnNone)
		, modifiers(mkNone)
		, clickType(clickNone)
		{
		}

		bool operator==(const Action& other) const
		{
			if ((button == other.button) &&
				(modifiers == other.modifiers) &&
				(clickType == other.clickType))
			{
				return true;
			}

			return false;
		}

		Button		button;
		DWORD		modifiers;
		ClickType	clickType;
	};

	struct CommandData
	{
		CommandData(Command cmd, wstring strCmd, wstring strDesc)
		: command(cmd)
		, strCommand(strCmd)
		, strDescription(strDesc)
		{
		}

		CommandData(Command cmd, wstring strCmd, UINT descID)
		: command(cmd)
		, strCommand(strCmd)
		{
			CAtlString description;
			description.LoadString (descID);
			strDescription = description;
		}

		Command	command;
		wstring	strCommand;
		wstring	strDescription;
		Action	action;
	};

	struct commandID{};
	struct commandName{};

	typedef multi_index_container<
				shared_ptr<CommandData>,
				indexed_by
				<
					sequenced<>,
					ordered_unique<tag<commandName>,		member<CommandData, wstring, &CommandData::strCommand> >,
					ordered_unique<tag<commandID>,			member<CommandData, Command, &CommandData::command> >
				> >									Commands;

	typedef nth_index<Commands,0>::type				CommandsSequence;
	// these typedefs seem to produce internal compiler errors with 7.1 compiler
	// we'll use them where needed in the code
/*
	typedef Commands::index<commandID>::type		CommandIDIndex;
	typedef Commands::index<commandName>::type		CommandNameIndex;
*/

	Commands	commands;
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
	, bUseDefaultIcon(false)
	, strShell(shell)
	, strInitialDir(initialDir)
	, bRunAsUser(false)
	, strUser()
	, dwCursorStyle(0)
	, crCursorColor(RGB(255, 255, 255))
	, backgroundImageType(bktypeNone)
	, crBackgroundColor(RGB(0, 0, 0))
	, menuBitmap()
	, imageData()
	{
	}

	// custom shell settings
	wstring							strTitle;
	wstring							strIcon;
	bool							bUseDefaultIcon;

	wstring							strShell;
	wstring							strInitialDir;
	bool							bRunAsUser;
	wstring							strUser;

	DWORD							dwCursorStyle;
	COLORREF						crCursorColor;

	BackgroundImageType				backgroundImageType;
	COLORREF						crBackgroundColor;

	CBitmap							menuBitmap;

	ImageData						imageData;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

typedef vector<shared_ptr<TabData> >	TabDataVector;

struct TabSettings : public SettingsBase
{
	TabSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

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

		enum SettingsDirType
		{
			dirTypeExe		= 0,
			dirTypeUser		= 1,
			dirTypeCustom	=2
		};

	public:
		SettingsHandler();
		~SettingsHandler();

	public:

		bool LoadSettings(const wstring& strSettingsFileName);
		bool SaveSettings();

		wstring	GetSettingsFileName() const { return m_strSettingsPath+m_strSettingsFileName; }

		SettingsDirType GetSettingsDirType() const { return m_settingsDirType; }
		void SetUserDataDir(SettingsDirType settingsDirType);

		ConsoleSettings& GetConsoleSettings() { return m_consoleSettings; }
		AppearanceSettings& GetAppearanceSettings() { return m_appearanceSettings; }
		BehaviorSettings& GetBehaviorSettings() { return m_behaviorSettings; }
		HotKeys& GetHotKeys() { return m_hotKeys; }
		MouseSettings& GetMouseSettings() { return m_mouseSettings; }
		TabSettings& GetTabSettings() { return m_tabSettings; }

	private:

		CComPtr<IXMLDOMDocument>	m_pSettingsDocument;
		CComPtr<IXMLDOMElement>		m_pSettingsRoot;

	private:

		wstring				m_strSettingsPath;
		wstring				m_strSettingsFileName;

		SettingsDirType		m_settingsDirType;

		ConsoleSettings		m_consoleSettings;
		AppearanceSettings	m_appearanceSettings;
		BehaviorSettings	m_behaviorSettings;
		HotKeys				m_hotKeys;
		MouseSettings		m_mouseSettings;

		TabSettings			m_tabSettings;
};

//////////////////////////////////////////////////////////////////////////////

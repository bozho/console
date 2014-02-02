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
	BYTE		backgroundTextOpacity;
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
	DWORD			dwExtraWidth;
	bool			bBold;
	bool			bItalic;
	FontSmoothing	fontSmoothing;
	bool			bBoldIntensified;
	bool			bItalicIntensified;

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
	wstring			strMainTitleFormat;
	wstring			strTabTitleFormat;
	bool			bUseTabIcon;
	bool			bUseTabTitles;
	DWORD			dwTrimTabTitles;
	DWORD			dwTrimTabTitlesRight;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct FullScreenSettings : public SettingsBase
{
	FullScreenSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	FullScreenSettings& operator=(const FullScreenSettings& other);

	bool      bStartInFullScreen;
	DWORD     dwFullScreenMonitor;
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
	bool			bHideTabIcons;
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
	bool			bQuake;
	bool			bJumplist;
	bool			bIntegratedIME;
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

	bool         bSavePosition;
	bool         bSaveSize;
	int          nX;
	int          nY;
	int          nW;
	int          nH;
	ZOrder       zOrder;
	DockPosition dockPosition;
	int          nSnapDistance;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum TransparencyType
{
	transNone		= 0,
	transAlpha		= 1,
	transColorKey	= 2,
	transGlass		= 3
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

	FontSettings         fontSettings;
	WindowSettings       windowSettings;
	ControlsSettings     controlsSettings;
	StylesSettings       stylesSettings;
	PositionSettings     positionSettings;
	TransparencySettings transparencySettings;
	FullScreenSettings   fullScreenSettings;
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

	bool			bIncludeLeftDelimiter;
	bool			bIncludeRightDelimiter;
	std::wstring strLeftDelimiters;
	std::wstring strRightDelimiters;

	CopyNewlineChar	copyNewlineChar;
	DWORD			dwEOLSpaces;
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

struct CloseSettings : public SettingsBase
{
	CloseSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	CloseSettings& operator=(const CloseSettings& other);

	bool bAllowClosingLastView;
	bool bConfirmClosingMultipleViews;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct FocusSettings : public SettingsBase
{
	FocusSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	FocusSettings& operator=(const FocusSettings& other);

	bool bFollowMouse;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct InstanceSettings : public SettingsBase
{
	InstanceSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	InstanceSettings& operator=(const InstanceSettings& other);

	bool bAllowMultipleInstances;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct BehaviorSettings : public SettingsBase
{
	BehaviorSettings ();

	bool Load(const CComPtr<IXMLDOMElement>& pSettingsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pSettingsRoot);

	BehaviorSettings& operator=(const BehaviorSettings& other);

	CopyPasteSettings    copyPasteSettings;
	ScrollSettings       scrollSettings;
	TabHighlightSettings tabHighlightSettings;
	CloseSettings        closeSettings;
	FocusSettings        focusSettings;
	InstanceSettings     instanceSettings;
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
		, bWin(false)
		{
			::ZeroMemory(&accelHotkey, sizeof(ACCEL));
		}

    std::wstring GetHotKeyName(void)
    {
      std::wstring strHotKeyName;

      if( this->accelHotkey.fVirt || this->accelHotkey.key )
      {
        LONG    lScanCode;
        wchar_t szKeyName[32];

        if (this->accelHotkey.fVirt & FCONTROL)
        {
          lScanCode = ::MapVirtualKey(VK_CONTROL, 0) << 16;
          if( ::GetKeyNameText(lScanCode, szKeyName, ARRAYSIZE(szKeyName)) )
            strHotKeyName += szKeyName;
          strHotKeyName += L"+";
        }

        if (this->accelHotkey.fVirt & FSHIFT)
        {
          lScanCode = ::MapVirtualKey(VK_SHIFT, 0) << 16;
          if( ::GetKeyNameText(lScanCode, szKeyName, ARRAYSIZE(szKeyName)) )
            strHotKeyName += szKeyName;
          strHotKeyName += L"+";
        }

        if (this->accelHotkey.fVirt & FALT)
        {
          lScanCode = ::MapVirtualKey(VK_MENU, 0) << 16;
          if( ::GetKeyNameText(lScanCode, szKeyName, ARRAYSIZE(szKeyName)) )
            strHotKeyName += szKeyName;
          strHotKeyName += L"+";
        }

        lScanCode = ::MapVirtualKey(this->accelHotkey.key, 0) << 16;
        if (this->bExtended) lScanCode |= 0x01000000L;

        if( ::GetKeyNameText(lScanCode, szKeyName, ARRAYSIZE(szKeyName)) )
        {
          // it's used in menu, so we double &
          if (szKeyName[0] == L'&')
            strHotKeyName += L"&&";
          else
            strHotKeyName += szKeyName;
        }
      }

      return strHotKeyName;
    }

		wstring	strCommand;
		WORD	wCommandID;
		wstring	strDescription;
		ACCEL	accelHotkey;
		bool	bExtended;
		bool	bGlobal;
		bool	bWin;
	};

	struct command{};
	struct commandID{};

	typedef multi_index_container<
				std::shared_ptr<CommandData>,
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
		cmdNone         = 0,
		cmdCopy         = 1,
		cmdSelect       = 2,
		cmdPaste        = 3,
		cmdDrag         = 4,
		cmdMenu1        = 5,
		cmdMenu2        = 6,
		cmdMenu3        = 7,
		cmdColumnSelect = 8,
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

		Command	command;
		wstring	strCommand;
		wstring	strDescription;
		Action	action;
	};

	struct commandID{};
	struct commandName{};

	typedef multi_index_container<
				std::shared_ptr<CommandData>,
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
	, bNetOnly(false)
	, bRunAsAdministrator(false)
	, dwCursorStyle(0)
	, crCursorColor(RGB(255, 255, 255))
	, backgroundImageType(bktypeNone)
	, crBackgroundColor(RGB(0, 0, 0))
	, iconMenu()
	, imageData()
	, bInheritedColors(true)
	, nIndex(0)
	{
		consoleColors[0]  = 0x000000;
		consoleColors[1]  = 0x800000;
		consoleColors[2]  = 0x008000;
		consoleColors[3]  = 0x808000;
		consoleColors[4]  = 0x000080;
		consoleColors[5]  = 0x800080;
		consoleColors[6]  = 0x008080;
		consoleColors[7]  = 0xC0C0C0;
		consoleColors[8]  = 0x808080;
		consoleColors[9]  = 0xFF0000;
		consoleColors[10] = 0x00FF00;
		consoleColors[11] = 0xFFFF00;
		consoleColors[12] = 0x0000FF;
		consoleColors[13] = 0xFF00FF;
		consoleColors[14] = 0x00FFFF;
		consoleColors[15] = 0xFFFFFF;
	}

	// custom shell settings
	wstring							strTitle;
	wstring							strIcon;
	bool							bUseDefaultIcon;

	wstring							strShell;
	wstring							strInitialDir;
	bool							bRunAsUser;
	wstring							strUser;
	bool							bNetOnly;
	bool							bRunAsAdministrator;

	DWORD							dwCursorStyle;
	COLORREF						crCursorColor;

	BackgroundImageType				backgroundImageType;
	COLORREF						crBackgroundColor;

	CIcon							iconMenu;

	ImageData						imageData;

	bool							bInheritedColors;
	COLORREF						consoleColors[16];

	long                nIndex;

	void SetColors(const COLORREF colors[16], const bool bForced)
	{
		if (bInheritedColors || bForced)
			::CopyMemory(consoleColors, colors, sizeof(consoleColors));
	}

	HICON GetMenuIcon(void)
	{
		if (iconMenu.IsNull())
		{
			// load small icon
			iconMenu.Attach(Helpers::LoadTabIcon(false, bUseDefaultIcon, strIcon, strShell));
		}
		return iconMenu;
	}
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

typedef vector<std::shared_ptr<TabData> >	TabDataVector;

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
		wstring	GetSettingsPath() const { return m_strSettingsPath; }
		wstring	GetSettingsTitle() const { return m_strSettingsFileName; }

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

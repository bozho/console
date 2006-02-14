#pragma once

#include "resource.h"

#include <msxml.h>

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct SettingsBase {

	virtual bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot) = 0;
	virtual bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot) = 0;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleSettings : public SettingsBase {

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

struct FontSettings : public SettingsBase {

	FontSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	wstring		strName;
	DWORD		dwSize;
	bool		bBold;
	bool		bItalic;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum TransparencyType {

	transNone		= 0,
	transFake		= 1,
	transAlpha		= 2,
	transColorKey	= 3
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct TransparencySettings : public SettingsBase {

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

struct AppearanceSettings : public SettingsBase {

	AppearanceSettings();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	FontSettings			fontSettings;
	TransparencySettings	transparencySettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct HotKeys : public SettingsBase {

	HotKeys();

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	struct CommandData {
		
		CommandData(const wchar_t* pszCommand, const wchar_t* pszDescription, WORD commandID)
		: strCommand(pszCommand)
		, strDescription(pszDescription)
		, wCommandID(commandID)
		{}
		
		wstring	strCommand;
		wstring	strDescription;
		WORD	wCommandID;
	};

	struct HotkeyData {

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

struct TabData {

	TabData(const ConsoleSettings& conSettings)
	: strShell(conSettings.strShell)
	, strInitialDir(conSettings.strInitialDir)
	, strName(L"Console")
	, dwCursorStyle(0)
	, crCursorColor(RGB(255, 255, 255))
	, bImageBackground(false)
	, crBackgroundColor(RGB(0, 0, 0))
	, tabIcon()
	, tabBackground()
	, consoleSettings(conSettings)
	{
	}

	// custom shell settings
	wstring							strShell;
	wstring							strInitialDir;

	wstring							strName;
	DWORD							dwCursorStyle;
	COLORREF						crCursorColor;

	bool							bImageBackground;
	COLORREF						crBackgroundColor;

	CIcon							tabIcon;
	shared_ptr<ImageData>			tabBackground;

private:
	const ConsoleSettings&			consoleSettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

typedef vector<shared_ptr<TabData> >	TabDataVector;

struct TabSettings : public SettingsBase {

	TabSettings(const ConsoleSettings& conSettings);

	bool Load(const CComPtr<IXMLDOMElement>& pOptionsRoot);
	bool Save(const CComPtr<IXMLDOMElement>& pOptionsRoot);

	TabDataVector	tabDataVector;

private:
	const ConsoleSettings&			consoleSettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class SettingsHandler {

	public:
		SettingsHandler();
		~SettingsHandler();

	public:

		bool LoadSettings(const wstring& strOptionsFileName);

		ConsoleSettings& GetConsoleSettings() { return m_consoleSettings; }
		AppearanceSettings& GetAppearanceSettings() { return m_appearanceSettings; }
		HotKeys& GetHotKeys() { return m_hotKeys; }
		TabSettings& GetTabSettings() { return m_tabSettings; }

	private:

		CComPtr<IXMLDOMDocument>	m_pOptionsDocument;
		CComPtr<IXMLDOMElement>		m_pOptionsRoot;

	private:

		ConsoleSettings		m_consoleSettings;
		AppearanceSettings	m_appearanceSettings;
		HotKeys				m_hotKeys;

		TabSettings			m_tabSettings;
};

//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <msxml.h>

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////


struct ConsoleSettings {

	ConsoleSettings()
	: dwRefreshInterval(100)
	, dwChangeRefreshInterval(10)
	, dwRows(25)
	, dwColumns(80)
	, dwBufferRows(200)
	{
		consoleColors[0]	= 0x000000;
		consoleColors[1]	= 0x800000;
		consoleColors[2]	= 0x008000;
		consoleColors[3]	= 0x808000;
		consoleColors[4]	= 0x000080;
		consoleColors[5]	= 0x800080;
		consoleColors[6]	= 0x008080;
		consoleColors[7]	= 0xC0C0C0;
		consoleColors[8]	= 0x808080;
		consoleColors[9]	= 0xFF0000;
		consoleColors[10]	= 0x00FF00;
		consoleColors[11]	= 0xFFFF00;
		consoleColors[12]	= 0x0000FF;
		consoleColors[13]	= 0xFF00FF;
		consoleColors[14]	= 0x00FFFF;
		consoleColors[15]	= 0xFFFFFF;
	}

	DWORD		dwRefreshInterval;
	DWORD		dwChangeRefreshInterval;
	DWORD		dwRows;
	DWORD		dwColumns;
	DWORD		dwBufferRows;

	COLORREF	consoleColors[16];
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct FontSettings {

	FontSettings()
	: strName(L"Courier New")
	, dwSize(10)
	, bBold(false)
	, bItalic(false)
	{
	}

	wstring		strName;
	DWORD		dwSize;
	bool		bBold;
	bool		bItalic;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum TransparencyStyle {

	transNone		= 0,
	transAlpha		= 1,
	transColorKey	= 2,
	transFake		= 3
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct TransparencySettings {

	TransparencySettings()
	: transStyle(transNone)
	, byActiveAlpha(255)
	, byInactiveAlpha(255)
	{
	}

	TransparencyStyle	transStyle;
	BYTE				byActiveAlpha;
	BYTE				byInactiveAlpha;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct AppearanceSettings {

	AppearanceSettings() {}

	FontSettings			fontSettings;
	TransparencySettings	transparencySettings;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

#define ID_HOTKEY_FIRST		40000

struct HotKey {

	HotKey()
	: strCommand(L"")
	{
		::ZeroMemory(&hotKey, sizeof(ACCEL));
	}

	ACCEL	hotKey;
	wstring	strCommand;
};

//////////////////////////////////////////////////////////////////////////////

typedef vector<shared_ptr<HotKey> >		HotKeys;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct TabSettings {

	TabSettings()
	: strId(L"")
	, dwCursorStyle(0)
	, crCursorColor(RGB(255, 255, 255))
	{
	}

	wstring		strId;
	DWORD		dwCursorStyle;
	COLORREF	crCursorColor;
};

//////////////////////////////////////////////////////////////////////////////

typedef vector<shared_ptr<TabSettings> >	TabSettingsVector;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class SettingsHandler {

	public:
		SettingsHandler();
		~SettingsHandler();

	public:

		bool LoadOptions(const wstring& strOptionsFileName);

		ConsoleSettings& GetConsoleSettings() { return m_consoleSettings; }
		AppearanceSettings& GetAppearanceSettings() { return m_appearanceSettings; }
		HotKeys& GetHotKeys() { return m_hotKeys; }
		TabSettingsVector& GetTabSettings() { return m_tabSettings; }

	private:

		void LoadConsoleSettings();
		void LoadAppearanceSettings();
		void LoadHotKeys();
		void LoadTabSettings();

		void LoadFontSettings();
		void LoadTransparencySettings();

	private:

		HRESULT GetDomElement(const CComPtr<IXMLDOMNode>& pRootNode, const CComBSTR& bstrPath, CComPtr<IXMLDOMElement>& pElement);

		void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD& dwValue, DWORD dwDefaultValue);
		void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, BYTE& byValue, BYTE byDefaultValue);
		void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool& bValue, bool bDefaultValue);
		void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, wstring& strValue, const wstring& strDefaultValue);

		void GetRGBAttribute(const CComPtr<IXMLDOMElement>& pElement, COLORREF& crValue, COLORREF crDefaultValue);

	private:

		CComPtr<IXMLDOMDocument> m_pOptionsDocument;

	private:

		ConsoleSettings		m_consoleSettings;
		AppearanceSettings	m_appearanceSettings;
		HotKeys				m_hotKeys;

		TabSettingsVector	m_tabSettings;
};

//////////////////////////////////////////////////////////////////////////////

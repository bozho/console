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
	{
	}

	DWORD	dwRefreshInterval;
	DWORD	dwChangeRefreshInterval;
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

	wstring		strName;
	DWORD		dwSize;
	bool		bBold;
	bool		bItalic;

	COLORREF	consoleColors[16];
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
	: dwStyle(0)
	, crColor(RGB(255, 255, 255))
	{
	}

	DWORD		dwStyle;
	COLORREF	crColor;
};

//////////////////////////////////////////////////////////////////////////////

typedef map<wstring, shared_ptr<TabSettings> >	TabSettingsMap;

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
		FontSettings& GetFontSettings() { return m_fontSettings; }
		HotKeys& GetHotKeys() { return m_hotKeys; }
		shared_ptr<TabSettings> GetTabSettings(const wstring& strId);

	private:

		void LoadConsoleSettings();
		void LoadFontSettings();
		void LoadHotKeys();
		void LoadTabSettings();

	private:

		HRESULT GetDomElement(const CComPtr<IXMLDOMNode>& pRootNode, const CComBSTR& bstrPath, CComPtr<IXMLDOMElement>& pElement);

		void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD& dwValue, DWORD dwDefaultValue);
		void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool& bValue, bool bDefaultValue);
		void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, wstring& strValue, const wstring& strDefaultValue);

		void GetRGBAttribute(const CComPtr<IXMLDOMElement>& pElement, COLORREF& crValue, COLORREF crDefaultValue);

	private:

		CComPtr<IXMLDOMDocument> m_pOptionsDocument;

	private:

		ConsoleSettings	m_consoleSettings;
		FontSettings	m_fontSettings;
		HotKeys			m_hotKeys;

		TabSettingsMap	m_tabSettings;
};

//////////////////////////////////////////////////////////////////////////////

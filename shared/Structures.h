#pragma once

//////////////////////////////////////////////////////////////////////////////

struct ConsoleParams
{
	ConsoleParams()
	: dwParentProcessId(0)
	, dwNotificationTimeout(0)
	, dwRefreshInterval(0)
	, dwRows(0)
	, dwColumns(0)
	, dwBufferRows(0)
	, dwBufferColumns(0)
	, dwMaxRows(0)
	, dwMaxColumns(0)
	, hwndConsoleWindow(NULL)
	, dwHookThreadId(0)
	{
	}

	ConsoleParams(const ConsoleParams& other)
	: dwParentProcessId(other.dwParentProcessId)
	, dwNotificationTimeout(other.dwNotificationTimeout)
	, dwRefreshInterval(other.dwRefreshInterval)
	, dwRows(other.dwRows)
	, dwColumns(other.dwColumns)
	, dwBufferRows(other.dwBufferRows)
	, dwBufferColumns(other.dwBufferColumns)
	, dwMaxRows(other.dwMaxRows)
	, dwMaxColumns(other.dwMaxColumns)
	, hwndConsoleWindow(other.hwndConsoleWindow)
	, dwHookThreadId(other.dwHookThreadId)
	{
	}

	// startup stuff
	DWORD	dwParentProcessId;
	DWORD	dwNotificationTimeout;
	DWORD	dwRefreshInterval;
	DWORD	dwRows;
	DWORD	dwColumns;
	DWORD	dwBufferRows;
	DWORD	dwBufferColumns;

	// stuff set by console hook
	DWORD	dwMaxRows;
	DWORD	dwMaxColumns;
	union
	{
		HWND	hwndConsoleWindow;
		// padding for 32-bit processes started from 64-bit Console
		__int64	padding;
	};

	DWORD	dwHookThreadId;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleSize
{
	ConsoleSize()
	: dwRows(0)
	, dwColumns(0)
	, dwResizeWindowEdge(0)
	{
	}

	DWORD	dwRows;
	DWORD	dwColumns;

	// window edge used for resizing, one of WMSZ_* constants
	DWORD	dwResizeWindowEdge;
};


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum CopyNewlineChar
{
	newlineCRLF	= 0,
	newlineLF	= 1
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleCopy
{
	ConsoleCopy()
	: coordStart()
	, coordEnd()
	, bNoWrap(false)
	, bTrimSpaces(false)
	, copyNewlineChar(newlineCRLF)
	, bBold(false)
	, bItalic(false)
	, dwSize(24)
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

		szFontName[0] = 0;
	}

	COORD	coordStart;
	COORD	coordEnd;
	bool	bNoWrap;
	bool	bTrimSpaces;
	CopyNewlineChar	copyNewlineChar;
	bool  bBold;
	bool  bItalic;
	DWORD dwSize;
	char szFontName [256];
	COLORREF consoleColors[16];
};


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleInfo
{
	ConsoleInfo()
	: csbi()
	, textChanged(false)
	{
	}

	CONSOLE_SCREEN_BUFFER_INFO	csbi;
	bool						textChanged;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct CharInfo
{
	CharInfo()
	: changed(false)
	{
		// This is actually a shortcut for:
		// charInfo.Attributes = 0;
		// charInfo.Char.UnicodeChar = L' ';
		// Is it faster, though?
		*(reinterpret_cast<DWORD*>(&charInfo)) = 0x00000020;
	}

  inline void copy(CHAR_INFO* pnewCharInfo)
  {
    DWORD* pold = reinterpret_cast<DWORD*>(&charInfo);
    DWORD* pnew = reinterpret_cast<DWORD*>(pnewCharInfo);
    if( *pold != *pnew )
    {
      *pold = *pnew;
      changed = true;
    }
  }

	CHAR_INFO	charInfo;
	bool		changed;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct NamedPipeMessage
{
	enum
	{
		POSTMESSAGE,
		SENDMESSAGE,
		SHOWWINDOW,
		SETWINDOWPOS,
		SENDTEXT,
	} type;

	union
	{
		//POSTMESSAGE
		//SENDMESSAGE
		struct
		{
			UINT  msg;
			DWORD wparam;
			DWORD lparam;
		} winmsg;

		//SHOWWINDOW
		struct
		{
			int nCmdShow;
		} show;

		//SETWINDOWPOS
		struct
		{
			int  X;
			int  Y;
			int  cx;
			int  cy;
			UINT uFlags;
		} windowpos;

		//SENDTEXT
		struct
		{
			DWORD dwTextLen;
		} text;
	} data;
};

#pragma once

//////////////////////////////////////////////////////////////////////////////

struct ConsoleParams {

	ConsoleParams()
	: dwConsoleMainThreadId(0)
	, dwParentProcessId(0)
	, dwNotificationTimeout(0)
	, dwRefreshInterval(0)
	, dwRows(0)
	, dwColumns(0)
	, dwBufferRows(0)
	, dwMaxRows(0)
	, dwMaxColumns(0)
	, hwndConsoleWindow(NULL)
	{
	}

	ConsoleParams(DWORD consoleMainThreadId, DWORD parentProcessId, DWORD notificationTimeout, DWORD refreshInterval, DWORD rows, DWORD columns, DWORD bufferRows)
	: dwConsoleMainThreadId(consoleMainThreadId)
	, dwParentProcessId(parentProcessId)
	, dwNotificationTimeout(notificationTimeout)
	, dwRefreshInterval(refreshInterval)
	, dwRows(rows)
	, dwColumns(columns)
	, dwBufferRows(bufferRows)
	, dwMaxRows(0)
	, dwMaxColumns(0)
	, hwndConsoleWindow(NULL)
	{
	}

	ConsoleParams(const ConsoleParams& other)
	: dwConsoleMainThreadId(other.dwConsoleMainThreadId)
	, dwParentProcessId(other.dwParentProcessId)
	, dwNotificationTimeout(other.dwNotificationTimeout)
	, dwRefreshInterval(other.dwRefreshInterval)
	, dwRows(other.dwRows)
	, dwColumns(other.dwColumns)
	, dwBufferRows(other.dwBufferRows)
	, dwMaxRows(other.dwMaxRows)
	, dwMaxColumns(other.dwMaxColumns)
	, hwndConsoleWindow(other.hwndConsoleWindow)
	{
	}

	// startup stuff
	DWORD	dwConsoleMainThreadId;
	DWORD	dwParentProcessId;
	DWORD	dwNotificationTimeout;
	DWORD	dwRefreshInterval;
	DWORD	dwRows;
	DWORD	dwColumns;
	DWORD	dwBufferRows;

	// stuff set by console hook
	DWORD	dwMaxRows;
	DWORD	dwMaxColumns;
	HWND	hwndConsoleWindow;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ConsoleSize {

	ConsoleSize(DWORD rows, DWORD columns)
	: dwRows(rows)
	, dwColumns(columns)
	{
	}

	DWORD	dwRows;
	DWORD	dwColumns;
};


//////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////

struct ConsoleParams {

	ConsoleParams()
	: dwParentProcessId(0)
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

	ConsoleParams(DWORD parentProcessId, DWORD notificationTimeout, DWORD refreshInterval, DWORD rows, DWORD columns, DWORD bufferRows)
	: dwParentProcessId(parentProcessId)
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
	: dwParentProcessId(other.dwParentProcessId)
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

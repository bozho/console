#pragma once

//////////////////////////////////////////////////////////////////////////////

struct ConsoleStartupParams {

	ConsoleStartupParams(DWORD parentProcessId, DWORD notificationTimeout, DWORD refreshInterval, DWORD rows, DWORD columns, DWORD bufferRows)
	: dwParentProcessId(parentProcessId)
	, dwNotificationTimeout(notificationTimeout)
	, dwRefreshInterval(refreshInterval)
	, dwRows(rows)
	, dwColumns(columns)
	, dwBufferRows(bufferRows)
	, hwndConsoleWindow(NULL)
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


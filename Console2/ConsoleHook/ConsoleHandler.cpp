#include "stdafx.h"
using namespace std;
using namespace boost;

#include "../shared/SharedMemNames.h"
#include "ConsoleHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ConsoleHandler::ConsoleHandler()
: m_hParentProcess()
, m_consoleParams()
, m_consoleInfo()
, m_cursorInfo()
, m_consoleBuffer()
, m_consolePaste()
, m_newConsoleSize()
, m_newScrollPos()
, m_hMonitorThread()
, m_hMonitorThreadExit(shared_ptr<void>(::CreateEvent(NULL, FALSE, FALSE, NULL), ::CloseHandle))
, m_dwScreenBufferSize(0)
{
}

ConsoleHandler::~ConsoleHandler()
{
	StopMonitorThread();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD ConsoleHandler::StartMonitorThread()
{
	DWORD dwThreadId = 0;
	m_hMonitorThread = shared_ptr<void>(
							::CreateThread(
								NULL,
								0, 
								MonitorThreadStatic, 
								reinterpret_cast<void*>(this), 
								0, 
								&dwThreadId),
							::CloseHandle);

	return dwThreadId;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::StopMonitorThread()
{
	::SetEvent(m_hMonitorThreadExit.get());
	::WaitForSingleObject(m_hMonitorThread.get(), 10000);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::OpenSharedMemory()
{
	// open startup params  memory object
	DWORD dwProcessId = ::GetCurrentProcessId();

	// TODO: error handling
	m_consoleParams.Open((SharedMemNames::formatConsoleParams % dwProcessId).str());

	// open console info shared memory object
	m_consoleInfo.Open((SharedMemNames::formatInfo % dwProcessId).str());

	// open console info shared memory object
	m_cursorInfo.Open((SharedMemNames::formatCursorInfo % dwProcessId).str());

	// open console buffer shared memory object
	m_consoleBuffer.Open((SharedMemNames::formatBuffer % dwProcessId).str());

	// paste info 
	m_consolePaste.Open((SharedMemNames::formatPasteInfo % dwProcessId).str());

	// open new console size shared memory object
	m_newConsoleSize.Open((SharedMemNames::formatNewConsoleSize % dwProcessId).str());

	// new scroll position
	m_newScrollPos.Open((SharedMemNames::formatNewScrollPos % dwProcessId).str());

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ReadConsoleBuffer()
{
	// we take a fresh STDOUT handle - seems to work better (in case a program
	// has opened a new screen output buffer)
	shared_ptr<void> hStdOut(
						::CreateFile(
							L"CONOUT$",
							GENERIC_WRITE | GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							0,
							0),
							::CloseHandle);

	// get total console size
	CONSOLE_SCREEN_BUFFER_INFO	csbiConsole;
	COORD						coordConsoleSize;

	::GetConsoleScreenBufferInfo(hStdOut.get(), &csbiConsole);

	coordConsoleSize.X	= csbiConsole.srWindow.Right - csbiConsole.srWindow.Left + 1;
	coordConsoleSize.Y	= csbiConsole.srWindow.Bottom - csbiConsole.srWindow.Top + 1;

/*
	TRACE(L"ReadConsoleBuffer console buffer size: %ix%i\n", csbiConsole.dwSize.X, csbiConsole.dwSize.Y);
	TRACE(L"ReadConsoleBuffer console rect: %ix%i - %ix%i\n", csbiConsole.srWindow.Left, csbiConsole.srWindow.Top, csbiConsole.srWindow.Right, csbiConsole.srWindow.Bottom);
	TRACE(L"console window rect: (%i, %i) - (%i, %i)\n", csbiConsole.srWindow.Top, csbiConsole.srWindow.Left, csbiConsole.srWindow.Bottom, csbiConsole.srWindow.Right);
*/

	// do console output buffer reading
	DWORD					dwScreenBufferSize	= coordConsoleSize.X * coordConsoleSize.Y;
	DWORD					dwScreenBufferOffset= 0;

	shared_array<CHAR_INFO> pScreenBuffer(new CHAR_INFO[dwScreenBufferSize]);

	COORD		coordBufferSize;
	COORD		coordStart;
	SMALL_RECT	srBuffer;

//	TRACE(L"===================================================================\n");

	// start coordinates for the buffer are always (0, 0) - we use offset
	coordStart.X		= 0;
	coordStart.Y		= 0;

	// ReadConsoleOutput seems to fail for large (around 8k-CHAR_INFO's) buffers
	// here we calculate max buffer size (row count) for safe reading
	coordBufferSize.X	= csbiConsole.srWindow.Right - csbiConsole.srWindow.Left + 1;
	coordBufferSize.Y	= 8192 / coordBufferSize.X;

	// initialize reading rectangle
	srBuffer.Top		= csbiConsole.srWindow.Top;
	srBuffer.Bottom		= csbiConsole.srWindow.Top + coordBufferSize.Y - 1;
	srBuffer.Left		= csbiConsole.srWindow.Left;
	srBuffer.Right		= csbiConsole.srWindow.Left + csbiConsole.srWindow.Right - csbiConsole.srWindow.Left;

/*
	TRACE(L"Buffer size for loop reads: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);
	TRACE(L"-------------------------------------------------------------------\n");
*/

	// read rows 'chunks'
	for (SHORT i = 0; i < coordConsoleSize.Y / coordBufferSize.Y; ++i)
	{
//		TRACE(L"Reading region: (%i, %i) - (%i, %i)\n", srBuffer.Left, srBuffer.Top, srBuffer.Right, srBuffer.Bottom);

		::ReadConsoleOutput(
			hStdOut.get(), 
			pScreenBuffer.get() + dwScreenBufferOffset, 
			coordBufferSize, 
			coordStart, 
			&srBuffer);

		srBuffer.Top		= srBuffer.Top + coordBufferSize.Y;
		srBuffer.Bottom		= srBuffer.Bottom + coordBufferSize.Y;

		dwScreenBufferOffset += coordBufferSize.X * coordBufferSize.Y;
	}

	// read the last 'chunk', we need to calculate the number of rows in the
	// last chunk and update bottom coordinate for the region
	coordBufferSize.Y	= coordConsoleSize.Y - i * coordBufferSize.Y;
	srBuffer.Bottom		= csbiConsole.srWindow.Bottom;

/*
	TRACE(L"Buffer size for last read: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);
	TRACE(L"-------------------------------------------------------------------\n");
	TRACE(L"Reading region: (%i, %i) - (%i, %i)\n", srBuffer.Left, srBuffer.Top, srBuffer.Right, srBuffer.Bottom);
*/

	::ReadConsoleOutput(
		hStdOut.get(), 
		pScreenBuffer.get() + dwScreenBufferOffset, 
		coordBufferSize, 
		coordStart, 
		&srBuffer);


//	TRACE(L"===================================================================\n");

	// compare previous buffer, and if different notify Console
	if ((::memcmp(m_consoleInfo.Get(), &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO)) != 0) ||
		(m_dwScreenBufferSize != dwScreenBufferSize) ||
		(::memcmp(m_consoleBuffer.Get(), pScreenBuffer.get(), m_dwScreenBufferSize*sizeof(CHAR_INFO)) != 0))
	{
		SharedMemoryLock memLock(m_consoleBuffer);

		// update screen buffer variables
		m_dwScreenBufferSize = dwScreenBufferSize;
		::CopyMemory(m_consoleBuffer.Get(), pScreenBuffer.get(), m_dwScreenBufferSize*sizeof(CHAR_INFO));
		::CopyMemory(m_consoleInfo.Get(), &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
		::GetConsoleCursorInfo(hStdOut.get(), m_cursorInfo.Get());

		m_consoleBuffer.SetEvent();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ResizeConsoleWindow(HANDLE hStdOut, DWORD& dwColumns, DWORD& dwRows)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	::GetConsoleScreenBufferInfo(hStdOut, &csbi);
	TRACE(L"Console size: %ix%i\n", csbi.dwSize.X, csbi.dwSize.Y);

	// check against max size
	TRACE(L"Columns: %i\n", dwColumns);
	TRACE(L"Max columns: %i\n", m_consoleParams->dwMaxColumns);

	TRACE(L"Rows: %i\n", dwRows);
	TRACE(L"Max rows: %i\n", m_consoleParams->dwMaxRows);

	if (dwColumns > m_consoleParams->dwMaxColumns) dwColumns = m_consoleParams->dwMaxColumns;
	if (dwRows > m_consoleParams->dwMaxRows) dwRows = m_consoleParams->dwMaxRows;

	COORD		coordBufferSize;

	TRACE(L"Screen buffer: %ix%i\n", m_consoleParams->dwBufferRows, m_consoleParams->dwBufferColumns);

	if (m_consoleParams->dwBufferColumns == 0)
	{
		TRACE(L"1: %i\n", dwColumns);
		coordBufferSize.X = static_cast<SHORT>(dwColumns);
	}
	else
	{
		TRACE(L"2\n");
		coordBufferSize.X = static_cast<SHORT>(m_consoleParams->dwBufferColumns);
	}

	if (m_consoleParams->dwBufferRows == 0)
	{
		TRACE(L"3\n");
		coordBufferSize.Y = static_cast<SHORT>(dwRows);
	}
	else
	{
		TRACE(L"4\n");
		coordBufferSize.Y = static_cast<SHORT>(m_consoleParams->dwBufferRows);
	}
	
	SMALL_RECT	srConsoleRect;
	srConsoleRect.Top	= 0;
	srConsoleRect.Left	= 0;
	srConsoleRect.Right	= static_cast<SHORT>(dwColumns - 1);
	srConsoleRect.Bottom= static_cast<SHORT>(dwRows - 1);

	TRACE(L"New console size: %ix%i\n", srConsoleRect.Right, srConsoleRect.Bottom);
	TRACE(L"Buffer size: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);

/*
	::SetConsoleScreenBufferSize(hStdOut, coordBufferSize);
	::SetConsoleWindowInfo(hStdOut, TRUE, &srConsoleRect);
*/

	// order of setting window size and screen buffer size depends on current and desired dimensions
	if ((dwColumns < (DWORD) csbi.dwSize.X) ||
		((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) dwColumns * m_consoleParams->dwBufferRows))
	{
//		((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) m_consoleParams->dwBufferColumns * m_consoleParams->dwBufferRows)) {
		
		TRACE(L"Console 1\n");
/*
		if ((m_consoleParams->dwBufferRows > dwRows) && 
			(static_cast<DWORD>(csbi.dwSize.Y) > m_consoleParams->dwBufferRows))
		{
			
			TRACE(L"Console 1.1\n");
			coordBuffersSize.Y				= csbi.dwSize.Y;
			m_consoleParams->dwBufferRows	= static_cast<DWORD>(csbi.dwSize.Y);
		}
*/
		
		::SetConsoleWindowInfo(hStdOut, TRUE, &srConsoleRect);
		::SetConsoleScreenBufferSize(hStdOut, coordBufferSize);
		
		//	} else if (((DWORD)csbi.dwSize.X < m_dwColumns) || ((DWORD)csbi.dwSize.Y < m_dwBufferRows) || ((DWORD)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1) != m_dwRows)) {
	}
	else if ((dwRows < (DWORD) csbi.dwSize.Y) ||
			((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) dwColumns * m_consoleParams->dwBufferRows))
	{
//				((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) m_consoleParams->dwBufferColumns * m_consoleParams->dwBufferRows)) {

		// why did we need this???
/*
		if (csbi.dwSize.Y < m_consoleParams->dwBufferRows)
		{
			m_consoleParams->dwBufferRows = coordBuffersSize.Y = csbi.dwSize.Y;
		}
*/
		TRACE(L"Console 2\n");
		
		::SetConsoleScreenBufferSize(hStdOut, coordBufferSize);
		::SetConsoleWindowInfo(hStdOut, TRUE, &srConsoleRect);
	}


	::GetConsoleScreenBufferInfo(hStdOut, &csbi);

	dwColumns	= csbi.srWindow.Right - csbi.srWindow.Left + 1;
	dwRows		= csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	TRACE(L"console buffer size: %ix%i\n", csbi.dwSize.X, csbi.dwSize.Y);
	TRACE(L"console rect: %ix%i - %ix%i\n", csbi.srWindow.Left, csbi.srWindow.Top, csbi.srWindow.Right, csbi.srWindow.Bottom);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::PasteConsoleText(HANDLE hStdIn, const shared_ptr<wchar_t>& pszText)
{
	size_t	textLen			= wcslen(pszText.get());
	DWORD	dwTextWritten	= 0;
	
	scoped_array<INPUT_RECORD> pKeyEvents(new INPUT_RECORD[textLen]);
	::ZeroMemory(pKeyEvents.get(), sizeof(INPUT_RECORD)*textLen);
	
	for (size_t i = 0; i < textLen; ++i)
	{
		if ((pszText.get()[i] == L'\r') && (pszText.get()[i+1] == L'\n')) continue;

		pKeyEvents[i].EventType							= KEY_EVENT;
		pKeyEvents[i].Event.KeyEvent.bKeyDown			= TRUE;
		pKeyEvents[i].Event.KeyEvent.wRepeatCount		= 1;
		pKeyEvents[i].Event.KeyEvent.wVirtualKeyCode	= LOBYTE(::VkKeyScan(pszText.get()[i]));
		pKeyEvents[i].Event.KeyEvent.wVirtualScanCode	= 0;
		pKeyEvents[i].Event.KeyEvent.uChar.UnicodeChar	= pszText.get()[i];
		pKeyEvents[i].Event.KeyEvent.dwControlKeyState	= 0;
	}
	::WriteConsoleInput(hStdIn, pKeyEvents.get(), static_cast<DWORD>(textLen), &dwTextWritten);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ScrollConsole(HANDLE hStdOut, int nXDelta, int nYDelta)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	::GetConsoleScreenBufferInfo(hStdOut, &csbi);

	int nXCurrentPos = csbi.srWindow.Right - m_consoleParams->dwColumns + 1;
	int nYCurrentPos = csbi.srWindow.Bottom - m_consoleParams->dwRows + 1;

	// limit deltas
	nXDelta = max(-nXCurrentPos, min(nXDelta, (int)(m_consoleParams->dwBufferColumns-m_consoleParams->dwColumns) - nXCurrentPos));
	nYDelta = max(-nYCurrentPos, min(nYDelta, (int)(m_consoleParams->dwBufferRows-m_consoleParams->dwRows) - nYCurrentPos));

	SMALL_RECT sr;
	sr.Top		= static_cast<SHORT>(nYDelta);
	sr.Bottom	= static_cast<SHORT>(nYDelta);
	sr.Left		= static_cast<SHORT>(nXDelta);
	sr.Right	= static_cast<SHORT>(nXDelta);

	::SetConsoleWindowInfo(hStdOut, FALSE, &sr);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SetConsoleParams(HANDLE hStdOut)
{
	// get max console size
	COORD		coordMaxSize;
	coordMaxSize = ::GetLargestConsoleWindowSize(hStdOut);

	m_consoleParams->dwMaxRows		= coordMaxSize.Y;
	m_consoleParams->dwMaxColumns	= coordMaxSize.X;

	// check rows and columns
	if (m_consoleParams->dwRows > static_cast<DWORD>(coordMaxSize.Y)) m_consoleParams->dwRows = coordMaxSize.Y;
	if (m_consoleParams->dwColumns > static_cast<DWORD>(coordMaxSize.X)) m_consoleParams->dwColumns = coordMaxSize.X;

	// buffer rows cannot be less than max console size
//	if (m_consoleParams->dwBufferRows < static_cast<DWORD>(coordMaxSize.Y)) m_consoleParams->dwBufferRows = coordMaxSize.Y;
//	if (m_consoleParams->dwBufferColumns < static_cast<DWORD>(coordMaxSize.X)) m_consoleParams->dwBufferColumns = coordMaxSize.X;

	if ((m_consoleParams->dwBufferRows != 0) && (m_consoleParams->dwMaxRows > m_consoleParams->dwBufferRows)) m_consoleParams->dwMaxRows = m_consoleParams->dwBufferRows;
	if ((m_consoleParams->dwBufferColumns != 0) && (m_consoleParams->dwMaxColumns > m_consoleParams->dwBufferColumns)) m_consoleParams->dwMaxColumns = m_consoleParams->dwBufferColumns;

	// set console window handle
	m_consoleParams->hwndConsoleWindow = ::GetConsoleWindow();

	TRACE(L"Max columns: %i, max rows: %i\n", m_consoleParams->dwMaxColumns, m_consoleParams->dwMaxRows);

	// get initial window and cursor info
	::GetConsoleScreenBufferInfo(hStdOut, m_consoleInfo.Get());
	::GetConsoleCursorInfo(hStdOut, m_cursorInfo.Get());

	m_consoleParams.SetEvent();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ConsoleHandler::MonitorThreadStatic(LPVOID lpParameter)
{
	ConsoleHandler* pConsoleHandler = reinterpret_cast<ConsoleHandler*>(lpParameter);
	return pConsoleHandler->MonitorThread();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD ConsoleHandler::MonitorThread()
{
	TRACE(L"Hook!\n");

	// TODO: error handling
	// open shared memory objects
	OpenSharedMemory();
	
	// read parent process ID and get process handle
	m_hParentProcess = shared_ptr<void>(
							::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_consoleParams->dwParentProcessId),
							::CloseHandle);

	TRACE(L"Parent process handle: 0x%08X\n", m_hParentProcess.get());

	HANDLE	hStdOut			= ::GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE	hStdIn			= ::GetStdHandle(STD_INPUT_HANDLE);
	HANDLE	hStdErr			= ::GetStdHandle(STD_ERROR_HANDLE);

	SetConsoleParams(hStdOut);
	ResizeConsoleWindow(hStdOut, m_consoleParams->dwColumns, m_consoleParams->dwRows);

	HANDLE	arrWaitHandles[] =
	{
		m_hMonitorThreadExit.get(), 
		hStdOut, 
		hStdErr, 
		m_consolePaste.GetEvent(), 
		m_newConsoleSize.GetEvent(),
		m_newScrollPos.GetEvent()
	};

	DWORD	dwWaitRes		= 0;

	while ((dwWaitRes = ::WaitForMultipleObjects(
							sizeof(arrWaitHandles)/sizeof(arrWaitHandles[0]), 
							arrWaitHandles, 
							FALSE, 
							m_consoleParams->dwRefreshInterval)) != WAIT_OBJECT_0)
	{
		switch (dwWaitRes)
		{
			case WAIT_OBJECT_0 + 1 :
			case WAIT_OBJECT_0 + 2 :
				// something changed in the console
				::Sleep(m_consoleParams->dwNotificationTimeout);
			case WAIT_TIMEOUT :
			{
				// refresh timer
				ReadConsoleBuffer();
				::ResetEvent(hStdOut);
				::ResetEvent(hStdErr);
				break;
			}

			// paste request
			case WAIT_OBJECT_0 + 3 :
			{
				shared_ptr<wchar_t>	pszPasteBuffer(
										reinterpret_cast<wchar_t*>(*m_consolePaste.Get()),
										bind<BOOL>(::VirtualFreeEx, ::GetCurrentProcess(), _1, NULL, MEM_RELEASE));

				PasteConsoleText(hStdIn, pszPasteBuffer);
				break;
			}

			// console resize request
			case WAIT_OBJECT_0 + 4 :
			{
				SharedMemoryLock memLock(m_newConsoleSize);

				ResizeConsoleWindow(hStdOut, m_newConsoleSize->dwColumns, m_newConsoleSize->dwRows);
				ReadConsoleBuffer();

				::ResetEvent(hStdOut);
				::ResetEvent(hStdErr);
				break;
			}

			// console scroll request
			case WAIT_OBJECT_0 + 5 :
			{
				SharedMemoryLock memLock(m_newScrollPos);

				ScrollConsole(hStdOut, m_newScrollPos->cx, m_newScrollPos->cy);
				ReadConsoleBuffer();

				::ResetEvent(hStdOut);
				::ResetEvent(hStdErr);
				break;
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


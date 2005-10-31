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
, m_consoleBuffer()
, m_hMonitorThread()
, m_hMonitorThreadExit(shared_ptr<void>(::CreateEvent(NULL, FALSE, FALSE, NULL), ::CloseHandle))
, m_dwScreenBufferSize(0)
{
}

ConsoleHandler::~ConsoleHandler() {

	StopMonitorThread();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD ConsoleHandler::StartMonitorThread() {

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

void ConsoleHandler::StopMonitorThread() {

	::SetEvent(m_hMonitorThreadExit.get());
	::WaitForSingleObject(m_hMonitorThread.get(), 10000);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::OpenSharedMemory() {

	// open startup params  memory object
	DWORD dwProcessId = ::GetCurrentProcessId();

	// TODO: error handling
	m_consoleParams.Open((SharedMemNames::formatConsoleParams % dwProcessId).str());
	TRACE(L"Parent process id: %i\n", m_consoleParams->dwParentProcessId);

	// create console info shared memory object
	m_consoleInfo.Open((SharedMemNames::formatInfo % dwProcessId).str());
	TRACE(L"Console info address: 0x%08X\n", (void*)m_consoleInfo.Get());

	// open console buffer shared memory object
	m_consoleBuffer.Open((SharedMemNames::formatBuffer % dwProcessId).str());
	TRACE(L"Console buffer address: 0x%08X\n", (void*)m_consoleBuffer.Get());

	// open new console size shared memory object
	m_newConsoleSize.Open((SharedMemNames::formatNewConsoleSize % dwProcessId).str());
	TRACE(L"New console size address: 0x%08X\n", (void*)m_newConsoleSize.Get());

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ReadConsoleBuffer() {

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

	CONSOLE_SCREEN_BUFFER_INFO	csbiConsole;
	COORD						coordBufferSize;
	COORD						coordStart;

	::GetConsoleScreenBufferInfo(hStdOut.get(), &csbiConsole);

	coordStart.X		= 0;
	coordStart.Y		= 0;

	coordBufferSize.X	= csbiConsole.srWindow.Right - csbiConsole.srWindow.Left + 1;
	coordBufferSize.Y	= csbiConsole.srWindow.Bottom - csbiConsole.srWindow.Top + 1;

//	TRACE(L"console window rect: (%i, %i) - (%i, %i)\n", csbiConsole.srWindow.Top, csbiConsole.srWindow.Left, csbiConsole.srWindow.Bottom, csbiConsole.srWindow.Right);

	DWORD					dwScreenBufferSize = (coordBufferSize.X + 1) * (coordBufferSize.Y + 1);
	shared_array<CHAR_INFO> pScreenBuffer(new CHAR_INFO[dwScreenBufferSize]);

	::ReadConsoleOutput(
		hStdOut.get(), 
		pScreenBuffer.get(), 
		coordBufferSize, 
		coordStart, 
		&csbiConsole.srWindow);

//	TRACE(L"Console screen buffer size: %i\n", dwScreenBufferSize);

	// compare previous buffer, and if different notify Console
	if ((::memcmp(m_consoleInfo.Get(), &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO)) != 0) ||
		(m_dwScreenBufferSize != dwScreenBufferSize) ||
		(::memcmp(m_consoleBuffer.Get(), pScreenBuffer.get(), m_dwScreenBufferSize*sizeof(CHAR_INFO)) != 0)) {

		SharedMemoryLock memLock(m_consoleBuffer);

		// update row and column count
/*
		m_consoleParams->dwColumns	= coordBufferSize.X;
		m_consoleParams->dwRows		= coordBufferSize.Y;
*/

		// update screen buffer variables
		m_dwScreenBufferSize = dwScreenBufferSize;
		::CopyMemory(m_consoleBuffer.Get(), pScreenBuffer.get(), m_dwScreenBufferSize*sizeof(CHAR_INFO));
		::CopyMemory(m_consoleInfo.Get(), &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO));

		m_consoleBuffer.SetEvent();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ResizeConsoleWindow(HANDLE hStdOut, DWORD& dwColumns, DWORD& dwRows) {

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	::GetConsoleScreenBufferInfo(hStdOut, &csbi);

	// check against max size
	if (dwColumns > m_consoleParams->dwMaxColumns) dwColumns = m_consoleParams->dwMaxColumns;
	if (dwRows > m_consoleParams->dwMaxRows) dwRows = m_consoleParams->dwMaxRows;

	COORD		coordBuffersSize;
	coordBuffersSize.X = static_cast<SHORT>(dwColumns);
	coordBuffersSize.Y = static_cast<SHORT>(m_consoleParams->dwBufferRows);
	
	SMALL_RECT	srConsoleRect;
	srConsoleRect.Top	= 0;
	srConsoleRect.Left	= 0;
	srConsoleRect.Right	= static_cast<SHORT>(dwColumns - 1);
	srConsoleRect.Bottom= static_cast<SHORT>(dwRows - 1);

//	TRACE(L"Console size: %ix%i\n", csbi.dwSize.X, csbi.dwSize.Y);
	
	// order of setting window size and screen buffer size depends on current and desired dimensions
	if ((dwColumns < (DWORD) csbi.dwSize.X) ||
		((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) dwColumns * m_consoleParams->dwBufferRows)) {
		
		TRACE(L"Console 1\n");
		if ((m_consoleParams->dwBufferRows > dwRows) && 
			(static_cast<DWORD>(csbi.dwSize.Y) > m_consoleParams->dwBufferRows)) {
			
			coordBuffersSize.Y				= csbi.dwSize.Y;
			m_consoleParams->dwBufferRows	= static_cast<DWORD>(csbi.dwSize.Y);
		}
		
		::SetConsoleWindowInfo(hStdOut, TRUE, &srConsoleRect);
		::SetConsoleScreenBufferSize(hStdOut, coordBuffersSize);
		
		//	} else if (((DWORD)csbi.dwSize.X < m_dwColumns) || ((DWORD)csbi.dwSize.Y < m_dwBufferRows) || ((DWORD)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1) != m_dwRows)) {
	} else if ((dwRows < (DWORD) csbi.dwSize.Y) ||
				((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) dwColumns * m_consoleParams->dwBufferRows)) {

		// why did we need this???
/*
		if (csbi.dwSize.Y < m_consoleParams->dwBufferRows) {
			m_consoleParams->dwBufferRows = coordBuffersSize.Y = csbi.dwSize.Y;
		}
*/
		TRACE(L"Console 2\n");
		
		::SetConsoleScreenBufferSize(hStdOut, coordBuffersSize);
		::SetConsoleWindowInfo(hStdOut, TRUE, &srConsoleRect);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SetConsoleParams(HANDLE hStdOut) {

	// get max console size
	COORD		coordMaxSize;
	coordMaxSize = ::GetLargestConsoleWindowSize(hStdOut);

	m_consoleParams->dwMaxColumns	= coordMaxSize.X;
	m_consoleParams->dwMaxRows		= coordMaxSize.Y;

//	TRACE(L"Max columns: %i, max rows: %i\n", coordMaxSize.X, coordMaxSize.Y);

	// check rows and columns
	if (m_consoleParams->dwColumns > static_cast<DWORD>(coordMaxSize.X)) m_consoleParams->dwColumns = coordMaxSize.X;
	if (m_consoleParams->dwRows > static_cast<DWORD>(coordMaxSize.Y)) m_consoleParams->dwRows = coordMaxSize.Y;

	// buffer rows cannot be smaller than max size
	if (m_consoleParams->dwBufferRows < static_cast<DWORD>(coordMaxSize.X)) m_consoleParams->dwBufferRows = coordMaxSize.X;

	// set console window handle
	m_consoleParams->hwndConsoleWindow = ::GetConsoleWindow();

	m_consoleParams.SetEvent();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ConsoleHandler::MonitorThreadStatic(LPVOID lpParameter) {

	ConsoleHandler* pConsoleHandler = reinterpret_cast<ConsoleHandler*>(lpParameter);
	return pConsoleHandler->MonitorThread();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD ConsoleHandler::MonitorThread() {

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
	HANDLE	hStdErr			= ::GetStdHandle(STD_ERROR_HANDLE);

	SetConsoleParams(hStdOut);
	ResizeConsoleWindow(hStdOut, m_consoleParams->dwColumns, m_consoleParams->dwRows);

	HANDLE	arrWaitHandles[]= { m_hMonitorThreadExit.get(), hStdOut, hStdErr, m_newConsoleSize.GetEvent() };
	DWORD	dwWaitRes		= 0;

	while ((dwWaitRes = ::WaitForMultipleObjects(sizeof(arrWaitHandles)/sizeof(arrWaitHandles[0]), arrWaitHandles, FALSE, m_consoleParams->dwRefreshInterval)) != WAIT_OBJECT_0) {

		switch (dwWaitRes) {

			case WAIT_OBJECT_0 + 1 :
			case WAIT_OBJECT_0 + 2 :
				// something changed in the console
				::Sleep(m_consoleParams->dwNotificationTimeout);
			case WAIT_TIMEOUT : {
				// refresh timer
				ReadConsoleBuffer();
				::ResetEvent(hStdOut);
				::ResetEvent(hStdErr);
				break;
			}

			case WAIT_OBJECT_0 + 3 : {

				SharedMemoryLock memLock(m_newConsoleSize);

				TRACE(L"hua!\n");
				ResizeConsoleWindow(hStdOut, m_newConsoleSize->dwColumns, m_newConsoleSize->dwRows);

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


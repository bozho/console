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
, m_startupParams()
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
	m_startupParams.Open((SharedMemNames::formatConsoleStartupParams % dwProcessId).str());
	TRACE(L"Parent process id: %i\n", m_startupParams->dwParentProcessId);

	// create console info shared memory object
	m_consoleInfo.Open((SharedMemNames::formatInfo % dwProcessId).str());
	TRACE(L"Console info address: 0x%08X\n", (void*)m_consoleInfo.Get());

	// open console buffer shared memory object
	m_consoleBuffer.Open((SharedMemNames::formatBuffer % dwProcessId).str());
	TRACE(L"Console buffer address: 0x%08X\n", (void*)m_consoleBuffer.Get());

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
//	SMALL_RECT					srRegion;

	::GetConsoleScreenBufferInfo(hStdOut.get(), &csbiConsole);

	coordStart.X		= 0;
	coordStart.Y		= 0;

	coordBufferSize.X	= csbiConsole.srWindow.Right - csbiConsole.srWindow.Left + 1;
	coordBufferSize.Y	= csbiConsole.srWindow.Bottom - csbiConsole.srWindow.Top + 1;

//	::CopyMemory(&srRegion, &csbiConsole.srWindow, sizeof(SMALL_RECT));

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

		// update screen buffer variables
		m_dwScreenBufferSize = dwScreenBufferSize;
		::CopyMemory(m_consoleBuffer.Get(), pScreenBuffer.get(), m_dwScreenBufferSize*sizeof(CHAR_INFO));
		::CopyMemory(m_consoleInfo.Get(), &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO));

		m_consoleBuffer.SetEvent();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ResizeConsoleWindow(HANDLE hStdOut) {

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	::GetConsoleScreenBufferInfo(hStdOut, &csbi);

	COORD		coordBuffersSize;
	coordBuffersSize.X = static_cast<SHORT>(m_startupParams->dwColumns);
	coordBuffersSize.Y = static_cast<SHORT>(m_startupParams->dwBufferRows);
	
	SMALL_RECT	srConsoleRect;
	srConsoleRect.Top	= 0;
	srConsoleRect.Left	= 0;
	srConsoleRect.Right	= static_cast<SHORT>(m_startupParams->dwColumns - 1);
	srConsoleRect.Bottom= static_cast<SHORT>(m_startupParams->dwRows - 1);

//	TRACE(L"Console size: %ix%i\n", csbi.dwSize.X, csbi.dwSize.Y);
	
	// order of setting window size and screen buffer size depends on current and desired dimensions
	if ((m_startupParams->dwColumns < (DWORD) csbi.dwSize.X) ||
		((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) m_startupParams->dwColumns * m_startupParams->dwBufferRows)) {
		
//		TRACE(L"Console 1\n");
		if ((m_startupParams->dwBufferRows > m_startupParams->dwRows) && 
			(static_cast<DWORD>(csbi.dwSize.Y) > m_startupParams->dwBufferRows)) {
			
			coordBuffersSize.Y				= csbi.dwSize.Y;
			m_startupParams->dwBufferRows	= static_cast<DWORD>(csbi.dwSize.Y);
		}
		
		::SetConsoleWindowInfo(hStdOut, TRUE, &srConsoleRect);
		::SetConsoleScreenBufferSize(hStdOut, coordBuffersSize);
		
		//	} else if (((DWORD)csbi.dwSize.X < m_dwColumns) || ((DWORD)csbi.dwSize.Y < m_dwBufferRows) || ((DWORD)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1) != m_dwRows)) {
	} else if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) m_startupParams->dwColumns * m_startupParams->dwBufferRows) {

		// why did we need this???
/*
		if (csbi.dwSize.Y < m_startupParams->dwBufferRows) {
			m_startupParams->dwBufferRows = coordBuffersSize.Y = csbi.dwSize.Y;
		}
*/
//		TRACE(L"Console 2\n");
		
		::SetConsoleScreenBufferSize(hStdOut, coordBuffersSize);
		::SetConsoleWindowInfo(hStdOut, TRUE, &srConsoleRect);
	}
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
							::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_startupParams->dwParentProcessId),
							::CloseHandle);

	TRACE(L"Parent process handle: 0x%08X\n", m_hParentProcess.get());

	HANDLE	hStdOut			= ::GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE	hStdErr			= ::GetStdHandle(STD_ERROR_HANDLE);

	ResizeConsoleWindow(hStdOut);

	// set console window handle
	m_startupParams->hwndConsoleWindow = ::GetConsoleWindow();
	m_startupParams.SetEvent();

	HANDLE	arrWaitHandles[]= { m_hMonitorThreadExit.get(), hStdOut, hStdErr };
	DWORD	dwWaitRes		= 0;

	while ((dwWaitRes = ::WaitForMultipleObjects(sizeof(arrWaitHandles)/sizeof(arrWaitHandles[0]), arrWaitHandles, FALSE, m_startupParams->dwRefreshInterval)) != WAIT_OBJECT_0) {

		switch (dwWaitRes) {

			case WAIT_OBJECT_0 + 1 :
			case WAIT_OBJECT_0 + 2 :
				::Sleep(m_startupParams->dwNotificationTimeout);
			case WAIT_TIMEOUT : {
				// something changed in the console
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


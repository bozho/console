#include "stdafx.h"

#include "Console.h"

#include "../shared/SharedMemNames.h"
#include "ConsoleHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ConsoleHandler::ConsoleHandler()
: m_hConsoleProcess()
, m_consoleParams()
, m_consoleInfo()
, m_consoleBuffer()
, m_consoleCopyInfo()
, m_consolePasteInfo()
, m_consoleMouseEvent()
, m_newConsoleSize()
, m_newScrollPos()
, m_hMonitorThread()
, m_hMonitorThreadExit(shared_ptr<void>(::CreateEvent(NULL, FALSE, FALSE, NULL), ::CloseHandle))
{
}

ConsoleHandler::~ConsoleHandler()
{
	StopMonitorThread();
	
	if ((m_consoleParams.Get() != NULL) && 
		(m_consoleParams->hwndConsoleWindow))
	{
		::SendMessage(m_consoleParams->hwndConsoleWindow, WM_CLOSE, 0, 0);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SetupDelegates(ConsoleChangeDelegate consoleChangeDelegate, ConsoleCloseDelegate consoleCloseDelegate)
{
	m_consoleChangeDelegate	= consoleChangeDelegate;
	m_consoleCloseDelegate	= consoleCloseDelegate;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::StartShellProcess(const wstring& strCustomShell, const wstring& strInitialDir, const wstring& strInitialCmd, const wstring& strConsoleTitle, DWORD dwStartupRows, DWORD dwStartupColumns, bool bDebugFlag)
{
	wstring	strShellCmdLine(strCustomShell);
	
	if (strShellCmdLine.length() == 0)
	{
		wchar_t	szComspec[MAX_PATH];

		::ZeroMemory(szComspec, MAX_PATH*sizeof(wchar_t));

		if (::GetEnvironmentVariable(L"COMSPEC", szComspec, MAX_PATH) > 0)
		{
			strShellCmdLine = szComspec;		
		}
		else
		{
			strShellCmdLine = L"cmd.exe";
		}
	}

	if (strInitialCmd.length() > 0)
	{
		strShellCmdLine += L" ";
		strShellCmdLine += strInitialCmd;
	}

	wstring	strStartupTitle(strConsoleTitle);

	if (strStartupTitle.length() == 0)
	{
		strStartupTitle = L"Console2 command window";
//		strStartupTitle = str(wformat(L"Console2 command window 0x%08X") % this);
	}

	wstring strStartupDir(strInitialDir);

	if (strStartupDir.length())
	{
		// check if startup directory exists
		DWORD dwDirAttributes = ::GetFileAttributes(strStartupDir.c_str());

		if ((dwDirAttributes == INVALID_FILE_ATTRIBUTES) ||
			(dwDirAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			// no directory, use Console.exe directory
			strStartupDir = Helpers::GetModulePath(NULL);
		}
	}

	// setup the startup info struct
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(STARTUPINFO));
	si.dwFlags		= STARTF_USESHOWWINDOW;
	si.cb			= sizeof(STARTUPINFO);
	si.wShowWindow	= SW_HIDE;
	si.lpTitle		= const_cast<wchar_t*>(strStartupTitle.c_str());

	PROCESS_INFORMATION pi;
	DWORD				dwStartupFlags = CREATE_NEW_CONSOLE|CREATE_SUSPENDED;

	// TODO: not supported yet
	//if (bDebugFlag) dwStartupFlags |= DEBUG_PROCESS;

	if (!::CreateProcess(
			NULL,
			const_cast<wchar_t*>(Helpers::ExpandEnvironmentStrings(strShellCmdLine).c_str()),
			NULL,
			NULL,
			FALSE,
			dwStartupFlags,
			NULL,
			(strStartupDir.length() > 0) ? const_cast<wchar_t*>(Helpers::ExpandEnvironmentStrings(strStartupDir).c_str()) : NULL,
			&si,
			&pi))
	{
		return false;
	}

	// create shared memory objects
	CreateSharedObjects(pi.dwProcessId);

	// write startup params
	m_consoleParams->dwConsoleMainThreadId	= pi.dwThreadId;
	m_consoleParams->dwParentProcessId		= ::GetCurrentProcessId();
	m_consoleParams->dwNotificationTimeout	= g_settingsHandler->GetConsoleSettings().dwChangeRefreshInterval;
	m_consoleParams->dwRefreshInterval		= g_settingsHandler->GetConsoleSettings().dwRefreshInterval;
	m_consoleParams->dwRows					= dwStartupRows;
	m_consoleParams->dwColumns				= dwStartupColumns;
	m_consoleParams->dwBufferRows			= g_settingsHandler->GetConsoleSettings().dwBufferRows;
	m_consoleParams->dwBufferColumns		= g_settingsHandler->GetConsoleSettings().dwBufferColumns;

	m_hConsoleProcess = shared_ptr<void>(pi.hProcess, ::CloseHandle);

	// inject our hook DLL into console process
	if (!InjectHookDLL()) return false;

	// resume the console process
	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hThread);

	// wait for hook DLL to set console handle
	if (::WaitForSingleObject(m_consoleParams.GetReqEvent(), 10000) == WAIT_TIMEOUT) return false;

	return true;
}

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

void ConsoleHandler::SendMouseEvent(const COORD& mousePos, DWORD dwMouseButtonState, DWORD dwControlKeyState, DWORD dwEventFlags)
{
	{
		SharedMemoryLock	memLock(m_consoleMouseEvent);

		// TODO: implement
		m_consoleMouseEvent->dwMousePosition	= mousePos;
		m_consoleMouseEvent->dwButtonState		= dwMouseButtonState;
		m_consoleMouseEvent->dwControlKeyState	= dwControlKeyState;
		m_consoleMouseEvent->dwEventFlags		= dwEventFlags;

		m_consoleMouseEvent.SetReqEvent();
	}

	::WaitForSingleObject(m_consoleMouseEvent.GetRespEvent(), INFINITE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::CreateSharedObjects(DWORD dwConsoleProcessId)
{
	// create startup params shared memory
	m_consoleParams.Create((SharedMemNames::formatConsoleParams % dwConsoleProcessId).str(), 1, syncObjRequest);

	// create console info shared memory
	m_consoleInfo.Create((SharedMemNames::formatInfo % dwConsoleProcessId).str(), 1, syncObjRequest);

	// create console info shared memory
	m_cursorInfo.Create((SharedMemNames::formatCursorInfo % dwConsoleProcessId).str(), 1, syncObjRequest);

	// TODO: max console size
	m_consoleBuffer.Create((SharedMemNames::formatBuffer % dwConsoleProcessId).str(), 200*200, syncObjRequest);

	// copy info
	m_consoleCopyInfo.Create((SharedMemNames::formatCopyInfo % dwConsoleProcessId).str(), 1, syncObjBoth);

	// paste info (used for pasting and sending text to console)
	m_consolePasteInfo.Create((SharedMemNames::formatPasteInfo % dwConsoleProcessId).str(), 1, syncObjBoth);

	// mouse event
	m_consoleMouseEvent.Create((SharedMemNames::formatMouseEvent % dwConsoleProcessId).str(), 1, syncObjBoth);

	// new console size
	m_newConsoleSize.Create((SharedMemNames::formatNewConsoleSize % dwConsoleProcessId).str(), 1, syncObjRequest);

	// new scroll position
	m_newScrollPos.Create((SharedMemNames::formatNewScrollPos % dwConsoleProcessId).str(), 1, syncObjRequest);

	// TODO: separate function for default settings
	m_consoleParams->dwRows		= 25;
	m_consoleParams->dwColumns	= 80;


	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::InjectHookDLL()
{
	// allocate memory for parameter in the remote process
	wstring				strHookDllPath(GetModulePath(NULL) + wstring(L"\\ConsoleHook.dll"));

	if (::GetFileAttributes(strHookDllPath.c_str()) == INVALID_FILE_ATTRIBUTES) return false;

	shared_ptr<void>	hRemoteThread;
	shared_ptr<wchar_t>	pszHookDllPathRemote(
							static_cast<wchar_t*>(::VirtualAllocEx(
														m_hConsoleProcess.get(), 
														NULL, 
														strHookDllPath.length()*sizeof(wchar_t), 
														MEM_COMMIT, 
														PAGE_READWRITE)),
							bind<BOOL>(::VirtualFreeEx, m_hConsoleProcess.get(), _1, NULL, MEM_RELEASE));


#pragma warning(push)
#pragma warning(disable: 4127)

	if (pszHookDllPathRemote.get() == NULL) return false;

	// write the memory
	if (!::WriteProcessMemory(
				m_hConsoleProcess.get(), 
				(PVOID)pszHookDllPathRemote.get(), 
				(PVOID)strHookDllPath.c_str(), 
				strHookDllPath.length()*sizeof(wchar_t), 
				NULL))
	{
		return false;
	}

	// get address to LoadLibraryW function
	PTHREAD_START_ROUTINE pfnThreadRoutine = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(L"Kernel32.dll"), "LoadLibraryW");
	if (pfnThreadRoutine == NULL) return false;

	// start the remote thread
	hRemoteThread = shared_ptr<void>(
						::CreateRemoteThread(
							m_hConsoleProcess.get(), 
							NULL, 
							0, 
							pfnThreadRoutine, 
							(PVOID)pszHookDllPathRemote.get(), 
							0, 
							NULL),
						::CloseHandle);

	if (hRemoteThread.get() == NULL) return false;

	// wait for the thread to finish
//	::WaitForSingleObject(hRemoteThread.get(), INFINITE);
	if (::WaitForSingleObject(hRemoteThread.get(), 10000) == WAIT_TIMEOUT) return false;

#pragma warning(pop)

	return true;
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
	HANDLE arrWaitHandles[] = { m_hConsoleProcess.get(), m_hMonitorThreadExit.get(), m_consoleBuffer.GetReqEvent() };
	while (::WaitForMultipleObjects(sizeof(arrWaitHandles)/sizeof(arrWaitHandles[0]), arrWaitHandles, FALSE, INFINITE) > WAIT_OBJECT_0 + 1)
	{
		DWORD				dwColumns	= m_consoleInfo->srWindow.Right - m_consoleInfo->srWindow.Left + 1;
		DWORD				dwRows		= m_consoleInfo->srWindow.Bottom - m_consoleInfo->srWindow.Top + 1;
		DWORD				dwBufferColumns	= m_consoleInfo->dwSize.X;
		DWORD				dwBufferRows	= m_consoleInfo->dwSize.Y;
		bool				bResize		= false;

		if ((m_consoleParams->dwColumns != dwColumns) ||
			(m_consoleParams->dwRows != dwRows) ||
			((m_consoleParams->dwBufferColumns != 0) && (m_consoleParams->dwBufferColumns != dwBufferColumns)) ||
			((m_consoleParams->dwBufferRows != 0) && (m_consoleParams->dwBufferRows != dwBufferRows)))
		{
			m_consoleParams->dwColumns	= dwColumns;
			m_consoleParams->dwRows		= dwRows;

			// TODO: improve this
			// this will handle console applications that change console buffer 
			// size (like Far manager).
			// This is not a perfect solution, but it's the best one I have
			// for now
			if (m_consoleParams->dwBufferColumns != 0)	m_consoleParams->dwBufferColumns= dwBufferColumns;
			if (m_consoleParams->dwBufferRows != 0)		m_consoleParams->dwBufferRows	= dwBufferRows;
			bResize = true;
		}

		m_consoleChangeDelegate(bResize);
	}

	TRACE(L"exiting thread\n");
	// exiting thread
	m_consoleCloseDelegate();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wstring ConsoleHandler::GetModulePath(HMODULE hModule)
{
	wchar_t szModulePath[MAX_PATH+1];
	::ZeroMemory(szModulePath, (MAX_PATH+1)*sizeof(wchar_t));

	::GetModuleFileName(hModule, szModulePath, MAX_PATH);

	wstring strPath(szModulePath);

	return strPath.substr(0, strPath.rfind(L'\\'));
}

//////////////////////////////////////////////////////////////////////////////


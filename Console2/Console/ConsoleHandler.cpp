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
, m_consolePaste()
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

bool ConsoleHandler::StartShellProcess(const wstring& strCustomShell, const wstring& strInitialDir, const wstring& strConsoleTitle, DWORD dwStartupRows, DWORD dwStartupColumns)
{
	wstring	strShell(strCustomShell);
	
	if (strShell.length() == 0)
	{
		wchar_t	szComspec[MAX_PATH];

		::ZeroMemory(szComspec, MAX_PATH*sizeof(wchar_t));

		if (::GetEnvironmentVariable(L"COMSPEC", szComspec, MAX_PATH) > 0)
		{
			strShell = szComspec;		
		}
		else
		{
			strShell = L"cmd.exe";
		}
	}

	// TODO: build command line
	wstring	strShellCmdLine(strShell);
	wstring	strStartupTitle(strConsoleTitle);

	if (strStartupTitle.length() == 0)
	{
		strStartupTitle = str(wformat(L"Console command window 0x%08X") % this);
	}

	// setup the startup info struct
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(STARTUPINFO));
	si.dwFlags		= STARTF_USESHOWWINDOW;
	si.cb			= sizeof(STARTUPINFO);
	si.wShowWindow	= SW_HIDE;
	si.lpTitle		= (wchar_t*)(strStartupTitle.c_str());

	PROCESS_INFORMATION pi;

	if (!::CreateProcess(
			NULL,
			(wchar_t*)(strShellCmdLine.c_str()),
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE|CREATE_SUSPENDED,
			NULL,
			(strInitialDir.length() > 0) ? strInitialDir.c_str() : NULL,
			&si,
			&pi))
	{
		return false;
	}

	// create shared memory objects
	CreateSharedMemory(pi.dwProcessId);

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
	if (::WaitForSingleObject(m_consoleParams.GetEvent(), 10000) == WAIT_TIMEOUT) return false;

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
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::CreateSharedMemory(DWORD dwConsoleProcessId)
{
	// create startup params shared memory
	m_consoleParams.Create((SharedMemNames::formatConsoleParams % dwConsoleProcessId).str());

	// create console info shared memory
	m_consoleInfo.Create((SharedMemNames::formatInfo % dwConsoleProcessId).str());

	// create console info shared memory
	m_cursorInfo.Create((SharedMemNames::formatCursorInfo % dwConsoleProcessId).str());

	// TODO: max console size
	m_consoleBuffer.Create((SharedMemNames::formatBuffer % dwConsoleProcessId).str(), 200*200);

	// paste info 
	m_consolePaste.Create((SharedMemNames::formatPasteInfo % dwConsoleProcessId).str());

	// new console size
	m_newConsoleSize.Create((SharedMemNames::formatNewConsoleSize % dwConsoleProcessId).str());

	// new scroll position
	m_newScrollPos.Create((SharedMemNames::formatNewScrollPos % dwConsoleProcessId).str());

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
	::WaitForSingleObject(hRemoteThread.get(), INFINITE);

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
	HANDLE arrWaitHandles[] = { m_hConsoleProcess.get(), m_hMonitorThreadExit.get(), m_consoleBuffer.GetEvent() };
	while (::WaitForMultipleObjects(sizeof(arrWaitHandles)/sizeof(arrWaitHandles[0]), arrWaitHandles, FALSE, INFINITE) > WAIT_OBJECT_0 + 1)
	{
		SharedMemoryLock	memLock(m_consoleBuffer);

		DWORD				dwColumns	= m_consoleInfo->srWindow.Right - m_consoleInfo->srWindow.Left + 1;
		DWORD				dwRows		= m_consoleInfo->srWindow.Bottom - m_consoleInfo->srWindow.Top + 1;
		bool				bResize		= false;

		if ((m_consoleParams->dwColumns != dwColumns) ||
			(m_consoleParams->dwRows != dwRows))
		{
			m_consoleParams->dwColumns	= dwColumns;
			m_consoleParams->dwRows		= dwRows;
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


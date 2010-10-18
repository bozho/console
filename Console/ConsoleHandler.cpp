#include "stdafx.h"

#include "resource.h"
#include "Console.h"

#include "../shared/SharedMemNames.h"
#include "ConsoleException.h"
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

shared_ptr<Mutex>	ConsoleHandler::s_parentProcessWatchdog;
shared_ptr<void>	ConsoleHandler::s_environmentBlock;

//////////////////////////////////////////////////////////////////////////////

ConsoleHandler::ConsoleHandler()
: m_hConsoleProcess()
, m_consoleParams()
, m_consoleInfo()
, m_consoleBuffer()
, m_consoleCopyInfo()
, m_consoleTextInfo()
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

bool ConsoleHandler::StartShellProcess
(
	const wstring& strCustomShell, 
	const wstring& strInitialDir, 
	const wstring& strUser,
	const wstring& strPassword,
	const wstring& strInitialCmd, 
	const wstring& strConsoleTitle, 
	DWORD dwStartupRows, 
	DWORD dwStartupColumns, 
	bool bDebugFlag
)
{
	wstring strUsername(strUser);
	wstring strDomain;

//	shared_ptr<void> userProfileKey;
//	shared_ptr<void> userEnvironment;
//	shared_ptr<void> userToken;

	if (strUsername.length() > 0)
	{
		size_t pos = strUsername.find(L'\\');
		if (pos != wstring::npos)
		{
			strDomain	= strUsername.substr(0, pos);
			strUsername	= strUsername.substr(pos+1);
		}
/*
		// logon user
		HANDLE hUserToken = NULL;
		::LogonUser(
			strUsername.c_str(), 
			strDomain.length() > 0 ? strDomain.c_str() : NULL, 
			strPassword.c_str(), 
			LOGON32_LOGON_INTERACTIVE, 
			LOGON32_PROVIDER_DEFAULT, 
			&hUserToken);

		userToken.reset(hUserToken, ::CloseHandle);
*/

/*
		::ImpersonateLoggedOnUser(userToken.get());

		wchar_t	szComspec[MAX_PATH];
		::GetEnvironmentVariable(L"COMSPEC", szComspec, MAX_PATH);
*/


/*
		// load user's profile
		// seems to be necessary on WinXP for environment strings' expainsion to work properly
		PROFILEINFO userProfile;
		::ZeroMemory(&userProfile, sizeof(PROFILEINFO));
		userProfile.dwSize = sizeof(PROFILEINFO);
		userProfile.lpUserName = const_cast<wchar_t*>(strUser.c_str());
		
		::LoadUserProfile(userToken.get(), &userProfile);
		userProfileKey.reset(userProfile.hProfile, bind<BOOL>(::UnloadUserProfile, userToken.get(), _1));
*/
/*		// load user's environment
		void*	pEnvironment	= NULL;
		::CreateEnvironmentBlock(&pEnvironment, userToken.get(), FALSE);
		userEnvironment.reset(pEnvironment, ::DestroyEnvironmentBlock);
		::RevertToSelf();
*/
	}

	wstring	strShellCmdLine(strCustomShell);
	
	if (strShellCmdLine.length() == 0)
	{
		wchar_t	szComspec[MAX_PATH];

		::ZeroMemory(szComspec, MAX_PATH*sizeof(wchar_t));

		if (strUsername.length() > 0)
		{
/*			// resolve comspec when running as another user
			wchar_t* pszComspec = reinterpret_cast<wchar_t*>(userEnvironment.get());

			while ((pszComspec[0] != L'\x00') && (_wcsnicmp(pszComspec, L"comspec", 7) != 0)) pszComspec += wcslen(pszComspec)+1;

			if (pszComspec[0] != L'\x00')
			{
				strShellCmdLine = (pszComspec + 8);
			}
*/
			if (strShellCmdLine.length() == 0) strShellCmdLine = L"cmd.exe";
		}
		else
		{
			if (::GetEnvironmentVariable(L"COMSPEC", szComspec, MAX_PATH) > 0)
			{
				strShellCmdLine = szComspec;		
			}

			if (strShellCmdLine.length() == 0) strShellCmdLine = L"cmd.exe";
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

//	wstring strStartupDir((strUsername.length() > 0) ? Helpers::ExpandEnvironmentStringsForUser(userToken, strInitialDir) : Helpers::ExpandEnvironmentStrings(strInitialDir));
	wstring strStartupDir((strUsername.length() > 0) ? strInitialDir : Helpers::ExpandEnvironmentStrings(strInitialDir));

	if (strStartupDir.length() > 0)
	{
		if ((*(strStartupDir.end() - 1) == L'\"') && (*strStartupDir.begin() != L'\"'))
		{
			// startup dir name ends with ", but doesn't start with ", the user passed
			// something like "C:\" as the parameter, it got parsed to C:", remove the trailing "
			//
			// This is a common mistake, thus the check...
			strStartupDir = strStartupDir.substr(0, strStartupDir.length()-1);
		}

		// startup dir doesn't end with \, add it
		if (*(strStartupDir.end() - 1) != L'\\') strStartupDir += L'\\';

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

	si.cb			= sizeof(STARTUPINFO);
	si.lpTitle		= const_cast<wchar_t*>(strStartupTitle.c_str());

	if (g_settingsHandler->GetConsoleSettings().bStartHidden)
	{
		// Starting Windows console window hidden causes problems with 
		// some GUI apps started from Console that use SW_SHOWDEFAULT to 
		// initially show their main window (i.e. the window inherits our 
		// SW_HIDE flag and remains invisible :-)
		si.dwFlags		= STARTF_USESHOWWINDOW;
		si.wShowWindow	= SW_HIDE;
	}
	else
	{
		// To avoid invisible GUI windows, default settings will create
		// a Windows console window far offscreen and hide the window
		// after it has been created.
		//
		// This approach can flash console window's taskbar button and
		// taskbar button can sometimes remain visible, but nothing is perfect :)
		si.dwFlags		= STARTF_USEPOSITION;
		si.dwX			= 0x7FFF;
		si.dwY			= 0x7FFF;
	}
	
	PROCESS_INFORMATION pi;
	// we must use CREATE_UNICODE_ENVIRONMENT here, since s_environmentBlock contains Unicode strings
	DWORD dwStartupFlags = CREATE_NEW_CONSOLE|CREATE_SUSPENDED|CREATE_UNICODE_ENVIRONMENT;

	// TODO: not supported yet
	//if (bDebugFlag) dwStartupFlags |= DEBUG_PROCESS;

	
	if (strUsername.length() > 0)
	{
		if (!::CreateProcessWithLogonW(
			strUsername.c_str(), 
			strDomain.length() > 0 ? strDomain.c_str() : NULL, 
			strPassword.c_str(), 
			LOGON_WITH_PROFILE,
			NULL,
//			const_cast<wchar_t*>(Helpers::ExpandEnvironmentStringsForUser(userToken, strShellCmdLine).c_str()),
			const_cast<wchar_t*>(strShellCmdLine.c_str()),
			dwStartupFlags,
//			userEnvironment.get(),
			NULL,
			(strStartupDir.length() > 0) ? const_cast<wchar_t*>(strStartupDir.c_str()) : NULL,
			&si,
			&pi))
		{
			throw ConsoleException(str(wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % strShellCmdLine % strUser));
		}
	}
	else
	{
		if (!::CreateProcess(
				NULL,
				const_cast<wchar_t*>(Helpers::ExpandEnvironmentStrings(strShellCmdLine).c_str()),
				NULL,
				NULL,
				FALSE,
				dwStartupFlags,
				s_environmentBlock.get(),
				(strStartupDir.length() > 0) ? const_cast<wchar_t*>(strStartupDir.c_str()) : NULL,
				&si,
				&pi))
		{
			throw ConsoleException(str(wformat(Helpers::LoadString(IDS_ERR_CANT_START_SHELL)) % strShellCmdLine));
		}
	}

	// create shared memory objects
	CreateSharedObjects(pi.dwProcessId, strUser);
	CreateWatchdog();

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
	if (!InjectHookDLL(pi)) return false;

	// resume the console process
	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hThread);

	// wait for hook DLL to set console handle
	if (::WaitForSingleObject(m_consoleParams.GetReqEvent(), 10000) == WAIT_TIMEOUT) return false;

	::ShowWindow(m_consoleParams->hwndConsoleWindow, SW_HIDE);

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

void ConsoleHandler::StopScrolling()
{
	// emulate 'Mark' sysmenu item click in Windows console window (will stop scrolling until the user presses ESC)
	// or a selection is cleared (copied or not)
	::SendMessage(m_consoleParams->hwndConsoleWindow, WM_SYSCOMMAND, SC_CONSOLE_MARK, 0);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ResumeScrolling()
{
	// emulate ESC keypress to end 'mark' command (we send a mark command just in case 
	// a user has already pressed ESC as I don't know an easy way to detect if the mark
	// command is active or not)
	::SendMessage(m_consoleParams->hwndConsoleWindow, WM_SYSCOMMAND, SC_CONSOLE_MARK, 0);
	::SendMessage(m_consoleParams->hwndConsoleWindow, WM_KEYDOWN, VK_ESCAPE, 0x00010001);
	::SendMessage(m_consoleParams->hwndConsoleWindow, WM_KEYUP, VK_ESCAPE, 0xC0010001);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::UpdateEnvironmentBlock()
{
	void*	pEnvironment	= NULL;
	HANDLE	hProcessToken	= NULL;

	::OpenProcessToken(::GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcessToken);
	::CreateEnvironmentBlock(&pEnvironment, hProcessToken, FALSE);
	::CloseHandle(hProcessToken);

	s_environmentBlock.reset(pEnvironment, ::DestroyEnvironmentBlock);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::CreateSharedObjects(DWORD dwConsoleProcessId, const wstring& strUser)
{
	// create startup params shared memory
	m_consoleParams.Create((SharedMemNames::formatConsoleParams % dwConsoleProcessId).str(), 1, syncObjBoth, strUser);

	// create console info shared memory
	m_consoleInfo.Create((SharedMemNames::formatInfo % dwConsoleProcessId).str(), 1, syncObjRequest, strUser);

	// create console info shared memory
	m_cursorInfo.Create((SharedMemNames::formatCursorInfo % dwConsoleProcessId).str(), 1, syncObjRequest, strUser);

	// TODO: max console size
	m_consoleBuffer.Create((SharedMemNames::formatBuffer % dwConsoleProcessId).str(), 200*200, syncObjRequest, strUser);

	// initialize buffer with spaces
	CHAR_INFO ci;
	ci.Attributes		= 0;
	ci.Char.UnicodeChar	= L' ';
	for (int i = 0; i < 200*200; ++i) ::CopyMemory(&m_consoleBuffer[i], &ci, sizeof(CHAR_INFO));

	// copy info
	m_consoleCopyInfo.Create((SharedMemNames::formatCopyInfo % dwConsoleProcessId).str(), 1, syncObjBoth, strUser);

	// text info (used for sending text to console)
	m_consoleTextInfo.Create((SharedMemNames::formatTextInfo % dwConsoleProcessId).str(), 1, syncObjBoth, strUser);

	// mouse event
	m_consoleMouseEvent.Create((SharedMemNames::formatMouseEvent % dwConsoleProcessId).str(), 1, syncObjBoth, strUser);

	// new console size
	m_newConsoleSize.Create((SharedMemNames::formatNewConsoleSize % dwConsoleProcessId).str(), 1, syncObjRequest, strUser);

	// new scroll position
	m_newScrollPos.Create((SharedMemNames::formatNewScrollPos % dwConsoleProcessId).str(), 1, syncObjRequest, strUser);

	// TODO: separate function for default settings
	m_consoleParams->dwRows		= 25;
	m_consoleParams->dwColumns	= 80;

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::CreateWatchdog()
{
	if (!s_parentProcessWatchdog)
	{
		shared_ptr<void>	sd;	// PSECURITY_DESCRIPTOR

		sd.reset(::LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH), ::LocalFree);
		if (::InitializeSecurityDescriptor(sd.get(), SECURITY_DESCRIPTOR_REVISION))
		{
			::SetSecurityDescriptorDacl(
				sd.get(), 
				TRUE,		// bDaclPresent flag   
				NULL,		// full access to everyone
				FALSE);		// not a default DACL 
		}

		SECURITY_ATTRIBUTES	sa;

		::ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
		sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle		= FALSE;
		sa.lpSecurityDescriptor	= sd.get();

		s_parentProcessWatchdog.reset(new Mutex(&sa, TRUE, (LPCTSTR)((SharedMemNames::formatWatchdog % ::GetCurrentProcessId()).str().c_str())));
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::InjectHookDLL(PROCESS_INFORMATION& pi)
{

	// allocate memory for parameter in the remote process
	wstring				strHookDllPath(GetModulePath(NULL));

	if (::GetFileAttributes(strHookDllPath.c_str()) == INVALID_FILE_ATTRIBUTES) return false;

	CONTEXT		context;
	
	void*		mem				= NULL;
	size_t		memLen			= 0;
	UINT_PTR	fnLoadLibrary	= NULL;

	size_t		codeSize;
	BOOL		isWow64Process	= FALSE;

#ifdef _WIN64
	WOW64_CONTEXT 	wow64Context;
	DWORD			fnWow64LoadLibrary	= 0;

	::ZeroMemory(&wow64Context, sizeof(WOW64_CONTEXT));
	::IsWow64Process(pi.hProcess, &isWow64Process);
	codeSize = isWow64Process ? 20 : 91;
#else
	codeSize = 20;
#endif

	if (isWow64Process)
	{
		// starting a 32-bit process from a 64-bit console
		strHookDllPath += wstring(L"\\ConsoleHook32.dll");
	}
	else
	{
		// same bitness :-)
		strHookDllPath += wstring(L"\\ConsoleHook.dll");
	}

	::ZeroMemory(&context, sizeof(CONTEXT));

	shared_array<BYTE> code(new BYTE[codeSize + (MAX_PATH*sizeof(wchar_t))]);

	memLen = (strHookDllPath.length()+1)*sizeof(wchar_t);
	if (memLen > MAX_PATH*sizeof(wchar_t)) return false;

	::CopyMemory(code.get() + codeSize, strHookDllPath.c_str(), memLen);
	memLen += codeSize;

#ifdef _WIN64

	if (isWow64Process)
	{
		wow64Context.ContextFlags = CONTEXT_FULL;
		::Wow64GetThreadContext(pi.hThread, &wow64Context);

		mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		// get 32-bit kernel32
		wstring strConsoleWowPath(GetModulePath(NULL) + wstring(L"\\ConsoleWow.exe"));

		STARTUPINFO siWow;
		::ZeroMemory(&siWow, sizeof(STARTUPINFO));

		siWow.cb			= sizeof(STARTUPINFO);
		siWow.dwFlags		= STARTF_USESHOWWINDOW;
		siWow.wShowWindow	= SW_HIDE;
		
		PROCESS_INFORMATION piWow;

		if (!::CreateProcess(
				NULL,
				const_cast<wchar_t*>(strConsoleWowPath.c_str()),
				NULL,
				NULL,
				FALSE,
				0,
				NULL,
				NULL,
				&siWow,
				&piWow))
		{
			return false;
		}

		shared_ptr<void> wowProcess(piWow.hProcess, ::CloseHandle);
		shared_ptr<void> wowThread(piWow.hThread, ::CloseHandle);

		if (::WaitForSingleObject(wowProcess.get(), 5000) == WAIT_TIMEOUT)
		{
			return false;
		}

		::GetExitCodeProcess(wowProcess.get(), reinterpret_cast<DWORD*>(&fnWow64LoadLibrary));
	}
	else
	{
		context.ContextFlags = CONTEXT_FULL;
		::GetThreadContext(pi.hThread, &context);

		mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	}


#else
	context.ContextFlags = CONTEXT_FULL;
	::GetThreadContext(pi.hThread, &context);

	mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
#endif

	union
	{
		PBYTE  pB;
		PINT   pI;
		PULONGLONG pL;
	} ip;

	ip.pB = code.get();

#ifdef _WIN64

	if (isWow64Process)
	{
		*ip.pB++ = 0x68;			// push  eip
		*ip.pI++ = wow64Context.Eip;
		*ip.pB++ = 0x9c;			// pushf
		*ip.pB++ = 0x60;			// pusha
		*ip.pB++ = 0x68;			// push  "path\to\our.dll"
		*ip.pI++ = (DWORD)mem + codeSize;
		*ip.pB++ = 0xe8;			// call  LoadLibraryW
		*ip.pI++ = (DWORD)fnWow64LoadLibrary - ((DWORD)mem + (ip.pB+4 - code.get()));
		*ip.pB++ = 0x61;			// popa
		*ip.pB++ = 0x9d;			// popf
		*ip.pB++ = 0xc3;			// ret

		::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL);
		::FlushInstructionCache(pi.hProcess, mem, memLen);
		wow64Context.Eip = (DWORD)mem;
		::Wow64SetThreadContext(pi.hThread, &wow64Context);
	}
	else
	{
		*ip.pL++ = context.Rip;
		*ip.pL++ = fnLoadLibrary;
		*ip.pB++ = 0x9C;					// pushfq
		*ip.pB++ = 0x50;					// push  rax
		*ip.pB++ = 0x51;					// push  rcx
		*ip.pB++ = 0x52;					// push  rdx
		*ip.pB++ = 0x53;					// push  rbx
		*ip.pB++ = 0x55;					// push  rbp
		*ip.pB++ = 0x56;					// push  rsi
		*ip.pB++ = 0x57;					// push  rdi
		*ip.pB++ = 0x41; *ip.pB++ = 0x50;	// push  r8
		*ip.pB++ = 0x41; *ip.pB++ = 0x51;	// push  r9
		*ip.pB++ = 0x41; *ip.pB++ = 0x52;	// push  r10
		*ip.pB++ = 0x41; *ip.pB++ = 0x53;	// push  r11
		*ip.pB++ = 0x41; *ip.pB++ = 0x54;	// push  r12
		*ip.pB++ = 0x41; *ip.pB++ = 0x55;	// push  r13
		*ip.pB++ = 0x41; *ip.pB++ = 0x56;	// push  r14
		*ip.pB++ = 0x41; *ip.pB++ = 0x57;	// push  r15
		*ip.pB++ = 0x48;					// sub   rsp, 40
		*ip.pB++ = 0x83;
		*ip.pB++ = 0xEC;
		*ip.pB++ = 0x28;

		*ip.pB++ = 0x48;					// lea	 ecx, "path\to\our.dll"
		*ip.pB++ = 0x8D;
		*ip.pB++ = 0x0D;
		*ip.pI++ = 40;

		*ip.pB++ = 0xFF;					// call  LoadLibraryW
		*ip.pB++ = 0x15;
		*ip.pI++ = -49;
		
		*ip.pB++ = 0x48;					// add   rsp, 40
		*ip.pB++ = 0x83;
		*ip.pB++ = 0xC4;
		*ip.pB++ = 0x28;

		*ip.pB++ = 0x41; *ip.pB++ = 0x5F;	// pop   r15
		*ip.pB++ = 0x41; *ip.pB++ = 0x5E;	// pop   r14
		*ip.pB++ = 0x41; *ip.pB++ = 0x5D;	// pop   r13
		*ip.pB++ = 0x41; *ip.pB++ = 0x5C;	// pop   r12
		*ip.pB++ = 0x41; *ip.pB++ = 0x5B;	// pop   r11
		*ip.pB++ = 0x41; *ip.pB++ = 0x5A;	// pop   r10
		*ip.pB++ = 0x41; *ip.pB++ = 0x59;	// pop   r9
		*ip.pB++ = 0x41; *ip.pB++ = 0x58;	// pop   r8
		*ip.pB++ = 0x5F;					// pop	 rdi
		*ip.pB++ = 0x5E;					// pop	 rsi
		*ip.pB++ = 0x5D;					// pop	 rbp
		*ip.pB++ = 0x5B;					// pop	 rbx
		*ip.pB++ = 0x5A;					// pop	 rdx
		*ip.pB++ = 0x59;					// pop	 rcx
		*ip.pB++ = 0x58;					// pop	 rax
		*ip.pB++ = 0x9D;					// popfq
		*ip.pB++ = 0xff;					// jmp	 Rip
		*ip.pB++ = 0x25;
		*ip.pI++ = -91;

		::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL);
		::FlushInstructionCache(pi.hProcess, mem, memLen);
		context.Rip = (UINT_PTR)mem + 16;
		::SetThreadContext(pi.hThread, &context);
	}

#else

	*ip.pB++ = 0x68;			// push  eip
	*ip.pI++ = context.Eip;
	*ip.pB++ = 0x9c;			// pushf
	*ip.pB++ = 0x60;			// pusha
	*ip.pB++ = 0x68;			// push  "path\to\our.dll"
	*ip.pI++ = (UINT_PTR)mem + codeSize;
	*ip.pB++ = 0xe8;			// call  LoadLibraryW
	*ip.pI++ = (UINT_PTR)fnLoadLibrary - ((UINT_PTR)mem + (ip.pB+4 - code.get()));
	*ip.pB++ = 0x61;			// popa
	*ip.pB++ = 0x9d;			// popf
	*ip.pB++ = 0xc3;			// ret

	::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL);
	::FlushInstructionCache(pi.hProcess, mem, memLen);
	context.Eip = (UINT_PTR)mem;
	::SetThreadContext(pi.hThread, &context);
#endif

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
	// resume ConsoleHook's thread
	m_consoleParams.SetRespEvent();

	HANDLE arrWaitHandles[] = { m_hConsoleProcess.get(), m_hMonitorThreadExit.get(), m_consoleBuffer.GetReqEvent() };
	while (::WaitForMultipleObjects(sizeof(arrWaitHandles)/sizeof(arrWaitHandles[0]), arrWaitHandles, FALSE, INFINITE) > WAIT_OBJECT_0 + 1)
	{
		DWORD				dwColumns	= m_consoleInfo->csbi.srWindow.Right - m_consoleInfo->csbi.srWindow.Left + 1;
		DWORD				dwRows		= m_consoleInfo->csbi.srWindow.Bottom - m_consoleInfo->csbi.srWindow.Top + 1;
		DWORD				dwBufferColumns	= m_consoleInfo->csbi.dwSize.X;
		DWORD				dwBufferRows	= m_consoleInfo->csbi.dwSize.Y;
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


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

std::shared_ptr<Mutex>	ConsoleHandler::s_parentProcessWatchdog;
std::shared_ptr<void>	ConsoleHandler::s_environmentBlock;

//////////////////////////////////////////////////////////////////////////////

ConsoleHandler::ConsoleHandler()
: m_hConsoleProcess()
, m_consoleParams()
, m_consoleInfo()
, m_consoleBuffer()
, m_consoleCopyInfo()
, m_consoleMouseEvent()
, m_newConsoleSize()
, m_newScrollPos()
, m_hMonitorThread()
, m_hMonitorThreadExit(std::shared_ptr<void>(::CreateEvent(NULL, FALSE, FALSE, NULL), ::CloseHandle))
, m_bufferMutex(NULL, FALSE, NULL)
, m_dwConsolePid(0)
, m_boolIsElevated(false)
, m_boolDetaching(false)
{
}

ConsoleHandler::~ConsoleHandler()
{
	if( m_hMonitorThread.get() )
		StopMonitorThread();

	if( m_consoleParams.Get() && m_consoleParams->hwndConsoleWindow )
	{
		if( m_boolDetaching )
		{
			CPoint point;
			::GetCursorPos(&point);
			SetWindowPos(point.x, point.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
			ShowWindow(SW_SHOW);

			NamedPipeMessage npmsg;
			npmsg.type = NamedPipeMessage::DETACH;
			try
			{
				m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
			}
			catch(std::exception&) { }
		}
		else
		{
			SendMessage(WM_CLOSE, 0, 0);
		}
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

void ConsoleHandler::RunAsAdministrator
(
	const wstring& strSyncName,
	const wstring& strTitle,
	const wstring& strInitialDir,
	const wstring& strInitialCmd,
	DWORD dwBasePriority
)
{
	std::wstring strFile = Helpers::GetModuleFileName(nullptr);

	std::wstring strParams;
	// admin sync name
	strParams += L"-a ";
	strParams += Helpers::EscapeCommandLineArg(strSyncName);
	// config file
	strParams += L" -c ";
	strParams += Helpers::EscapeCommandLineArg(g_settingsHandler->GetSettingsFileName());
	// tab name
	strParams += L" -t ";
	strParams += Helpers::EscapeCommandLineArg(strTitle);
	// directory
	if (!strInitialDir.empty())
	{
		strParams += L" -d ";
		strParams += Helpers::EscapeCommandLineArg(strInitialDir);
	}
	// startup shell command
	if (!strInitialCmd.empty())
	{
		strParams += L" -r ";
		strParams += Helpers::EscapeCommandLineArg(strInitialCmd);
	}
	// priority
	strParams += L" -p ";
	strParams += TabData::PriorityToString(dwBasePriority);

	SHELLEXECUTEINFO sei = {sizeof(sei)};

	sei.hwnd = nullptr;
	sei.fMask = /*SEE_MASK_NOCLOSEPROCESS|*/SEE_MASK_NOASYNC;
	sei.lpVerb = L"runas";
	sei.lpFile = strFile.c_str();
	sei.lpParameters = strParams.length() > 0 ? strParams.c_str() : nullptr;
	sei.lpDirectory = nullptr,
	sei.nShow = SW_SHOWMINIMIZED;

	if(!::ShellExecuteEx(&sei))
	{
		Win32Exception err("ShellExecuteEx", ::GetLastError());
		throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_CANT_START_SHELL_AS_ADMIN)) % strFile % strParams % err.what()));
	}
}

std::wstring MergeEnvironmentVariables(
	const void * environmentBlock,
	const std::vector<std::shared_ptr<VarEnv>>& extraEnv)
{
	std::wstring strNewEnvironment;

	std::map<std::wstring, std::wstring, __case_insensitive_compare> dictionary;

	for(const wchar_t * p = static_cast<const wchar_t *>(environmentBlock);
	    p && p[0];
	    p += wcslen(p) + 1)
	{
		const wchar_t * equal = wcschr(p , L'=');
		if( equal == nullptr ) continue;

		dictionary[std::wstring(p, equal - p)] = std::wstring(equal + 1);
	}

	std::map<std::wstring, std::wstring, __case_insensitive_compare> extra;

	for(auto i = extraEnv.cbegin(); i != extraEnv.cend(); ++i)
	{
		if( !i->get()->bEnvChecked ) continue;

		extra[i->get()->strEnvVariable] = i->get()->strEnvValue;
	}

	// variable name has only one restriction: no =
	std::wregex regexEnvVar(L"%[^=]+%");

	// first pass (we add all variables without %%)
	for(auto i = extra.begin(); i != extra.end();)
	{
		if( std::regex_search(i->second, regexEnvVar) )
		{
			++i;
		}
		else
		{
			dictionary[i->first] = i->second;

			i = extra.erase(i);
		}
	}

	// second pass (we add all variables with %%)
	for(auto i = extra.begin(); i != extra.end(); ++i)
	{
		std::wstring str = Helpers::ExpandEnvironmentStrings(dictionary, i->second);
		dictionary[i->first] = str;
	}

	for(auto i = dictionary.cbegin(); i != dictionary.cend(); ++i)
	{
		strNewEnvironment += i->first;
		strNewEnvironment += L'=';
		strNewEnvironment += i->second;
		strNewEnvironment += L'\0';
	}

	strNewEnvironment += L'\0';

	return strNewEnvironment;
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::CreateShellProcess
(
	const wstring& strShell,
	const wstring& strInitialDir,
	const UserCredentials& userCredentials,
	const wstring& strInitialCmd,
	DWORD dwBasePriority,
	const std::vector<std::shared_ptr<VarEnv>>& extraEnv,
	PROCESS_INFORMATION& pi
)
{
	std::unique_ptr<void, DestroyEnvironmentBlockHelper> userEnvironment;
	std::unique_ptr<void, CloseHandleHelper>             userToken;
	std::shared_ptr<void>                                userProfileKey;
	RevertToSelfHelper                                   revertToSelfHelper;
	std::vector<std::shared_ptr<VarEnv>>                 homeEnv;

	if (userCredentials.strUsername.length() > 0)
	{
		if (!userCredentials.netOnly)
		{
			// logon user
			HANDLE hUserToken = NULL;
			if( !::LogonUser(
				userCredentials.strUsername.c_str(),
				userCredentials.strDomain.length() > 0 ? userCredentials.strDomain.c_str() : NULL,
				userCredentials.password.c_str(),
				LOGON32_LOGON_INTERACTIVE,
				LOGON32_PROVIDER_DEFAULT,
				&hUserToken) )
			{
				Win32Exception err("LogonUser", ::GetLastError());
				throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % userCredentials.user % err.what()));
			}
			userToken.reset(hUserToken);

			if( !::ImpersonateLoggedOnUser(userToken.get()) )
			{
				Win32Exception err("ImpersonateLoggedOnUser", ::GetLastError());
				throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % userCredentials.user % err.what()));
			}
			revertToSelfHelper.on();

#if 0
			// load user's profile
			// seems to be necessary on WinXP for environment strings' expainsion to work properly
			// only administrators or LOCALSYSTEM can load a profile since Windows XP SP2
			PROFILEINFO userProfile;
			::ZeroMemory(&userProfile, sizeof(PROFILEINFO));
			userProfile.dwSize = sizeof(PROFILEINFO);
			userProfile.lpUserName = const_cast<wchar_t*>(userCredentials.strUsername.c_str());

			if( !::LoadUserProfile(userToken.get(), &userProfile) )
			{
				Win32Exception err("LoadUserProfile", ::GetLastError());
				throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % userCredentials.user % err.what()));
			}
			userProfileKey.reset(userProfile.hProfile, std::bind<BOOL>(::UnloadUserProfile, userToken.get(), std::placeholders::_1));
#endif

			// load user's environment
			void* pEnvironment = nullptr;
			if( !::CreateEnvironmentBlock(&pEnvironment, userToken.get(), FALSE) )
			{
				Win32Exception err("CreateEnvironmentBlock", ::GetLastError());
				throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % userCredentials.user % err.what()));
			}
			userEnvironment.reset(pEnvironment);

#if 0
			BYTE dummy[1024];
			PSID psid = reinterpret_cast<PSID>(dummy);
			DWORD cbSid = sizeof(dummy);
			wchar_t szReferencedDomainName[DNLEN + 1] = L"";
			DWORD cchReferencedDomainName = ARRAYSIZE(szReferencedDomainName);
			SID_NAME_USE eUse;
			if(!::LookupAccountName(
				userCredentials.strDomain.c_str(),
				userCredentials.strUsername.c_str(),
				psid, &cbSid,
				szReferencedDomainName, &cchReferencedDomainName,
				&eUse))
			{
				Win32Exception err("LookupAccountName", ::GetLastError());
				throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % userCredentials.user % err.what()));
			}
#endif

			NET_API_STATUS nStatus;
			LPWSTR pszComputerName = nullptr;
			if(userCredentials.strDomain != Helpers::GetComputerName())
			{
				nStatus = ::NetGetDCName(NULL, userCredentials.strDomain.c_str(), reinterpret_cast<LPBYTE *>(&pszComputerName));
				if(nStatus != NERR_Success && nStatus != NERR_DCNotFound)
				{
					Win32Exception err("NetGetDCName", nStatus);
					throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % userCredentials.user % err.what()));
				}
			}
			std::unique_ptr<void, NetApiBufferFreeHelper> dcName(pszComputerName);

			LPUSER_INFO_3 pBuf3 = nullptr;
			nStatus = ::NetUserGetInfo(pszComputerName, userCredentials.strUsername.c_str(), 3, reinterpret_cast<LPBYTE *>(&pBuf3));
			if( nStatus != NERR_Success )
			{
				Win32Exception err("NetUserGetInfo", nStatus);
				throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % userCredentials.user % err.what()));
			}
			std::unique_ptr<void, NetApiBufferFreeHelper> buf3(pBuf3);

			std::wstring strHomeShare;
			std::wstring strHomeDrive;
			std::wstring strHomePath;
			if( pBuf3->usri3_home_dir_drive && *pBuf3->usri3_home_dir_drive )
			{
				// home is on a net share
				strHomeShare = pBuf3->usri3_home_dir;
				strHomeDrive = pBuf3->usri3_home_dir_drive;
				strHomePath  = L"\\";
			}
			else
			{
				std::wstring strHomeDir;

				// local
				if( pBuf3->usri3_home_dir && *pBuf3->usri3_home_dir )
				{
					// defined
					strHomeDir = pBuf3->usri3_home_dir;
				}
				else
				{
					// undefined
					// same as profile
					wchar_t szUserProfileDirectory[_MAX_PATH] = L"";
					DWORD   dwUserProfileDirectoryLen         = ARRAYSIZE(szUserProfileDirectory);
					if( !::GetUserProfileDirectory(userToken.get(), szUserProfileDirectory, &dwUserProfileDirectoryLen) )
					{
						Win32Exception err("GetUserProfileDirectory", ::GetLastError());
						throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % L"?" % userCredentials.user % err.what()));
					}

					strHomeDir = szUserProfileDirectory;
				}

				strHomeDrive = strHomeDir.substr(0, 2);
				strHomePath  = strHomeDir.substr(2);
			}

			TRACE(
				L"HOMESHARE=%s\nHOMEDRIVE=%s\nHOMEPATH=%s\n",
				strHomeShare.c_str(),
				strHomeDrive.c_str(),
				strHomePath.c_str());

			if( !strHomeShare.empty() )
				homeEnv.push_back(
					std::shared_ptr<VarEnv>(
						new VarEnv(std::wstring(L"HOMESHARE"), strHomeShare)));
			if( !strHomeDrive.empty() )
				homeEnv.push_back(
					std::shared_ptr<VarEnv>(
						new VarEnv(std::wstring(L"HOMEDRIVE"), strHomeDrive)));
			if( !strHomePath.empty() )
				homeEnv.push_back(
					std::shared_ptr<VarEnv>(
						new VarEnv(std::wstring(L"HOMEPATH"), strHomePath)));
		}
	}

	// load environment block
	if( !s_environmentBlock.get() )
		ConsoleHandler::UpdateCurrentUserEnvironmentBlock();

	// add specific environment variables defined in tad settings
	wstring strNewEnvironment = MergeEnvironmentVariables(
		userEnvironment.get()? userEnvironment.get() : s_environmentBlock.get(),
		extraEnv);

	// add missing home variables
	if( !homeEnv.empty() )
		strNewEnvironment = MergeEnvironmentVariables(
		strNewEnvironment.c_str(),
		homeEnv);

	wstring	strShellCmdLine(strShell);

	if( strShellCmdLine.empty() )
	{
		strShellCmdLine = Helpers::GetEnvironmentVariable(strNewEnvironment.c_str(), L"ComSpec");
		if( strShellCmdLine.empty() ) strShellCmdLine = L"cmd.exe";
	}

	if( !strInitialCmd.empty())
	{
		strShellCmdLine += L" ";
		strShellCmdLine += strInitialCmd;
	}

	wstring strStartupDir = Helpers::ExpandEnvironmentStrings(strNewEnvironment.c_str(), strInitialDir);

	if( !strStartupDir.empty() )
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

	wstring strCmdLine = Helpers::ExpandEnvironmentStrings(strNewEnvironment.c_str(), strShellCmdLine);

	revertToSelfHelper.off();

	// setup the startup info struct
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(STARTUPINFO));

	si.cb      = sizeof(STARTUPINFO);
	si.lpTitle = DEFAULT_CONSOLE_COMMAND;

	if (g_settingsHandler->GetConsoleSettings().bStartHidden)
	{
		// Starting Windows console window hidden causes problems with 
		// some GUI apps started from ConsoleZ that use SW_SHOWDEFAULT to 
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

	// we must use CREATE_UNICODE_ENVIRONMENT here, since s_environmentBlock contains Unicode strings
	DWORD dwStartupFlags = CREATE_NEW_CONSOLE|CREATE_SUSPENDED|CREATE_UNICODE_ENVIRONMENT|TabData::GetPriorityClass(dwBasePriority);

	if (userCredentials.strUsername.length() > 0)
	{
		if( !::CreateProcessWithLogonW(
			userCredentials.strUsername.c_str(),
			userCredentials.strDomain.length() > 0 ? userCredentials.strDomain.c_str() : NULL,
			userCredentials.password.c_str(), 
			userCredentials.netOnly? LOGON_NETCREDENTIALS_ONLY : LOGON_WITH_PROFILE,
			NULL,
			const_cast<wchar_t*>(strCmdLine.c_str()),
			dwStartupFlags,
			const_cast<wchar_t*>(strNewEnvironment.c_str()),
			(strStartupDir.length() > 0) ? const_cast<wchar_t*>(strStartupDir.c_str()) : NULL,
			&si,
			&pi))
		{
			Win32Exception err("CreateProcessWithLogonW", ::GetLastError());
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadStringW(IDS_ERR_CANT_START_SHELL_AS_USER)) % strShellCmdLine % userCredentials.user % err.what()));
		}
	}
	else
	{
		if (!::CreateProcess(
			NULL,
			const_cast<wchar_t*>(strCmdLine.c_str()),
			NULL,
			NULL,
			FALSE,
			dwStartupFlags,
			const_cast<wchar_t*>(strNewEnvironment.c_str()),
			(strStartupDir.length() > 0) ? const_cast<wchar_t*>(strStartupDir.c_str()) : NULL,
			&si,
			&pi))
		{
			Win32Exception err("CreateProcess", ::GetLastError());
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_CANT_START_SHELL)) % strShellCmdLine % err.what()));
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::StartShellProcess
(
	const wstring& strTitle,
	const wstring& strShell,
	const wstring& strInitialDir,
	const UserCredentials& userCredentials,
	const wstring& strInitialCmd,
	DWORD dwBasePriority,
	const std::vector<std::shared_ptr<VarEnv>>& extraEnv,
	DWORD dwStartupRows,
	DWORD dwStartupColumns
)
{
	PROCESS_INFORMATION pi = {0, 0, 0, 0};

	bool runAsAdministrator = userCredentials.runAsAdministrator;
	bool isElevated = false;

	try
	{
		if (Helpers::CheckOSVersion(6, 0))
		{
			if( Helpers::IsElevated() )
			{
				// process already running in elevated mode or UAC disabled
				runAsAdministrator = false;
				isElevated = true;
			}
		}
		else
		{
			// UAC doesn't exist in current OS
			runAsAdministrator = false;
		}
	}
	catch(std::exception& err)
	{
		if (runAsAdministrator)
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_CANT_GET_ELEVATION_TYPE)) % err.what()));
	}

	m_boolIsElevated = isElevated || runAsAdministrator;

	SharedMemory<DWORD> pid;

	if (runAsAdministrator)
	{
		std::wstring strSyncName = (SharedMemNames::formatAdmin % ::GetCurrentProcessId()).str();

		pid.Create(strSyncName, 1, syncObjBoth, L"");

		RunAsAdministrator(
			strSyncName,
			strTitle,
			strInitialDir,
			strInitialCmd,
			dwBasePriority
		);

		// wait for PID of shell launched in admin ConsoleZ
		if (::WaitForSingleObject(pid.GetReqEvent(), 10000) == WAIT_TIMEOUT)
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % L"timeout (PID)"));

		pi.dwProcessId = *pid.Get();
		pi.hProcess = ::OpenProcess(SYNCHRONIZE, FALSE, pi.dwProcessId);
		if( pi.hProcess == NULL )
		{
			Win32Exception err("OpenProcess", ::GetLastError());
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % err.what()));
		}
	}
	else
	{
		CreateShellProcess(
			strShell,
			strInitialDir,
			userCredentials,
			strInitialCmd,
			dwBasePriority,
			extraEnv,
			pi
		);
	}

	// create shared memory objects
	try
	{
		CreateSharedObjects(pi.dwProcessId, userCredentials.netOnly? L"" : userCredentials.strAccountName);
		CreateWatchdog();
	}
	catch(std::exception& err)
	{
		throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_CREATE_SHARED_OBJECTS_FAILED)) % err.what()));
	}

	// write startup params
	m_consoleParams->dwParentProcessId     = ::GetCurrentProcessId();
	m_consoleParams->dwNotificationTimeout = g_settingsHandler->GetConsoleSettings().dwChangeRefreshInterval;
	m_consoleParams->dwRefreshInterval     = g_settingsHandler->GetConsoleSettings().dwRefreshInterval;
	m_consoleParams->dwRows                = dwStartupRows;
	m_consoleParams->dwColumns             = dwStartupColumns;
	m_consoleParams->dwBufferRows          = g_settingsHandler->GetConsoleSettings().dwBufferRows;
	m_consoleParams->dwBufferColumns       = g_settingsHandler->GetConsoleSettings().dwBufferColumns;

	m_hConsoleProcess.reset(pi.hProcess);
	m_dwConsolePid    = pi.dwProcessId;

	if (runAsAdministrator)
	{
		::SetEvent(pid.GetRespEvent());
	}
	else
	{
		// inject our hook DLL into console process
		try
		{
			InjectHookDLL(pi);
		}
		catch(std::exception err)
		{
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % err.what()));
		}

		// resume the console process
		::ResumeThread(pi.hThread);
		::CloseHandle(pi.hThread);
	}

	try
	{
		m_consoleMsgPipe.WaitConnect();
	}
	catch(std::exception& err)
	{
		throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % err.what()));
	}

	// wait for hook DLL to set console handle
	if (::WaitForSingleObject(m_consoleParams.GetReqEvent(), 10000) == WAIT_TIMEOUT)
		throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % L"timeout (hook)"));

	ShowWindow(SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::StartShellProcessAsAdministrator
(
	const wstring& strSyncName,
	const wstring& strShell,
	const wstring& strInitialDir,
	const wstring& strInitialCmd,
	DWORD dwBasePriority,
	const std::vector<std::shared_ptr<VarEnv>>& extraEnv
)
{
	SharedMemory<DWORD> pid;
	pid.Open(strSyncName, syncObjBoth);

	UserCredentials userCredentials;
	PROCESS_INFORMATION pi = {0, 0, 0, 0};

	CreateShellProcess(
		strShell,
		strInitialDir,
		userCredentials,
		strInitialCmd,
		dwBasePriority,
		extraEnv,
		pi
	);

	*pid.Get() = pi.dwProcessId;
	::SetEvent(pid.GetReqEvent());

	// wait for shared objects creation
	if (::WaitForSingleObject(pid.GetRespEvent(), 10000) == WAIT_TIMEOUT)
		throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % L"timeout (shared objects creation)"));

	// inject our hook DLL into console process
	try
	{
		InjectHookDLL(pi);
	}
	catch(std::exception err)
	{
		throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % err.what()));
	}

	// resume the console process
	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hThread);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::AttachToShellProcess(
	DWORD dwProcessId,
	DWORD dwStartupRows,
	DWORD dwStartupColumns)
{
	try
	{
		PROCESS_INFORMATION pi = {0, 0, dwProcessId, 0};

		std::unique_ptr<void, CloseHandleHelper> hProcess(::OpenProcess(SYNCHRONIZE|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ|PROCESS_CREATE_THREAD, FALSE, pi.dwProcessId));
		pi.hProcess = hProcess.get();
		if( pi.hProcess == NULL )
			Win32Exception::ThrowFromLastError("OpenProcess");

		// create shared memory objects
		try
		{
			CreateSharedObjects(pi.dwProcessId, /*userCredentials.netOnly? L"" : userCredentials.strAccountName*/L"");
			CreateWatchdog();
		}
		catch(Win32Exception& err)
		{
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_CREATE_SHARED_OBJECTS_FAILED)) % err.what()));
		}

		// write startup params
		m_consoleParams->dwParentProcessId     = ::GetCurrentProcessId();
		m_consoleParams->dwNotificationTimeout = g_settingsHandler->GetConsoleSettings().dwChangeRefreshInterval;
		m_consoleParams->dwRefreshInterval     = g_settingsHandler->GetConsoleSettings().dwRefreshInterval;
		m_consoleParams->dwRows                = dwStartupRows;
		m_consoleParams->dwColumns             = dwStartupColumns;
		m_consoleParams->dwBufferRows          = g_settingsHandler->GetConsoleSettings().dwBufferRows;
		m_consoleParams->dwBufferColumns       = g_settingsHandler->GetConsoleSettings().dwBufferColumns;

		m_hConsoleProcess.reset(hProcess.release());
		m_dwConsolePid    = pi.dwProcessId;

		// inject our hook DLL into console process
		InjectHookDLL2(pi);

		::CloseHandle(pi.hThread);

		m_consoleMsgPipe.WaitConnect();

		// wait for hook DLL to set console handle
		if (::WaitForSingleObject(m_consoleParams.GetReqEvent(), 10000) == WAIT_TIMEOUT)
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % L"timeout (hook2)"));

		ShowWindow(SW_HIDE);
	}
	catch(std::exception& err)
	{
		throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % err.what()));
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD ConsoleHandler::StartMonitorThread()
{
	DWORD dwThreadId = 0;
	m_hMonitorThread = std::shared_ptr<void>(
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
	SendMessage(WM_SYSCOMMAND, SC_CONSOLE_MARK, 0);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ResumeScrolling()
{
	// emulate ESC keypress to end 'mark' command (we send a mark command just in case 
	// a user has already pressed ESC as I don't know an easy way to detect if the mark
	// command is active or not)
	SendMessage(WM_SYSCOMMAND, SC_CONSOLE_MARK, 0);
	SendMessage(WM_KEYDOWN,    VK_ESCAPE,       0x00010001);
	SendMessage(WM_KEYUP,      VK_ESCAPE,       0xC0010001);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring ConsoleHandler::GetCurrentDirectory(void) const
{
	std::wstring result;

	m_multipleInfo->fMask = MULTIPLEINFO_CURRENT_DIRECTORY;
	if( ::SetEvent(m_multipleInfo.GetReqEvent()) &&
	    ::WaitForSingleObject(m_multipleInfo.GetRespEvent(), 2000) == WAIT_OBJECT_0 )
		result = m_multipleInfo->szCurrentDirectory;

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DWORD ConsoleHandler::GetLastProcessId(void) const
{
	m_multipleInfo->fMask = MULTIPLEINFO_PROCESS_LIST;
	if( ::SetEvent(m_multipleInfo.GetReqEvent()) &&
	    ::WaitForSingleObject(m_multipleInfo.GetRespEvent(), 2000) == WAIT_OBJECT_0 &&
	    m_multipleInfo->dwProcessCount &&
	    m_multipleInfo->dwProcessCount < 256 )
		return m_multipleInfo->lpdwProcessList[0];

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::SelectWord(const COORD& coordCurrent, COORD& coordLeft, COORD& coordRight) const
{
	m_multipleInfo->fMask = MULTIPLEINFO_SELECT_WORD;
	m_multipleInfo->coordCurrent = coordCurrent;

	CopyPasteSettings& copyPasteSettings = g_settingsHandler->GetBehaviorSettings().copyPasteSettings;
	wcscpy_s<MAX_WORD_DELIMITERS>(m_multipleInfo->u.select_word.szLeftDelimiters,  copyPasteSettings.strLeftDelimiters.c_str());
	wcscpy_s<MAX_WORD_DELIMITERS>(m_multipleInfo->u.select_word.szRightDelimiters, copyPasteSettings.strRightDelimiters.c_str());
	m_multipleInfo->u.select_word.bIncludeLeftDelimiter  = copyPasteSettings.bIncludeLeftDelimiter;
	m_multipleInfo->u.select_word.bIncludeRightDelimiter = copyPasteSettings.bIncludeRightDelimiter;

	if( ::SetEvent(m_multipleInfo.GetReqEvent()) &&
	    ::WaitForSingleObject(m_multipleInfo.GetRespEvent(), 2000) == WAIT_OBJECT_0 )
	{
		coordLeft  = m_multipleInfo->coordLeft;
		coordRight = m_multipleInfo->coordRight;

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::ClickLink(const COORD& coordCurrent) const
{
	m_multipleInfo->fMask = MULTIPLEINFO_CLICK_LINK;
	m_multipleInfo->coordCurrent = coordCurrent;

	CopyPasteSettings& copyPasteSettings = g_settingsHandler->GetBehaviorSettings().copyPasteSettings;
	wcscpy_s<MAX_WORD_DELIMITERS>(m_multipleInfo->u.select_word.szLeftDelimiters,  copyPasteSettings.strLeftDelimiters.c_str());
	wcscpy_s<MAX_WORD_DELIMITERS>(m_multipleInfo->u.select_word.szRightDelimiters, copyPasteSettings.strRightDelimiters.c_str());

	if( ::SetEvent(m_multipleInfo.GetReqEvent()) &&
	    ::WaitForSingleObject(m_multipleInfo.GetRespEvent(), 2000) == WAIT_OBJECT_0 )
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::wstring ConsoleHandler::GetFontInfo(void) const
{
	std::wstring result(L"");

	m_multipleInfo->fMask = MULTIPLEINFO_FONT;
	if( ::SetEvent(m_multipleInfo.GetReqEvent()) &&
	    ::WaitForSingleObject(m_multipleInfo.GetRespEvent(), 2000) == WAIT_OBJECT_0 )
	{
		result += L"font index: ";
		result += std::to_wstring(m_multipleInfo->consoleFontInfo.nFont);
		result += L"\r\nface name: ";
		result += m_multipleInfo->consoleFontInfo.FaceName;
		result += L"\r\nfont familly: ";
		result += std::to_wstring(m_multipleInfo->consoleFontInfo.FontFamily);

		if((m_multipleInfo->consoleFontInfo.FontFamily & 0xf0) == FF_DECORATIVE ) result += L" DECORATIVE";
		if((m_multipleInfo->consoleFontInfo.FontFamily & 0xf0) == FF_DONTCARE   ) result += L" DONTCARE";
		if((m_multipleInfo->consoleFontInfo.FontFamily & 0xf0) == FF_MODERN     ) result += L" MODERN";
		if((m_multipleInfo->consoleFontInfo.FontFamily & 0xf0) == FF_ROMAN      ) result += L" ROMAN";
		if((m_multipleInfo->consoleFontInfo.FontFamily & 0xf0) == FF_SCRIPT     ) result += L" SCRIPT";
		if((m_multipleInfo->consoleFontInfo.FontFamily & 0xf0) == FF_SWISS      ) result += L" SWISS";

		if((m_multipleInfo->consoleFontInfo.FontFamily & TMPF_FIXED_PITCH) == TMPF_FIXED_PITCH ) result += L" fixed pitch";
		if((m_multipleInfo->consoleFontInfo.FontFamily & TMPF_VECTOR)      == TMPF_VECTOR      ) result += L" vector";
		if((m_multipleInfo->consoleFontInfo.FontFamily & TMPF_DEVICE)      == TMPF_DEVICE      ) result += L" device";
		if((m_multipleInfo->consoleFontInfo.FontFamily & TMPF_TRUETYPE)    == TMPF_TRUETYPE    ) result += L" true type";

		result += L"\r\nfont weight: ";
		result += std::to_wstring(m_multipleInfo->consoleFontInfo.FontWeight);
		result += L"\r\nfont size: width=";
		result += std::to_wstring(m_multipleInfo->coordFontSize.X);
		result += L" height=";
		result += std::to_wstring(m_multipleInfo->coordFontSize.Y);
		result += L"\r\nmax window size: cols=";
		result += std::to_wstring(m_consoleInfo->csbi.dwMaximumWindowSize.X);
		//result += std::to_wstring(m_multipleInfo->consoleFontInfo.dwFontSize.X);
		result += L" rows=";
		result += std::to_wstring(m_consoleInfo->csbi.dwMaximumWindowSize.Y);
		//result += std::to_wstring(m_multipleInfo->consoleFontInfo.dwFontSize.Y);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleHandler::SearchText(CString& text, bool bNext, const COORD& coordCurrent, COORD& coordLeft, COORD& coordRight) const
{
	m_multipleInfo->fMask = MULTIPLEINFO_SEARCH_TEXT;
	m_multipleInfo->coordCurrent = coordCurrent;

	SearchSettings& searchSettings = g_settingsHandler->GetBehaviorSettings2().searchSettings;
	m_multipleInfo->u.search_text.bMatchCase = searchSettings.bMatchCase;
	m_multipleInfo->u.search_text.bMatchWholeWord = searchSettings.bMatchWholeWord;
	m_multipleInfo->u.search_text.bNext = bNext;
	wcscpy_s<MAX_SEARCH_TEXT>(m_multipleInfo->u.search_text.szText, text.GetString());

	if( ::SetEvent(m_multipleInfo.GetReqEvent()) &&
	    ::WaitForSingleObject(m_multipleInfo.GetRespEvent(), 2000) == WAIT_OBJECT_0 )
	{
		coordLeft  = m_multipleInfo->coordLeft;
		coordRight = m_multipleInfo->coordRight;

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::UpdateCurrentUserEnvironmentBlock()
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
	m_consoleBuffer.Create((SharedMemNames::formatBuffer % dwConsoleProcessId).str(), MAX_WINDOW_COLUMNS*MAX_WINDOW_ROWS, syncObjRequest, strUser);

	// initialize buffer with spaces
	CHAR_INFO ci;
	ci.Attributes		= 0;
	ci.Char.UnicodeChar	= L' ';
	for (int i = 0; i < MAX_WINDOW_COLUMNS*MAX_WINDOW_ROWS; ++i) ::CopyMemory(&m_consoleBuffer[i], &ci, sizeof(CHAR_INFO));

	// copy info
	m_consoleCopyInfo.Create((SharedMemNames::formatCopyInfo % dwConsoleProcessId).str(), 1, syncObjBoth, strUser);

	// mouse event
	m_consoleMouseEvent.Create((SharedMemNames::formatMouseEvent % dwConsoleProcessId).str(), 1, syncObjBoth, strUser);

	// new console size
	m_newConsoleSize.Create((SharedMemNames::formatNewConsoleSize % dwConsoleProcessId).str(), 1, syncObjRequest, strUser);

	// new scroll position
	m_newScrollPos.Create((SharedMemNames::formatNewScrollPos % dwConsoleProcessId).str(), 1, syncObjRequest, strUser);

	// message pipe (workaround for User Interface Privilege Isolation messages filtering)
	m_consoleMsgPipe.Create((SharedMemNames::formatPipeName % dwConsoleProcessId).str(), strUser);

	// multiple info :
	//  current directory
	//  process list
	m_multipleInfo.Create((SharedMemNames::formatMultipleInfo % dwConsoleProcessId).str(), 1, syncObjBoth, strUser);

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
		std::shared_ptr<void>	sd;	// PSECURITY_DESCRIPTOR

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

void ConsoleHandler::InjectHookDLL(PROCESS_INFORMATION& pi)
{
	// allocate memory for parameter in the remote process
	wstring				strHookDllPath(GetModulePath(NULL));

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
	if( !::IsWow64Process(pi.hProcess, &isWow64Process) )
		Win32Exception::ThrowFromLastError("IsWow64Process");

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

  if (::GetFileAttributes(strHookDllPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_HOOK_MISSING)) % strHookDllPath.c_str()));

	::ZeroMemory(&context, sizeof(CONTEXT));

	memLen = (strHookDllPath.length()+1)*sizeof(wchar_t);
	std::unique_ptr<BYTE[]> code(new BYTE[codeSize + memLen]);

	::CopyMemory(code.get() + codeSize, strHookDllPath.c_str(), memLen);
	memLen += codeSize;

#ifdef _WIN64

	if (isWow64Process)
	{
		wow64Context.ContextFlags = CONTEXT_FULL;
		if( !::Wow64GetThreadContext(pi.hThread, &wow64Context) )
			Win32Exception::ThrowFromLastError("Wow64GetThreadContext");

		mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if( mem == NULL )
			Win32Exception::ThrowFromLastError("VirtualAllocEx");

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
			Win32Exception err("CreateProcess", ::GetLastError());
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_CANT_START_SHELL)) % strConsoleWowPath.c_str() % err.what()));
		}

		std::shared_ptr<void> wowProcess(piWow.hProcess, ::CloseHandle);
		std::shared_ptr<void> wowThread(piWow.hThread, ::CloseHandle);

		if (::WaitForSingleObject(wowProcess.get(), 5000) == WAIT_TIMEOUT)
		{
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % L"timeout (ConsoleWow.exe)"));
		}

		::GetExitCodeProcess(wowProcess.get(), reinterpret_cast<DWORD*>(&fnWow64LoadLibrary));
	}
	else
	{
		context.ContextFlags = CONTEXT_FULL;
		if( !::GetThreadContext(pi.hThread, &context) )
			Win32Exception::ThrowFromLastError("GetThreadContext");

		mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if( mem == NULL )
			Win32Exception::ThrowFromLastError("VirtualAllocEx");

		fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	}


#else

	context.ContextFlags = CONTEXT_FULL;
	if( !::GetThreadContext(pi.hThread, &context) )
		Win32Exception::ThrowFromLastError("GetThreadContext");

	mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if( mem == NULL )
		Win32Exception::ThrowFromLastError("VirtualAllocEx");

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
		*ip.pI++ = static_cast<INT>(reinterpret_cast<UINT_PTR>(mem) + codeSize);
		*ip.pB++ = 0xe8;			// call  LoadLibraryW
		*ip.pI++ = static_cast<INT>(fnWow64LoadLibrary - (reinterpret_cast<UINT_PTR>(mem) + (ip.pB+4 - code.get())));
		*ip.pB++ = 0x61;			// popa
		*ip.pB++ = 0x9d;			// popf
		*ip.pB++ = 0xc3;			// ret

		if( !::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL) )
			Win32Exception::ThrowFromLastError("WriteProcessMemory");
		if( !::FlushInstructionCache(pi.hProcess, mem, memLen) )
			Win32Exception::ThrowFromLastError("FlushInstructionCache");
		wow64Context.Eip = static_cast<DWORD>(reinterpret_cast<UINT_PTR>(mem));
		if( !::Wow64SetThreadContext(pi.hThread, &wow64Context) )
			Win32Exception::ThrowFromLastError("Wow64SetThreadContext");
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

		if( !::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL) )
			Win32Exception::ThrowFromLastError("WriteProcessMemory");
		if( !::FlushInstructionCache(pi.hProcess, mem, memLen) )
			Win32Exception::ThrowFromLastError("FlushInstructionCache");
		context.Rip = reinterpret_cast<UINT_PTR>(mem) + 16;
		if( !::SetThreadContext(pi.hThread, &context) )
			Win32Exception::ThrowFromLastError("SetThreadContext");
	}

#else

	*ip.pB++ = 0x68;			// push  eip
	*ip.pI++ = context.Eip;
	*ip.pB++ = 0x9c;			// pushf
	*ip.pB++ = 0x60;			// pusha
	*ip.pB++ = 0x68;			// push  "path\to\our.dll"
	*ip.pI++ = reinterpret_cast<UINT_PTR>(mem) + codeSize;
	*ip.pB++ = 0xe8;			// call  LoadLibraryW
	*ip.pI++ = fnLoadLibrary - (reinterpret_cast<UINT_PTR>(mem) + (ip.pB+4 - code.get()));
	*ip.pB++ = 0x61;			// popa
	*ip.pB++ = 0x9d;			// popf
	*ip.pB++ = 0xc3;			// ret

	if( !::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL) )
		Win32Exception::ThrowFromLastError("WriteProcessMemory");
	if( !::FlushInstructionCache(pi.hProcess, mem, memLen) )
		Win32Exception::ThrowFromLastError("FlushInstructionCache");
	context.Eip = reinterpret_cast<UINT_PTR>(mem);
	if( !::SetThreadContext(pi.hThread, &context) )
		Win32Exception::ThrowFromLastError("SetThreadContext");

#endif
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::InjectHookDLL2(PROCESS_INFORMATION& pi)
{
	// allocate memory for parameter in the remote process
	wstring   strHookDllPath(GetModulePath(NULL));
	UINT_PTR  fnLoadLibrary	= NULL;
	void*     mem = NULL;
	size_t    memLen = 0;
	BOOL      isWow64Process	= FALSE;

#ifdef _WIN64
	DWORD     fnWow64LoadLibrary = 0;

	if( !::IsWow64Process(pi.hProcess, &isWow64Process) )
		Win32Exception::ThrowFromLastError("IsWow64Process");
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

	if (::GetFileAttributes(strHookDllPath.c_str()) == INVALID_FILE_ATTRIBUTES)
		throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_HOOK_MISSING)) % strHookDllPath.c_str()));

#ifdef _WIN64

	if (isWow64Process)
	{
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
			Win32Exception err("CreateProcess", ::GetLastError());
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_CANT_START_SHELL)) % strConsoleWowPath.c_str() % err.what()));
		}

		std::shared_ptr<void> wowProcess(piWow.hProcess, ::CloseHandle);
		std::shared_ptr<void> wowThread(piWow.hThread, ::CloseHandle);

		if (::WaitForSingleObject(wowProcess.get(), 5000) == WAIT_TIMEOUT)
		{
			throw ConsoleException(boost::str(boost::wformat(Helpers::LoadString(IDS_ERR_DLL_INJECTION_FAILED)) % L"timeout (ConsoleWow.exe)"));
		}

		::GetExitCodeProcess(wowProcess.get(), reinterpret_cast<DWORD*>(&fnWow64LoadLibrary));
	}
	else
	{
		fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	}


#else

	fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

#endif

	memLen = (strHookDllPath.length()+1)*sizeof(wchar_t);
	mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_READWRITE);
	if( mem == NULL )
		Win32Exception::ThrowFromLastError("VirtualAllocEx");

	if( !::WriteProcessMemory(pi.hProcess, mem, strHookDllPath.c_str(), memLen, NULL) )
		Win32Exception::ThrowFromLastError("WriteProcessMemory");

#ifdef _WIN64

	if (isWow64Process)
	{
		pi.hThread =  ::CreateRemoteThread(pi.hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(fnWow64LoadLibrary), mem, 0, &pi.dwThreadId);
	}
	else
	{
		pi.hThread =  ::CreateRemoteThread(pi.hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(fnLoadLibrary), mem, 0, &pi.dwThreadId);
	}

#else

	pi.hThread =  ::CreateRemoteThread(pi.hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(fnLoadLibrary), mem, 0, &pi.dwThreadId);

#endif

	if( pi.hThread == NULL )
		Win32Exception::ThrowFromLastError("CreateRemoteThread");
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
			MutexLock handlerLock(m_bufferMutex);

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

void ConsoleHandler::PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	const wchar_t * strMsg = L"???";
	switch( Msg )
	{
	case WM_INPUTLANGCHANGEREQUEST: strMsg = L"WM_INPUTLANGCHANGEREQUEST"; break;
	case WM_INPUTLANGCHANGE:        strMsg = L"WM_INPUTLANGCHANGE";        break;
	case WM_KEYDOWN :               strMsg = L"WM_KEYDOWN";                break;
	};

	TRACE(
		L"PostMessage Msg = 0x%08lx (%s) WPARAM = %p LPARAM = %p\n",
		Msg, strMsg,
		wParam, lParam);
#endif

	NamedPipeMessage npmsg;
	npmsg.type = NamedPipeMessage::POSTMESSAGE;
	npmsg.data.winmsg.msg = Msg;
	npmsg.data.winmsg.wparam = static_cast<DWORD>(wParam);
	npmsg.data.winmsg.lparam = static_cast<DWORD>(lParam);

	try
	{
		m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
	}
#ifdef _DEBUG
	catch(std::exception& e)
	{
		TRACE(
			L"PostMessage(pipe) Msg = 0x%08lx (%s) WPARAM = %p LPARAM = %p fails (reason: %S)\n",
			Msg, strMsg,
			wParam, lParam,
			e.what());
	}
#else
	catch(std::exception&) { }
#endif
}

void ConsoleHandler::WriteConsoleInput(KEY_EVENT_RECORD* pkeyEvent)
{
	NamedPipeMessage npmsg;
	npmsg.type = NamedPipeMessage::WRITECONSOLEINPUT;
	npmsg.data.keyEvent = *pkeyEvent;

	try
	{
		m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
	}
#ifdef _DEBUG
	catch(std::exception& e)
	{
		TRACE(
			L"WriteConsoleInput(pipe) fails (reason: %S)\n",
			L"  bKeyDown          = %s\n"
			L"  dwControlKeyState = 0x%08lx\n"
			L"  UnicodeChar       = 0x%04hx\n"
			L"  wRepeatCount      = %hu\n"
			L"  wVirtualKeyCode   = 0x%04hx\n"
			L"  wVirtualScanCode  = 0x%04hx\n",
			e.what(),
			npmsg.data.keyEvent.bKeyDown?"TRUE":"FALSE",
			npmsg.data.keyEvent.dwControlKeyState,
			npmsg.data.keyEvent.uChar.UnicodeChar,
			npmsg.data.keyEvent.wRepeatCount,
			npmsg.data.keyEvent.wVirtualKeyCode,
			npmsg.data.keyEvent.wVirtualScanCode);
	}
#else
	catch(std::exception&) { }
#endif


}

void ConsoleHandler::SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	const wchar_t * strMsg = L"???";
	switch( Msg )
	{
	case WM_CLOSE:      strMsg = L"WM_CLOSE";      break;
	case WM_KEYDOWN:    strMsg = L"WM_KEYDOWN";    break;
	case WM_KEYUP:      strMsg = L"WM_KEYUP";      break;
	case WM_SYSCOMMAND: strMsg = L"WM_SYSCOMMAND"; break;
	}

	TRACE(
		L"SendMessage Msg = 0x%08lx (%s) WPARAM = %p LPARAM = %p\n",
		Msg, strMsg,
		wParam, lParam);
#endif

	if( (Msg >= WM_KEYFIRST && Msg <= WM_KEYLAST) || Msg == WM_CLOSE || Msg == WM_SYSCOMMAND )
	{
		NamedPipeMessage npmsg;
		npmsg.type = NamedPipeMessage::SENDMESSAGE;
		npmsg.data.winmsg.msg = Msg;
		npmsg.data.winmsg.wparam = static_cast<DWORD>(wParam);
		npmsg.data.winmsg.lparam = static_cast<DWORD>(lParam);

		try
		{
			m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
		}
#ifdef _DEBUG
		catch(std::exception& e)
		{
			TRACE(
				L"SendMessage(pipe) Msg = 0x%08lx (%s) WPARAM = %p LPARAM = %p fails (reason: %S)\n",
				Msg, strMsg,
				wParam, lParam,
				e.what());
		}
#else
		catch(std::exception&) { }
#endif
	}
	else
	{
#ifdef _DEBUG
		LRESULT res = ::SendMessage(m_consoleParams->hwndConsoleWindow, Msg, wParam, lParam);
		TRACE(
			L"SendMessage Msg = 0x%08lx (%s) WPARAM = %p LPARAM = %p returns %p (last error 0x%08lx)\n",
			Msg, strMsg,
			wParam, lParam,
			res,
			GetLastError());
#else
		::SendMessage(m_consoleParams->hwndConsoleWindow, Msg, wParam, lParam);
#endif
	}
}

void ConsoleHandler::SetWindowPos(int X, int Y, int cx, int cy, UINT uFlags)
{
	NamedPipeMessage npmsg;
	npmsg.type = NamedPipeMessage::SETWINDOWPOS;
	npmsg.data.windowpos.X               = X;
	npmsg.data.windowpos.Y               = Y;
	npmsg.data.windowpos.cx              = cx;
	npmsg.data.windowpos.cy              = cy;
	npmsg.data.windowpos.uFlags          = uFlags;

	try
	{
		m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
	}
#ifdef _DEBUG
	catch(std::exception& e)
	{
		TRACE(
			L"SetWindowPos(pipe) X = %d Y = %d cx = %d cy = %d uFlags = 0x%08lx fails (reason: %S)\n",
			X, Y,
			cx, cy,
			uFlags,
			e.what());
	}
#else
	catch(std::exception&) { }
#endif
}

void ConsoleHandler::ShowWindow(int nCmdShow)
{
	NamedPipeMessage npmsg;
	npmsg.type = NamedPipeMessage::SHOWWINDOW;
	npmsg.data.show.nCmdShow = nCmdShow;

	try
	{
		m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
	}
#ifdef _DEBUG
	catch(std::exception& e)
	{
		TRACE(
			L"ShowWindow(pipe) nCmdShow = 0x%08lx fails (reason: %S)\n",
			nCmdShow,
			e.what());
	}
#else
	catch(std::exception&) { }
#endif
}

void ConsoleHandler::SendTextToConsole(const wchar_t* pszText)
{
	if (pszText == NULL) return;

	size_t textLen = wcslen(pszText);

	if (textLen == 0) return;

	NamedPipeMessage npmsg;
	npmsg.type = NamedPipeMessage::SENDTEXT;
	npmsg.data.text.dwTextLen = static_cast<DWORD>(textLen);

	try
	{
		m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
		m_consoleMsgPipe.Write(pszText, textLen * sizeof(wchar_t));
	}
#ifdef _DEBUG
	catch(std::exception& e)
	{
		TRACE(
			L"SendTextToConsole(pipe) %s fails (reason: %S)\n",
			pszText,
			e.what());
	}
#else
	catch(std::exception&) { }
#endif
}

void ConsoleHandler::Clear()
{
	NamedPipeMessage npmsg;
	npmsg.type = NamedPipeMessage::CLEAR;

	try
	{
		m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
	}
	catch(std::exception&)
	{
	}
}

void ConsoleHandler::SendCtrlC()
{
	NamedPipeMessage npmsg;
	npmsg.type = NamedPipeMessage::CTRL_C;

	try
	{
		m_consoleMsgPipe.Write(&npmsg, sizeof(npmsg));
	}
	catch(std::exception&)
	{
	}
}

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


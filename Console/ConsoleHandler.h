#pragma once

#pragma warning(push)
#pragma warning(disable: 4100)
#include "FastDelegate.h"
#pragma warning(pop)


//////////////////////////////////////////////////////////////////////////////

typedef fastdelegate::FastDelegate1<bool>	ConsoleChangeDelegate;
typedef fastdelegate::FastDelegate0<>		ConsoleCloseDelegate;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct UserCredentials
{
	UserCredentials()
	: user()
	, password()
	, netOnly(false)
	, runAsAdministrator(false)
	, strUsername()
	, strDomain()
	, strAccountName()
	{
	}

	~UserCredentials()
	{
		if (password.length() > 0)
		{
			::SecureZeroMemory(reinterpret_cast<void*>(const_cast<wchar_t*>(password.c_str())), password.length());
		}
	}

	void SetUser(const wchar_t * szUser)
	{
		user = szUser;
		strUsername = user;

		if (!strUsername.empty())
		{
			size_t pos;
			if ((pos= strUsername.find(L'\\')) != wstring::npos)
			{
				strDomain	= strUsername.substr(0, pos);
				strUsername	= strUsername.substr(pos+1);
			}
			else if ((pos= strUsername.find(L'@')) != wstring::npos)
			{
				// UNC format
				strDomain	= strUsername.substr(pos + 1);
				strUsername	= strUsername.substr(0, pos);
			}
			else
			{
				// CreateProcessWithLogonW & LOGON_NETCREDENTIALS_ONLY fails if domain is NULL
				wchar_t szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
				DWORD   dwComputerNameLen = ARRAYSIZE(szComputerName);
				if( ::GetComputerName(szComputerName, &dwComputerNameLen) )
					strDomain = szComputerName;
			}

			if (!strDomain.empty())
				strAccountName = strDomain + L"\\";
			strAccountName += strUsername;
		}
	}

	wstring	user;
	wstring password;
	bool netOnly;
	bool runAsAdministrator;
	wstring strUsername;
	wstring strDomain;
	wstring strAccountName;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ConsoleHandler
{
	public:
		ConsoleHandler();
		~ConsoleHandler();

		Mutex m_bufferMutex;

	public:

		void SetupDelegates(ConsoleChangeDelegate consoleChangeDelegate, ConsoleCloseDelegate consoleCloseDelegate);
		void StartShellProcess
		(
			const wstring& strTitle,
			const wstring& strShell,
			const wstring& strInitialDir,
			const UserCredentials& userCredentials,
			const wstring& strInitialCmd,
			const wstring& strConsoleTitle,
			DWORD dwStartupRows,
			DWORD dwStartupColumns
		);

		void StartShellProcessAsAdministrator
		(
			const wstring& strSyncName,
			const wstring& strShell,
			const wstring& strInitialDir,
			const wstring& strInitialCmd
		);

		DWORD StartMonitorThread();
		void StopMonitorThread();

		std::shared_ptr<void> GetConsoleHandle() const					{ return m_hConsoleProcess; }

		SharedMemory<ConsoleParams>& GetConsoleParams()				{ return m_consoleParams; }
		SharedMemory<ConsoleInfo>& GetConsoleInfo()	{ return m_consoleInfo; }
		SharedMemory<CONSOLE_CURSOR_INFO>& GetCursorInfo()			{ return m_cursorInfo; }
		SharedMemory<CHAR_INFO>& GetConsoleBuffer()					{ return m_consoleBuffer; }
		SharedMemory<ConsoleCopy>& GetCopyInfo()					{ return m_consoleCopyInfo; }
		SharedMemory<ConsoleSize>& GetNewConsoleSize()				{ return m_newConsoleSize; }
		SharedMemory<SIZE>& GetNewScrollPos()						{ return m_newScrollPos; }

		void SendMouseEvent(const COORD& mousePos, DWORD dwMouseButtonState, DWORD dwControlKeyState, DWORD dwEventFlags);

		void StopScrolling();
		void ResumeScrolling();

		static void UpdateEnvironmentBlock();

		inline DWORD GetConsolePid(void) const { return m_dwConsolePid; }

		void PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
		void WriteConsoleInput(KEY_EVENT_RECORD* pkeyEvent);
		void SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
		void SetWindowPos(int X, int Y, int cx, int cy, UINT uFlags);
		void ShowWindow(int nCmdShow);
		void SendTextToConsole(const wchar_t* pszText);

	private:

		bool CreateSharedObjects(DWORD dwConsoleProcessId, const wstring& strUser);
		void CreateWatchdog();

		bool InjectHookDLL(PROCESS_INFORMATION& pi);

		void CreateShellProcess
		(
			const wstring& strShell,
			const wstring& strInitialDir,
			const UserCredentials& userCredentials,
			const wstring& strInitialCmd,
			const wstring& strConsoleTitle,
			PROCESS_INFORMATION& pi
		);

		void RunAsAdministrator
		(
			const wstring& strSyncName,
			const wstring& strTitle,
			const wstring& strInitialDir,
			const wstring& strInitialCmd,
			PROCESS_INFORMATION& pi
		);

	private:

		static DWORD WINAPI MonitorThreadStatic(LPVOID lpParameter);
		DWORD MonitorThread();


	private:

		wstring GetModulePath(HMODULE hModule);


	private:

    ConsoleChangeDelegate             m_consoleChangeDelegate;
    ConsoleCloseDelegate              m_consoleCloseDelegate;

    std::shared_ptr<void>             m_hConsoleProcess;

    SharedMemory<ConsoleParams>       m_consoleParams;
    SharedMemory<ConsoleInfo>         m_consoleInfo;
    SharedMemory<CONSOLE_CURSOR_INFO> m_cursorInfo;
    SharedMemory<CHAR_INFO>           m_consoleBuffer;
    SharedMemory<ConsoleCopy>         m_consoleCopyInfo;
    SharedMemory<MOUSE_EVENT_RECORD>  m_consoleMouseEvent;

    SharedMemory<ConsoleSize>         m_newConsoleSize;
    SharedMemory<SIZE>                m_newScrollPos;

    NamedPipe                         m_consoleMsgPipe;

    std::shared_ptr<void>             m_hMonitorThread;
    std::shared_ptr<void>             m_hMonitorThreadExit;

    static std::shared_ptr<void>      s_environmentBlock;
    static std::shared_ptr<Mutex>     s_parentProcessWatchdog;

    DWORD                             m_dwConsolePid;

};

//////////////////////////////////////////////////////////////////////////////

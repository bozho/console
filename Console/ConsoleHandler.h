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
	{
	}

	~UserCredentials()
	{
		if (password.length() > 0)
		{
			::SecureZeroMemory(reinterpret_cast<void*>(const_cast<wchar_t*>(password.c_str())), password.length());
		}
	}

	wstring	user;
	wstring password;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ConsoleHandler
{
	public:
		ConsoleHandler();
		~ConsoleHandler();

	public:

		void SetupDelegates(ConsoleChangeDelegate consoleChangeDelegate, ConsoleCloseDelegate consoleCloseDelegate);
		bool StartShellProcess
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
		);

		DWORD StartMonitorThread();
		void StopMonitorThread();

		shared_ptr<void> GetConsoleHandle() const					{ return m_hConsoleProcess; }

		SharedMemory<ConsoleParams>& GetConsoleParams()				{ return m_consoleParams; }
		SharedMemory<ConsoleInfo>& GetConsoleInfo()	{ return m_consoleInfo; }
		SharedMemory<CONSOLE_CURSOR_INFO>& GetCursorInfo()			{ return m_cursorInfo; }
		SharedMemory<CHAR_INFO>& GetConsoleBuffer()					{ return m_consoleBuffer; }
		SharedMemory<ConsoleCopy>& GetCopyInfo()					{ return m_consoleCopyInfo; }
		SharedMemory<TextInfo>& GetTextInfo()						{ return m_consoleTextInfo; }
		SharedMemory<ConsoleSize>& GetNewConsoleSize()				{ return m_newConsoleSize; }
		SharedMemory<SIZE>& GetNewScrollPos()						{ return m_newScrollPos; }

		void SendMouseEvent(const COORD& mousePos, DWORD dwMouseButtonState, DWORD dwControlKeyState, DWORD dwEventFlags);

		void StopScrolling();
		void ResumeScrolling();

		static void UpdateEnvironmentBlock();

	private:

		bool CreateSharedObjects(DWORD dwConsoleProcessId, const wstring& strUser);
		void CreateWatchdog();

		bool InjectHookDLL(PROCESS_INFORMATION& pi);

	private:
		
		static DWORD WINAPI MonitorThreadStatic(LPVOID lpParameter);
		DWORD MonitorThread();


	private:
		
		wstring GetModulePath(HMODULE hModule);


	private:

		ConsoleChangeDelegate						m_consoleChangeDelegate;
		ConsoleCloseDelegate						m_consoleCloseDelegate;

		shared_ptr<void>							m_hConsoleProcess;

		SharedMemory<ConsoleParams>					m_consoleParams;
		SharedMemory<ConsoleInfo>	m_consoleInfo;
		SharedMemory<CONSOLE_CURSOR_INFO>			m_cursorInfo;
		SharedMemory<CHAR_INFO>						m_consoleBuffer;
		SharedMemory<ConsoleCopy>					m_consoleCopyInfo;
		SharedMemory<TextInfo>						m_consoleTextInfo;
		SharedMemory<MOUSE_EVENT_RECORD>			m_consoleMouseEvent;

		SharedMemory<ConsoleSize>					m_newConsoleSize;
		SharedMemory<SIZE>							m_newScrollPos;

		shared_ptr<void>							m_hMonitorThread;
		shared_ptr<void>							m_hMonitorThreadExit;

		static shared_ptr<void>						s_environmentBlock;
		static shared_ptr<Mutex>					s_parentProcessWatchdog;

};

//////////////////////////////////////////////////////////////////////////////

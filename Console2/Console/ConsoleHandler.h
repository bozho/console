#pragma once

#pragma warning(push)
#pragma warning(disable: 4100)
#include "FastDelegate.h"
#pragma warning(pop)

#include "../shared/SharedMemory.h"
#include "../shared/Structures.h"

//////////////////////////////////////////////////////////////////////////////

typedef fastdelegate::FastDelegate1<bool>	ConsoleChangeDelegate;
typedef fastdelegate::FastDelegate0<>		ConsoleCloseDelegate;

//////////////////////////////////////////////////////////////////////////////

class ConsoleHandler
{
	public:
		ConsoleHandler();
		~ConsoleHandler();

	public:

		void SetupDelegates(ConsoleChangeDelegate consoleChangeDelegate, ConsoleCloseDelegate consoleCloseDelegate);
		bool StartShellProcess(const wstring& strCustomShell, const wstring& strInitialDir, const wstring& strInitialCmd, const wstring& strConsoleTitle, DWORD dwStartupRows, DWORD dwStartupColumns, bool bDebugFlag);

		DWORD StartMonitorThread();
		void StopMonitorThread();

		shared_ptr<void> GetConsoleHandle() const					{ return m_hConsoleProcess; }

		SharedMemory<ConsoleParams>& GetConsoleParams()				{ return m_consoleParams; }
		SharedMemory<CONSOLE_SCREEN_BUFFER_INFO>& GetConsoleInfo()	{ return m_consoleInfo; }
		SharedMemory<CONSOLE_CURSOR_INFO>& GetCursorInfo()			{ return m_cursorInfo; }
		SharedMemory<CHAR_INFO>& GetConsoleBuffer()					{ return m_consoleBuffer; }
		SharedMemory<ConsoleCopy>& GetCopyInfo()					{ return m_consoleCopyInfo; }
		SharedMemory<UINT_PTR>& GetPasteInfo()						{ return m_consolePasteInfo; }
		SharedMemory<ConsoleSize>& GetNewConsoleSize()				{ return m_newConsoleSize; }
		SharedMemory<SIZE>& GetNewScrollPos()						{ return m_newScrollPos; }

		void SendMouseEvent(const COORD& mousePos, DWORD dwMouseButtonState, DWORD dwControlKeyState, DWORD dwEventFlags);

	private:

		bool CreateSharedObjects(DWORD dwConsoleProcessId);

		bool InjectHookDLL();

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
		SharedMemory<CONSOLE_SCREEN_BUFFER_INFO>	m_consoleInfo;
		SharedMemory<CONSOLE_CURSOR_INFO>			m_cursorInfo;
		SharedMemory<CHAR_INFO>						m_consoleBuffer;
		SharedMemory<ConsoleCopy>					m_consoleCopyInfo;
		SharedMemory<UINT_PTR>						m_consolePasteInfo;
		SharedMemory<MOUSE_EVENT_RECORD>			m_consoleMouseEvent;

		SharedMemory<ConsoleSize>					m_newConsoleSize;
		SharedMemory<SIZE>							m_newScrollPos;

		shared_ptr<void>							m_hMonitorThread;
		shared_ptr<void>							m_hMonitorThreadExit;
};

//////////////////////////////////////////////////////////////////////////////

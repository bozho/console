#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ConsoleHandler
{
	public:
		ConsoleHandler();
		~ConsoleHandler();

	public:

		DWORD StartMonitorThread();
		void StopMonitorThread();

	private:

		bool OpenSharedMemory();

		void ReadConsoleBuffer();

		void ResizeConsoleWindow(HANDLE hStdOut, DWORD& dwColumns, DWORD& dwRows);

		void PasteConsoleText(HANDLE hStdIn, const shared_ptr<wchar_t>& pszText);

		void ScrollConsole(HANDLE hStdOut, int nXDelta, int nYDelta);

		void SetConsoleParams(HANDLE hStdOut);

	private:

		static DWORD WINAPI MonitorThreadStatic(LPVOID lpParameter);
		DWORD MonitorThread();

	private:

		shared_ptr<void>							m_hParentProcess;

		SharedMemory<ConsoleParams>					m_consoleParams;
		SharedMemory<CONSOLE_SCREEN_BUFFER_INFO>	m_consoleInfo;
		SharedMemory<CONSOLE_CURSOR_INFO>			m_cursorInfo;
		SharedMemory<CHAR_INFO>						m_consoleBuffer;
		SharedMemory<UINT_PTR>						m_consolePaste;

		SharedMemory<ConsoleSize>					m_newConsoleSize;
		SharedMemory<SIZE>							m_newScrollPos;

		shared_ptr<void>							m_hMonitorThread;
		shared_ptr<void>							m_hMonitorThreadExit;

		DWORD										m_dwScreenBufferSize;
};

//////////////////////////////////////////////////////////////////////////////



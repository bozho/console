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

		bool OpenSharedObjects();

		void ReadConsoleBuffer();

		void ResizeConsoleWindow(HANDLE hStdOut, DWORD& dwColumns, DWORD& dwRows, DWORD dwResizeWindowEdge);

		void CopyConsoleText();

		void PasteConsoleText(HANDLE hStdIn, const shared_ptr<wchar_t>& pszPasteBuffer);

		void SetResetKeyInput(scoped_array<INPUT>& kbdInputs, WORD wVk, short& sCount);

		void WriteConsoleInput(HANDLE hStdIn, scoped_array<INPUT_RECORD>& consoleInputs, size_t& consoleInputCount, size_t maxConsoleInputCount);

		void SendMouseEvent(HANDLE hStdIn);

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
		SharedMemory<ConsoleCopy>					m_consoleCopyInfo;
		SharedMemory<UINT_PTR>						m_consolePasteInfo;
		SharedMemory<MOUSE_EVENT_RECORD>			m_consoleMouseEvent;

		SharedMemory<ConsoleSize>					m_newConsoleSize;
		SharedMemory<SIZE>							m_newScrollPos;

		shared_ptr<void>							m_hMonitorThread;
		shared_ptr<void>							m_hMonitorThreadExit;

		DWORD										m_dwScreenBufferSize;
};

//////////////////////////////////////////////////////////////////////////////



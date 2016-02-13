#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class StdOutHandle
{
public:
	StdOutHandle(bool bReadOnly)
	{
		if( bReadOnly )
		{
			m_hStdOut = ::CreateFile(
				L"CONOUT$",
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				0);
		}
		else
		{
			m_hStdOut = ::CreateFile(
				L"CONOUT$",
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				0);
		}

		if( m_hStdOut == INVALID_HANDLE_VALUE )
		{
			Win32Exception err("CreateFile", ::GetLastError());
			TRACE(L"CreateFile returns error (%lu) : %S\n", err.GetErrorCode(), err.what());
		}
	}

	~StdOutHandle()
	{
		if( m_hStdOut && m_hStdOut != INVALID_HANDLE_VALUE )
			::CloseHandle(m_hStdOut);
	}

	operator HANDLE() { return m_hStdOut; }


private:
	HANDLE m_hStdOut;
};

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

class ClipboardData;

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
		void ForceReadConsoleBuffer();
		void RealReadConsoleBuffer();

		void SetConsoleFont();

		void ResizeConsoleWindow(DWORD& dwColumns, DWORD& dwRows, DWORD dwResizeWindowEdge);

		void CopyConsoleText();

		void SendConsoleText(HANDLE hStdIn, const wchar_t*	pszText, size_t	textLen);

		void SendMouseEvent(HANDLE hStdIn);

		void ScrollConsole(int nXDelta, int nYDelta);

		void SetConsoleParams();

		void GetFontInfo();

		void GetSelectionPart();

	private:

		static DWORD WINAPI MonitorThreadStatic(LPVOID lpParameter);
		DWORD MonitorThread();

		void CopyConsoleTextLine  (HANDLE hStdOut, std::unique_ptr<ClipboardData> clipboardDataPtr[], size_t clipboardDataCount);
		void CopyConsoleTextColumn(HANDLE hStdOut, std::unique_ptr<ClipboardData> clipboardDataPtr[], size_t clipboardDataCount);
		void SelectWord           ();
		void ClickLink            ();
		void SearchText           ();
		void Clear                ();
		bool GetPowerShellProgress(HANDLE hStdOut, CONSOLE_SCREEN_BUFFER_INFO& csbiConsole, unsigned long long & ullProgressCompleted, unsigned long long & ullProgressTotal);

	private:

		SharedMemory<ConsoleParams>       m_consoleParams;
		SharedMemory<ConsoleInfo>         m_consoleInfo;
		SharedMemory<CONSOLE_CURSOR_INFO> m_cursorInfo;
		SharedMemory<CHAR_INFO>           m_consoleBuffer;
		SharedMemory<ConsoleCopy>         m_consoleCopyInfo;
		SharedMemory<MOUSE_EVENT_RECORD>  m_consoleMouseEvent;

		SharedMemory<ConsoleSize>         m_newConsoleSize;
		SharedMemory<SIZE>                m_newScrollPos;
		SharedMemory<MultipleInfo>        m_multipleInfo;

		NamedPipe                         m_consoleMsgPipe;

		std::shared_ptr<void>             m_hMonitorThread;
		std::shared_ptr<void>             m_hMonitorThreadExit;

		DWORD                             m_dwScreenBufferSize;

		wchar_t                           m_szConsoleTitle[1024];

		DWORD                                          m_dwWaitingTime;
		std::chrono::system_clock::time_point m_timePoint;

		std::unique_ptr<ClipboardData>    m_selectionFullText;

		StdOutHandle                      m_hStdOut;
};

//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
using namespace std;
using namespace boost;

#include "../shared/SharedMemNames.h"
#include "ConsoleHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

ConsoleHandler::ConsoleHandler()
: m_consoleParams()
, m_consoleInfo()
, m_cursorInfo()
, m_consoleBuffer()
, m_consoleCopyInfo()
, m_consoleTextInfo()
, m_consoleMouseEvent()
, m_newConsoleSize()
, m_newScrollPos()
, m_hMonitorThread()
, m_hMonitorThreadExit(shared_ptr<void>(::CreateEvent(NULL, FALSE, FALSE, NULL), ::CloseHandle))
, m_dwScreenBufferSize(0)
{
}

ConsoleHandler::~ConsoleHandler()
{
	StopMonitorThread();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
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

bool ConsoleHandler::OpenSharedObjects()
{
	// open startup params  memory object
	DWORD dwProcessId = ::GetCurrentProcessId();

	// TODO: error handling
	m_consoleParams.Open((SharedMemNames::formatConsoleParams % dwProcessId).str(), syncObjRequest);

	// open console info shared memory object
	m_consoleInfo.Open((SharedMemNames::formatInfo % dwProcessId).str(), syncObjRequest);

	// open console info shared memory object
	m_cursorInfo.Open((SharedMemNames::formatCursorInfo % dwProcessId).str(), syncObjRequest);

	// open console buffer shared memory object
	m_consoleBuffer.Open((SharedMemNames::formatBuffer % dwProcessId).str(), syncObjRequest);

	// copy info
	m_consoleCopyInfo.Open((SharedMemNames::formatCopyInfo % dwProcessId).str(), syncObjBoth);

	// text info (used for sending text to console)
	m_consoleTextInfo.Open((SharedMemNames::formatTextInfo % dwProcessId).str(), syncObjBoth);

	// mouse event
	m_consoleMouseEvent.Open((SharedMemNames::formatMouseEvent % dwProcessId).str(), syncObjBoth);

	// open new console size shared memory object
	m_newConsoleSize.Open((SharedMemNames::formatNewConsoleSize % dwProcessId).str(), syncObjRequest);

	// new scroll position
	m_newScrollPos.Open((SharedMemNames::formatNewScrollPos % dwProcessId).str(), syncObjRequest);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ReadConsoleBuffer()
{
	// we take a fresh STDOUT handle - seems to work better (in case a program
	// has opened a new screen output buffer)
	// no need to call CloseHandle when done, we're reusing console handles
	shared_ptr<void> hStdOut(::CreateFile(
								L"CONOUT$",
								GENERIC_WRITE | GENERIC_READ,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								0,
								0),
								::CloseHandle);

	CONSOLE_SCREEN_BUFFER_INFO	csbiConsole;
	COORD						coordConsoleSize;

	::GetConsoleScreenBufferInfo(hStdOut.get(), &csbiConsole);

	coordConsoleSize.X	= csbiConsole.srWindow.Right - csbiConsole.srWindow.Left + 1;
	coordConsoleSize.Y	= csbiConsole.srWindow.Bottom - csbiConsole.srWindow.Top + 1;

/*
	TRACE(L"ReadConsoleBuffer console buffer size: %ix%i\n", csbiConsole.dwSize.X, csbiConsole.dwSize.Y);
	TRACE(L"ReadConsoleBuffer console rect: %ix%i - %ix%i\n", csbiConsole.srWindow.Left, csbiConsole.srWindow.Top, csbiConsole.srWindow.Right, csbiConsole.srWindow.Bottom);
	TRACE(L"console window rect: (%i, %i) - (%i, %i)\n", csbiConsole.srWindow.Top, csbiConsole.srWindow.Left, csbiConsole.srWindow.Bottom, csbiConsole.srWindow.Right);
*/

	// do console output buffer reading
	DWORD					dwScreenBufferSize	= coordConsoleSize.X * coordConsoleSize.Y;
	DWORD					dwScreenBufferOffset= 0;

	shared_array<CHAR_INFO> pScreenBuffer(new CHAR_INFO[dwScreenBufferSize]);

	COORD		coordBufferSize;
	// start coordinates for the buffer are always (0, 0) - we use offset
	COORD		coordStart = {0, 0};
	SMALL_RECT	srBuffer;

//	TRACE(L"===================================================================\n");

	// ReadConsoleOutput seems to fail for large (around 6k CHAR_INFO's) buffers
	// here we calculate max buffer size (row count) for safe reading
	coordBufferSize.X	= csbiConsole.srWindow.Right - csbiConsole.srWindow.Left + 1;
	coordBufferSize.Y	= 6144 / coordBufferSize.X;

	// initialize reading rectangle
	srBuffer.Top		= csbiConsole.srWindow.Top;
	srBuffer.Bottom		= csbiConsole.srWindow.Top + coordBufferSize.Y - 1;
	srBuffer.Left		= csbiConsole.srWindow.Left;
	srBuffer.Right		= csbiConsole.srWindow.Left + csbiConsole.srWindow.Right - csbiConsole.srWindow.Left;

/*
	TRACE(L"Buffer size for loop reads: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);
	TRACE(L"-------------------------------------------------------------------\n");
*/

	// read rows 'chunks'
	SHORT i = 0;
	for (; i < coordConsoleSize.Y / coordBufferSize.Y; ++i)
	{
//		TRACE(L"Reading region: (%i, %i) - (%i, %i)\n", srBuffer.Left, srBuffer.Top, srBuffer.Right, srBuffer.Bottom);

		::ReadConsoleOutput(
			hStdOut.get(), 
			pScreenBuffer.get() + dwScreenBufferOffset, 
			coordBufferSize, 
			coordStart, 
			&srBuffer);

		srBuffer.Top		= srBuffer.Top + coordBufferSize.Y;
		srBuffer.Bottom		= srBuffer.Bottom + coordBufferSize.Y;

		dwScreenBufferOffset += coordBufferSize.X * coordBufferSize.Y;
	}

	// read the last 'chunk', we need to calculate the number of rows in the
	// last chunk and update bottom coordinate for the region
	coordBufferSize.Y	= coordConsoleSize.Y - i * coordBufferSize.Y;
	srBuffer.Bottom		= csbiConsole.srWindow.Bottom;

/*
	TRACE(L"Buffer size for last read: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);
	TRACE(L"-------------------------------------------------------------------\n");
	TRACE(L"Reading region: (%i, %i) - (%i, %i)\n", srBuffer.Left, srBuffer.Top, srBuffer.Right, srBuffer.Bottom);
*/

	::ReadConsoleOutput(
		hStdOut.get(), 
		pScreenBuffer.get() + dwScreenBufferOffset, 
		coordBufferSize, 
		coordStart, 
		&srBuffer);


//	TRACE(L"===================================================================\n");

	// compare previous buffer, and if different notify Console
	SharedMemoryLock consoleInfoLock(m_consoleInfo);
	SharedMemoryLock bufferLock(m_consoleBuffer);

	bool textChanged = (::memcmp(m_consoleBuffer.Get(), pScreenBuffer.get(), m_dwScreenBufferSize*sizeof(CHAR_INFO)) != 0);

	if ((::memcmp(&m_consoleInfo->csbi, &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO)) != 0) ||
		(m_dwScreenBufferSize != dwScreenBufferSize) ||
		textChanged)
	{
		// update screen buffer variables
		m_dwScreenBufferSize = dwScreenBufferSize;

		::CopyMemory(&m_consoleInfo->csbi, &csbiConsole, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
		
		// only Console sets the flag to false, after it's done repainting text
		if (textChanged) m_consoleInfo->textChanged = true;

		::CopyMemory(m_consoleBuffer.Get(), pScreenBuffer.get(), m_dwScreenBufferSize*sizeof(CHAR_INFO));

		::GetConsoleCursorInfo(hStdOut.get(), m_cursorInfo.Get());

		m_consoleBuffer.SetReqEvent();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ResizeConsoleWindow(HANDLE hStdOut, DWORD& dwColumns, DWORD& dwRows, DWORD dwResizeWindowEdge)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	::GetConsoleScreenBufferInfo(hStdOut, &csbi);
	TRACE(L"Console size: %ix%i\n", csbi.dwSize.X, csbi.dwSize.Y);
	TRACE(L"Old win pos: %ix%i - %ix%i\n", csbi.srWindow.Left, csbi.srWindow.Top, csbi.srWindow.Right, csbi.srWindow.Bottom);
	
	// check against max size
	TRACE(L"Columns: %i\n", dwColumns);
	TRACE(L"Max columns: %i\n", m_consoleParams->dwMaxColumns);

	TRACE(L"Rows: %i\n", dwRows);
	TRACE(L"Max rows: %i\n", m_consoleParams->dwMaxRows);

	if (dwColumns > m_consoleParams->dwMaxColumns) dwColumns = m_consoleParams->dwMaxColumns;
	if (dwRows > m_consoleParams->dwMaxRows) dwRows = m_consoleParams->dwMaxRows;

	COORD		coordBufferSize;
	SMALL_RECT	srConsoleRect;

	TRACE(L"Screen buffer: %ix%i\n", m_consoleParams->dwBufferRows, m_consoleParams->dwBufferColumns);

	// determine new buffer size
	if (m_consoleParams->dwBufferColumns == 0)
	{
		coordBufferSize.X = static_cast<SHORT>(dwColumns);
	}
	else
	{
		coordBufferSize.X = static_cast<SHORT>(m_consoleParams->dwBufferColumns);
	}

	if (m_consoleParams->dwBufferRows == 0)
	{
		coordBufferSize.Y = static_cast<SHORT>(dwRows);
	}
	else
	{
		coordBufferSize.Y = static_cast<SHORT>(m_consoleParams->dwBufferRows);
	}


	// determine new window size
	// vertical size
	switch (dwResizeWindowEdge)
	{
		case WMSZ_TOP :
		case WMSZ_TOPLEFT :
		case WMSZ_TOPRIGHT :
		{
			if ((csbi.srWindow.Top == 0) || (csbi.srWindow.Bottom - static_cast<SHORT>(dwRows - 1) <= 0))
			{
				srConsoleRect.Top	= 0;
				srConsoleRect.Bottom= static_cast<SHORT>(dwRows - 1);
			}
			else
			{
				srConsoleRect.Top	= csbi.srWindow.Bottom - static_cast<SHORT>(dwRows);
				srConsoleRect.Bottom= csbi.srWindow.Bottom;
			}

			break;
		}

		case WMSZ_BOTTOM :
		case WMSZ_BOTTOMLEFT :
		case WMSZ_BOTTOMRIGHT :
		{
			if ((m_consoleParams->dwBufferRows > 0) && (csbi.srWindow.Top + static_cast<SHORT>(dwRows) > static_cast<SHORT>(m_consoleParams->dwBufferRows)))
			{
				srConsoleRect.Top	= static_cast<SHORT>(m_consoleParams->dwBufferRows - dwRows);
				srConsoleRect.Bottom= static_cast<SHORT>(m_consoleParams->dwBufferRows - 1);
			}
			else
			{
				srConsoleRect.Top	= csbi.srWindow.Top;
				srConsoleRect.Bottom= csbi.srWindow.Top + static_cast<SHORT>(dwRows - 1);
			}

			break;
		}

		default :
		{
			srConsoleRect.Top	= csbi.srWindow.Top;
			srConsoleRect.Bottom= csbi.srWindow.Top + static_cast<SHORT>(dwRows - 1);
		}
	}

	// horizontal size
	switch (dwResizeWindowEdge)
	{
		case WMSZ_LEFT :
		case WMSZ_TOPLEFT :
		case WMSZ_BOTTOMLEFT :
		{
			if ((csbi.srWindow.Left == 0) || (csbi.srWindow.Right - static_cast<SHORT>(dwColumns - 1) <= 0))
			{
				srConsoleRect.Left	= 0;
				srConsoleRect.Right	= static_cast<SHORT>(dwColumns - 1);
			}
			else
			{
				srConsoleRect.Left	= csbi.srWindow.Right - static_cast<SHORT>(dwColumns);
				srConsoleRect.Right	= csbi.srWindow.Right;
			}

			break;
		}

		case WMSZ_RIGHT :
		case WMSZ_TOPRIGHT :
		case WMSZ_BOTTOMRIGHT :
		{
			if ((m_consoleParams->dwBufferColumns != 0) && (csbi.srWindow.Left + static_cast<SHORT>(dwColumns) > static_cast<SHORT>(m_consoleParams->dwBufferColumns)))
			{
				srConsoleRect.Left	= static_cast<SHORT>(m_consoleParams->dwBufferColumns - dwColumns);
				srConsoleRect.Right	= static_cast<SHORT>(m_consoleParams->dwBufferColumns - 1);
			}
			else
			{
				srConsoleRect.Left	= csbi.srWindow.Left;
				srConsoleRect.Right	= csbi.srWindow.Left + static_cast<SHORT>(dwColumns - 1);
			}

			break;
		}

		default :
		{
			srConsoleRect.Left	= csbi.srWindow.Left;
			srConsoleRect.Right	= csbi.srWindow.Left + static_cast<SHORT>(dwColumns - 1);
		}
	}

	TRACE(L"New win pos: %ix%i - %ix%i\n", srConsoleRect.Left, srConsoleRect.Top, srConsoleRect.Right, srConsoleRect.Bottom);
	TRACE(L"Buffer size: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);

	COORD		finalCoordBufferSize;
	SMALL_RECT	finalConsoleRect;

	// first, resize rows
	finalCoordBufferSize.X	= csbi.dwSize.X;
	finalCoordBufferSize.Y	= coordBufferSize.Y;

	finalConsoleRect.Left	= csbi.srWindow.Left;
	finalConsoleRect.Right	= csbi.srWindow.Right;

	finalConsoleRect.Top	= srConsoleRect.Top;
	finalConsoleRect.Bottom	= srConsoleRect.Bottom;

	if (coordBufferSize.Y > csbi.dwSize.Y)
	{
		// if new buffer size is > than old one, we need to resize the buffer first
		::SetConsoleScreenBufferSize(hStdOut, finalCoordBufferSize);
		::SetConsoleWindowInfo(hStdOut, TRUE, &finalConsoleRect);
	}
	else
	{
		::SetConsoleWindowInfo(hStdOut, TRUE, &finalConsoleRect);
		::SetConsoleScreenBufferSize(hStdOut, finalCoordBufferSize);
	}

	// then, resize columns
	finalCoordBufferSize.X	= coordBufferSize.X;

	finalConsoleRect.Left	= srConsoleRect.Left;
	finalConsoleRect.Right	= srConsoleRect.Right;

	if (coordBufferSize.X > csbi.dwSize.X)
	{
		// if new buffer size is > than old one, we need to resize the buffer first
		::SetConsoleScreenBufferSize(hStdOut, finalCoordBufferSize);
		::SetConsoleWindowInfo(hStdOut, TRUE, &finalConsoleRect);
	}
	else
	{
		::SetConsoleWindowInfo(hStdOut, TRUE, &finalConsoleRect);
		::SetConsoleScreenBufferSize(hStdOut, finalCoordBufferSize);
	}

	TRACE(L"console buffer size: %ix%i\n", coordBufferSize.X, coordBufferSize.Y);
	TRACE(L"console rect: %ix%i - %ix%i\n", finalConsoleRect.Left, finalConsoleRect.Top, finalConsoleRect.Right, finalConsoleRect.Bottom);

	::GetConsoleScreenBufferInfo(hStdOut, &csbi);

	dwColumns	= csbi.srWindow.Right - csbi.srWindow.Left + 1;
	dwRows		= csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

//	TRACE(L"console buffer size: %ix%i\n", csbi.dwSize.X, csbi.dwSize.Y);
//	TRACE(L"console rect: %ix%i - %ix%i\n", csbi.srWindow.Left, csbi.srWindow.Top, csbi.srWindow.Right, csbi.srWindow.Bottom);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::CopyConsoleText()
{
	if (!::OpenClipboard(NULL)) return;

	COORD&	coordStart	= m_consoleCopyInfo->coordStart;
	COORD&	coordEnd	= m_consoleCopyInfo->coordEnd;

//	TRACE(L"Copy request: %ix%i - %ix%i\n", coordStart.X, coordStart.Y, coordEnd.X, coordEnd.Y);

	shared_ptr<void> hStdOut(
						::CreateFile(
							L"CONOUT$",
							GENERIC_WRITE | GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							0,
							0),
							::CloseHandle);

	// get total console size
	CONSOLE_SCREEN_BUFFER_INFO	csbiConsole;
	wstring						strText(L"");

	::GetConsoleScreenBufferInfo(hStdOut.get(), &csbiConsole);

	for (SHORT i = coordStart.Y; i <= coordEnd.Y; ++i)
	{
		SMALL_RECT				srBuffer;
		COORD					coordFrom		= {0, 0};
		COORD					coordBufferSize	= {(m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns) : static_cast<SHORT>(m_consoleParams->dwColumns), 1};
		shared_array<CHAR_INFO> pScreenBuffer(new CHAR_INFO[coordBufferSize.X]);

//		TRACE(L"i: %i, coordStart.Y: %i, coordStart.X: %i\n", i, coordStart.Y, coordStart.X);
		srBuffer.Left	= (i == coordStart.Y) ? coordStart.X : 0;
		srBuffer.Top	= i;
//		srBuffer.Right	= ((coordEnd.Y > coordStart.Y) && (i == coordEnd.Y)) ? coordEnd.X : (m_consoleParams->dwBufferColumns > 0) ? m_consoleParams->dwBufferColumns - 1 : m_consoleParams->dwColumns - 1;
		srBuffer.Right	= (i == coordEnd.Y) ? coordEnd.X : (m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns - 1) : static_cast<SHORT>(m_consoleParams->dwColumns - 1);
		srBuffer.Bottom	= i;


//		TRACE(L"Reading region: (%i, %i) - (%i, %i)\n", srBuffer.Left, srBuffer.Top, srBuffer.Right, srBuffer.Bottom);

		::ReadConsoleOutput(
			hStdOut.get(), 
			pScreenBuffer.get(), 
			coordBufferSize, 
			coordFrom, 
			&srBuffer);

//		TRACE(L"Read region:    (%i, %i) - (%i, %i)\n", srBuffer.Left, srBuffer.Top, srBuffer.Right, srBuffer.Bottom);

		wstring strRow(L"");
		bool	bWrap = true;

		for (SHORT x = 0; x <= srBuffer.Right - srBuffer.Left; ++x)
		{
			if (pScreenBuffer[x].Attributes & COMMON_LVB_TRAILING_BYTE) continue;
			strRow += pScreenBuffer[x].Char.UnicodeChar;
		}

		// handle trim/wrap settings
		if (i == coordStart.Y)
		{
			// first row
			if
			(
				(coordStart.Y == coordEnd.Y)
				||
				(
					m_consoleCopyInfo->bNoWrap
					&& 
					(coordStart.Y < coordEnd.Y)
					&&
					(strRow[strRow.length() - 1] != L' ')
				)
			)
			{
				bWrap = false;
			}
		}
		else if (i == coordEnd.Y)
		{
			// last row
			if (strRow.length() < static_cast<size_t>(coordBufferSize.X))
			{
				bWrap = false;
			}
		}
		else
		{
			// rows in between
			if (m_consoleCopyInfo->bNoWrap && (strRow[strRow.length() - 1] != L' '))
			{
				bWrap = false;
			}
		}

		if (m_consoleCopyInfo->bTrimSpaces) trim_right(strRow);
		if (bWrap)
		{
			switch(m_consoleCopyInfo->copyNewlineChar)
			{
				case newlineCRLF: strRow += wstring(L"\r\n"); break;
				case newlineLF	: strRow += wstring(L"\n"); break;
				default			: strRow += wstring(L"\r\n"); break;
			}
		}

		strText += strRow;
	}

	::EmptyClipboard();

	HGLOBAL hText = ::GlobalAlloc(GMEM_MOVEABLE, (strText.length()+1)*sizeof(wchar_t));

	if (hText == NULL)
	{ 
		::CloseClipboard();
		return;
	} 

	::CopyMemory(static_cast<wchar_t*>(::GlobalLock(hText)), strText.c_str(), (strText.length()+1)*sizeof(wchar_t));

	::GlobalUnlock(hText);

	if (::SetClipboardData(CF_UNICODETEXT, hText) == NULL)
	{
		// we need to global-free data only if copying failed
		::GlobalFree(hText);
	}
	::CloseClipboard();
	// !!! No call to GlobalFree here. Next app that uses clipboard will call EmptyClipboard to free the data
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SendConsoleText(HANDLE hStdIn, const shared_ptr<wchar_t>& textBuffer)
{
	wchar_t*	pszText	= textBuffer.get();
	size_t		textLen = wcslen(pszText);
	size_t		partLen	= 512;
	size_t		parts	= textLen/partLen;
	size_t		offset	= 0;

	for (size_t part = 0; part < parts+1; ++part)
	{
		size_t	keyEventCount = 0;
		
		if (part == parts)
		{
			// last part, modify part size
			partLen = textLen - parts*partLen;
		}

		scoped_array<INPUT_RECORD> pKeyEvents(new INPUT_RECORD[partLen]);
		::ZeroMemory(pKeyEvents.get(), sizeof(INPUT_RECORD)*partLen);

		for (size_t i = 0; (i < partLen) && (offset < textLen); ++i, ++offset, ++keyEventCount)
		{
			if ((pszText[offset] == L'\r') || (pszText[offset] == L'\n'))
			{
				if ((pszText[offset] == L'\r') && (pszText[offset+1] == L'\n')) ++offset;

				if (keyEventCount > 0)
				{
					DWORD dwTextWritten = 0;
					::WriteConsoleInput(hStdIn, pKeyEvents.get(), static_cast<DWORD>(keyEventCount), &dwTextWritten);
				}

				::PostMessage(m_consoleParams->hwndConsoleWindow, WM_KEYDOWN, VK_RETURN, 0x001C0001);
				::PostMessage(m_consoleParams->hwndConsoleWindow, WM_KEYUP, VK_RETURN, 0xC01C0001);

				keyEventCount = static_cast<size_t>(-1);
				partLen -= i;
				i = static_cast<size_t>(-1);
			}
			else
			{
				pKeyEvents[i].EventType							= KEY_EVENT;
				pKeyEvents[i].Event.KeyEvent.bKeyDown			= TRUE;
				pKeyEvents[i].Event.KeyEvent.wRepeatCount		= 1;
				pKeyEvents[i].Event.KeyEvent.wVirtualKeyCode	= LOBYTE(::VkKeyScan(pszText[offset]));
				pKeyEvents[i].Event.KeyEvent.wVirtualScanCode	= 0;
				pKeyEvents[i].Event.KeyEvent.uChar.UnicodeChar	= pszText[offset];
				pKeyEvents[i].Event.KeyEvent.dwControlKeyState	= 0;
			}
		}

		if (keyEventCount > 0)
		{
			DWORD dwTextWritten = 0;
			::WriteConsoleInput(hStdIn, pKeyEvents.get(), static_cast<DWORD>(keyEventCount), &dwTextWritten);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SetResetKeyInput(scoped_array<INPUT>& kbdInputs, WORD wVk, short& sCount)
{
	if ((::GetAsyncKeyState(wVk) & 0x8000) != 0)
	{
		kbdInputs[sCount].type			= INPUT_KEYBOARD;
		kbdInputs[sCount].ki.wVk		= wVk;
		kbdInputs[sCount].ki.dwFlags	= KEYEVENTF_KEYUP;
		++sCount;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SendMouseEvent(HANDLE hStdIn)
{
	DWORD	dwEvents = 0;

	INPUT_RECORD mouseEvent;
	::ZeroMemory(&mouseEvent, sizeof(INPUT_RECORD));

	mouseEvent.EventType	= MOUSE_EVENT;

	::CopyMemory(&mouseEvent.Event.MouseEvent, m_consoleMouseEvent.Get(), sizeof(MOUSE_EVENT_RECORD));
	::WriteConsoleInput(hStdIn, &mouseEvent, 1, &dwEvents);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::ScrollConsole(HANDLE hStdOut, int nXDelta, int nYDelta)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	::GetConsoleScreenBufferInfo(hStdOut, &csbi);

	int nXCurrentPos = csbi.srWindow.Right - m_consoleParams->dwColumns + 1;
	int nYCurrentPos = csbi.srWindow.Bottom - m_consoleParams->dwRows + 1;

	// limit deltas
	nXDelta = max(-nXCurrentPos, min(nXDelta, (int)(m_consoleParams->dwBufferColumns-m_consoleParams->dwColumns) - nXCurrentPos));
	nYDelta = max(-nYCurrentPos, min(nYDelta, (int)(m_consoleParams->dwBufferRows-m_consoleParams->dwRows) - nYCurrentPos));

	SMALL_RECT sr;
	sr.Top		= static_cast<SHORT>(nYDelta);
	sr.Bottom	= static_cast<SHORT>(nYDelta);
	sr.Left		= static_cast<SHORT>(nXDelta);
	sr.Right	= static_cast<SHORT>(nXDelta);

	::SetConsoleWindowInfo(hStdOut, FALSE, &sr);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleHandler::SetConsoleParams(DWORD dwHookThreadId, HANDLE hStdOut)
{
	// get max console size
	COORD		coordMaxSize;
	coordMaxSize = ::GetLargestConsoleWindowSize(hStdOut);

	m_consoleParams->dwMaxRows		= coordMaxSize.Y;
	m_consoleParams->dwMaxColumns	= coordMaxSize.X;

	// check rows and columns
	if (m_consoleParams->dwRows > static_cast<DWORD>(coordMaxSize.Y)) m_consoleParams->dwRows = coordMaxSize.Y;
	if (m_consoleParams->dwColumns > static_cast<DWORD>(coordMaxSize.X)) m_consoleParams->dwColumns = coordMaxSize.X;

	// buffer rows cannot be less than max console size
//	if (m_consoleParams->dwBufferRows < static_cast<DWORD>(coordMaxSize.Y)) m_consoleParams->dwBufferRows = coordMaxSize.Y;
//	if (m_consoleParams->dwBufferColumns < static_cast<DWORD>(coordMaxSize.X)) m_consoleParams->dwBufferColumns = coordMaxSize.X;

	if ((m_consoleParams->dwBufferRows != 0) && (m_consoleParams->dwMaxRows > m_consoleParams->dwBufferRows)) m_consoleParams->dwMaxRows = m_consoleParams->dwBufferRows;
	if ((m_consoleParams->dwBufferColumns != 0) && (m_consoleParams->dwMaxColumns > m_consoleParams->dwBufferColumns)) m_consoleParams->dwMaxColumns = m_consoleParams->dwBufferColumns;

	// set console window handle and hook monitor thread id
	m_consoleParams->hwndConsoleWindow	= ::GetConsoleWindow();
	m_consoleParams->dwHookThreadId		= dwHookThreadId;

	TRACE(L"Max columns: %i, max rows: %i\n", m_consoleParams->dwMaxColumns, m_consoleParams->dwMaxRows);

	// get initial window and cursor info
	::GetConsoleScreenBufferInfo(hStdOut, &m_consoleInfo->csbi);
	::GetConsoleCursorInfo(hStdOut, m_cursorInfo.Get());

	m_consoleParams.SetReqEvent();
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
	TRACE(L"Hook!\n");

	// TODO: error handling
	// open shared objects (shared memory, events, etc)
	if (!OpenSharedObjects()) return 0;
	
	HANDLE hStdOut = ::CreateFile(
						L"CONOUT$",
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,
						0);

	HANDLE hStdIn = ::CreateFile(
						L"CONIN$",
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,
						0);

	SetConsoleParams(::GetCurrentThreadId(), hStdOut);

	if (::WaitForSingleObject(m_consoleParams.GetRespEvent(), 10000) == WAIT_TIMEOUT) return 0;

	ResizeConsoleWindow(hStdOut, m_consoleParams->dwColumns, m_consoleParams->dwRows, 0);

	// FIX: this seems to case problems on startup
//	ReadConsoleBuffer();

	shared_ptr<void> parentProcessWatchdog(::OpenMutex(SYNCHRONIZE, FALSE, (LPCTSTR)((SharedMemNames::formatWatchdog % m_consoleParams->dwParentProcessId).str().c_str())), ::CloseHandle);
	TRACE(L"Watchdog handle: 0x%08X\n", parentProcessWatchdog.get());

	HANDLE	arrWaitHandles[] =
	{
		m_hMonitorThreadExit.get(), 
		m_consoleCopyInfo.GetReqEvent(), 
		m_consoleTextInfo.GetReqEvent(), 
		m_newScrollPos.GetReqEvent(),
		m_consoleMouseEvent.GetReqEvent(), 
		m_newConsoleSize.GetReqEvent(),
		hStdOut,
	};

	DWORD	dwWaitRes		= 0;

	while ((dwWaitRes = ::WaitForMultipleObjects(
							sizeof(arrWaitHandles)/sizeof(arrWaitHandles[0]),
							arrWaitHandles, 
							FALSE, 
							m_consoleParams->dwRefreshInterval)) != WAIT_OBJECT_0)
	{
		if ((parentProcessWatchdog.get() != NULL) && (::WaitForSingleObject(parentProcessWatchdog.get(), 0) == WAIT_ABANDONED))
		{
			TRACE(L"Watchdog 0x%08X died. Time to exit", parentProcessWatchdog.get());
			::SendMessage(m_consoleParams->hwndConsoleWindow, WM_CLOSE, 0, 0);
			break;
		}

		switch (dwWaitRes)
		{
			// copy request
			case WAIT_OBJECT_0 + 1 :
			{
				SharedMemoryLock memLock(m_consoleCopyInfo);

				CopyConsoleText();
				m_consoleCopyInfo.SetRespEvent();
				break;
			}

			// send text request
			case WAIT_OBJECT_0 + 2 :
			{
				SharedMemoryLock memLock(m_consoleTextInfo);

				shared_ptr<wchar_t>	textBuffer;
				
				if (m_consoleTextInfo->mem != NULL)
				{
					textBuffer.reset(
									reinterpret_cast<wchar_t*>(m_consoleTextInfo->mem),
									bind<BOOL>(::VirtualFreeEx, ::GetCurrentProcess(), _1, NULL, MEM_RELEASE));
				}

				SendConsoleText(hStdIn, textBuffer);
				m_consoleTextInfo.SetRespEvent();
				break;
			}

			// console scroll request
			case WAIT_OBJECT_0 + 3 :
			{
				SharedMemoryLock memLock(m_newScrollPos);

				ScrollConsole(hStdOut, m_newScrollPos->cx, m_newScrollPos->cy);
				ReadConsoleBuffer();
				break;
			}

			// mouse event request
			case WAIT_OBJECT_0 + 4 :
			{
				SharedMemoryLock memLock(m_consoleMouseEvent);

				SendMouseEvent(hStdIn);
				m_consoleMouseEvent.SetRespEvent();
				break;
			}

			// console resize request
			case WAIT_OBJECT_0 + 5 :
			{
				SharedMemoryLock memLock(m_newConsoleSize);

				ResizeConsoleWindow(hStdOut, m_newConsoleSize->dwColumns, m_newConsoleSize->dwRows, m_newConsoleSize->dwResizeWindowEdge);
				ReadConsoleBuffer();
				break;
			}

			case WAIT_OBJECT_0 + 6 :
				// something changed in the console
				// this has to be the last event, since it's the most 
				// frequent one
				::Sleep(m_consoleParams->dwNotificationTimeout);
			case WAIT_TIMEOUT :
			{
				// refresh timer
				ReadConsoleBuffer();
				break;
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


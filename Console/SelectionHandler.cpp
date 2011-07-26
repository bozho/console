#include "stdAfx.h"

#include "Console.h"
#include "SelectionHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

SelectionHandler::SelectionHandler(
					const CWindow& consoleView, 
					const CDC& dcConsoleView, 
					const CRect& rectConsoleView, 
					ConsoleHandler& consoleHandler,
					SharedMemory<ConsoleParams>& consoleParams, 
					SharedMemory<ConsoleInfo>& consoleInfo, 
					SharedMemory<ConsoleCopy>& consoleCopyInfo, 
					int nCharWidth, 
					int nCharHeight)
: m_consoleView(consoleView)
, m_dcSelection(::CreateCompatibleDC(NULL))
//, m_bmpSelection(::CreateCompatibleBitmap(dcConsoleView, rectConsoleView.Width(), rectConsoleView.Height()))
, m_bmpSelection(NULL)
, m_rectConsoleView(rectConsoleView)
, m_nCharWidth(nCharWidth)
, m_consoleHandler(consoleHandler)
, m_consoleParams(consoleParams)
, m_consoleInfo(consoleInfo)
, m_consoleCopyInfo(consoleCopyInfo)
, m_nCharHeight(nCharHeight)
, m_paintBrush()
, m_backgroundBrush(::CreateSolidBrush(RGB(0, 0, 0)))
, m_selectionState(selstateNoSelection)
, m_coordInitial()
, m_coordCurrent()
/*
, (m_consoleParams->dwColumns - 1)(0)
, (m_consoleParams->dwRows - 1)(0)
*/
{
	Helpers::CreateBitmap(dcConsoleView, rectConsoleView.Width(), rectConsoleView.Height(), m_bmpSelection);
	m_dcSelection.SelectBitmap(m_bmpSelection);
	m_dcSelection.SetBkColor(RGB(0, 0, 0));
}

SelectionHandler::~SelectionHandler()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::SelectWord(const COORD& coordInit, COLORREF crSelectionColor, shared_array<CharInfo> screenBuffer)
{
	if (m_selectionState > selstateNoSelection) return;

	// stop console scrolling while selecting
	m_consoleHandler.StopScrolling();

	if (!m_paintBrush.IsNull()) m_paintBrush.DeleteObject();
	m_paintBrush.CreateSolidBrush(crSelectionColor);

	m_consoleView.SetCapture();

	m_coordInitial		= coordInit;

	m_coordCurrent.X	= m_coordInitial.X;
	m_coordCurrent.Y	= m_coordInitial.Y;

	SMALL_RECT&	 srWindow = m_consoleInfo->csbi.srWindow;

	int nDeltaX = m_coordCurrent.X - srWindow.Left;
	int nDeltaY = m_coordCurrent.Y - srWindow.Top;

	if (nDeltaX < 0) nDeltaX = 0;
	if (nDeltaY < 0) nDeltaY = 0;

	int nStartSel = nDeltaY * m_consoleParams->dwColumns + nDeltaX - 1;
	while(nStartSel >= 0)
	{
		if (screenBuffer[nStartSel].charInfo.Char.AsciiChar == ' ')
			break;

		--nStartSel;
	}

	++nStartSel;
	m_coordInitial.X = short(nStartSel % m_consoleParams->dwColumns + srWindow.Left);
	m_coordInitial.Y = short(nStartSel / m_consoleParams->dwColumns + srWindow.Top);

	DWORD nEndSel = nDeltaY * m_consoleParams->dwColumns + nDeltaX;
	while (nEndSel < m_consoleParams->dwColumns * m_consoleParams->dwRows)
	{
		if (screenBuffer[nEndSel].charInfo.Char.AsciiChar == ' ')
			break;

		++nEndSel;
	}

	// --nEndSel;
	m_coordCurrent.X = short(nEndSel % m_consoleParams->dwColumns + srWindow.Left);
	m_coordCurrent.Y = short(nEndSel / m_consoleParams->dwColumns + srWindow.Top);

	m_selectionState	= selstateSelectWord;

	UpdateSelection();
}

//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::StartSelection(const COORD& coordInit, COLORREF crSelectionColor, shared_array<CharInfo> screenBuffer)
{
	if (m_selectionState > selstateNoSelection) return;

	// stop console scrolling while selecting
	m_consoleHandler.StopScrolling();

	if (!m_paintBrush.IsNull()) m_paintBrush.DeleteObject();
	m_paintBrush.CreateSolidBrush(crSelectionColor);

	m_consoleView.SetCapture();

	m_coordInitial		= coordInit;

	TRACE(L"Start sel. X: %i Y: %i\n", m_coordInitial.X, m_coordInitial.Y);

	m_coordCurrent.X	= m_coordInitial.X;
	m_coordCurrent.Y	= m_coordInitial.Y;

	SMALL_RECT&	 srWindow = m_consoleInfo->csbi.srWindow;

	int nDeltaX = m_coordCurrent.X - srWindow.Left;
	int nDeltaY = m_coordCurrent.Y - srWindow.Top;

	if (nDeltaX < 0) nDeltaX = 0;
	if (nDeltaY < 0) nDeltaY = 0;

 	if (screenBuffer[nDeltaY * m_consoleParams->dwColumns + nDeltaX].charInfo.Attributes & COMMON_LVB_LEADING_BYTE)
 	{
		++m_coordCurrent.X;
	}

	m_selectionState	= selstateStartedSelecting;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::UpdateSelection(const COORD& coordCurrent, shared_array<CharInfo> screenBuffer)
{
	if ((m_selectionState != selstateStartedSelecting) &&
		(m_selectionState != selstateSelecting))
	{
		return;
	}

	if ((coordCurrent.X == m_coordCurrent.X) && (coordCurrent.Y == m_coordCurrent.Y)) return;

//	TRACE(L"Update selection current: %ix%i\n", coordCurrent.X, coordCurrent.Y);
	m_coordCurrent = coordCurrent;


	SMALL_RECT&	 srWindow = m_consoleInfo->csbi.srWindow;

	int nDeltaX = m_coordCurrent.X - srWindow.Left;
	int nDeltaY = m_coordCurrent.Y - srWindow.Top;

	if (nDeltaX < 0) nDeltaX = 0;
	if (nDeltaY < 0) nDeltaY = 0;

 	if (screenBuffer[nDeltaY * m_consoleParams->dwColumns + nDeltaX].charInfo.Attributes & COMMON_LVB_LEADING_BYTE)
	{
		++m_coordCurrent.X;
	}

	UpdateSelection();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::UpdateSelection()
{
	if (m_selectionState < selstateStartedSelecting) return;

	m_dcSelection.FillRect(&m_rectConsoleView, m_backgroundBrush);

	COORD	coordStart;
	COORD	coordEnd;

	GetSelectionCoordinates(coordStart, coordEnd);

	if ((m_selectionState == selstateStartedSelecting) && 
		(coordStart.X == coordEnd.X) &&
		(coordStart.Y == coordEnd.Y))
	{
		// the cursor didn't move enough yet
		return;
	}

	if (m_selectionState < selstateSelecting) m_selectionState = selstateSelecting;

/*
	TRACE(L"Member coord: %ix%i - %ix%i\n", m_coordInitial.X, m_coordInitial.Y, m_coordCurrent.X, m_coordCurrent.Y);
	TRACE(L"Sel coord: %ix%i - %ix%i\n", coordStart.X, coordStart.Y, coordEnd.X, coordEnd.Y);
*/

	CRect	fillRect;
	COORD	fillStart;
	COORD	fillEnd;
	SHORT	maxX = (m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns - 1) : static_cast<SHORT>(m_consoleParams->dwColumns - 1);

	// paint the first row rect
	fillStart.X = coordStart.X;
	fillStart.Y = coordStart.Y;

	//                                multi-row select  single row select
	fillEnd.X = (coordStart.Y < coordEnd.Y) ? maxX : coordEnd.X;
	fillEnd.Y = coordStart.Y;

	GetFillRect(fillStart, fillEnd, fillRect);
	m_dcSelection.FillRect(&fillRect, m_paintBrush);


	// paint the rows in between
	if (coordStart.Y < coordEnd.Y - 1)
	{
		fillStart.X = 0;
		fillStart.Y = coordStart.Y + 1;

		fillEnd.X = maxX;
		fillEnd.Y = coordEnd.Y - 1;

		GetFillRect(fillStart, fillEnd, fillRect);
		m_dcSelection.FillRect(&fillRect, m_paintBrush);
	}

	// paint the last row
	if (coordStart.Y < coordEnd.Y)
	{
		fillStart.X = 0;
		fillStart.Y = coordEnd.Y;

		fillEnd.X = coordEnd.X;
		fillEnd.Y = coordEnd.Y;

		GetFillRect(fillStart, fillEnd, fillRect);
//		TRACE(L"fill rect: (%ix%i) - (%ix%i)\n", fillRect.left, fillRect.top, fillRect.right, fillRect.bottom);
		m_dcSelection.FillRect(&fillRect, m_paintBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool SelectionHandler::CopySelection(const COORD& coordCurrent)
{
	if (m_selectionState < selstateSelecting) return false;

	bool	bCopy = false;
	COORD	coordStart;
	COORD	coordEnd;

	GetSelectionCoordinates(coordStart, coordEnd);

	// verbose tests, just to make things a bit easier to follow :-)
	if (coordStart.Y == coordEnd.Y)
	{
		// single line selected, click must be inside the selection rectangle
		if ((coordCurrent.Y == coordStart.Y) && (coordCurrent.X >= coordStart.X) && (coordCurrent.X <= coordEnd.X))
		{
			bCopy = true;
		}

		// multiple lines selected
	}
	else if ( ((coordCurrent.Y == coordStart.Y) && (coordCurrent.X >= coordStart.X)) ||	// first line
		((coordCurrent.Y > coordStart.Y)  && (coordCurrent.Y < coordEnd.Y))    ||	// lines between the first and the last lines
		((coordCurrent.Y == coordEnd.Y)   && (coordCurrent.X <= coordEnd.X)))		// last line
	{
		bCopy = true;
	}

	if (bCopy) CopySelection();

	return bCopy;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::CopySelection()
{
	if (m_selectionState < selstateSelecting) return;

	{
		SharedMemoryLock	memLock(m_consoleCopyInfo);
		
		GetSelectionCoordinates(m_consoleCopyInfo->coordStart, m_consoleCopyInfo->coordEnd);

		m_consoleCopyInfo->bNoWrap		= g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bNoWrap;
		m_consoleCopyInfo->bTrimSpaces	= g_settingsHandler->GetBehaviorSettings().copyPasteSettings.bTrimSpaces;
		m_consoleCopyInfo->copyNewlineChar= g_settingsHandler->GetBehaviorSettings().copyPasteSettings.copyNewlineChar;

		m_consoleCopyInfo.SetReqEvent();
	}

	::WaitForSingleObject(m_consoleCopyInfo.GetRespEvent(), INFINITE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::EndSelection()
{
	m_selectionState = selstateSelected;
	::ReleaseCapture();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::ClearSelection()
{
	m_coordCurrent.X= 0;
	m_coordCurrent.Y= 0;

	m_coordInitial.X= 0;
	m_coordInitial.Y= 0;

	m_selectionState = selstateNoSelection;

	m_dcSelection.FillRect(&m_rectConsoleView, m_backgroundBrush);

	m_consoleHandler.ResumeScrolling();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::BitBlt(CDC& offscreenDC)
{
	if (m_selectionState == selstateNoSelection) return;

	COORD	coordStart;
	COORD	coordEnd;
	SHORT	maxX = (m_consoleParams->dwBufferColumns > 0) ? static_cast<SHORT>(m_consoleParams->dwBufferColumns - 1) : static_cast<SHORT>(m_consoleParams->dwColumns - 1);

	GetSelectionCoordinates(coordStart, coordEnd);
	coordStart.X	= 0;
	coordEnd.X		= maxX;

	CRect	selectionRect;
	GetFillRect(coordStart, coordEnd, selectionRect);

	offscreenDC.BitBlt(
					selectionRect.left, 
					selectionRect.top, 
					selectionRect.Width(), 
					selectionRect.Height(), 
					m_dcSelection, 
					selectionRect.left, 
					selectionRect.top, 
					SRCINVERT);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::GetSelectionCoordinates(COORD& coordStart, COORD& coordEnd)
{
	if (m_coordInitial.Y == m_coordCurrent.Y)
	{
		if (m_coordInitial.X <= m_coordCurrent.X)
		{
			::CopyMemory(&coordStart, &m_coordInitial, sizeof(COORD));
			::CopyMemory(&coordEnd, &m_coordCurrent, sizeof(COORD));
		}
		else
		{
			::CopyMemory(&coordStart, &m_coordCurrent, sizeof(COORD));
			::CopyMemory(&coordEnd, &m_coordInitial, sizeof(COORD));
		}
	}
	else if (m_coordInitial.Y < m_coordCurrent.Y)
	{
		::CopyMemory(&coordStart, &m_coordInitial, sizeof(COORD));
		::CopyMemory(&coordEnd, &m_coordCurrent, sizeof(COORD));
	}
	else
	{
		::CopyMemory(&coordStart, &m_coordCurrent, sizeof(COORD));
		::CopyMemory(&coordEnd, &m_coordInitial, sizeof(COORD));
	}
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void SelectionHandler::GetFillRect(const COORD& coordStart, const COORD& coordEnd, CRect& fillRect)
{
	StylesSettings&	stylesSettings	= g_settingsHandler->GetAppearanceSettings().stylesSettings;
	SMALL_RECT&		srWindow		= m_consoleInfo->csbi.srWindow;
	CRect			rectConsoleView;

	m_consoleView.GetClientRect(&rectConsoleView);

	fillRect.left	= (coordStart.X - srWindow.Left) * m_nCharWidth + static_cast<LONG>(stylesSettings.dwInsideBorder);
	fillRect.top	= (coordStart.Y - srWindow.Top) * m_nCharHeight + static_cast<LONG>(stylesSettings.dwInsideBorder);

	fillRect.right	= (coordEnd.X - srWindow.Left + 1) * m_nCharWidth + static_cast<LONG>(stylesSettings.dwInsideBorder);
	fillRect.bottom	= (coordEnd.Y - srWindow.Top + 1) * m_nCharHeight + static_cast<LONG>(stylesSettings.dwInsideBorder);

	if (fillRect.left < static_cast<LONG>(stylesSettings.dwInsideBorder)) fillRect.left = stylesSettings.dwInsideBorder;
	if (fillRect.top < static_cast<LONG>(stylesSettings.dwInsideBorder)) fillRect.top = stylesSettings.dwInsideBorder;

	if (fillRect.left > (rectConsoleView.right - static_cast<LONG>(stylesSettings.dwInsideBorder))) fillRect.right = rectConsoleView.right - static_cast<LONG>(stylesSettings.dwInsideBorder);
	if (fillRect.top > (rectConsoleView.bottom - static_cast<LONG>(stylesSettings.dwInsideBorder))) fillRect.bottom = rectConsoleView.bottom - static_cast<LONG>(stylesSettings.dwInsideBorder);

	if (fillRect.right < static_cast<LONG>(stylesSettings.dwInsideBorder)) fillRect.right = stylesSettings.dwInsideBorder;
	if (fillRect.bottom < static_cast<LONG>(stylesSettings.dwInsideBorder)) fillRect.bottom = stylesSettings.dwInsideBorder;

	if (fillRect.right > (rectConsoleView.right - static_cast<LONG>(stylesSettings.dwInsideBorder))) fillRect.right = rectConsoleView.right - static_cast<LONG>(stylesSettings.dwInsideBorder);
	if (fillRect.bottom > (rectConsoleView.bottom - static_cast<LONG>(stylesSettings.dwInsideBorder))) fillRect.bottom = rectConsoleView.bottom - static_cast<LONG>(stylesSettings.dwInsideBorder);
}

/////////////////////////////////////////////////////////////////////////////

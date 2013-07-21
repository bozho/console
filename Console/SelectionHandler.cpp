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
#ifndef _USE_AERO
					const CDC& dcConsoleView, 
					const CRect& rectConsoleView, 
#endif //_USE_AERO
					ConsoleHandler& consoleHandler,
					SharedMemory<ConsoleParams>& consoleParams, 
					SharedMemory<ConsoleInfo>& consoleInfo, 
					SharedMemory<ConsoleCopy>& consoleCopyInfo, 
					int nCharWidth, 
					int nCharHeight,
					int nVInsideBorder,
					int nHInsideBorder,
					std::shared_ptr<TabData> tabData)
: m_consoleView(consoleView)
#ifndef _USE_AERO
, m_dcSelection(::CreateCompatibleDC(NULL))
, m_bmpSelection(NULL)
, m_rectConsoleView(rectConsoleView)
, m_paintBrush(::CreateSolidBrush(g_settingsHandler->GetAppearanceSettings().stylesSettings.crSelectionColor))
, m_backgroundBrush(::CreateSolidBrush(RGB(0, 0, 0)))
#endif //_USE_AERO
, m_nCharWidth(nCharWidth)
, m_consoleHandler(consoleHandler)
, m_consoleParams(consoleParams)
, m_consoleInfo(consoleInfo)
, m_consoleCopyInfo(consoleCopyInfo)
, m_nCharHeight(nCharHeight)
, m_nVInsideBorder(nVInsideBorder)
, m_nHInsideBorder(nHInsideBorder)
, m_selectionState(selstateNoSelection)
, m_coordInitial()
, m_coordCurrent()
, m_tabData(tabData)
{
#ifndef _USE_AERO
	Helpers::CreateBitmap(dcConsoleView, rectConsoleView.Width(), rectConsoleView.Height(), m_bmpSelection);
	m_dcSelection.SelectBitmap(m_bmpSelection);
	m_dcSelection.SetBkColor(RGB(0, 0, 0));
#endif //_USE_AERO
}

SelectionHandler::~SelectionHandler()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::SelectWord(const COORD& coordInit, CharInfo screenBuffer [])
{
  if (m_selectionState > selstateNoSelection) return;

  // stop console scrolling while selecting
  m_consoleHandler.StopScrolling();

  m_consoleView.SetCapture();

  m_coordInitial    = coordInit;

  m_coordCurrent.X  = m_coordInitial.X;
  m_coordCurrent.Y  = m_coordInitial.Y;

  SMALL_RECT&	 srWindow = m_consoleInfo->csbi.srWindow;

  int nDeltaX = m_coordCurrent.X - srWindow.Left;
  int nDeltaY = m_coordCurrent.Y - srWindow.Top;

  if (nDeltaX < 0) nDeltaX = 0;
  if (nDeltaY < 0) nDeltaY = 0;

  CopyPasteSettings& copyPasteSettings = g_settingsHandler->GetBehaviorSettings().copyPasteSettings;

  int nStartSel = nDeltaY * m_consoleParams->dwColumns + nDeltaX - 1;
  while(nStartSel >= 0)
  {
    if( copyPasteSettings.strLeftDelimiters.find(screenBuffer[nStartSel].charInfo.Char.UnicodeChar) != std::wstring::npos )
    {
      if( !copyPasteSettings.bIncludeLeftDelimiter )
        ++nStartSel;
      break;
    }

    --nStartSel;
  }
  if( nStartSel < 0 ) nStartSel = 0;

  m_coordInitial.X = short(nStartSel % m_consoleParams->dwColumns + srWindow.Left);
  m_coordInitial.Y = short(nStartSel / m_consoleParams->dwColumns + srWindow.Top);

  DWORD nEndSel = nDeltaY * m_consoleParams->dwColumns + nDeltaX;
  while (nEndSel < m_consoleParams->dwColumns * m_consoleParams->dwRows)
  {
    if( copyPasteSettings.strRightDelimiters.find(screenBuffer[nEndSel].charInfo.Char.UnicodeChar) != std::wstring::npos )
    {
      if( !copyPasteSettings.bIncludeRightDelimiter )
        --nEndSel;
      break;
    }

    ++nEndSel;
  }
  if( nEndSel >= m_consoleParams->dwColumns * m_consoleParams->dwRows ) nEndSel = m_consoleParams->dwColumns * m_consoleParams->dwRows - 1;

  m_coordCurrent.X = short(nEndSel % m_consoleParams->dwColumns + srWindow.Left);
  m_coordCurrent.Y = short(nEndSel / m_consoleParams->dwColumns + srWindow.Top);

  m_selectionState = selstateSelectWord;

  UpdateSelection();
}

//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::StartSelection(const COORD& coordInit, CharInfo screenBuffer [])
{
	if (m_selectionState > selstateNoSelection) return;

	// stop console scrolling while selecting
	m_consoleHandler.StopScrolling();

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

	m_coordInitialXLeading = m_coordInitialXTrailing = m_coordInitial.X;
	WORD wAttributes = screenBuffer[nDeltaY * m_consoleParams->dwColumns + nDeltaX].charInfo.Attributes;
	if (wAttributes & COMMON_LVB_LEADING_BYTE)
	{
		++m_coordInitial.X;
		m_coordInitialXTrailing = m_coordInitial.X;
	}
	else if (wAttributes & COMMON_LVB_TRAILING_BYTE)
	{
		--m_coordInitial.X;
		m_coordInitialXLeading = m_coordInitial.X;
	}

	m_selectionState	= selstateStartedSelecting;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::UpdateSelection(const COORD& coordCurrent, CharInfo screenBuffer [])
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

	if( (m_coordCurrent.Y * m_consoleParams->dwBufferColumns + m_coordCurrent.X) >
		  (m_coordInitial.Y * m_consoleParams->dwBufferColumns + m_coordInitial.X) )
	{
		// selecting from top left to bottom right
		if (screenBuffer[nDeltaY * m_consoleParams->dwColumns + nDeltaX].charInfo.Attributes & COMMON_LVB_LEADING_BYTE)
			++m_coordCurrent.X;
		m_coordInitial.X = m_coordInitialXLeading;
	}
	else
	{
		// selecting from bottom right to top left
		if (screenBuffer[nDeltaY * m_consoleParams->dwColumns + nDeltaX].charInfo.Attributes & COMMON_LVB_TRAILING_BYTE)
			--m_coordCurrent.X;
		m_coordInitial.X = m_coordInitialXTrailing;
	}

	UpdateSelection();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::UpdateSelection()
{
	if (m_selectionState < selstateStartedSelecting) return;

#ifndef _USE_AERO
	m_dcSelection.FillRect(&m_rectConsoleView, m_backgroundBrush);
#endif //_USE_AERO

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

#ifndef _USE_AERO
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
#endif //_USE_AERO
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::SelectAll(void)
{
  m_coordInitial.X = 0;
  m_coordInitial.Y = 0;
  m_coordCurrent.X = m_consoleInfo->csbi.dwSize.X - 1;
  m_coordCurrent.Y = m_consoleInfo->csbi.dwSize.Y - 1;
  m_selectionState = selstateSelected;

  UpdateSelection();
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
    _snprintf_s(
      m_consoleCopyInfo->szFontName, sizeof(m_consoleCopyInfo->szFontName),
      _TRUNCATE,
      "%ws",
      g_settingsHandler->GetAppearanceSettings().fontSettings.strName.c_str());
    m_consoleCopyInfo->bBold = g_settingsHandler->GetAppearanceSettings().fontSettings.bBold;
    m_consoleCopyInfo->bItalic = g_settingsHandler->GetAppearanceSettings().fontSettings.bItalic;
    m_consoleCopyInfo->dwSize  = g_settingsHandler->GetAppearanceSettings().fontSettings.dwSize * 2;
    ::CopyMemory(m_consoleCopyInfo->consoleColors, m_tabData->consoleColors, sizeof(m_consoleCopyInfo->consoleColors));

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

#ifndef _USE_AERO
	m_dcSelection.FillRect(&m_rectConsoleView, m_backgroundBrush);
#endif //_USE_AERO

	m_consoleHandler.ResumeScrolling();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
#ifdef _USE_AERO

void SelectionHandler::Draw(CDC& offscreenDC)
{
  if (m_selectionState == selstateNoSelection) return;

  COORD coordStart;
  COORD coordEnd;
  SHORT maxX = (m_consoleParams->dwBufferColumns > 0) ?
    static_cast<SHORT>(m_consoleParams->dwBufferColumns - 1) :
    static_cast<SHORT>(m_consoleParams->dwColumns - 1);

  GetSelectionCoordinates(coordStart, coordEnd);

  SMALL_RECT& srWindow = m_consoleInfo->csbi.srWindow;

  if(   coordEnd.Y < srWindow.Top    ||
      coordStart.Y > srWindow.Bottom ) return;

  INT nXStart = (static_cast<INT>(coordStart.X) - static_cast<INT>(srWindow.Left)) * m_nCharWidth  + m_nVInsideBorder;
  INT nYStart = (static_cast<INT>(coordStart.Y) - static_cast<INT>(srWindow.Top) ) * m_nCharHeight + m_nHInsideBorder;
  INT nXEnd   = (static_cast<INT>(  coordEnd.X) - static_cast<INT>(srWindow.Left)) * m_nCharWidth  + m_nVInsideBorder;
  INT nYEnd   = (static_cast<INT>(  coordEnd.Y) - static_cast<INT>(srWindow.Top) ) * m_nCharHeight + m_nHInsideBorder;
  INT nXmin   = (static_cast<INT>(0)            - static_cast<INT>(srWindow.Left)) * m_nCharWidth  + m_nVInsideBorder;
  INT nXmax   = (static_cast<INT>(maxX)         - static_cast<INT>(srWindow.Left)) * m_nCharWidth  + m_nVInsideBorder;

  Gdiplus::Graphics gr(offscreenDC);

  Gdiplus::Color selectionColor;
  selectionColor.SetFromCOLORREF(g_settingsHandler->GetAppearanceSettings().stylesSettings.crSelectionColor);
  Gdiplus::Pen        pen  (selectionColor);
  Gdiplus::SolidBrush brush(Gdiplus::Color(64,  selectionColor.GetR(), selectionColor.GetG(), selectionColor.GetB()));
  Gdiplus::GraphicsPath gp;

  if( nYStart == nYEnd )
  {
    Gdiplus::Rect rect(
      nXStart,
      nYStart,
      (nXEnd - nXStart) + m_nCharWidth,
      m_nCharHeight);
    gp.AddRectangle(rect);
  }
  else
  {
    /*
           2_________3
    0______|         |
    |      1     5___|
    |____________|   4
    7            6
    */

    Gdiplus::Point points[8];

    points[0].X = nXmin;
    points[0].Y = nYStart + m_nCharHeight;

    points[1].X = nXStart;
    points[1].Y = points[0].Y;

    points[2].X = points[1].X;
    points[2].Y = nYStart;

    points[3].X = nXmax + m_nCharWidth;
    points[3].Y = points[2].Y;

    points[4].X = points[3].X;
    points[4].Y = nYEnd;

    points[5].X = nXEnd + m_nCharWidth;
    points[5].Y = points[4].Y;

    points[6].X = points[5].X;
    points[6].Y = nYEnd + m_nCharHeight;

    points[7].X = points[0].X;
    points[7].Y = points[6].Y;

    gp.AddPolygon(points, 8);
  }

  gr.FillPath(&brush, &gp);
  gr.DrawPath(&pen, &gp);
}

#else //_USE_AERO

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

#endif //_USE_AERO

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

DWORD SelectionHandler::GetSelectionSize(void)
{
  if (m_selectionState < selstateSelecting) return 0;

  COORD	coordStart;
  COORD	coordEnd;

  GetSelectionCoordinates(coordStart, coordEnd);

  DWORD dwColumns = m_consoleParams->dwBufferColumns > 0 ? m_consoleParams->dwBufferColumns : m_consoleParams->dwColumns;
  return dwColumns * (coordEnd.Y - coordStart.Y) + (coordEnd.X - coordStart.X) + 1;
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

#ifndef _USE_AERO

void SelectionHandler::GetFillRect(const COORD& coordStart, const COORD& coordEnd, CRect& fillRect)
{
	SMALL_RECT&		srWindow		= m_consoleInfo->csbi.srWindow;
	CRect			rectConsoleView;

	m_consoleView.GetClientRect(&rectConsoleView);

	fillRect.left	= (coordStart.X - srWindow.Left) * m_nCharWidth + m_nVInsideBorder;
	fillRect.top	= (coordStart.Y - srWindow.Top) * m_nCharHeight + m_nHInsideBorder;

	fillRect.right	= (coordEnd.X - srWindow.Left + 1) * m_nCharWidth + m_nVInsideBorder;
	fillRect.bottom	= (coordEnd.Y - srWindow.Top + 1) * m_nCharHeight + m_nHInsideBorder;

	if (fillRect.left < m_nVInsideBorder) fillRect.left = m_nVInsideBorder;
	if (fillRect.top  < m_nHInsideBorder) fillRect.top  = m_nHInsideBorder;

	if (fillRect.left > (rectConsoleView.right  - m_nVInsideBorder)) fillRect.right  = rectConsoleView.right  - m_nVInsideBorder;
	if (fillRect.top  > (rectConsoleView.bottom - m_nHInsideBorder)) fillRect.bottom = rectConsoleView.bottom - m_nHInsideBorder;

	if (fillRect.right  < m_nVInsideBorder) fillRect.right  = m_nVInsideBorder;
	if (fillRect.bottom < m_nHInsideBorder) fillRect.bottom = m_nHInsideBorder;

	if (fillRect.right  > (rectConsoleView.right  - m_nVInsideBorder)) fillRect.right  = rectConsoleView.right  - m_nVInsideBorder;
	if (fillRect.bottom > (rectConsoleView.bottom - m_nHInsideBorder)) fillRect.bottom = rectConsoleView.bottom - m_nHInsideBorder;
}

#endif //_USE_AERO

/////////////////////////////////////////////////////////////////////////////

#include "stdAfx.h"
#include "SelectionHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

SelectionHandler::SelectionHandler(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectConsoleView, int nCharWidth, int nCharHeight, COLORREF crSelectionColor)
: m_consoleView(hwndConsoleView)
, m_dcSelection(::CreateCompatibleDC(NULL))
, m_bmpSelection(::CreateCompatibleBitmap(dcConsoleView, rectConsoleView.right - rectConsoleView.left, rectConsoleView.bottom - rectConsoleView.top))
, m_rectConsoleView(rectConsoleView)
, m_nCharWidth(nCharWidth)
, m_nCharHeight(nCharHeight)
, m_paintBrush(::CreateSolidBrush(crSelectionColor))
, m_backgroundBrush(::CreateSolidBrush(RGB(0, 0, 0)))
, m_selectionState(selstateNoSelection)
, m_coordInitial()
, m_coordCurrent()
, m_sXMax(0)
, m_sYMax(0)
{
	m_dcSelection.SelectBitmap(m_bmpSelection);
	m_dcSelection.SetBkColor(RGB(0, 0, 0));
}

SelectionHandler::~SelectionHandler() {
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::StartSelection(const CPoint& pointInitial, SHORT sXMax, SHORT sYMax) {

	if (m_selectionState > selstateNoSelection) return;

	m_consoleView.SetCapture();

	m_coordInitial.X= static_cast<SHORT>(pointInitial.x / m_nCharWidth);
	m_coordInitial.Y= static_cast<SHORT>(pointInitial.y / m_nCharHeight);
	m_coordCurrent.X= m_coordInitial.X;
	m_coordCurrent.Y= m_coordInitial.Y;

	m_sXMax			= sXMax;
	m_sYMax			= sYMax;

	CRect rect;
	rect.left	= m_coordCurrent.X * m_nCharWidth + 1;
	rect.top	= m_coordCurrent.Y * m_nCharHeight + 1;
	rect.right	= (m_coordCurrent.X + 1) * m_nCharWidth + 1;
	rect.bottom = (m_coordCurrent.Y + 1) * m_nCharHeight + 1;

	m_dcSelection.FillRect(&rect, m_paintBrush);

	m_selectionState = selstateSelecting;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::UpdateSelection(const CPoint& point) {

	if (m_selectionState != selstateSelecting) return;

	COORD	coordCurrent = { static_cast<SHORT>(point.x / m_nCharWidth), static_cast<SHORT>(point.y / m_nCharHeight) };

	if (coordCurrent.X > m_sXMax) coordCurrent.X = m_sXMax;
	if (coordCurrent.Y > m_sYMax) coordCurrent.Y = m_sYMax;

	if ((coordCurrent.X == m_coordCurrent.X) && (coordCurrent.Y == m_coordCurrent.Y)) return;

	m_dcSelection.FillRect(&m_rectConsoleView, m_backgroundBrush);

	m_coordCurrent.X = coordCurrent.X;
	m_coordCurrent.Y = coordCurrent.Y;

	COORD	coordStart;
	COORD	coordEnd;

	GetSelectionCoordinates(coordStart, coordEnd);

/*
	TRACE(L"Member coord: %ix%i - %ix%i\n", m_coordInitial.X, m_coordInitial.Y, m_coordCurrent.X, m_coordCurrent.Y);
	TRACE(L"Sel coord: %ix%i - %ix%i\n", coordStart.X, coordStart.Y, coordEnd.X, coordEnd.Y);
*/

	// paint the first row rect
	CRect rect;

	rect.left	= coordStart.X * m_nCharWidth + 1;
	rect.top	= coordStart.Y * m_nCharHeight + 1;

	rect.right	= (coordStart.Y < coordEnd.Y) ? (m_sXMax + 1) * m_nCharWidth + 1 : (coordEnd.X + 1) * m_nCharWidth + 1;
	rect.bottom	= (coordStart.Y + 1) * m_nCharHeight + 1;

//	TRACE(L"Sel update rect: %ix%i - %ix%i\n", rect.left, rect.top, rect.right, rect.bottom);
	m_dcSelection.FillRect(&rect, m_paintBrush);

	// paint the rows in between
	if (coordStart.Y < coordEnd.Y - 1) {

		rect.left	= 1;
		rect.top	= (coordStart.Y + 1) * m_nCharHeight + 1;

		rect.right	= (m_sXMax + 1) * m_nCharWidth + 1;
		rect.bottom	= coordEnd.Y * m_nCharHeight + 1;

		m_dcSelection.FillRect(&rect, m_paintBrush);
	}

	// paint the last row
	if (coordStart.Y < coordEnd.Y) {

		rect.left	= 1;
		rect.top	= (coordStart.Y + 1) * m_nCharHeight + 1;

		rect.right	= (coordEnd.X + 1) * m_nCharWidth + 1;
		rect.bottom	= (coordEnd.Y + 1) * m_nCharHeight + 1;

		m_dcSelection.FillRect(&rect, m_paintBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::CopySelection(const CPoint* pPoint, const SharedMemory<CHAR_INFO>& consoleBuffer) {

	if (m_selectionState == selstateNoSelection) return;

	bool	bCopy = false;
	COORD	coordStart;
	COORD	coordEnd;

	GetSelectionCoordinates(coordStart, coordEnd);

	if (pPoint != NULL) {
		
		COORD	coordCurrent = { static_cast<SHORT>(pPoint->x / m_nCharWidth), static_cast<SHORT>(pPoint->y / m_nCharHeight) };

		// verbose tests, just to make things a bit easier to follow :-)
		if (coordStart.Y == coordEnd.Y) {
			// single line selected, click must be inside the selection rectangle
			if ((coordCurrent.Y == coordStart.Y) && (coordCurrent.X >= coordStart.X) && (coordCurrent.X <= coordEnd.X)) {

				bCopy = true;
			}

			// multiple lines selected
		} else if ( ((coordCurrent.Y == coordStart.Y) && (coordCurrent.X >= coordStart.X)) ||	// first line
			((coordCurrent.Y > coordStart.Y)  && (coordCurrent.Y < coordEnd.Y))    ||	// lines between the first and the last lines
			((coordCurrent.Y == coordEnd.Y)   && (coordCurrent.X <= coordEnd.X))) {		// last line

				bCopy = true;
		}

	} else {
		bCopy = true;
	}

	if (bCopy) {

		if (!m_consoleView.OpenClipboard()) return;

		::EmptyClipboard();

		// we leave 2 spaces on the end of each line for a \r\n combination, m_sXMax is zero-basd, so we add one more there
		DWORD	dwMaxTextLen = (m_sXMax + 3) * (coordEnd.Y - coordStart.Y + 1) + 1;
		HGLOBAL hText = ::GlobalAlloc(GMEM_MOVEABLE, dwMaxTextLen*sizeof(wchar_t));

		if (hText == NULL) { 
			::CloseClipboard();
			return;
		} 

		wchar_t* pszText = static_cast<wchar_t*>(::GlobalLock(hText));

		SHORT	X		= 0;
		SHORT	Y		= coordStart.Y;
		DWORD	dwOffset= 0;

		// first row
		for (X = coordStart.X; X <= ((coordStart.Y < coordEnd.Y) ? m_sXMax : coordEnd.X); ++X) {
			pszText[dwOffset++] = consoleBuffer[Y * (m_sXMax+1) + X].Char.UnicodeChar;
		}

		if (coordStart.Y < coordEnd.Y) {
//			pszText[dwOffset++] = L'\r';
			pszText[dwOffset++] = L'\n';
		}

		// rows in between
		for (Y = coordStart.Y + 1; Y < coordEnd.Y; ++Y) {
			for (X = 0; X <= m_sXMax; ++X) {
				pszText[dwOffset++] = consoleBuffer[Y * (m_sXMax+1) + X].Char.UnicodeChar;
			}

//			pszText[dwOffset++] = L'\r';
			pszText[dwOffset++] = L'\n';
		}

		// last row
		if (coordEnd.Y > coordStart.Y) {
			Y = coordEnd.Y;

			for (X = 0; X <= coordEnd.X; ++X) {
				pszText[dwOffset++] = consoleBuffer[Y * (m_sXMax+1) + X].Char.UnicodeChar;
			}

			if (coordEnd.X == m_sXMax) {
//				pszText[dwOffset++] = L'\r';
				pszText[dwOffset++] = L'\n';
			}
		}

		pszText[dwOffset++] = L'\x0';

		::GlobalUnlock(hText);

		if (::SetClipboardData(CF_UNICODETEXT, hText) == NULL) {
			// we need to global-free data only if copying failed
			::GlobalFree(hText);
		}
		::CloseClipboard();
		// !!! No call to GlobalFree here. Next app that uses clipboard will call EmptyClipboard to free the data
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::EndSelection() {

	if (m_selectionState != selstateSelecting) return;

	m_selectionState = selstateSelected;
	::ReleaseCapture();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::ClearSelection() {

	m_coordCurrent.X= 0;
	m_coordCurrent.Y= 0;

	m_coordInitial.X= 0;
	m_coordInitial.Y= 0;

	m_sXMax			= 0;
	m_sYMax			= 0;

	m_selectionState = selstateNoSelection;

	m_dcSelection.FillRect(&m_rectConsoleView, m_backgroundBrush);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void SelectionHandler::BitBlt(CDC& offscreenDC) {

	if (m_selectionState == selstateNoSelection) return;

	COORD	coordStart;
	COORD	coordEnd;
	CRect	selectionRect;

	GetSelectionCoordinates(coordStart, coordEnd);

	selectionRect.left	= 1;
	selectionRect.top	= coordStart.Y * m_nCharHeight + 1;

	selectionRect.right	= (m_sXMax + 1) * m_nCharWidth + 1;
	selectionRect.bottom= (coordEnd.Y + 1) * m_nCharHeight + 1;

	offscreenDC.BitBlt(
					selectionRect.left, 
					selectionRect.top, 
					selectionRect.right - selectionRect.left, 
					selectionRect.bottom - selectionRect.top, 
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

void SelectionHandler::GetSelectionCoordinates(COORD& coordStart, COORD& coordEnd) {

	if (m_coordInitial.Y == m_coordCurrent.Y) {
		if (m_coordInitial.X <= m_coordCurrent.X) {
			::CopyMemory(&coordStart, &m_coordInitial, sizeof(COORD));
			::CopyMemory(&coordEnd, &m_coordCurrent, sizeof(COORD));
		} else {
			::CopyMemory(&coordStart, &m_coordCurrent, sizeof(COORD));
			::CopyMemory(&coordEnd, &m_coordInitial, sizeof(COORD));
		}
	} else if (m_coordInitial.Y < m_coordCurrent.Y) {
		::CopyMemory(&coordStart, &m_coordInitial, sizeof(COORD));
		::CopyMemory(&coordEnd, &m_coordCurrent, sizeof(COORD));
	} else {
		::CopyMemory(&coordStart, &m_coordCurrent, sizeof(COORD));
		::CopyMemory(&coordEnd, &m_coordInitial, sizeof(COORD));
	}
}

//////////////////////////////////////////////////////////////////////////////

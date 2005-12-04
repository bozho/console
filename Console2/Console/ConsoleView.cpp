#include "stdafx.h"
#include "resource.h"

#include "ConsoleView.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

ConsoleView::ConsoleView(const ConsoleParams& consoleStartupParams)
: m_bInitializing(true)
, m_bAppActive(true)
, m_consoleStartupParams(consoleStartupParams)
, m_consoleHandler()
, m_nCharHeight(0)
, m_nCharWidth(0)
, m_screenBuffer()
, m_bImageBackground(false)
, m_crConsoleBackground(RGB(0, 0, 0))
, m_nInsideBorder(1)
, m_bUseFontColor(false)
, m_crFontColor(RGB(0, 0, 0))
, m_bMouseDragable(false)
, m_bInverseShift(false)
, m_cursor()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL ConsoleView::PreTranslateMessage(MSG* pMsg) {

	pMsg;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	m_dcOffscreen.CreateCompatibleDC(NULL);
	m_dcText.CreateCompatibleDC(NULL);
	m_dcBackground.CreateCompatibleDC(NULL);

	m_consoleHandler.SetupDelegates(
						fastdelegate::MakeDelegate(this, &ConsoleView::OnConsoleChange), 
						fastdelegate::MakeDelegate(this, &ConsoleView::OnConsoleClose));

	// TODO: error handling
	m_consoleHandler.StartShellProcess(m_consoleStartupParams);
	m_bInitializing = false;

	CreateOffscreenBuffers();
	SetDefaultConsoleColors();

	// TODO: put this in console size change handler
	m_screenBuffer.reset(new CHAR_INFO[m_consoleHandler.GetConsoleParams()->dwRows*m_consoleHandler.GetConsoleParams()->dwColumns]);
	::ZeroMemory(m_screenBuffer.get(), sizeof(CHAR_INFO)*m_consoleHandler.GetConsoleParams()->dwRows*m_consoleHandler.GetConsoleParams()->dwColumns);

	m_consoleHandler.StartMonitorThread();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	bHandled = TRUE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	CPaintDC	dc(m_hWnd);
	RECT		rectWindow;

	GetClientRect(&rectWindow);

	dc.BitBlt(
		0, 
		0, 
		rectWindow.right, 
		rectWindow.bottom, 
		m_dcOffscreen, 
		0, 
		0, 
		SRCCOPY);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {

	WINDOWPOS* pWinPos = reinterpret_cast<WINDOWPOS*>(lParam);

	// showing the view, repaint
	if (pWinPos->flags & SWP_SHOWWINDOW) Repaint();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnSysKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	::PostMessage(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, uMsg, wParam, lParam);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	::PostMessage(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, uMsg, wParam, lParam);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	UINT	uiFlags = static_cast<UINT>(wParam);
	CPoint	point(LOWORD(lParam), HIWORD(lParam));

	if (!m_bMouseDragable || (m_bInverseShift == !(uiFlags & MK_SHIFT))) {
		
		if (m_nCharWidth) {

			if (m_selectionHandler->GetState() == SelectionHandler::selstateSelected) return 0;

			m_selectionHandler->StartSelection(point, static_cast<SHORT>(m_consoleHandler.GetConsoleParams()->dwColumns - 1), static_cast<SHORT>(m_consoleHandler.GetConsoleParams()->dwRows - 1));
			BitBltOffscreen();
		}
		
	} else {
		// TODO: drag window
/*
		if (m_nTextSelection) {
			return;
		} else if (m_bMouseDragable) {
			// start to drag window
			::SetCapture(m_hWnd);
		}
*/
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {

	CPoint	point(LOWORD(lParam), HIWORD(lParam));

	if (m_selectionHandler->GetState() == SelectionHandler::selstateSelected) {
		
		// TODO: copy on select

		m_selectionHandler->CopySelection(point, m_consoleHandler.GetConsoleBuffer());
        
		m_selectionHandler->ClearSelection();
		BitBltOffscreen();
	} else if (m_selectionHandler->GetState() == SelectionHandler::selstateSelecting) {
		m_selectionHandler->EndSelection();

	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	UINT	uiFlags = static_cast<UINT>(wParam);
	CPoint	point(LOWORD(lParam), HIWORD(lParam));

	if (uiFlags & MK_LBUTTON) {

		if (m_selectionHandler->GetState() == SelectionHandler::selstateSelecting) {

			m_selectionHandler->UpdateSelection(point);
			BitBltOffscreen();
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	if (wParam == CURSOR_TIMER) {
		m_cursor->PrepareNext();
		m_cursor->Draw(m_bAppActive);
		BitBltOffscreen();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::GetRect(RECT& clientRect) {

	// TODO: handle variable fonts
	clientRect.left		= 0;
	clientRect.top		= 0;
	clientRect.right	= m_consoleHandler.GetConsoleParams()->dwColumns*m_nCharWidth + 2*m_nInsideBorder;
	clientRect.bottom	= m_consoleHandler.GetConsoleParams()->dwRows*m_nCharHeight + 2*m_nInsideBorder;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::GetMaxRect(RECT& maxClientRect) {

	if (m_bInitializing) return false;

	// TODO: handle variable fonts
	// TODO: take care of max window size
	maxClientRect.left	= 0;
	maxClientRect.top	= 0;
	maxClientRect.right	= m_consoleHandler.GetConsoleParams()->dwMaxColumns*m_nCharWidth + 2*m_nInsideBorder;
	maxClientRect.bottom= m_consoleHandler.GetConsoleParams()->dwMaxRows*m_nCharHeight + 2*m_nInsideBorder;

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::AdjustRectAndResize(RECT& clientRect) {

	GetWindowRect(&clientRect);
	TRACE(L"rect: %ix%i - %ix%i\n", clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

	// TODO: handle variable fonts
	DWORD dwColumns	= (clientRect.right - clientRect.left - 2*m_nInsideBorder) / m_nCharWidth;
	DWORD dwRows	= (clientRect.bottom - clientRect.top - 2*m_nInsideBorder) / m_nCharHeight;

	clientRect.right	= clientRect.left + dwColumns*m_nCharWidth + 2*m_nInsideBorder;
	clientRect.bottom	= clientRect.top + dwRows*m_nCharHeight + 2*m_nInsideBorder;

	SharedMemoryLock memLock(m_consoleHandler.GetNewConsoleSize());

	m_consoleHandler.GetNewConsoleSize()->dwColumns = dwColumns;
	m_consoleHandler.GetNewConsoleSize()->dwRows	= dwRows;

	m_consoleHandler.GetNewConsoleSize().SetEvent();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetAppActiveStatus(bool bAppActive) {

	m_bAppActive = bAppActive;
	m_cursor->Draw(m_bAppActive);
	BitBltOffscreen();

}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::OnConsoleChange(bool bResize) {

	// console size changed, resize offscreen buffers
	if (bResize) {
		CreateOffscreenBuffers();
		// TODO: put this in console size change handler
		m_screenBuffer.reset(new CHAR_INFO[m_consoleHandler.GetConsoleParams()->dwRows*m_consoleHandler.GetConsoleParams()->dwColumns]);
		::ZeroMemory(m_screenBuffer.get(), sizeof(CHAR_INFO)*m_consoleHandler.GetConsoleParams()->dwRows*m_consoleHandler.GetConsoleParams()->dwColumns);

		// notify parent about resize
		GetParent().SendMessage(UM_CONSOLE_RESIZED, 0, 0);
	}

	// if the view is not visible, don't repaint
	if (!IsWindowVisible()) return;

	Repaint();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::OnConsoleClose() {

	if (::IsWindow(m_hWnd)) ::PostMessage(GetParent(), UM_CONSOLE_CLOSED, 0, reinterpret_cast<LPARAM>(m_hWnd));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::CreateOffscreenBuffers() {

	CPaintDC	dcWindow(m_hWnd);
	RECT		rectWindow;

	// initial paint brush
	CBrush brushBackground;
	brushBackground.CreateSolidBrush(m_crConsoleBackground);

	// create font
	if (!m_fontText.IsNull()) m_fontText.DeleteObject();
	m_fontText.CreateFont(
		-::MulDiv(10, m_dcText.GetDeviceCaps(LOGPIXELSY), 72),
		0,
		0,
		0,
		0, //m_bBold ? FW_BOLD : 0,
		0, //m_bItalic,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,						
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
//		L"FixedMedium6x13");
		L"Courier New");

	// set text DC stuff
	m_dcText.SetBkMode(OPAQUE);
	m_dcText.FillRect(&rectWindow, brushBackground);
	m_dcText.SelectFont(m_fontText);

	// get window info based on font and console size
	GetTextSize();
	GetMaxRect(rectWindow);

	RECT	rectCursor = { 0, 0, m_nCharWidth, m_nCharHeight };

	// create offscreen bitmaps
	CreateOffscreenBitmap(dcWindow, rectWindow, m_dcOffscreen, m_bmpOffscreen);
	CreateOffscreenBitmap(dcWindow, rectWindow, m_dcText, m_bmpText);
	CreateOffscreenBitmap(dcWindow, rectWindow, m_dcBackground, m_bmpBackground);

	m_dcOffscreen.FillRect(&rectWindow, brushBackground);
	m_dcBackground.FillRect(&rectWindow, brushBackground);

	// create and initialize cursor
	m_cursor.reset();
	m_cursor = CursorFactory::CreateCursor(m_hWnd, m_bAppActive, cstyleXTerm, dcWindow, rectCursor, RGB(255, 255, 255));

	// create 
	m_selectionHandler.reset(new SelectionHandler(m_hWnd, dcWindow, rectWindow, m_nCharWidth, m_nCharHeight, RGB(255, 255, 255)));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::CreateOffscreenBitmap(const CPaintDC& dcWindow, const RECT& rect, CDC& cdc, CBitmap& bitmap) {

	if (!bitmap.IsNull()) bitmap.DeleteObject();
	bitmap.CreateCompatibleBitmap(dcWindow, rect.right, rect.bottom);
	cdc.SelectBitmap(bitmap);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::GetTextSize() {

	TEXTMETRIC	textMetric;
	m_dcText.GetTextMetrics(&textMetric);

	m_nCharHeight = textMetric.tmHeight;

	if (!(textMetric.tmPitchAndFamily & TMPF_FIXED_PITCH)) {
		// fixed pitch font (TMPF_FIXED_PITCH is cleared!!!)
		m_nCharWidth = textMetric.tmAveCharWidth;
	}
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

DWORD ConsoleView::GetBufferDifference() {

	SharedMemory<CHAR_INFO>& sharedScreenBuffer = m_consoleHandler.GetConsoleBuffer();

	DWORD dwCount				= m_consoleHandler.GetConsoleParams()->dwRows * m_consoleHandler.GetConsoleParams()->dwColumns;
	DWORD dwChangedPositions	= 0;

	for (DWORD i = 0; i < dwCount; ++i) {
		if (sharedScreenBuffer[i].Char.UnicodeChar != m_screenBuffer[i].Char.UnicodeChar) ++dwChangedPositions;
	}

	return dwChangedPositions*100/dwCount;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetDefaultConsoleColors() {

	m_arrConsoleColors[0]	= 0x000000;
	m_arrConsoleColors[1]	= 0x800000;
	m_arrConsoleColors[2]	= 0x008000;
	m_arrConsoleColors[3]	= 0x808000;
	m_arrConsoleColors[4]	= 0x000080;
	m_arrConsoleColors[5]	= 0x800080;
	m_arrConsoleColors[6]	= 0x008080;
	m_arrConsoleColors[7]	= 0xC0C0C0;
	m_arrConsoleColors[8]	= 0x808080;
	m_arrConsoleColors[9]	= 0xFF0000;
	m_arrConsoleColors[10]	= 0x00FF00;
	m_arrConsoleColors[11]	= 0xFFFF00;
	m_arrConsoleColors[12]	= 0x0000FF;
	m_arrConsoleColors[13]	= 0xFF00FF;
	m_arrConsoleColors[14]	= 0x00FFFF;
	m_arrConsoleColors[15]	= 0xFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::Repaint() {

	// repaint text layer
	if (GetBufferDifference() > 15) {
		RepaintText();
	} else {
		RepaintTextChanges();
	}

	BitBltOffscreen();
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::RepaintText() {

	SIZE	bitmapSize;
	RECT	bitmapRect;
	CBrush	bkgdBrush;

	m_bmpText.GetSize(bitmapSize);
	bitmapRect.left		= 0;
	bitmapRect.top		= 0;
	bitmapRect.right	= bitmapSize.cx;
	bitmapRect.bottom	= bitmapSize.cy;

	bkgdBrush.CreateSolidBrush(m_crConsoleBackground);
	m_dcText.FillRect(&bitmapRect, bkgdBrush);
	
	DWORD dwX			= m_nInsideBorder;
	DWORD dwY			= m_nInsideBorder;
	DWORD dwOffset		= 0;
	
	WORD attrBG;

	// stuff used for caching
//	int			nBkMode		= TRANSPARENT;
	COLORREF	crBkColor	= RGB(0, 0, 0);
	COLORREF	crTxtColor	= RGB(0, 0, 0);
	
/*
	int			nNewBkMode		= TRANSPARENT;
	COLORREF	crNewBkColor	= RGB(0, 0, 0);
	COLORREF	crNewTxtColor	= RGB(0, 0, 0);
*/
	
	bool		bTextOut		= false;
	
	wstring		strText(L"");

	::CopyMemory(
		m_screenBuffer.get(), 
		m_consoleHandler.GetConsoleBuffer().Get(), 
		sizeof(CHAR_INFO) * m_consoleHandler.GetConsoleParams()->dwRows * m_consoleHandler.GetConsoleParams()->dwColumns);

	if (m_nCharWidth > 0) {
		// fixed pitch font
		for (DWORD i = 0; i < m_consoleHandler.GetConsoleParams()->dwRows; ++i) {
			
			dwX = m_nInsideBorder;
			dwY = i*m_nCharHeight + m_nInsideBorder;

//			nBkMode			= TRANSPARENT;
			crBkColor		= RGB(0, 0, 0);
			crTxtColor		= RGB(0, 0, 0);
			
			bTextOut		= false;
			
			attrBG = m_screenBuffer[dwOffset].Attributes >> 4;
			
			// here we decide how to paint text over the background
/*
			if (m_arrConsoleColors[attrBG] == m_crConsoleBackground) {
				m_dcText.SetBkMode(TRANSPARENT);
				nBkMode		= TRANSPARENT;
			} else {
				m_dcText.SetBkMode(OPAQUE);
				nBkMode		= OPAQUE;
				m_dcText.SetBkColor(m_arrConsoleColors[attrBG]);
				crBkColor	= m_arrConsoleColors[attrBG];
			}
*/

			crBkColor	= m_arrConsoleColors[attrBG];
			m_dcText.SetBkColor(m_arrConsoleColors[attrBG]);
			
			m_dcText.SetTextColor(m_bUseFontColor ? m_crFontColor : m_arrConsoleColors[m_screenBuffer[dwOffset].Attributes & 0xF]);
			crTxtColor		= m_bUseFontColor ? m_crFontColor : m_arrConsoleColors[m_screenBuffer[dwOffset].Attributes & 0xF];
			
			strText = m_screenBuffer[dwOffset].Char.UnicodeChar;
			++dwOffset;

			for (DWORD j = 1; j < m_consoleHandler.GetConsoleParams()->dwColumns; ++j) {
				
				attrBG = m_screenBuffer[dwOffset].Attributes >> 4;

/*
				if (m_arrConsoleColors[attrBG] == m_crConsoleBackground) {
					if (nBkMode != TRANSPARENT) {
						nBkMode = TRANSPARENT;
						bTextOut = true;
					}
				} else {
					if (nBkMode != OPAQUE) {
						nBkMode = OPAQUE;
						bTextOut = true;
					}
*/
					if (crBkColor != m_arrConsoleColors[attrBG]) {
						crBkColor = m_arrConsoleColors[attrBG];
						bTextOut = true;
					}
//				}

				if (crTxtColor != (m_bUseFontColor ? m_crFontColor : m_arrConsoleColors[m_screenBuffer[dwOffset].Attributes & 0xF])) {
					crTxtColor = m_bUseFontColor ? m_crFontColor : m_arrConsoleColors[m_screenBuffer[dwOffset].Attributes & 0xF];
					bTextOut = true;
				}

				if (bTextOut) {

					m_dcText.TextOut(dwX, dwY, strText.c_str(), strText.length());
					dwX += strText.length() * m_nCharWidth;

//					m_dcText.SetBkMode(nBkMode);
					m_dcText.SetBkColor(crBkColor);
					m_dcText.SetTextColor(crTxtColor);

					strText = m_screenBuffer[dwOffset].Char.UnicodeChar;
					
				} else {
					strText += m_screenBuffer[dwOffset].Char.UnicodeChar;
				}
					
				++dwOffset;
			}

			if (strText.length() > 0) {
				m_dcText.TextOut(dwX, dwY, strText.c_str(), strText.length());
			}
		}

	} else {
		
		// variable pitch font
		for (DWORD i = 0; i < m_consoleHandler.GetConsoleParams()->dwRows; ++i) {
			
			dwX = m_nInsideBorder;
			dwY = i*m_nCharHeight + m_nInsideBorder;
			
			for (DWORD j = 0; j < m_consoleHandler.GetConsoleParams()->dwColumns; ++j) {
				
				attrBG = m_screenBuffer[dwOffset].Attributes >> 4;

				// here we decide how to paint text over the backgound
/*
				if (m_arrConsoleColors[attrBG] == m_crConsoleBackground) {
					m_dcText.SetBkMode(TRANSPARENT);
				} else {
					m_dcText.SetBkMode(OPAQUE);
*/
					m_dcText.SetBkColor(m_arrConsoleColors[attrBG]);
//				}
				
				m_dcText.SetTextColor(m_bUseFontColor ? m_crFontColor : m_arrConsoleColors[m_screenBuffer[dwOffset].Attributes & 0xF]);
				m_dcText.TextOut(dwX, dwY, &(m_screenBuffer[dwOffset].Char.UnicodeChar), 1);
				int nWidth;
				m_dcText.GetCharWidth32(m_screenBuffer[dwOffset].Char.UnicodeChar, m_screenBuffer[dwOffset].Char.UnicodeChar, &nWidth);
				dwX += nWidth;
				++dwOffset;
			}
		}
	}
	
//	if (m_pCursor) DrawCursor(TRUE);
	
//	InvalidateRect(NULL, FALSE);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::RepaintTextChanges() {
	
	SIZE	bitmapSize;
	CBrush	bkgdBrush;

	m_bmpText.GetSize(bitmapSize);
	bkgdBrush.CreateSolidBrush(m_crConsoleBackground);

	DWORD	dwX			= m_nInsideBorder;
	DWORD	dwY			= m_nInsideBorder;
	DWORD	dwOffset	= 0;
	
	WORD	attrBG;

	SharedMemory<CHAR_INFO>& sharedScreenBuffer = m_consoleHandler.GetConsoleBuffer();


	if (m_nCharWidth > 0) {

		// fixed pitch font
		for (DWORD i = 0; i < m_consoleHandler.GetConsoleParams()->dwRows; ++i) {
			
			dwX = m_nInsideBorder;
			dwY = i*m_nCharHeight + m_nInsideBorder;

			for (DWORD j = 0; j < m_consoleHandler.GetConsoleParams()->dwColumns; ++j) {

				if (memcmp(&(m_screenBuffer[dwOffset]), &(sharedScreenBuffer[dwOffset]), sizeof(CHAR_INFO))) {

					memcpy(&(m_screenBuffer[dwOffset]), &(sharedScreenBuffer[dwOffset]), sizeof(CHAR_INFO));

					RECT rect;
					rect.top	= dwY;
					rect.left	= dwX;
					rect.bottom	= dwY + m_nCharHeight;
					rect.right	= dwX + m_nCharWidth;
					
/*
					if (m_bBitmapBackground) {
						if (m_bRelativeBackground) {
							::BitBlt(m_hdcConsole, dwX, dwY, m_nCharWidth, m_nCharHeight, m_hdcBackground, m_nX+m_nXBorderSize-m_nBackgroundOffsetX+(int)dwX, m_nY+m_nCaptionSize+m_nYBorderSize-m_nBackgroundOffsetY+(int)dwY, SRCCOPY);
						} else {
							::BitBlt(m_hdcConsole, dwX, dwY, m_nCharWidth, m_nCharHeight, m_hdcBackground, dwX, dwY, SRCCOPY);
						}
					} else {
						::FillRect(m_hdcConsole, &rect, m_hBkBrush);
					}
*/
					
					m_dcText.FillRect(&rect, bkgdBrush);
					attrBG = m_screenBuffer[dwOffset].Attributes >> 4;

					// here we decide how to paint text over the backgound
/*
					if (m_arrConsoleColors[attrBG] == m_crConsoleBackground) {
						::SetBkMode(m_hdcConsole, TRANSPARENT);
					} else {
						::SetBkMode(m_hdcConsole, OPAQUE);
						::SetBkColor(m_hdcConsole, m_arrConsoleColors[attrBG]);
					}
*/
					
					m_dcText.SetBkColor(m_arrConsoleColors[attrBG]);
					m_dcText.SetTextColor(m_bUseFontColor ? m_crFontColor : m_arrConsoleColors[m_screenBuffer[dwOffset].Attributes & 0xF]);
					m_dcText.TextOut(dwX, dwY, &(m_screenBuffer[dwOffset].Char.UnicodeChar), 1);

//					::InvalidateRect(m_hWnd, &rect, FALSE);
				}

				dwX += m_nCharWidth;
				++dwOffset;
			}
		}
		
	} else {
		
		// variable pitch font
		memcpy(m_screenBuffer.get(), sharedScreenBuffer.Get(), sizeof(CHAR_INFO)*m_consoleHandler.GetConsoleParams()->dwRows * m_consoleHandler.GetConsoleParams()->dwColumns);
	
		RECT rect;
		rect.top	= 0;
		rect.left	= 0;
		rect.bottom	= bitmapSize.cy;
		rect.right	= bitmapSize.cx;
		
/*
		if (m_bBitmapBackground) {
			if (m_bRelativeBackground) {
				::BitBlt(m_hdcConsole, 0, 0, m_nClientWidth, m_nClientHeight, m_hdcBackground, m_nX+m_nXBorderSize-m_nBackgroundOffsetX, m_nY+m_nCaptionSize+m_nYBorderSize-m_nBackgroundOffsetY, SRCCOPY);
			} else {
				::BitBlt(m_hdcConsole, 0, 0, m_nClientWidth, m_nClientHeight, m_hdcBackground, 0, 0, SRCCOPY);
			}
		} else {
			::FillRect(m_hdcConsole, &rect, m_hBkBrush);
		}
*/
		m_dcText.FillRect(&rect, bkgdBrush);
		
		for (DWORD i = 0; i < m_consoleHandler.GetConsoleParams()->dwRows; ++i) {
			
			dwX = m_nInsideBorder;
			dwY = i*m_nCharHeight + m_nInsideBorder;
			
			for (DWORD j = 0; j < m_consoleHandler.GetConsoleParams()->dwColumns; ++j) {
				
				attrBG = m_screenBuffer[dwOffset].Attributes >> 4;
				
				// here we decide how to paint text over the backgound
/*
				if (m_arrConsoleColors[attrBG] == m_crConsoleBackground) {
					::SetBkMode(m_hdcConsole, TRANSPARENT);
				} else {
					::SetBkMode(m_hdcConsole, OPAQUE);
					::SetBkColor(m_hdcConsole, m_arrConsoleColors[attrBG]);
				}
*/

				m_dcText.SetBkColor(m_arrConsoleColors[attrBG]);
				m_dcText.SetTextColor(m_bUseFontColor ? m_crFontColor : m_arrConsoleColors[m_screenBuffer[dwOffset].Attributes & 0xF]);
				m_dcText.TextOut(dwX, dwY, &(m_screenBuffer[dwOffset].Char.UnicodeChar), 1);
				int nWidth;
				m_dcText.GetCharWidth32(m_screenBuffer[dwOffset].Char.UnicodeChar, m_screenBuffer[dwOffset].Char.UnicodeChar, &nWidth);
				dwX += nWidth;
				++dwOffset;
			}
		}
		
//		::InvalidateRect(m_hWnd, NULL, FALSE);
//		}
	}
	
//	if (m_pCursor) DrawCursor();
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::BitBltOffscreen() {

	RECT		rectWindow;

	GetClientRect(&rectWindow);

	if (m_bImageBackground) {

		m_dcOffscreen.BitBlt(
						0, 
						0, 
						rectWindow.right, 
						rectWindow.bottom, 
						m_dcBackground, 
						0, 
						0, 
						SRCCOPY);

		m_dcOffscreen.TransparentBlt(
						0, 
						0, 
						rectWindow.right, 
						rectWindow.bottom, 
						m_dcText, 
						0, 
						0, 
						rectWindow.right, 
						rectWindow.bottom, 
						m_crConsoleBackground);

	} else {
		
		m_dcOffscreen.BitBlt(
						0, 
						0, 
						rectWindow.right, 
						rectWindow.bottom, 
						m_dcText, 
						0, 
						0, 
						SRCCOPY);
	}

	// blit cursor
	if (m_consoleHandler.GetCursorInfo()->bVisible) {

		SharedMemory<CONSOLE_SCREEN_BUFFER_INFO>& consoleInfo = m_consoleHandler.GetConsoleInfo();

		m_cursor->BitBlt(
			m_dcOffscreen, 
			consoleInfo->dwCursorPosition.X * m_nCharWidth + m_nInsideBorder, 
			(consoleInfo->dwCursorPosition.Y - consoleInfo->srWindow.Top) * m_nCharHeight + m_nInsideBorder);
	}

	// blit selection
	m_selectionHandler->BitBlt(m_dcOffscreen);

	InvalidateRect(NULL, FALSE);
}

/////////////////////////////////////////////////////////////////////////////

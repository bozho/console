#include "stdafx.h"
#include "resource.h"

#include "Console.h"
#include "ConsoleView.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CDC		ConsoleView::m_dcOffscreen(::CreateCompatibleDC(NULL));
CDC		ConsoleView::m_dcText(::CreateCompatibleDC(NULL));

CBitmap	ConsoleView::m_bmpOffscreen;
CBitmap	ConsoleView::m_bmpText;

CFont	ConsoleView::m_fontText;

int		ConsoleView::m_nCharHeight(0);
int		ConsoleView::m_nCharWidth(0);

//////////////////////////////////////////////////////////////////////////////

ConsoleView::ConsoleView(DWORD dwTabIndex, DWORD dwRows, DWORD dwColumns)
: m_bInitializing(true)
, m_bAppActive(true)
, m_bViewActive(true)
, m_bConsoleWindowVisible(false)
, m_dwStartupRows(dwRows)
, m_dwStartupColumns(dwColumns)
, m_bShowVScroll(false)
, m_bShowHScroll(false)
, m_nVScrollWidth(::GetSystemMetrics(SM_CXVSCROLL))
, m_nHScrollWidth(::GetSystemMetrics(SM_CXHSCROLL))
, m_consoleHandler()
, m_screenBuffer()
, m_nInsideBorder(1)
, m_bUseFontColor(false)
, m_crFontColor(RGB(0, 0, 0))
, m_bMouseDragable(false)
, m_bInverseShift(false)
, m_tabData(g_settingsHandler->GetTabSettings().tabDataVector[dwTabIndex])
, m_cursor()
, m_selectionHandler()
{
}

ConsoleView::~ConsoleView() {
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

/*
	m_dcOffscreen.CreateCompatibleDC(NULL);
	m_dcText.CreateCompatibleDC(NULL);
*/

	// set view title
	SetWindowText(m_tabData->strName.c_str());

	m_consoleHandler.SetupDelegates(
						fastdelegate::MakeDelegate(this, &ConsoleView::OnConsoleChange), 
						fastdelegate::MakeDelegate(this, &ConsoleView::OnConsoleClose));

	// load background image
	if (m_tabData->backgroundImageType != bktypeNone) {
		if (!g_imageHandler->LoadImage(m_tabData->tabBackground)) {
			m_tabData->backgroundImageType = bktypeNone;
		}
	}

	// TODO: error handling
	if (!m_consoleHandler.StartShellProcess(
								(m_tabData->strShell.length() > 0) ? m_tabData->strShell : g_settingsHandler->GetConsoleSettings().strShell, 
								(m_tabData->strInitialDir.length() > 0) ? m_tabData->strInitialDir : g_settingsHandler->GetConsoleSettings().strInitialDir, 
								m_dwStartupRows, 
								m_dwStartupColumns)) {
									
		return -1;
	}
	m_bInitializing = false;

	// scrollbar stuff
	InitializeScrollbars();

	// create offscreen buffers
	CreateOffscreenBuffers();

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

LRESULT ConsoleView::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	CPaintDC	dc(m_hWnd);

	if ((m_tabData->backgroundImageType != bktypeNone) && m_tabData->IsBackgroundRelative()) {
		// we need to update offscreen buffers here for relative backgrounds
		UpdateOffscreen(dc.m_ps.rcPaint);
	}

	dc.BitBlt(
		dc.m_ps.rcPaint.left, 
		dc.m_ps.rcPaint.top, 
		dc.m_ps.rcPaint.right, 
		dc.m_ps.rcPaint.bottom,
		m_dcOffscreen, 
		dc.m_ps.rcPaint.left, 
		dc.m_ps.rcPaint.top, 
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

LRESULT ConsoleView::OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	GetParent().SendMessage(UM_SHOW_POPUP_MENU, wParam, lParam);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	DoScroll(SB_VERT, LOWORD(wParam), HIWORD(wParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	DoScroll(SB_HORZ, LOWORD(wParam), HIWORD(wParam));
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
//			BitBltOffscreen();
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

	if (m_selectionHandler->GetState() == SelectionHandler::selstateStartedSelecting) {

		m_selectionHandler->EndSelection();
		m_selectionHandler->ClearSelection();

	} else if (m_selectionHandler->GetState() == SelectionHandler::selstateSelected) {
		
		// TODO: copy on select
		Copy(&point);

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

		if ((m_selectionHandler->GetState() == SelectionHandler::selstateStartedSelecting) ||
			(m_selectionHandler->GetState() == SelectionHandler::selstateSelecting)) {

			m_selectionHandler->UpdateSelection(point);
			BitBltOffscreen();
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT ConsoleView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	if (m_bViewActive && (wParam == CURSOR_TIMER) && (m_cursor.get() != NULL)) {
		m_cursor->PrepareNext();
		m_cursor->Draw(m_bAppActive);
		BitBltOffscreen(true);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::GetRect(CRect& clientRect) {

	// TODO: handle variable fonts
	clientRect.left		= 0;
	clientRect.top		= 0;
	clientRect.right	= m_consoleHandler.GetConsoleParams()->dwColumns*m_nCharWidth + 2*m_nInsideBorder;
	clientRect.bottom	= m_consoleHandler.GetConsoleParams()->dwRows*m_nCharHeight + 2*m_nInsideBorder;

	if (m_bShowVScroll) clientRect.right	+= m_nVScrollWidth;
	if (m_bShowHScroll) clientRect.bottom	+= m_nHScrollWidth;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::GetMaxRect(CRect& maxClientRect) {

	if (m_bInitializing) return false;

	// TODO: handle variable fonts
	// TODO: take care of max window size
	maxClientRect.left	= 0;
	maxClientRect.top	= 0;
	maxClientRect.right	= m_consoleHandler.GetConsoleParams()->dwMaxColumns*m_nCharWidth + 2*m_nInsideBorder;
	maxClientRect.bottom= m_consoleHandler.GetConsoleParams()->dwMaxRows*m_nCharHeight + 2*m_nInsideBorder;

	CWindow desktopWindow(::GetDesktopWindow());
	CRect	rectDesktop;
	bool	bRecalc = false;

	desktopWindow.GetWindowRect(rectDesktop);

	if (rectDesktop.Width() < maxClientRect.Width()) {
		m_consoleHandler.GetConsoleParams()->dwMaxColumns = (rectDesktop.Width() - 2*m_nInsideBorder) / m_nCharWidth;
		bRecalc = true;
	}

	if (rectDesktop.Height() < maxClientRect.Height()) {
		m_consoleHandler.GetConsoleParams()->dwMaxRows = (rectDesktop.Height() - 2*m_nInsideBorder) / m_nCharHeight;
		bRecalc = true;
	}

	if (bRecalc) {
		maxClientRect.right	= m_consoleHandler.GetConsoleParams()->dwMaxColumns*m_nCharWidth + 2*m_nInsideBorder;
		maxClientRect.bottom= m_consoleHandler.GetConsoleParams()->dwMaxRows*m_nCharHeight + 2*m_nInsideBorder;
	}

	if (m_bShowVScroll) maxClientRect.right	+= m_nVScrollWidth;
	if (m_bShowHScroll) maxClientRect.bottom+= m_nHScrollWidth;

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::AdjustRectAndResize(CRect& clientRect) {

	GetWindowRect(&clientRect);
/*
	TRACE(L"================================================================\n");
	TRACE(L"rect: %ix%i - %ix%i\n", clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
*/

	// exclude scrollbars from row/col calculation
	if (m_bShowVScroll) clientRect.right	-= m_nVScrollWidth;
	if (m_bShowHScroll) clientRect.bottom	-= m_nHScrollWidth;

	// TODO: handle variable fonts
	DWORD dwColumns	= (clientRect.Width() - 2*m_nInsideBorder) / m_nCharWidth;
	DWORD dwRows	= (clientRect.Height() - 2*m_nInsideBorder) / m_nCharHeight;

	clientRect.right	= clientRect.left + dwColumns*m_nCharWidth + 2*m_nInsideBorder;
	clientRect.bottom	= clientRect.top + dwRows*m_nCharHeight + 2*m_nInsideBorder;

	// adjust for scrollbars
	if (m_bShowVScroll) clientRect.right	+= m_nVScrollWidth;
	if (m_bShowHScroll) clientRect.bottom	+= m_nHScrollWidth;

	SharedMemoryLock memLock(m_consoleHandler.GetNewConsoleSize());

	m_consoleHandler.GetNewConsoleSize()->dwColumns	= dwColumns;
	m_consoleHandler.GetNewConsoleSize()->dwRows	= dwRows;

/*
	TRACE(L"console view: 0x%08X, adjusted: %ix%i\n", m_hWnd, dwRows, dwColumns);
	TRACE(L"================================================================\n");
*/

	m_consoleHandler.GetNewConsoleSize().SetEvent();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetConsoleWindowVisible(bool bVisible) {
	m_bConsoleWindowVisible = bVisible;
	::ShowWindow(m_consoleHandler.GetConsoleParams()->hwndConsoleWindow, bVisible ? SW_SHOW : SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetAppActiveStatus(bool bAppActive) {

	m_bAppActive = bAppActive;
	if (m_cursor.get() != NULL) m_cursor->Draw(m_bAppActive);
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::RecreateOffscreenBuffers() {
	m_fontText.DeleteObject();
	m_bmpOffscreen.DeleteObject();
	m_bmpText.DeleteObject();
	CreateOffscreenBuffers();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::RepaintView() {
	RepaintText();
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::SetViewActive(bool bActive) {

	m_bViewActive = bActive;
	if (m_bViewActive) RepaintView();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::Copy(const CPoint* pPoint /* = NULL */) {

	if ((m_selectionHandler->GetState() != SelectionHandler::selstateSelecting) &&
		(m_selectionHandler->GetState() != SelectionHandler::selstateSelected)) {

		return;
	}

	m_selectionHandler->CopySelection(pPoint, m_consoleHandler.GetConsoleBuffer());
	m_selectionHandler->ClearSelection();
	BitBltOffscreen();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::Paste() {

	if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return;
	
	if (::OpenClipboard(m_hWnd)) {
		HANDLE	hData = ::GetClipboardData(CF_UNICODETEXT);

		SendTextToConsole(reinterpret_cast<wchar_t*>(::GlobalLock(hData)));

		::GlobalUnlock(hData);
		::CloseClipboard();
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::OnConsoleChange(bool bResize) {

	// console size changed, resize offscreen buffers
	if (bResize) {
	
/*
		TRACE(L"================================================================\n");
		TRACE(L"Resizing console wnd: 0x%08X\n", m_hWnd);
*/
		InitializeScrollbars();

		if (m_bViewActive) RecreateOffscreenBuffers();

		// TODO: put this in console size change handler
		m_screenBuffer.reset(new CHAR_INFO[m_consoleHandler.GetConsoleParams()->dwRows*m_consoleHandler.GetConsoleParams()->dwColumns]);
		::ZeroMemory(m_screenBuffer.get(), sizeof(CHAR_INFO)*m_consoleHandler.GetConsoleParams()->dwRows*m_consoleHandler.GetConsoleParams()->dwColumns);

		// notify parent about resize
		GetParent().SendMessage(UM_CONSOLE_RESIZED, 0, 0);
	}

	// if the view is not visible, don't repaint
	if (!m_bViewActive) return;

	SharedMemory<CONSOLE_SCREEN_BUFFER_INFO>& consoleInfo = m_consoleHandler.GetConsoleInfo();

	if (m_bShowVScroll) {
		SCROLLINFO si;
		si.cbSize = sizeof(si); 
		si.fMask  = SIF_POS; 
		si.nPos   = consoleInfo->srWindow.Top; 
		::FlatSB_SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);

/*
		TRACE(L"----------------------------------------------------------------\n");
		TRACE(L"VScroll pos: %i\n", consoleInfo->srWindow.Top);
*/
	}

	if (m_bShowHScroll) {
		SCROLLINFO si;
		si.cbSize = sizeof(si); 
		si.fMask  = SIF_POS; 
		si.nPos   = consoleInfo->srWindow.Left; 
		::FlatSB_SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);
	}

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
	CRect		rectWindowMax;
//	CRect		rectWindow;

	// create font
	if (!CreateFont(g_settingsHandler->GetAppearanceSettings().fontSettings.strName)) {
		CreateFont(wstring(L"Courier New"));
	}

	// get max window rect based on font and console size
	GetMaxRect(rectWindowMax);
//	GetWindowRect(&rectWindow);

	// create offscreen bitmaps
	CreateOffscreenBitmap(dcWindow, rectWindowMax, m_dcOffscreen, m_bmpOffscreen);
	CreateOffscreenBitmap(dcWindow, rectWindowMax, m_dcText, m_bmpText);

	// initial paint brush
	CBrush brushBackground;
	brushBackground.CreateSolidBrush(m_tabData->crBackgroundColor);

	// initial offscreen paint
	m_dcOffscreen.FillRect(&rectWindowMax, brushBackground);

	// set text DC stuff
	m_dcText.SetBkMode(OPAQUE);
	m_dcText.FillRect(&rectWindowMax, brushBackground);

	// create selection handler
	m_selectionHandler.reset(new SelectionHandler(m_hWnd, dcWindow, rectWindowMax, m_nCharWidth, m_nCharHeight, RGB(255, 255, 255)));

	// create and initialize cursor
	CRect		rectCursor(0, 0, m_nCharWidth, m_nCharHeight);

	m_cursor.reset();
	m_cursor = CursorFactory::CreateCursor(
								m_hWnd, 
								m_bAppActive, 
								m_tabData.get() ? static_cast<CursorStyle>(m_tabData->dwCursorStyle) : cstyleConsole, 
								dcWindow, 
								rectCursor, 
								m_tabData.get() ? static_cast<CursorStyle>(m_tabData->crCursorColor) : RGB(255, 255, 255));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::CreateOffscreenBitmap(const CPaintDC& dcWindow, const CRect& rect, CDC& cdc, CBitmap& bitmap) {

	if (!bitmap.IsNull()) return;// bitmap.DeleteObject();
	bitmap.CreateCompatibleBitmap(dcWindow, rect.right, rect.bottom);
	cdc.SelectBitmap(bitmap);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ConsoleView::CreateFont(const wstring& strFontName) {

	if (!m_fontText.IsNull()) return true;// m_fontText.DeleteObject();
	m_fontText.CreateFont(
		-::MulDiv(g_settingsHandler->GetAppearanceSettings().fontSettings.dwSize , m_dcText.GetDeviceCaps(LOGPIXELSY), 72),
		0,
		0,
		0,
		g_settingsHandler->GetAppearanceSettings().fontSettings.bBold ? FW_BOLD : 0,
		g_settingsHandler->GetAppearanceSettings().fontSettings.bItalic,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,						
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		strFontName.c_str());

	TEXTMETRIC	textMetric;

	m_dcText.SelectFont(m_fontText);
	m_dcText.GetTextMetrics(&textMetric);

	if (textMetric.tmPitchAndFamily & TMPF_FIXED_PITCH) return false;

	// fixed pitch font (TMPF_FIXED_PITCH is cleared!!!)
	m_nCharWidth = textMetric.tmAveCharWidth;
	m_nCharHeight = textMetric.tmHeight;

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleView::InitializeScrollbars() {

	SharedMemory<ConsoleParams>& consoleParams = m_consoleHandler.GetConsoleParams();

	m_bShowVScroll = consoleParams->dwBufferRows > consoleParams->dwRows;
	m_bShowHScroll = consoleParams->dwBufferColumns > consoleParams->dwColumns;

//	if (m_nScrollbarStyle != FSB_REGULAR_MODE)
	::InitializeFlatSB(m_hWnd);

	::FlatSB_ShowScrollBar(m_hWnd, SB_VERT, m_bShowVScroll);
	::FlatSB_ShowScrollBar(m_hWnd, SB_HORZ, m_bShowHScroll);

/*
	TRACE(L"InitializeScrollbars, console wnd: 0x%08X\n", m_hWnd);
	TRACE(L"Sizes: %i, %i    %i, %i\n", consoleParams->dwRows, consoleParams->dwBufferRows - 1, consoleParams->dwColumns, consoleParams->dwBufferColumns - 1);
	TRACE(L"----------------------------------------------------------------\n");
*/

	if (m_bShowVScroll) {
		// set vertical scrollbar stuff
		SCROLLINFO	si ;

		si.cbSize	= sizeof(SCROLLINFO) ;
		si.fMask	= SIF_PAGE | SIF_RANGE ;
		si.nPage	= consoleParams->dwRows;
		si.nMax		= consoleParams->dwBufferRows - 1;
		si.nMin		= 0 ;

		::FlatSB_SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
	}

	if (m_bShowHScroll) {
		// set vertical scrollbar stuff
		SCROLLINFO	si ;

		si.cbSize	= sizeof(SCROLLINFO) ;
		si.fMask	= SIF_PAGE | SIF_RANGE ;
		si.nPage	= consoleParams->dwColumns;
		si.nMax		= consoleParams->dwBufferColumns - 1;
		si.nMin		= 0 ;

		::FlatSB_SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE) ;
	}

	::FlatSB_SetScrollProp(m_hWnd, WSB_PROP_VSTYLE, FSB_FLAT_MODE, TRUE);
	::FlatSB_SetScrollProp(m_hWnd, WSB_PROP_CXVSCROLL , 5, TRUE);

/*
	// set scrollbar properties
	::FlatSB_SetScrollProp(m_hWnd, WSB_PROP_VSTYLE, m_nScrollbarStyle, FALSE);
	::FlatSB_SetScrollProp(m_hWnd, WSB_PROP_VBKGCOLOR, m_crScrollbarColor, FALSE);
	::FlatSB_SetScrollProp(m_hWnd, WSB_PROP_CXVSCROLL , m_nScrollbarWidth, FALSE);
	::FlatSB_SetScrollProp(m_hWnd, WSB_PROP_CYVSCROLL, m_nScrollbarButtonHeight, FALSE);
	::FlatSB_SetScrollProp(m_hWnd, WSB_PROP_CYVTHUMB, m_nScrollbarThunmbHeight, TRUE);
*/
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::DoScroll(int nType, int nScrollCode, int nThumbPos) {

	int nCurrentPos = ::FlatSB_GetScrollPos(m_hWnd, nType);
	int nDelta = 0;
	
	switch(nScrollCode) { 
		
		case SB_PAGEUP: /* SB_PAGELEFT */
			nDelta = -5; 
			break; 
			
		case SB_PAGEDOWN: /* SB_PAGERIGHT */
			nDelta = 5; 
			break; 
			
		case SB_LINEUP: /* SB_LINELEFT */
			nDelta = -1; 
			break; 
			
		case SB_LINEDOWN: /* SB_LINERIGHT */
			nDelta = 1; 
			break; 
			
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			nDelta = nThumbPos - nCurrentPos; 
			break;
			
		case SB_ENDSCROLL:
			return;
			
		default: 
			return;
	}
	
	if (nDelta != 0) {

		SharedMemory<SIZE>& newScrollPos = m_consoleHandler.GetNewScrollPos();

		if (nType == SB_VERT) {
			newScrollPos->cx = 0;
			newScrollPos->cy = nDelta;
		} else {
			newScrollPos->cx = nDelta;
			newScrollPos->cy = 0;
		}

		newScrollPos.SetEvent();
	}
}

/////////////////////////////////////////////////////////////////////////////


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
	CRect	bitmapRect;
	CBrush	bkgdBrush;

	m_bmpText.GetSize(bitmapSize);
	bitmapRect.left		= 0;
	bitmapRect.top		= 0;
	bitmapRect.right	= bitmapSize.cx;
	bitmapRect.bottom	= bitmapSize.cy;

	bkgdBrush.CreateSolidBrush(m_tabData->crBackgroundColor);
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
			if (g_settingsHandler->GetFontSettings().consoleColors[attrBG] == m_crConsoleBackground) {
				m_dcText.SetBkMode(TRANSPARENT);
				nBkMode		= TRANSPARENT;
			} else {
				m_dcText.SetBkMode(OPAQUE);
				nBkMode		= OPAQUE;
				m_dcText.SetBkColor(g_settingsHandler->GetFontSettings().consoleColors[attrBG]);
				crBkColor	= g_settingsHandler->GetFontSettings().consoleColors[attrBG];
			}
*/

			crBkColor	= g_settingsHandler->GetConsoleSettings().consoleColors[attrBG];
			m_dcText.SetBkColor(g_settingsHandler->GetConsoleSettings().consoleColors[attrBG]);
			
			m_dcText.SetTextColor(m_bUseFontColor ? m_crFontColor : g_settingsHandler->GetConsoleSettings().consoleColors[m_screenBuffer[dwOffset].Attributes & 0xF]);
			crTxtColor		= m_bUseFontColor ? m_crFontColor : g_settingsHandler->GetConsoleSettings().consoleColors[m_screenBuffer[dwOffset].Attributes & 0xF];
			
			strText = m_screenBuffer[dwOffset].Char.UnicodeChar;
			++dwOffset;

			for (DWORD j = 1; j < m_consoleHandler.GetConsoleParams()->dwColumns; ++j) {
				
				attrBG = m_screenBuffer[dwOffset].Attributes >> 4;

/*
				if (g_settingsHandler->GetFontSettings().consoleColors[attrBG] == m_crConsoleBackground) {
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
					if (crBkColor != g_settingsHandler->GetConsoleSettings().consoleColors[attrBG]) {
						crBkColor = g_settingsHandler->GetConsoleSettings().consoleColors[attrBG];
						bTextOut = true;
					}
//				}

				if (crTxtColor != (m_bUseFontColor ? m_crFontColor : g_settingsHandler->GetConsoleSettings().consoleColors[m_screenBuffer[dwOffset].Attributes & 0xF])) {
					crTxtColor = m_bUseFontColor ? m_crFontColor : g_settingsHandler->GetConsoleSettings().consoleColors[m_screenBuffer[dwOffset].Attributes & 0xF];
					bTextOut = true;
				}

				if (bTextOut) {

					m_dcText.TextOut(dwX, dwY, strText.c_str(), static_cast<int>(strText.length()));
					dwX += static_cast<int>(strText.length() * m_nCharWidth);

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
				m_dcText.TextOut(dwX, dwY, strText.c_str(), static_cast<int>(strText.length()));
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
				if (g_settingsHandler->GetFontSettings().consoleColors[attrBG] == m_crConsoleBackground) {
					m_dcText.SetBkMode(TRANSPARENT);
				} else {
					m_dcText.SetBkMode(OPAQUE);
*/
					m_dcText.SetBkColor(g_settingsHandler->GetConsoleSettings().consoleColors[attrBG]);
//				}
				
				m_dcText.SetTextColor(m_bUseFontColor ? m_crFontColor : g_settingsHandler->GetConsoleSettings().consoleColors[m_screenBuffer[dwOffset].Attributes & 0xF]);
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
	bkgdBrush.CreateSolidBrush(m_tabData->crBackgroundColor);

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

					CRect rect;
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
					if (g_settingsHandler->GetFontSettings().consoleColors[attrBG] == m_crConsoleBackground) {
						::SetBkMode(m_hdcConsole, TRANSPARENT);
					} else {
						::SetBkMode(m_hdcConsole, OPAQUE);
						::SetBkColor(m_hdcConsole, g_settingsHandler->GetFontSettings().consoleColors[attrBG]);
					}
*/
					
					m_dcText.SetBkColor(g_settingsHandler->GetConsoleSettings().consoleColors[attrBG]);
					m_dcText.SetTextColor(m_bUseFontColor ? m_crFontColor : g_settingsHandler->GetConsoleSettings().consoleColors[m_screenBuffer[dwOffset].Attributes & 0xF]);
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
	
		CRect rect;
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
				if (g_settingsHandler->GetFontSettings().consoleColors[attrBG] == m_crConsoleBackground) {
					::SetBkMode(m_hdcConsole, TRANSPARENT);
				} else {
					::SetBkMode(m_hdcConsole, OPAQUE);
					::SetBkColor(m_hdcConsole, g_settingsHandler->GetFontSettings().consoleColors[attrBG]);
				}
*/

				m_dcText.SetBkColor(g_settingsHandler->GetConsoleSettings().consoleColors[attrBG]);
				m_dcText.SetTextColor(m_bUseFontColor ? m_crFontColor : g_settingsHandler->GetConsoleSettings().consoleColors[m_screenBuffer[dwOffset].Attributes & 0xF]);
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

void ConsoleView::BitBltOffscreen(bool bOnlyCursor /*= false*/) {

	CRect	rectBlit;

	if (bOnlyCursor) {
		// blit only cursor
		if ((m_cursor.get() == NULL) || !m_consoleHandler.GetCursorInfo()->bVisible) return;

		SharedMemory<CONSOLE_SCREEN_BUFFER_INFO>& consoleInfo = m_consoleHandler.GetConsoleInfo();

		rectBlit		= m_cursor->GetCursorRect();
		rectBlit.left	+= (consoleInfo->dwCursorPosition.X - consoleInfo->srWindow.Left) * m_nCharWidth + m_nInsideBorder;
		rectBlit.top	+= (consoleInfo->dwCursorPosition.Y - consoleInfo->srWindow.Top) * m_nCharHeight + m_nInsideBorder;
		rectBlit.right	+= (consoleInfo->dwCursorPosition.X - consoleInfo->srWindow.Left) * m_nCharWidth + m_nInsideBorder;
		rectBlit.bottom	+= (consoleInfo->dwCursorPosition.Y - consoleInfo->srWindow.Top) * m_nCharHeight + m_nInsideBorder;
	} else {
		// blit rect is entire view
		GetClientRect(&rectBlit);
	}

	if ((m_tabData->backgroundImageType == bktypeNone) || !m_tabData->IsBackgroundRelative()) {
		// we don't do this for relative backgrounds here
		UpdateOffscreen(rectBlit);
	}

	InvalidateRect(&rectBlit, FALSE);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::UpdateOffscreen(const CRect& rectBlit) {

	CRect	rectWindow;
	GetClientRect(&rectWindow);

	if (m_tabData->backgroundImageType != bktypeNone) {

		POINT	pointClientScreen = {0, 0};

		ClientToScreen(&pointClientScreen);

		g_imageHandler->UpdateImageBitmap(m_dcOffscreen, rectWindow, m_tabData->tabBackground);

		m_dcOffscreen.BitBlt(
						rectBlit.left, 
						rectBlit.top, 
						rectBlit.right, 
						rectBlit.bottom, 
						m_tabData->tabBackground->dcImage, 
						m_tabData->tabBackground->bRelative ? rectBlit.left + pointClientScreen.x : rectBlit.left, 
						m_tabData->tabBackground->bRelative ? rectBlit.top + pointClientScreen.y : rectBlit.top, 
						SRCCOPY);

		// TransparentBlt seems to fail for small rectangles, so we blit entire window here
		m_dcOffscreen.TransparentBlt(
						rectWindow.left, 
						rectWindow.top, 
						rectWindow.right, 
						rectWindow.bottom, 
						m_dcText, 
						rectWindow.left, 
						rectWindow.top, 
						rectWindow.right, 
						rectWindow.bottom, 
						m_tabData->crBackgroundColor);

/*
		BOOL b = m_dcOffscreen.TransparentBlt(
						dc.m_ps.rcPaint.left, 
						dc.m_ps.rcPaint.top, 
						dc.m_ps.rcPaint.right, 
						dc.m_ps.rcPaint.bottom, 
						m_dcText, 
						dc.m_ps.rcPaint.left, 
						dc.m_ps.rcPaint.top, 
						dc.m_ps.rcPaint.right, 
						dc.m_ps.rcPaint.bottom, 
						m_tabData->crBackgroundColor);

		TRACE(L"B: %i, %i\n", b ? 1 : 0, ::GetLastError());
*/

	} else {
		
		m_dcOffscreen.BitBlt(
						rectBlit.left, 
						rectBlit.top, 
						rectBlit.right, 
						rectBlit.bottom, 
						m_dcText, 
						rectBlit.left, 
						rectBlit.top, 
						SRCCOPY);
	}

	// blit cursor
	if (m_consoleHandler.GetCursorInfo()->bVisible && (m_cursor.get() != NULL)) {
		CRect	rectCursor(0, 0, 0, 0);
		SharedMemory<CONSOLE_SCREEN_BUFFER_INFO>& consoleInfo = m_consoleHandler.GetConsoleInfo();

		rectCursor			= m_cursor->GetCursorRect();
		rectCursor.left		+= (consoleInfo->dwCursorPosition.X - consoleInfo->srWindow.Left) * m_nCharWidth + m_nInsideBorder;
		rectCursor.top		+= (consoleInfo->dwCursorPosition.Y - consoleInfo->srWindow.Top) * m_nCharHeight + m_nInsideBorder;
		rectCursor.right	+= (consoleInfo->dwCursorPosition.X - consoleInfo->srWindow.Left) * m_nCharWidth + m_nInsideBorder;
		rectCursor.bottom	+= (consoleInfo->dwCursorPosition.Y - consoleInfo->srWindow.Top) * m_nCharHeight + m_nInsideBorder;

		m_cursor->BitBlt(
					m_dcOffscreen, 
					rectCursor.left, 
					rectCursor.top);
	}

	// blit selection
	m_selectionHandler->BitBlt(m_dcOffscreen);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleView::SendTextToConsole(const wchar_t* pszText) {

	if (!pszText || (wcslen(pszText) == 0)) return;

	void* pRemoteMemory = ::VirtualAllocEx(
								m_consoleHandler.GetConsoleHandle().get(),
								NULL, 
								(wcslen(pszText)+1)*sizeof(wchar_t), 
								MEM_COMMIT, 
								PAGE_READWRITE);

	if (pRemoteMemory == NULL) return;

	if (!::WriteProcessMemory(
				m_consoleHandler.GetConsoleHandle().get(),
				pRemoteMemory, 
				(PVOID)pszText, 
				(wcslen(pszText)+1)*sizeof(wchar_t), 
				NULL)) {

		::VirtualFreeEx(m_consoleHandler.GetConsoleHandle().get(), pRemoteMemory, NULL, MEM_RELEASE);
		return;
	}

	m_consoleHandler.GetConsolePasteInfo() = reinterpret_cast<UINT_PTR>(pRemoteMemory);
	m_consoleHandler.GetConsolePasteInfo().SetEvent();
}

/////////////////////////////////////////////////////////////////////////////

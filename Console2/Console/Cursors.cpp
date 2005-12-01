/////////////////////////////////////////////////////////////////////////////
// Cursors.cpp - cursor classes

#include "stdafx.h"
#include "Cursors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// XTermCursor

XTermCursor::XTermCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor)
: Cursor(hwndConsoleView, cursorDC, cursorRect, crCursorColor)
{
	m_paintBrush.CreateSolidBrush(crCursorColor);
	cursorDC.SetBkColor(RGB(0, 0, 0));
}

XTermCursor::~XTermCursor() {
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void XTermCursor::Draw(bool bActive) {

	if (bActive) {
		m_cursorDC.FillRect(&m_cursorRect, m_paintBrush);
	} else {
		m_cursorDC.FillRect(&m_cursorRect, m_backgroundBrush);
		m_cursorDC.FrameRect(&m_cursorRect, m_paintBrush);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void XTermCursor::BitBlt(CDC& offscreenDC, int x, int y) {

	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_cursorRect.right - m_cursorRect.left, 
					m_cursorRect.bottom - m_cursorRect.top, 
					m_cursorDC, 
					0, 
					0, 
					m_cursorRect.right - m_cursorRect.left, 
					m_cursorRect.bottom - m_cursorRect.top, 
					RGB(0, 0, 0));

}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// BlockCursor

BlockCursor::BlockCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor)
: Cursor(hwndConsoleView, cursorDC, cursorRect, crCursorColor)
, m_bVisible(TRUE)
{
	m_paintBrush.CreateSolidBrush(crCursorColor);
	cursorDC.SetBkColor(RGB(0, 0, 0));

	m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, 750, NULL);
}

BlockCursor::~BlockCursor() {

	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void BlockCursor::Draw(bool bActive) {
	
	if (bActive && m_bVisible) {
		m_cursorDC.FillRect(&m_cursorRect, m_paintBrush);
	} else {
		m_cursorDC.FillRect(&m_cursorRect, m_backgroundBrush);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void BlockCursor::BitBlt(CDC& offscreenDC, int x, int y) {

	if (!m_bVisible) return;

	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_cursorRect.right - m_cursorRect.left, 
					m_cursorRect.bottom - m_cursorRect.top, 
					m_cursorDC, 
					0, 
					0, 
					m_cursorRect.right - m_cursorRect.left, 
					m_cursorRect.bottom - m_cursorRect.top, 
					RGB(0, 0, 0));
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void BlockCursor::PrepareNext() {

	m_bVisible = !m_bVisible;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// NBBlockCursor

NBBlockCursor::NBBlockCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor)
: Cursor(hwndConsoleView, cursorDC, cursorRect, crCursorColor)
{
}

NBBlockCursor::~NBBlockCursor() {
	
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void NBBlockCursor::Draw(bool /*bActive*/) {
	
	m_cursorDC.FillRect(&m_cursorRect, m_paintBrush);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void NBBlockCursor::BitBlt(CDC& offscreenDC, int x, int y) {

	offscreenDC.BitBlt(
		x, 
		y, 
		m_cursorRect.right - m_cursorRect.left, 
		m_cursorRect.bottom - m_cursorRect.top, 
		m_cursorDC, 
		0, 
		0, 
		SRCCOPY);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// PulseBlockCursor

PulseBlockCursor::PulseBlockCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor)
: Cursor(hwndConsoleView, cursorDC, cursorRect, crCursorColor)
, m_nSize(0)
, m_nMaxSize(0)
, m_nStep(0)
{
	m_uiTimer = ::SetTimer(m_hwndConsoleView, CURSOR_TIMER, 100, NULL);
}

PulseBlockCursor::~PulseBlockCursor() {
	
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void PulseBlockCursor::Draw(bool bActive) {
	
	// this is called only once, to set the size of the cursor
	if (m_nMaxSize == 0) {
		if ((m_cursorRect.right - m_cursorRect.left) < (m_cursorRect.bottom - m_cursorRect.top)) {
			m_nMaxSize = (m_cursorRect.right - m_cursorRect.left) >> 1;
		} else {
			m_nMaxSize = (m_cursorRect.bottom - m_cursorRect.top) >> 1;
		}
	}
	
	if (bActive) {
		RECT rect;
		::CopyMemory(&rect, &m_cursorRect, sizeof(CRect));
		rect.left	+= m_nSize;
		rect.top	+= m_nSize;
		rect.right	-= m_nSize;
		rect.bottom	-= m_nSize;

		m_cursorDC.FillRect(&m_cursorRect, m_backgroundBrush);
		m_cursorDC.FillRect(&rect, m_paintBrush);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void PulseBlockCursor::BitBlt(CDC& offscreenDC, int x, int y) {

	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_cursorRect.right - m_cursorRect.left, 
					m_cursorRect.bottom - m_cursorRect.top, 
					m_cursorDC, 
					0, 
					0, 
					m_cursorRect.right - m_cursorRect.left, 
					m_cursorRect.bottom - m_cursorRect.top, 
					RGB(0, 0, 0));
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void PulseBlockCursor::PrepareNext() {
	
	if (m_nSize == 0) {
		m_nStep = 1;
	} else if (m_nSize == m_nMaxSize) {
		m_nStep = -1;
	}
	
	m_nSize += m_nStep;

	RECT rect;
	::CopyMemory(&rect, &m_cursorRect, sizeof(CRect));
	rect.left	+= m_nSize;
	rect.top	+= m_nSize;
	rect.right	-= m_nSize;
	rect.bottom	-= m_nSize;

	m_cursorDC.FillRect(&m_cursorRect, m_backgroundBrush);
	m_cursorDC.FillRect(&rect, m_paintBrush);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
/////////////////////////////////////////////////////////////////////////////
// BarCursor

BarCursor::BarCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hPen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_bVisible(TRUE)
{
	m_uiTimer = ::SetTimer(hwndParent, CURSOR_TIMER, 750, NULL);
}

BarCursor::~BarCursor() {
	
	if (m_uiTimer) ::KillTimer(m_hwndParent, m_uiTimer);
	
	::DeleteObject(m_hPen);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void BarCursor::Draw(LPRECT pRect) {
	
	if (m_bActive && m_bVisible) {
		HPEN hOldPen = (HPEN)::SelectObject(m_hdcWindow, m_hPen);
		::MoveToEx(m_hdcWindow, pRect->left, pRect->top, NULL);
		::LineTo(m_hdcWindow, pRect->left, pRect->bottom);
		::SelectObject(m_hdcWindow, hOldPen);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void BarCursor::PrepareNext() {
	
	m_bVisible = !m_bVisible;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// ConsoleCursor

ConsoleCursor::ConsoleCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hStdOut(::CreateFile(_T("CONOUT$"), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0))
, m_hActiveBrush(::CreateSolidBrush(crCursorColor))
, m_bVisible(TRUE)
{
	m_uiTimer = ::SetTimer(hwndParent, CURSOR_TIMER, 750, NULL);
}

ConsoleCursor::~ConsoleCursor() {
	
	if (m_uiTimer) ::KillTimer(m_hwndParent, m_uiTimer);
	::DeleteObject(m_hActiveBrush);
	::CloseHandle(m_hStdOut);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleCursor::Draw(LPRECT pRect) {
	
	RECT rect;

	::CopyMemory(&rect, pRect, sizeof(RECT));

	if (m_bActive && m_bVisible) {

		CONSOLE_CURSOR_INFO	csi;
		::GetConsoleCursorInfo(m_hStdOut, &csi);

		rect.top += (rect.bottom - rect.top) * (100-csi.dwSize)/100;
		::FillRect(m_hdcWindow, &rect, m_hActiveBrush);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void ConsoleCursor::PrepareNext() {
	
	m_bVisible = !m_bVisible;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// NBHLineCursor

NBHLineCursor::NBHLineCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hPen(::CreatePen(PS_SOLID, 1, crCursorColor))
{
}

NBHLineCursor::~NBHLineCursor() {
	
	::DeleteObject(m_hPen);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void NBHLineCursor::Draw(LPRECT pRect) {
	
	HPEN hOldPen = (HPEN)::SelectObject(m_hdcWindow, m_hPen);
	::MoveToEx(m_hdcWindow, pRect->left, pRect->bottom-1, NULL);
	::LineTo(m_hdcWindow, pRect->right, pRect->bottom-1);
	::SelectObject(m_hdcWindow, hOldPen);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void NBHLineCursor::PrepareNext() {
	
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// HLineCursor

HLineCursor::HLineCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hPen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_nSize(0)
, m_nPosition(0)
, m_nStep(0)
{
	m_uiTimer = ::SetTimer(hwndParent, CURSOR_TIMER, 100, NULL);
}

HLineCursor::~HLineCursor() {
	
	if (m_uiTimer) ::KillTimer(m_hwndParent, m_uiTimer);
	
	::DeleteObject(m_hPen);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void HLineCursor::Draw(LPRECT pRect) {

	// this is called only once, to set the size of the cursor
	if (m_nSize != (pRect->bottom - pRect->top - 1)) {
		m_nSize = pRect->bottom - pRect->top - 1;
		m_nPosition = 0;
		m_nStep = 1;
	}

	if (m_bActive) {
		HPEN hOldPen = (HPEN)::SelectObject(m_hdcWindow, m_hPen);
		::MoveToEx(m_hdcWindow, pRect->left, pRect->top + m_nPosition, NULL);
		::LineTo(m_hdcWindow, pRect->right, pRect->top + m_nPosition);
		::SelectObject(m_hdcWindow, hOldPen);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void HLineCursor::PrepareNext() {

	if (m_nPosition == 0) {
		m_nStep = 1;
	} else if (m_nPosition == m_nSize) {
		m_nStep = -1;
	}

	m_nPosition += m_nStep;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// VLineCursor

VLineCursor::VLineCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hPen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_nSize(0)
, m_nPosition(0)
, m_nStep(0)
{
	m_uiTimer = ::SetTimer(hwndParent, CURSOR_TIMER, 100, NULL);
}

VLineCursor::~VLineCursor() {
	
	if (m_uiTimer) ::KillTimer(m_hwndParent, m_uiTimer);
	
	::DeleteObject(m_hPen);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void VLineCursor::Draw(LPRECT pRect) {
	
	// this is called only once, to set the size of the cursor
	if (m_nSize != (pRect->right - pRect->left - 1)) {
		m_nSize = pRect->right - pRect->left - 1;
		m_nPosition = 0;
		m_nStep = 1;
	}
	
	if (m_bActive) {
		HPEN hOldPen = (HPEN)::SelectObject(m_hdcWindow, m_hPen);
		::MoveToEx(m_hdcWindow, pRect->left + m_nPosition, pRect->top, NULL);
		::LineTo(m_hdcWindow, pRect->left + m_nPosition, pRect->bottom);
		::SelectObject(m_hdcWindow, hOldPen);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void VLineCursor::PrepareNext() {
	
	if (m_nPosition == 0) {
		m_nStep = 1;
	} else if (m_nPosition == m_nSize) {
		m_nStep = -1;
	}
	
	m_nPosition += m_nStep;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// RectCursor

RectCursor::RectCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hActiveBrush(::CreateSolidBrush(crCursorColor))
, m_bVisible(TRUE)
{
	m_uiTimer = ::SetTimer(hwndParent, CURSOR_TIMER, 750, NULL);
}

RectCursor::~RectCursor() {
	
	if (m_uiTimer) ::KillTimer(m_hwndParent, m_uiTimer);
	::DeleteObject(m_hActiveBrush);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void RectCursor::Draw(LPRECT pRect) {
	
	if (m_bActive && m_bVisible) {
		
		::FrameRect(m_hdcWindow, pRect, m_hActiveBrush);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void RectCursor::PrepareNext() {
	
	m_bVisible = !m_bVisible;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// NBRectCursor

NBRectCursor::NBRectCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hActiveBrush(::CreateSolidBrush(crCursorColor))
{
}

NBRectCursor::~NBRectCursor() {
	
	::DeleteObject(m_hActiveBrush);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void NBRectCursor::Draw(LPRECT pRect) {
	
	::FrameRect(m_hdcWindow, pRect, m_hActiveBrush);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void NBRectCursor::PrepareNext() {
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// PulseRectCursor

PulseRectCursor::PulseRectCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hActiveBrush(::CreateSolidBrush(crCursorColor))
, m_nSize(0)
, m_nMaxSize(0)
, m_nStep(0)
{
	::ZeroMemory(&m_rect, sizeof(RECT));
	m_uiTimer = ::SetTimer(hwndParent, CURSOR_TIMER, 100, NULL);
}

PulseRectCursor::~PulseRectCursor() {
	
	if (m_uiTimer) ::KillTimer(m_hwndParent, m_uiTimer);
	::DeleteObject(m_hActiveBrush);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void PulseRectCursor::Draw(LPRECT pRect) {
	
	// this is called only once, to set the size of the cursor
	if (m_nMaxSize == 0) {
		if ((pRect->right - pRect->left) < (pRect->bottom - pRect->top)) {
			m_nMaxSize = (pRect->right - pRect->left) >> 1;
		} else {
			m_nMaxSize = (pRect->bottom - pRect->top) >> 1;
		}
	}

	if (m_bActive) {
		RECT rect;
		::CopyMemory(&rect, pRect, sizeof(RECT));
		rect.left	+= m_nSize;
		rect.top	+= m_nSize;
		rect.right	-= m_nSize;
		rect.bottom	-= m_nSize;
		::FrameRect(m_hdcWindow, &rect, m_hActiveBrush);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void PulseRectCursor::PrepareNext() {
	
	if (m_nSize == 0) {
		m_nStep = 1;
	} else if (m_nSize == m_nMaxSize) {
		m_nStep = -1;
	}
	
	m_nSize += m_nStep;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// FadeBlockCursor

FadeBlockCursor::FadeBlockCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor, COLORREF crBkColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_crCursorColor(crCursorColor)
, m_nStep(1)
, m_crBkColor(crBkColor)
, m_nIndex(0)
, m_hUser32(NULL)
, m_hMemDC(NULL)
{
	m_uiTimer = ::SetTimer(hwndParent, CURSOR_TIMER, 35, NULL);
	
	if (g_bWin2000) {
		// on Win2000 we use real alpha blending

		// create a reasonable-sized bitmap, since AlphaBlt resizes 
		// destination rect if needed, and we don't need to redraw the mem DC 
		// each time
		m_nBmpWidth	= BLEND_BMP_WIDTH;
		m_nBmpHeight= BLEND_BMP_HEIGHT;
		m_hMemDC	= ::CreateCompatibleDC(hdcWindow);
		m_hBmp		= ::CreateCompatibleBitmap(hdcWindow, m_nBmpWidth, m_nBmpHeight);
		m_hBmpOld	= (HBITMAP)::SelectObject(m_hMemDC, m_hBmp);

		HBRUSH	hBrush= ::CreateSolidBrush(m_crCursorColor);
		RECT	rect;
		rect.left	= 0;
		rect.top	= 0;
		rect.right	= m_nBmpWidth;
		rect.bottom	= m_nBmpHeight;

		::FillRect(m_hMemDC, &rect, hBrush);
		::DeleteObject(hBrush);

		m_nStep	= -ALPHA_STEP;

		m_bfn.BlendOp				= AC_SRC_OVER;
		m_bfn.BlendFlags			= 0;
		m_bfn.SourceConstantAlpha	= 255;
		m_bfn.AlphaFormat			= 0;

	} else {
		FakeBlend();
	}
}

FadeBlockCursor::~FadeBlockCursor() {
	
	if (m_uiTimer) ::KillTimer(m_hwndParent, m_uiTimer);

	if (g_bWin2000) {
		::SelectObject(m_hMemDC, m_hBmpOld);
		::DeleteObject(m_hBmp);
		::DeleteDC(m_hMemDC);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void FadeBlockCursor::Draw(LPRECT pRect) {
	
	if (g_bWin2000) {
		
		g_pfnAlphaBlend(
			m_hdcWindow,
			pRect->left,
			pRect->top,
			pRect->right - pRect->left,
			pRect->bottom - pRect->top,
			m_hMemDC,
			0,
			0,
			BLEND_BMP_WIDTH,
			BLEND_BMP_HEIGHT,
			m_bfn);
		
	} else {

		HBRUSH hBrush = ::CreateSolidBrush(m_arrColors[m_nIndex]);
		::FillRect(m_hdcWindow, pRect, hBrush);
		::DeleteObject(hBrush);
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

void FadeBlockCursor::PrepareNext() {

	if (g_bWin2000){
		if (m_bfn.SourceConstantAlpha < ALPHA_STEP) {
			m_nStep = ALPHA_STEP;
		} else if ((DWORD)m_bfn.SourceConstantAlpha + ALPHA_STEP > 255) {
			m_nStep = -ALPHA_STEP;
		}
		
		m_bfn.SourceConstantAlpha += m_nStep;
	} else {
		if (m_nIndex == 0) {
			m_nStep = 1;
		} else if (m_nIndex == (FADE_STEPS)) {
			m_nStep = -1;
		}
		m_nIndex += m_nStep;
	}
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// This function is used to create a fake blending for WinNT (uuseful only 
// for solid background, though)

void FadeBlockCursor::FakeBlend() {
	
	int nDeltaR = ((GetRValue(m_crCursorColor) - GetRValue(m_crBkColor)) << 8) / FADE_STEPS;
	int nDeltaG = ((GetGValue(m_crCursorColor) - GetGValue(m_crBkColor)) << 8) / FADE_STEPS;
	int nDeltaB = ((GetBValue(m_crCursorColor) - GetBValue(m_crBkColor)) << 8) / FADE_STEPS;
	
	for (int i = 0; i < FADE_STEPS; ++i) {
		m_arrColors[i] = RGB(GetRValue(m_crCursorColor) - (nDeltaR*i >> 8), GetGValue(m_crCursorColor) - (nDeltaG*i >> 8), GetBValue(m_crCursorColor) - (nDeltaB*i >> 8));
	}
	
	m_arrColors[FADE_STEPS] = m_crBkColor;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

*/
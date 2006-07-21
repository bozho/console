//////////////////////////////////////////////////////////////////////////////
// Cursors.cpp - cursor classes

#include "stdafx.h"
#include "Cursors.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
*/

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wchar_t* Cursor::s_cursorNames[] =
{
	L"XTerm",
	L"Block",
	L"Nonblinking block",
	L"Pulse",
	L"Bar",
//	L"Console",
	L"Nonblinking horz. line",
	L"Horizontal line",
	L"Vertical line",
	L"Rect",
	L"Nonblinking rect",
	L"Pulsating rect",
	L"Fading block"
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

shared_ptr<Cursor> CursorFactory::CreateCursor(HWND hwndConsoleView, bool bAppActive, CursorStyle cursorStyle, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
{
	shared_ptr<Cursor> newCursor;

	switch (cursorStyle)
	{
		case cstyleXTerm :
			newCursor.reset(dynamic_cast<Cursor*>(new XTermCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstyleBlock :
			newCursor.reset(dynamic_cast<Cursor*>(new BlockCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstyleNBBlock :
			newCursor.reset(dynamic_cast<Cursor*>(new NBBlockCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstylePulseBlock :
			newCursor.reset(dynamic_cast<Cursor*>(new PulseBlockCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstyleBar :
			newCursor.reset(dynamic_cast<Cursor*>(new BarCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;
/*		case cstyleConsole :
			m_pCursor = (Cursor*)new ConsoleCursor(m_hWnd, m_hdcConsole, m_crCursorColor);
			break; */

		case cstyleNBHline :
			newCursor.reset(dynamic_cast<Cursor*>(new NBHLineCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstyleHLine :
			newCursor.reset(dynamic_cast<Cursor*>(new HLineCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstyleVLine :
			newCursor.reset(dynamic_cast<Cursor*>(new VLineCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstyleRect :
			newCursor.reset(dynamic_cast<Cursor*>(new RectCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstyleNBRect :
			newCursor.reset(dynamic_cast<Cursor*>(new NBRectCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstylePulseRect :
			newCursor.reset(dynamic_cast<Cursor*>(new PulseRectCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		case cstyleFadeBlock :
			newCursor.reset(dynamic_cast<Cursor*>(new FadeBlockCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
			break;

		default :
			newCursor.reset(dynamic_cast<Cursor*>(new NBBlockCursor(
															hwndConsoleView, 
															dcConsoleView, 
															rectCursor, 
															crCursorColor)));
	}

	if (newCursor.get() != NULL) newCursor->Draw(bAppActive);
	return newCursor;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// XTermCursor

XTermCursor::XTermCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
{
}

XTermCursor::~XTermCursor()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XTermCursor::Draw(bool bActive /* = true */)
{
	if (bActive)
	{
		m_dcCursor.FillRect(&m_rectCursor, m_paintBrush);
	}
	else
	{
		m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
		m_dcCursor.FrameRect(&m_rectCursor, m_paintBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XTermCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.BitBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					SRCINVERT);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// BlockCursor

BlockCursor::BlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_bVisible(true)
{
	m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, 750, NULL);
}

BlockCursor::~BlockCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void BlockCursor::Draw(bool bActive /* = true */)
{
	if (bActive && m_bVisible)
	{
		m_dcCursor.FillRect(&m_rectCursor, m_paintBrush);
	}
	else
	{
		m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void BlockCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	if (!m_bVisible) return;

	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void BlockCursor::PrepareNext()
{
	m_bVisible = !m_bVisible;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// NBBlockCursor

NBBlockCursor::NBBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
{
}

NBBlockCursor::~NBBlockCursor()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBBlockCursor::Draw(bool /*bActive = true */)
{
	m_dcCursor.FillRect(&m_rectCursor, m_paintBrush);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBBlockCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.BitBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					SRCCOPY);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseBlockCursor

PulseBlockCursor::PulseBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_nSize(0)
, m_nMaxSize(0)
, m_nStep(0)
{
	// set the max size of the cursor
	if ((m_rectCursor.right - m_rectCursor.left) < (m_rectCursor.bottom - m_rectCursor.top))
	{
		m_nMaxSize = (m_rectCursor.right - m_rectCursor.left) >> 1;
	}
	else
	{
		m_nMaxSize = (m_rectCursor.bottom - m_rectCursor.top) >> 1;
	}

	m_uiTimer = ::SetTimer(m_hwndConsoleView, CURSOR_TIMER, 100, NULL);
}

PulseBlockCursor::~PulseBlockCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PulseBlockCursor::Draw(bool bActive /* = true */)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);

	if (bActive)
	{
		CRect rect;
		::CopyMemory(&rect, &m_rectCursor, sizeof(CRect));
		rect.left	+= m_nSize;
		rect.top	+= m_nSize;
		rect.right	-= m_nSize;
		rect.bottom	-= m_nSize;

		m_dcCursor.FillRect(&rect, m_paintBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PulseBlockCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PulseBlockCursor::PrepareNext()
{
	if (m_nSize == 0)
	{
		m_nStep = 1;
	}
	else if (m_nSize == m_nMaxSize)
	{
		m_nStep = -1;
	}
	
	m_nSize += m_nStep;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// BarCursor

BarCursor::BarCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_bVisible(true)
{
	m_dcCursor.SelectPen(m_pen);

	m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, 750, NULL);
}

BarCursor::~BarCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void BarCursor::Draw(bool bActive /* = true */)
{
	if (bActive && m_bVisible)
	{
		m_dcCursor.MoveTo(m_rectCursor.left, m_rectCursor.top + 2, NULL);
		m_dcCursor.LineTo(m_rectCursor.left, m_rectCursor.bottom - 2);
	}
	else
	{
		m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void BarCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void BarCursor::PrepareNext()
{
	m_bVisible = !m_bVisible;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*
//////////////////////////////////////////////////////////////////////////////
// ConsoleCursor

ConsoleCursor::ConsoleCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor)
: Cursor(hwndParent, hdcWindow, crCursorColor)
, m_hStdOut(::CreateFile(_T("CONOUT$"), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0))
, m_hActiveBrush(::CreateSolidBrush(crCursorColor))
, m_bVisible(TRUE)
{
	m_uiTimer = ::SetTimer(hwndParent, CURSOR_TIMER, 750, NULL);
}

ConsoleCursor::~ConsoleCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndParent, m_uiTimer);
	::DeleteObject(m_hActiveBrush);
	::CloseHandle(m_hStdOut);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleCursor::Draw(LPCRect pRect)
{
	CRect rect;

	::CopyMemory(&rect, pRect, sizeof(CRect));

	if (m_bActive && m_bVisible)
	{
		CONSOLE_CURSOR_INFO	csi;
		::GetConsoleCursorInfo(m_hStdOut, &csi);

		rect.top += (rect.bottom - rect.top) * (100-csi.dwSize)/100;
		::FillRect(m_hdcWindow, &rect, m_hActiveBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleCursor::PrepareNext()
{
	m_bVisible = !m_bVisible;
}

//////////////////////////////////////////////////////////////////////////////
*/

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// NBHLineCursor

NBHLineCursor::NBHLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pen(::CreatePen(PS_SOLID, 1, crCursorColor))
{
	m_dcCursor.SelectPen(m_pen);
}

NBHLineCursor::~NBHLineCursor()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBHLineCursor::Draw(bool /*bActive = true */)
{
	m_dcCursor.MoveTo(m_rectCursor.left, m_rectCursor.bottom - 1, NULL);
	m_dcCursor.LineTo(m_rectCursor.right, m_rectCursor.bottom - 1);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBHLineCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// HLineCursor

HLineCursor::HLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_nSize(0)
, m_nPosition(0)
, m_nStep(0)
{
	// set the size of the cursor
	if (m_nSize != (m_rectCursor.bottom - m_rectCursor.top - 1))
	{
		m_nSize = m_rectCursor.bottom - m_rectCursor.top - 1;
		m_nPosition = 0;
		m_nStep = 1;
	}

	m_dcCursor.SelectPen(m_pen);

	m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, 100, NULL);
}

HLineCursor::~HLineCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void HLineCursor::Draw(bool bActive /* = true */)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);

	if (bActive)
	{
		m_dcCursor.MoveTo(m_rectCursor.left, m_rectCursor.top + m_nPosition, NULL);
		m_dcCursor.LineTo(m_rectCursor.right, m_rectCursor.top + m_nPosition);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void HLineCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void HLineCursor::PrepareNext()
{
	if (m_nPosition == 0)
	{
		m_nStep = 1;
	}
	else if (m_nPosition == m_nSize)
	{
		m_nStep = -1;
	}

	m_nPosition += m_nStep;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// VLineCursor

VLineCursor::VLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_nSize(0)
, m_nPosition(0)
, m_nStep(0)
{
	// set the size of the cursor
	if (m_nSize != (m_rectCursor.Width() - 1))
	{
		m_nSize = m_rectCursor.Width() - 1;
		m_nPosition = 0;
		m_nStep = 1;
	}

	m_dcCursor.SelectPen(m_pen);

	m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, 100, NULL);
}

VLineCursor::~VLineCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void VLineCursor::Draw(bool bActive /* = true */)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);

	if (bActive)
	{
		m_dcCursor.MoveTo(m_rectCursor.left + m_nPosition, m_rectCursor.top + 2, NULL);
		m_dcCursor.LineTo(m_rectCursor.left + m_nPosition, m_rectCursor.bottom - 2);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void VLineCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void VLineCursor::PrepareNext()
{
	if (m_nPosition == 0)
	{
		m_nStep = 1;
	}
	else if (m_nPosition == m_nSize)
	{
		m_nStep = -1;
	}
	
	m_nPosition += m_nStep;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// RectCursor

RectCursor::RectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_bVisible(true)
{
	m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, 750, NULL);
}

RectCursor::~RectCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void RectCursor::Draw(bool bActive /* = true */)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);

	if (bActive && m_bVisible)
	{
		m_dcCursor.FrameRect(&m_rectCursor, m_paintBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void RectCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void RectCursor::PrepareNext()
{
	m_bVisible = !m_bVisible;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// NBRectCursor

NBRectCursor::NBRectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
{
}

NBRectCursor::~NBRectCursor()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBRectCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBRectCursor::Draw(bool /* bActive = true */)
{
	m_dcCursor.FrameRect(&m_rectCursor, m_paintBrush);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseRectCursor

PulseRectCursor::PulseRectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_nSize(0)
, m_nMaxSize(0)
, m_nStep(0)
{
	// set the size of the cursor
	if ((m_rectCursor.right - m_rectCursor.left) < (m_rectCursor.bottom - m_rectCursor.top))
	{
		m_nMaxSize = (m_rectCursor.right - m_rectCursor.left) >> 1;
	}
	else
	{
		m_nMaxSize = (m_rectCursor.bottom - m_rectCursor.top) >> 1;
	}

	m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, 100, NULL);
}

PulseRectCursor::~PulseRectCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PulseRectCursor::Draw(bool bActive /* = true */)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);

	if (bActive)
	{
		CRect rect;
		::CopyMemory(&rect, m_rectCursor, sizeof(CRect));
		rect.left	+= m_nSize;
		rect.top	+= m_nSize;
		rect.right	-= m_nSize;
		rect.bottom	-= m_nSize;
		m_dcCursor.FrameRect(&rect, m_paintBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PulseRectCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.TransparentBlt(
					x, 
					y, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor, 
					0, 
					0, 
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					RGB(0, 0, 0));
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PulseRectCursor::PrepareNext()
{
	if (m_nSize == 0)
	{
		m_nStep = 1;
	}
	else if (m_nSize == m_nMaxSize)
	{
		m_nStep = -1;
	}
	
	m_nSize += m_nStep;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// FadeBlockCursor

FadeBlockCursor::FadeBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_nStep(-ALPHA_STEP)
{
	m_blendFunction.BlendOp				= AC_SRC_OVER;
	m_blendFunction.BlendFlags			= 0;
	m_blendFunction.SourceConstantAlpha	= 255;
	m_blendFunction.AlphaFormat			= 0;

	m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, 35, NULL);
}

FadeBlockCursor::~FadeBlockCursor()
{
	if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void FadeBlockCursor::Draw(bool bActive /* = true */)
{
	m_dcCursor.FillRect(&m_rectCursor, m_paintBrush);
/*
	if (bActive)
	{
		m_dcCursor.FillRect(&m_rectCursor, m_paintBrush);
	}
	else
	{
		m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	}
*/
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void FadeBlockCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	offscreenDC.AlphaBlend(
					x,
					y,
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_dcCursor,
					0,
					0,
					m_rectCursor.Width(),
					m_rectCursor.Height(),
					m_blendFunction);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void FadeBlockCursor::PrepareNext()
{
	if (m_blendFunction.SourceConstantAlpha < ALPHA_STEP)
	{
		m_nStep = ALPHA_STEP;
	}
	else if (m_blendFunction.SourceConstantAlpha + ALPHA_STEP > 255)
	{
		m_nStep = -ALPHA_STEP;
	}

#pragma warning(push)
#pragma warning(disable: 4244)
	m_blendFunction.SourceConstantAlpha += m_nStep;
#pragma warning(pop)
}

//////////////////////////////////////////////////////////////////////////////


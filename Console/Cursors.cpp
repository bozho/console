//////////////////////////////////////////////////////////////////////////////
// Cursors.cpp - cursor classes

#include "stdafx.h"
#include "Cursors.h"


//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Cursor> CursorFactory::CreateCursor(HWND hwndConsoleView, bool bAppActive, CursorStyle cursorStyle, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, CursorCharDrawer* pdrawer, bool bTimer)
{
	std::shared_ptr<Cursor> newCursor;

	switch (cursorStyle)
	{
		case cstyleXTerm :
			newCursor.reset(dynamic_cast<Cursor*>(new XTermCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															pdrawer)));
			break;

		case cstyleXTerm2 :
			newCursor.reset(dynamic_cast<Cursor*>(new XTerm2Cursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															pdrawer,
															bTimer)));
			break;

		case cstyleBlock :
			newCursor.reset(dynamic_cast<Cursor*>(new BlockCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															bTimer)));
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
															crCursorColor,
															bTimer)));
			break;

		case cstyleBar :
			newCursor.reset(dynamic_cast<Cursor*>(new BarCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															bTimer)));
			break;

		case cstyleHBar :
			newCursor.reset(dynamic_cast<Cursor*>(new HBarCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															bTimer)));
			break;

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
															crCursorColor,
															bTimer)));
			break;

		case cstyleVLine :
			newCursor.reset(dynamic_cast<Cursor*>(new VLineCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															bTimer)));
			break;

		case cstyleRect :
			newCursor.reset(dynamic_cast<Cursor*>(new RectCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															bTimer)));
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
															crCursorColor, bTimer)));
			break;

		case cstyleFadeBlock :
			newCursor.reset(dynamic_cast<Cursor*>(new FadeBlockCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															bTimer)));
			break;

		case cstyleConsole :
			newCursor.reset(dynamic_cast<Cursor*>(new ConsoleCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor,
															bTimer)));
			break;

		default :
			newCursor.reset(dynamic_cast<Cursor*>(new NBBlockCursor(
															hwndConsoleView,
															dcConsoleView,
															rectCursor,
															crCursorColor)));
	}

	if (newCursor.get() != NULL) newCursor->Draw(bAppActive, 40);
	return newCursor;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// XTermCursor

XTermCursor::XTermCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, CursorCharDrawer* pdrawer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pdrawer(pdrawer)
, m_bActive(false)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XTermCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	m_dcCursor.FrameRect(&m_rectCursor, m_paintBrush);
	m_bActive = bActive;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XTermCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	if( m_bActive )
	{
		m_pdrawer->RedrawCharOnCursor(offscreenDC);
	}
	else
	{
		Cursor::BitBlt(offscreenDC, x, y);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// XTerm2Cursor

XTerm2Cursor::XTerm2Cursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, CursorCharDrawer* pdrawer, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pdrawer(pdrawer)
, m_bActive(false)
, m_bVisible(true)
{
	UINT uiRate = ::GetCaretBlinkTime();
	if (uiRate == 0) uiRate = 500;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XTerm2Cursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	m_dcCursor.FrameRect(&m_rectCursor, m_paintBrush);
	m_bActive = bActive;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XTerm2Cursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	if( m_bActive )
	{
		if( !m_bVisible ) return;
		m_pdrawer->RedrawCharOnCursor(offscreenDC);
	}
	else
	{
		Cursor::BitBlt(offscreenDC, x, y);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XTerm2Cursor::PrepareNext()
{
	m_bVisible = !m_bVisible;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// BlockCursor

BlockCursor::BlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_bVisible(true)
{
	UINT uiRate = ::GetCaretBlinkTime();
	if (uiRate == 0) uiRate = 500;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void BlockCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
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

	Cursor::BitBlt(offscreenDC, x, y);
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

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBBlockCursor::Draw(bool /*bActive*/, DWORD /*dwCursorSize*/)
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

PulseBlockCursor::PulseBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
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

	UINT uiRate = ::GetCaretBlinkTime() / static_cast<UINT>(2*m_nMaxSize);
	if (uiRate < 50) uiRate = 50;

	if( bTimer )
		m_uiTimer = ::SetTimer(m_hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PulseBlockCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
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

BarCursor::BarCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_bVisible(true)
{
	m_dcCursor.SelectPen(m_pen);

	UINT uiRate = ::GetCaretBlinkTime();
	if (uiRate == 0) uiRate = 500;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void BarCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
{
	if (bActive && m_bVisible)
	{
		m_dcCursor.MoveTo(m_rectCursor.left, m_rectCursor.top + 1, NULL);
		m_dcCursor.LineTo(m_rectCursor.left, m_rectCursor.bottom);
	}
	else
	{
		m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	}
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


//////////////////////////////////////////////////////////////////////////////
// HBarCursor

HBarCursor::HBarCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_bVisible(true)
{
	m_dcCursor.SelectPen(m_pen);

	UINT uiRate = ::GetCaretBlinkTime();
	if (uiRate == 0) uiRate = 500;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void HBarCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
{
	if (bActive && m_bVisible)
	{
		m_dcCursor.MoveTo(m_rectCursor.left, m_rectCursor.bottom - 1, NULL);
		m_dcCursor.LineTo(m_rectCursor.right, m_rectCursor.bottom - 1);
	}
	else
	{
		m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void HBarCursor::PrepareNext()
{
	m_bVisible = !m_bVisible;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// ConsoleCursor

ConsoleCursor::ConsoleCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_bVisible(true)
{
	// we reset colors with black and white only!
	m_paintBrush.Attach(::CreateSolidBrush(0x00c0c0c0));
	m_backgroundBrush.Attach(::CreateSolidBrush(0x00000000));

	UINT uiRate = ::GetCaretBlinkTime();
	if (uiRate == 0) uiRate = 750;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleCursor::Draw(bool bActive , DWORD dwCursorSize)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);

	if (bActive && m_bVisible)
	{
		CRect rect = m_rectCursor;
		rect.top += ::MulDiv(rect.bottom - rect.top, 100 - dwCursorSize, 100);
		m_dcCursor.FillRect(&rect, m_paintBrush);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ConsoleCursor::BitBlt(CDC& offscreenDC, int x, int y)
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

void ConsoleCursor::PrepareNext()
{
	m_bVisible = !m_bVisible;
}

//////////////////////////////////////////////////////////////////////////////


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

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBHLineCursor::Draw(bool /*bActive*/, DWORD /*dwCursorSize*/)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	m_dcCursor.MoveTo(m_rectCursor.left, m_rectCursor.bottom - 1, NULL);
	m_dcCursor.LineTo(m_rectCursor.right, m_rectCursor.bottom - 1);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// HLineCursor

HLineCursor::HLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_nSize(0)
, m_nPosition(0)
, m_nStep(0)
{
	// set the size of the cursor
	m_nSize = m_rectCursor.bottom - m_rectCursor.top - 1;
	m_nPosition = 0;
	m_nStep = 1;

	m_dcCursor.SelectPen(m_pen);

	UINT uiRate = ::GetCaretBlinkTime()/static_cast<UINT>(m_nSize);
	if (uiRate < 50) uiRate = 50;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void HLineCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
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

VLineCursor::VLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_pen(::CreatePen(PS_SOLID, 1, crCursorColor))
, m_nSize(0)
, m_nPosition(0)
, m_nStep(0)
{
	// set the size of the cursor
	m_nSize = m_rectCursor.Width() - 1;
	m_nPosition = 0;
	m_nStep = 1;

	m_dcCursor.SelectPen(m_pen);

	UINT uiRate = ::GetCaretBlinkTime()/static_cast<UINT>(m_nSize);
	if (uiRate < 50) uiRate = 50;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void VLineCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
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

RectCursor::RectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_bVisible(true)
{
	UINT uiRate = ::GetCaretBlinkTime();
	if (uiRate == 0) uiRate = 500;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void RectCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);

	if (bActive && m_bVisible)
	{
		m_dcCursor.FrameRect(&m_rectCursor, m_paintBrush);
	}
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
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);
	m_dcCursor.FrameRect(&m_rectCursor, m_paintBrush);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void NBRectCursor::Draw(bool /*bActive*/, DWORD /*dwCursorSize*/)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseRectCursor

PulseRectCursor::PulseRectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
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

	UINT uiRate = ::GetCaretBlinkTime()/static_cast<UINT>(2*m_nMaxSize);
	if (uiRate < 50) uiRate = 50;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void PulseRectCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
{
	m_dcCursor.FillRect(&m_rectCursor, m_backgroundBrush);

	if (bActive)
	{
		CRect rect = m_rectCursor;
		rect.left	+= m_nSize;
		rect.top	+= m_nSize;
		rect.right	-= m_nSize;
		rect.bottom	-= m_nSize;
		m_dcCursor.FrameRect(&rect, m_paintBrush);
	}
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

FadeBlockCursor::FadeBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer)
: Cursor(hwndConsoleView, dcConsoleView, rectCursor, crCursorColor)
, m_nStep(-ALPHA_STEP)
, m_bActive(false)
{
	m_blendFunction.BlendOp				= AC_SRC_OVER;
	m_blendFunction.BlendFlags			= 0;
	m_blendFunction.SourceConstantAlpha	= 255;
	m_blendFunction.AlphaFormat			= 0;

	m_dcCursor.FillRect(&m_rectCursor, m_paintBrush);

	UINT uiRate = ::GetCaretBlinkTime()/static_cast<UINT>(ALPHA_STEP+2);
	if (uiRate < 50) uiRate = 50;

	if( bTimer )
		m_uiTimer = ::SetTimer(hwndConsoleView, CURSOR_TIMER, uiRate, NULL);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void FadeBlockCursor::Draw(bool bActive, DWORD /*dwCursorSize*/)
{
	m_bActive = bActive;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void FadeBlockCursor::BitBlt(CDC& offscreenDC, int x, int y)
{
	if( !m_bActive ) return;

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

	m_blendFunction.SourceConstantAlpha = static_cast<BYTE>(m_blendFunction.SourceConstantAlpha + m_nStep);
}

//////////////////////////////////////////////////////////////////////////////


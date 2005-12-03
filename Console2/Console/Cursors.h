//////////////////////////////////////////////////////////////////////////////
// Cursors.h - cursor classes

#pragma once

#define CURSOR_TIMER		42

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum CursorStyle {

	cstyleXTerm		=	0,
	cstyleBlock		=	1,
	cstyleNBBlock	=	2,
	cstylePulseBlock=	3,
	cstyleBar		=	4,
	cstyleConsole	=	5,
	cstyleNBHline	=	6,
	cstyleHLine		=	7,
	cstyleVLine		=	8,
	cstyleRect		=	9,
	cstyleNBRect	=	10,
	cstylePulseRect	=	11,
	cstyleFadeBlock	=	12,
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// A base class for all the cursors

class Cursor {
	public:
		Cursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor) 
		: m_hwndConsoleView(hwndConsoleView)
		, m_cursorDC(cursorDC)
		, m_cursorRect(cursorRect)
		, m_crCursorColor(crCursorColor)
		, m_backgroundBrush(::CreateSolidBrush(RGB(0, 0, 0)))
		{
			m_cursorDC.SetBkColor(RGB(0, 0, 0));
		}
		
		virtual ~Cursor(){};

		// used to draw current frame of the cursor
		virtual void Draw(bool bActive = true) = 0;

		// used to bit-blit the cursor DC
		virtual void BitBlt(CDC& offscreenDC, int x, int y) = 0;

		// used to prepare the next frame of cursor animation
		virtual void PrepareNext() {}

	protected:

		HWND		m_hwndConsoleView;
		CDC&		m_cursorDC;
		CRect		m_cursorRect;
		COLORREF	m_crCursorColor;

		CBrush		m_paintBrush;
		CBrush		m_backgroundBrush;

		UINT		m_uiTimer;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Cursor factory

class CursorFactory {

	public:
		static shared_ptr<Cursor> CreateCursor(HWND hwndConsoleView, bool bAppActive, CursorStyle cursorStyle, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// XTermCursor

class XTermCursor : public Cursor {
	public:
		XTermCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~XTermCursor();

		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// BlockCursor

class BlockCursor : public Cursor {
	public:
		BlockCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~BlockCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);
		
		void PrepareNext();

	private:

		bool	m_bVisible;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// NBBlockCursor

class NBBlockCursor : public Cursor {
	public:
		NBBlockCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~NBBlockCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);
		
	private:
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseBlockCursor

class PulseBlockCursor : public Cursor {
	public:
		PulseBlockCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~PulseBlockCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();
		
	private:
		int		m_nSize;
		int		m_nMaxSize;
		int		m_nStep;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// BarCursor

class BarCursor : public Cursor {
	public:
		BarCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~BarCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();
		
	private:
		CPen	m_pen;
		bool	m_bVisible;
};

//////////////////////////////////////////////////////////////////////////////

/*
//////////////////////////////////////////////////////////////////////////////
// ConsoleCursor

class ConsoleCursor : public Cursor {
	public:
		ConsoleCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor);
		~ConsoleCursor();
		
		void Draw(LPRECT pRect);
		void PrepareNext();
		
	private:
		HANDLE	m_hStdOut;
		HBRUSH	m_hActiveBrush;
		BOOL	m_bVisible;
};

//////////////////////////////////////////////////////////////////////////////
*/

//////////////////////////////////////////////////////////////////////////////
// NBHLineCursor

class NBHLineCursor : public Cursor {
	public:
		NBHLineCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~NBHLineCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);
		
	private:
		CPen	m_pen;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// HLineCursor

class HLineCursor : public Cursor {
	public:
		HLineCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~HLineCursor();
			
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();
			
	private:
		CPen	m_pen;
		int		m_nSize;
		int		m_nPosition;
		int		m_nStep;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// VLineCursor

class VLineCursor : public Cursor {
	public:
		VLineCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~VLineCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();
		
	private:
		CPen	m_pen;
		int		m_nSize;
		int		m_nPosition;
		int		m_nStep;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// RectCursor

class RectCursor : public Cursor {
	public:
		RectCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~RectCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();
		
	private:
		bool	m_bVisible;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// NBRectCursor

class NBRectCursor : public Cursor {
	public:
		NBRectCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~NBRectCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);
		
	private:
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseRectCursor

class PulseRectCursor : public Cursor {
	public:
		PulseRectCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~PulseRectCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();
		
	private:
		int		m_nSize;
		int		m_nMaxSize;
		int		m_nStep;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// FadeBlockCursor

#define ALPHA_STEP			12

class FadeBlockCursor : public Cursor {
	public:
		FadeBlockCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~FadeBlockCursor();

		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();

	private:

		int				m_nStep;
		BLENDFUNCTION	m_blendFunction;

};

//////////////////////////////////////////////////////////////////////////////

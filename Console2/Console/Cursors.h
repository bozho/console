//////////////////////////////////////////////////////////////////////////////
// Cursors.h - cursor classes

#pragma once

#define CURSOR_TIMER		42

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

enum CursorStyle
{
	cstyleXTerm		=	0,
	cstyleBlock		=	1,
	cstyleNBBlock	=	2,
	cstylePulseBlock=	3,
	cstyleBar		=	4,
//	cstyleConsole	=	5,
	cstyleNBHline	=	5,
	cstyleHLine		=	6,
	cstyleVLine		=	7,
	cstyleRect		=	8,
	cstyleNBRect	=	9,
	cstylePulseRect	=	10,
	cstyleFadeBlock	=	11,
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// A base class for all the cursors

class Cursor
{
	public:
		Cursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
		: m_hwndConsoleView(hwndConsoleView)
		, m_dcCursor(::CreateCompatibleDC(NULL))
//		, m_bmpCursor(::CreateCompatibleBitmap(dcConsoleView, rectCursor.Width(), rectCursor.Height()))
		, m_bmpCursor(NULL)
		, m_rectCursor(rectCursor)
		, m_crCursorColor(crCursorColor)
		, m_paintBrush(::CreateSolidBrush(crCursorColor))
		, m_backgroundBrush(::CreateSolidBrush(RGB(0, 0, 0)))
		{
			Helpers::CreateBitmap(dcConsoleView, rectCursor.Width(), rectCursor.Height(), m_bmpCursor);
			m_dcCursor.SelectBitmap(m_bmpCursor);
			m_dcCursor.SetBkColor(RGB(0, 0, 0));
		}
		
		virtual ~Cursor(){};

		// used to draw current frame of the cursor
		virtual void Draw(bool bActive = true) = 0;

		// used to bit-blit the cursor DC
		virtual void BitBlt(CDC& offscreenDC, int x, int y) = 0;

		// used to prepare the next frame of cursor animation
		virtual void PrepareNext() {}

		const CRect& GetCursorRect() const { return m_rectCursor; }

	protected:

		HWND		m_hwndConsoleView;

		CDC			m_dcCursor;
		CBitmap		m_bmpCursor;

		CRect		m_rectCursor;
		COLORREF	m_crCursorColor;

		CBrush		m_paintBrush;
		CBrush		m_backgroundBrush;

		UINT_PTR	m_uiTimer;

	public:

		static wchar_t*	s_cursorNames[];
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Cursor factory

class CursorFactory
{
	public:
		static shared_ptr<Cursor> CreateCursor(HWND hwndConsoleView, bool bAppActive, CursorStyle cursorStyle, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// XTermCursor

class XTermCursor : public Cursor
{
	public:
		XTermCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
		~XTermCursor();

		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// BlockCursor

class BlockCursor : public Cursor
{
	public:
		BlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
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

class NBBlockCursor : public Cursor
{
	public:
		NBBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
		~NBBlockCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);
		
	private:
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseBlockCursor

class PulseBlockCursor : public Cursor
{
	public:
		PulseBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
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

class BarCursor : public Cursor
{
	public:
		BarCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
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

class ConsoleCursor : public Cursor
{
	public:
		ConsoleCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
		~ConsoleCursor();
		
		void Draw(LPCRect pRect);
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

class NBHLineCursor : public Cursor
{
	public:
		NBHLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
		~NBHLineCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);
		
	private:
		CPen	m_pen;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// HLineCursor

class HLineCursor : public Cursor
{
	public:
		HLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
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

class VLineCursor : public Cursor
{
	public:
		VLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
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

class RectCursor : public Cursor
{
	public:
		RectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
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

class NBRectCursor : public Cursor
{
	public:
		NBRectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
		~NBRectCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);
		
	private:
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseRectCursor

class PulseRectCursor : public Cursor
{
	public:
		PulseRectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
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

class FadeBlockCursor : public Cursor
{
	public:
		FadeBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
		~FadeBlockCursor();

		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();

	private:

		int				m_nStep;
		BLENDFUNCTION	m_blendFunction;

};

//////////////////////////////////////////////////////////////////////////////

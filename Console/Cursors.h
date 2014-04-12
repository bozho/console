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
	cstyleXTerm      = 0,
	cstyleBlock      = 1,
	cstyleNBBlock    = 2,
	cstylePulseBlock = 3,
	cstyleBar        = 4,
	cstyleNBHline    = 5,
	cstyleHLine      = 6,
	cstyleVLine      = 7,
	cstyleRect       = 8,
	cstyleNBRect     = 9,
	cstylePulseRect  = 10,
	cstyleFadeBlock  = 11,
	cstyleConsole    = 12,
	cstyleXTerm2     = 13,
	cstyleHBar       = 14,
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class CursorCharDrawer
{
public:
  CursorCharDrawer() {}
  virtual ~CursorCharDrawer() {}

  virtual void RedrawCharOnCursor(CDC& dc) = 0;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// A base class for all the cursors

class Cursor
{
	public:
		Cursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor)
		: m_hwndConsoleView(hwndConsoleView)
		, m_dcCursor(::CreateCompatibleDC(NULL))
		, m_bmpCursor(NULL)
		, m_rectCursor(rectCursor)
		, m_crCursorColor(crCursorColor)
		, m_crBackgroundColor(crCursorColor ^ 0x00ffffff)
		, m_paintBrush(::CreateSolidBrush(crCursorColor))
		, m_backgroundBrush(::CreateSolidBrush(crCursorColor ^ 0x00ffffff))
		, m_uiTimer(0)
		, m_bTimer(false)
		{
			Helpers::CreateBitmap(dcConsoleView, rectCursor.Width(), rectCursor.Height(), m_bmpCursor);
			m_dcCursor.SelectBitmap(m_bmpCursor);
			m_dcCursor.SetBkColor(m_crBackgroundColor);
		}

		virtual ~Cursor()
		{
			if (m_uiTimer) ::KillTimer(m_hwndConsoleView, m_uiTimer);
		}

		// used to draw current frame of the cursor
		virtual void Draw(bool bActive, DWORD dwCursorSize) = 0;

		// used to bit-blit the cursor DC
		virtual void BitBlt(CDC& offscreenDC, int x, int y)
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
							m_crBackgroundColor);
		}

		// used to prepare the next frame of cursor animation
		virtual void PrepareNext() {}

		const CRect& GetCursorRect() const { return m_rectCursor; }

	protected:

		HWND		m_hwndConsoleView;

		CDC			m_dcCursor;
		CBitmap		m_bmpCursor;

		CRect		m_rectCursor;
		COLORREF	m_crCursorColor;
		COLORREF	m_crBackgroundColor;

		CBrush		m_paintBrush;
		CBrush		m_backgroundBrush;

		UINT_PTR	m_uiTimer;
		bool		m_bTimer;

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
		static std::shared_ptr<Cursor> CreateCursor(HWND hwndConsoleView, bool bAppActive, CursorStyle cursorStyle, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor ,CursorCharDrawer*, bool bTimer);
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
		XTermCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, CursorCharDrawer*);
		~XTermCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

		void BitBlt(CDC& offscreenDC, int x, int y);

private:
    CursorCharDrawer* m_pdrawer;
    bool              m_bActive;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// XTerm2Cursor

class XTerm2Cursor : public Cursor
{
	public:
		XTerm2Cursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, CursorCharDrawer*, bool bTimer);
		~XTerm2Cursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();

private:
		CursorCharDrawer* m_pdrawer;
		bool              m_bActive;
		bool              m_bVisible;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// BlockCursor

class BlockCursor : public Cursor
{
	public:
		BlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~BlockCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

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
		~NBBlockCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

		void BitBlt(CDC& offscreenDC, int x, int y);

	private:
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseBlockCursor

class PulseBlockCursor : public Cursor
{
	public:
		PulseBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~PulseBlockCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

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
		BarCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~BarCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

		void PrepareNext();

	private:
		CPen	m_pen;
		bool	m_bVisible;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// HBarCursor

class HBarCursor : public Cursor
{
	public:
		HBarCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~HBarCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

		void PrepareNext();

	private:
		CPen	m_pen;
		bool	m_bVisible;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// ConsoleCursor

class ConsoleCursor : public Cursor
{
	public:
		ConsoleCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~ConsoleCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);
		void BitBlt(CDC& offscreenDC, int x, int y);
		void PrepareNext();

	private:
		bool	m_bVisible;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// NBHLineCursor

class NBHLineCursor : public Cursor
{
	public:
		NBHLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor);
		~NBHLineCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

	private:
		CPen	m_pen;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// HLineCursor

class HLineCursor : public Cursor
{
	public:
		HLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~HLineCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

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
		VLineCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~VLineCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

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
		RectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~RectCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

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
		~NBRectCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

	private:
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// PulseRectCursor

class PulseRectCursor : public Cursor
{
	public:
		PulseRectCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~PulseRectCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

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
		FadeBlockCursor(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectCursor, COLORREF crCursorColor, bool bTimer);
		~FadeBlockCursor() {}

		void Draw(bool bActive, DWORD dwCursorSize);

		void BitBlt(CDC& offscreenDC, int x, int y);

		void PrepareNext();

	private:

		int				m_nStep;
		BLENDFUNCTION	m_blendFunction;
		bool			m_bActive;

};

//////////////////////////////////////////////////////////////////////////////

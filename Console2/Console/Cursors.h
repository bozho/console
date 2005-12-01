/////////////////////////////////////////////////////////////////////////////
// Cursors.h - cursor classes

#pragma once

#define CURSOR_TIMER		42

/////////////////////////////////////////////////////////////////////////////
// A base class for all the cursors

class Cursor {
	public:
		Cursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor) 
		: m_hwndConsoleView(hwndConsoleView)
		, m_cursorDC(cursorDC)
		, m_cursorRect(cursorRect)
		, m_crCursorColor(crCursorColor)
		{
			m_backgroundBrush.CreateSolidBrush(RGB(0, 0, 0));
		};
		
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

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// XTermCursor

class XTermCursor : public Cursor {
	public:
		XTermCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~XTermCursor();

		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);

};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// NBBlockCursor

class NBBlockCursor : public Cursor {
	public:
		NBBlockCursor(HWND hwndConsoleView, CDC& cursorDC, const CRect& cursorRect, COLORREF crCursorColor);
		~NBBlockCursor();
		
		void Draw(bool bActive = true);

		void BitBlt(CDC& offscreenDC, int x, int y);
		
	private:
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////

/*
/////////////////////////////////////////////////////////////////////////////
// BarCursor

class BarCursor : public Cursor {
	public:
		BarCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor);
		~BarCursor();
		
		void Draw(LPRECT pRect);
		void PrepareNext();
		
	private:
		HPEN	m_hPen;
		BOOL	m_bVisible;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// NBHLineCursor

class NBHLineCursor : public Cursor {
public:
	NBHLineCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor);
	~NBHLineCursor();
	
	void Draw(LPRECT pRect);
	void PrepareNext();
	
private:
	HPEN	m_hPen;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// HLineCursor

class HLineCursor : public Cursor {
	public:
		HLineCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor);
		~HLineCursor();
			
		void Draw(LPRECT pRect);
		void PrepareNext();
			
	private:
		HPEN	m_hPen;
		int		m_nSize;
		int		m_nPosition;
		int		m_nStep;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// VLineCursor

class VLineCursor : public Cursor {
	public:
		VLineCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor);
		~VLineCursor();
		
		void Draw(LPRECT pRect);
		void PrepareNext();
		
	private:
		HPEN	m_hPen;
		int		m_nSize;
		int		m_nPosition;
		int		m_nStep;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// RectCursor

class RectCursor : public Cursor {
	public:
		RectCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor);
		~RectCursor();
		
		void Draw(LPRECT pRect);
		void PrepareNext();
		
	private:
		HBRUSH	m_hActiveBrush;
		BOOL	m_bVisible;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// NBRectCursor

class NBRectCursor : public Cursor {
	public:
		NBRectCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor);
		~NBRectCursor();
		
		void Draw(LPRECT pRect);
		void PrepareNext();
		
	private:
		HBRUSH	m_hActiveBrush;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// PulseRectCursor

class PulseRectCursor : public Cursor {
	public:
		PulseRectCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor);
		~PulseRectCursor();
		
		void Draw(LPRECT pRect);
		void PrepareNext();
		
	private:
		HBRUSH	m_hActiveBrush;
		RECT	m_rect;
		int		m_nSize;
		int		m_nMaxSize;
		int		m_nStep;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// FadeBlockCursor

#define FADE_STEPS			20
#define ALPHA_STEP			12

#define BLEND_BMP_WIDTH		10
#define BLEND_BMP_HEIGHT	20

class FadeBlockCursor : public Cursor {
	public:
		FadeBlockCursor(HWND hwndParent, HDC hdcWindow, COLORREF crCursorColor, COLORREF crBkColor);
		~FadeBlockCursor();

		void Draw(LPRECT pRect);
		void PrepareNext();

	private:

		void FakeBlend();

		COLORREF	m_crCursorColor;
		int			m_nStep;
		
		// used under WinNT 4.0
		COLORREF	m_crBkColor;
		COLORREF	m_arrColors[FADE_STEPS+1];
		int			m_nIndex;

		// these are used under Win2000 only
		HMODULE			m_hUser32;
		BLENDFUNCTION	m_bfn;
		HDC				m_hMemDC;
		HBITMAP			m_hBmp;
		HBITMAP			m_hBmpOld;
		int				m_nBmpWidth;
		int				m_nBmpHeight;
};

/////////////////////////////////////////////////////////////////////////////
*/
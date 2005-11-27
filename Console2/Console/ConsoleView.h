#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// User-defined message for notifying ConsoleViewContainer

#define UM_CONSOLE_RESIZED	WM_USER + 0x1000
#define UM_CONSOLE_CLOSED	WM_USER + 0x1001

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ConsoleView : public CWindowImpl<ConsoleView> {

	public:
		DECLARE_WND_CLASS(NULL)

		ConsoleView(const ConsoleParams& consoleStartupParams);

		BOOL PreTranslateMessage(MSG* pMsg);

		BEGIN_MSG_MAP(ConsoleView)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
			MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKey)
			MESSAGE_HANDLER(WM_SYSKEYUP, OnSysKey)
			MESSAGE_HANDLER(WM_KEYDOWN, OnKey)
			MESSAGE_HANDLER(WM_KEYUP, OnKey)
		END_MSG_MAP()

//		Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnSysKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	public:

		void GetRect(RECT& clientRect);
		bool GetMaxRect(RECT& maxClientRect);
		void AdjustRectAndResize(RECT& clientRect);

		ConsoleHandler& GetConsoleHandler() { return m_consoleHandler; }

	private:

		void OnConsoleChange(bool bResize);
		void OnConsoleClose();

		void CreateOffscreenBuffers();
		void CreateOffscreenBitmap(const CPaintDC& dcWindow, const RECT& rect, CDC& cdc, CBitmap& bitmap);

		void GetTextSize();
		DWORD GetBufferDifference();
		void SetDefaultConsoleColors();

		void Repaint();

		void RepaintText();
		void RepaintTextChanges();

		void BitBltOffscreen();

	private:

		bool	m_bInitializing;

		ConsoleParams	m_consoleStartupParams;
		ConsoleHandler	m_consoleHandler;

		CDC		m_dcOffscreen;
		CDC		m_dcText;
		CDC		m_dcBackground;

		CBitmap	m_bmpOffscreen;
		CBitmap	m_bmpText;
		CBitmap	m_bmpBackground;

		CFont	m_fontText;

		int		m_nCharHeight;
		int		m_nCharWidth;

		shared_array<CHAR_INFO>	m_screenBuffer;
		// console colors
		COLORREF	m_arrConsoleColors[16];

		bool		m_bImageBackground;
		COLORREF	m_crConsoleBackground;

		int			m_nInsideBorder;

		bool		m_bUseFontColor;
		COLORREF	m_crFontColor;

};

//////////////////////////////////////////////////////////////////////////////

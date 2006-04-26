#pragma once

#include "Cursors.h"
#include "SelectionHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// User-defined message for notifying main window

#define UM_CONSOLE_RESIZED	WM_USER + 0x1000
#define UM_CONSOLE_CLOSED	WM_USER + 0x1001
#define UM_UPDATE_TITLES	WM_USER + 0x1002
#define UM_SHOW_POPUP_MENU	WM_USER + 0x1003
#define UM_START_MOUSE_DRAG	WM_USER + 0x1004
#define UM_TRAY_NOTIFY		WM_USER + 0x1005

#define IDC_TRAY_ICON		0x0001

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ConsoleView : public CWindowImpl<ConsoleView, CWindow, CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL, 0> >
{
	public:
		DECLARE_WND_CLASS(NULL)

		ConsoleView(DWORD dwTabIndex, DWORD dwRows, DWORD dwColumns);
		~ConsoleView();

		BOOL PreTranslateMessage(MSG* pMsg);

		BEGIN_MSG_MAP(ConsoleView)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
			MESSAGE_HANDLER(WM_SYSKEYDOWN, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_SYSKEYUP, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_KEYDOWN, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_KEYUP, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_MOUSEWHEEL, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
			MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_INPUTLANGCHANGEREQUEST, OnInputLangChangeRequest)
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
		LRESULT OnConsoleFwdMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnMButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnInputLangChangeRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	public:

		void GetRect(CRect& clientRect);
		bool GetMaxRect(CRect& maxClientRect);
		void AdjustRectAndResize(CRect& clientRect);

		ConsoleHandler& GetConsoleHandler() { return m_consoleHandler; }
		shared_ptr<TabData> GetTabData() { return m_tabData; }

		bool GetConsoleWindowVisible() const { return m_bConsoleWindowVisible; }
		void SetConsoleWindowVisible(bool bVisible);

		void SetAppActiveStatus(bool bAppActive);

		void RecreateOffscreenBuffers();
		void RepaintView();
		void SetActive(bool bActive);
		void SetTitle(const CString& strTitle);

		CString GetConsoleCommand();
		const CString& GetTitle() const { return m_strTitle; }
		CIcon& GetIcon(bool bBigIcon = true) { return bBigIcon ? bigIcon : smallIcon; }

		void Copy(const CPoint* pPoint = NULL);
		void Paste();

	private:

		void OnConsoleChange(bool bResize);
		void OnConsoleClose();

		void CreateOffscreenBuffers();
		void CreateOffscreenBitmap(const CPaintDC& dcWindow, const CRect& rect, CDC& cdc, CBitmap& bitmap);
		bool CreateFont(const wstring& strFontName);

		void InitializeScrollbars();
		void DoScroll(int nType, int nScrollCode, int nThumbPos);

		DWORD GetBufferDifference();

		void UpdateTitle();
		void Repaint();

		void RepaintText();
		void RepaintTextChanges();

		void BitBltOffscreen(bool bOnlyCursor = false);
		void UpdateOffscreen(const CRect& rectBlit);

		// sends text to the windows console
		void SendTextToConsole(const wchar_t* pszText);


	private:

		bool	m_bInitializing;
		bool	m_bAppActive;
		bool	m_bActive;
		bool	m_bConsoleWindowVisible;

		DWORD	m_dwStartupRows;
		DWORD	m_dwStartupColumns;

		bool	m_bShowVScroll;
		bool	m_bShowHScroll;
		int		m_nVScrollWidth;
		int		m_nHScrollWidth;

		CString	m_strTitle;

		CIcon	bigIcon;
		CIcon	smallIcon;


		ConsoleHandler					m_consoleHandler;

		shared_array<CHAR_INFO>			m_screenBuffer;

		ConsoleSettings&				m_consoleSettings;
		AppearanceSettings&				m_appearanceSettings;

		shared_ptr<TabData>				m_tabData;
		shared_ptr<BackgroundImage>		m_background;

		shared_ptr<Cursor>				m_cursor;
		shared_ptr<SelectionHandler>	m_selectionHandler;

// static members
private:

		static CDC						m_dcOffscreen;
		static CDC						m_dcText;

		static CBitmap					m_bmpOffscreen;
		static CBitmap					m_bmpText;

		static CFont					m_fontText;

		static int						m_nCharHeight;
		static int						m_nCharWidth;

};

//////////////////////////////////////////////////////////////////////////////

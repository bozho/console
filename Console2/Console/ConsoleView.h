#pragma once

#include "Cursors.h"
#include "SelectionHandler.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MainFrame;

//////////////////////////////////////////////////////////////////////////////

class ConsoleView
	: public CWindowImpl<ConsoleView, CWindow, CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL, 0> >
{
	public:
//		DECLARE_WND_CLASS(NULL)
		DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS, COLOR_WINDOW)

		ConsoleView(MainFrame& mainFrame, DWORD dwTabIndex, const wstring& strCmdLineInitialDir, const wstring& strInitialCmd, const wstring& strDbgCmdLine, DWORD dwRows, DWORD dwColumns);
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
			MESSAGE_HANDLER(WM_CHAR, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_SYSCHAR, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_DEADCHAR, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_SYSDEADCHAR, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_MOUSEWHEEL, OnConsoleFwdMsg)
			MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseButton)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseButton)
			MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouseButton)
			MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseButton)
			MESSAGE_HANDLER(WM_RBUTTONUP, OnMouseButton)
			MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMouseButton)
			MESSAGE_HANDLER(WM_MBUTTONUP, OnMouseButton)
			MESSAGE_HANDLER(WM_XBUTTONDOWN, OnMouseButton)
			MESSAGE_HANDLER(WM_XBUTTONUP, OnMouseButton)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_INPUTLANGCHANGEREQUEST, OnInputLangChangeRequest)
			MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
			COMMAND_RANGE_HANDLER(ID_SCROLL_UP, ID_SCROLL_ALL_RIGHT, OnScrollCommand)
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
		LRESULT OnSysKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnConsoleFwdMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnScrollCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnMouseButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnInputLangChangeRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnScrollCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);

	public:

		void GetRect(CRect& clientRect);
		bool GetMaxRect(CRect& maxClientRect);
		void AdjustRectAndResize(CRect& clientRect, DWORD dwResizeWindowEdge, bool bGetClientRect);

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


		CriticalSection& GetActiveCS() { return m_activeCritSec; }

		void DumpBuffer();

	private:

		void OnConsoleChange(bool bResize);
		void OnConsoleClose();

		void CreateOffscreenBuffers();
		void CreateOffscreenBitmap(const CWindowDC& dcWindow, const CRect& rect, CDC& cdc, CBitmap& bitmap);
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

		bool TranslateKeyDown(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/);
		void ForwardMouseClick(UINT uMsg, WPARAM wParam, const CPoint& point);

		COORD GetConsoleCoord(const CPoint& clientPoint);

	private:

		MainFrame& m_mainFrame;

		wstring m_strCmdLineInitialDir;
		wstring m_strInitialCmd;
		wstring	m_strDbgCmdLine;

		bool	m_bInitializing;
		bool	m_bAppActive;
		bool	m_bActive;
		bool	m_bUseTextAlphaBlend;
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
		HotKeys&						m_hotkeys;

		shared_ptr<TabData>				m_tabData;
		shared_ptr<BackgroundImage>		m_background;

		CBrush							m_backgroundBrush;

		shared_ptr<Cursor>				m_cursor;
		shared_ptr<SelectionHandler>	m_selectionHandler;

		MouseSettings::Command			m_mouseCommand;

		CriticalSection					m_activeCritSec;

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

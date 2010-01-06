#pragma once

#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

#define	TIMER_ANIMATE_WINDOW	42

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/** 
 *
 * @struct AnimationWindowOptions
 *
 * @brief Struct used to store animation window options.
 *
**/
//////////////////////////////////////////////////////////////////////////////
struct AnimationWindowOptions
{
	/// Default constructor.
	AnimationWindowOptions(HWND hwndOriginalWnd)
	: originalWnd(hwndOriginalWnd)
	{}
	
	/// Copy constructor.
	AnimationWindowOptions(const AnimationWindowOptions& windowData)
	: originalWnd(windowData.originalWnd)
	{
	}

 	/// Original window handle.
 	CWindow			originalWnd;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/** 
 * 
 * @class AnimationWindow
 *
 * @brief Animation window class
 *
 * Animation window class.
 * 
**/
//////////////////////////////////////////////////////////////////////////////
class AnimationWindow : public CWindowImpl<AnimationWindow, CWindow, CWinTraits<WS_POPUP, WS_EX_TOOLWINDOW> >								
{
	
	///////////////
	// Constructor
	///////////////
	public:
		DECLARE_WND_CLASS_EX(L"Console animation window", CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS, COLOR_WINDOW)
										
		/// Constructor
		AnimationWindow(const AnimationWindowOptions& windowOptions);
						
		
	/////////////
	// Overrides
	/////////////
	public:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual void OnFinalMessage(HWND /*hWnd*/);


	//////////////
	// Operations
	//////////////
	public:
		/// Creates a new popup window.
		HWND Create();
		/// Closes the window.
		void CloseWindow();

		void SA();
		void HA();


	////////////////////
	// Message handlers
	////////////////////
	public:

		BEGIN_MSG_MAP(AnimationWindow)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
		END_MSG_MAP()
										
		// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
					
		/// WM_CREATE handler					
		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		/// WM_CLOSE handler					
		LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		/// WM_ERASEBKGND handler. Overriden to NOOP.
		LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		/// WM_NCPAINT handler. Paints window border.
		LRESULT OnNcPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		/// WM_PAINT handler. Paints window client area.
		LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		/// WM_TIMER handler. Closes the window.
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		/// Handles restore window command
		LRESULT OnRestoreWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		/// Handles set Z-order commands
		LRESULT OnSetZOrder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


	//////////////////////
	// Private operations
	//////////////////////
	private:
		void UpdateThumbnail(const CRect& rectOrigWnd);

	/////////////
	// Variables
	/////////////
	private:

		/// Window options
		AnimationWindowOptions	m_windowOptions;

		/// Original window's placement flags
		WINDOWPLACEMENT		m_origWindowPlacement;

		/// Set to true if original window is layered
		bool				m_bOrigWndTopmost;

		/// Window rectangle.
		CRect	m_rectWindow;

		/// Client rectangle
		CRect	m_rectClient;

		/// Window client area HDC.
		CDC		m_dcWindow;

		/// Window client area bitmap.
		CBitmap	m_bmpWindow;

		/// Thumbnail DC
		CDC		m_dcThumbnail;
		/// Thumbnail bitmap
		CBitmap	m_bmpThumbnail;

		int		m_nIterations;
};

//////////////////////////////////////////////////////////////////////////////

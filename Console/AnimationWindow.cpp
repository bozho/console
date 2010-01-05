#include "stdafx.h"

#include "AnimationWindow.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Constructor

AnimationWindow::AnimationWindow(const AnimationWindowOptions& windowOptions)
: m_windowOptions(windowOptions)
, m_nIterations(0)
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Overrides

//////////////////////////////////////////////////////////////////////////////

BOOL AnimationWindow::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void AnimationWindow::OnFinalMessage(HWND /*hWnd*/)
{
/*
	// find the window pair
	AnimationWindowMap&				AnimationWindowMap	= GetAnimationWindowMap();
	AnimationWindowMap::iterator	it				= AnimationWindowMap.find(m_windowOptions.originalWnd);

	if (it != AnimationWindowMap.end())
	{
		if (m_windowOptions.originalWnd.IsWindow())
		{
			// if needed, remove topmost attribute
			if (!m_bOrigWndTopmost)
			{
				m_windowOptions.originalWnd.SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			}

			// show the window
			if ((m_origWindowPlacement.showCmd == SW_SHOWNORMAL) && !m_windowOptions.bRestoreOriginalPosition)
			{
				::SetWindowPos(it->first, HWND_TOP, m_rectWindow.left, m_rectWindow.top, 0, 0, SWP_NOSIZE);
			}
			else
			{
				::SetWindowPos(it->first, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
			}
			::ShowWindow(it->first, SW_SHOW);
			::SetForegroundWindow(it->first);
		}

		if (m_bRemoveFromMap) AnimationWindowMap.erase(it);
	}
*/
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Operations

HWND AnimationWindow::Create()
{
	// Call parent Create
	return CWindowImpl<
			AnimationWindow, 
			CWindow, 
			CWinTraits<WS_POPUP, WS_EX_TOOLWINDOW> >::Create(
														NULL, 
														m_rectWindow);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void AnimationWindow::CloseWindow()
{
	DestroyWindow();
//	SendMessage(WM_CLOSE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void AnimationWindow::SA()
{
	TRACE(L"AnimationWindow SA\n");

	// set window check timer
	SetTimer(TIMER_ANIMATE_WINDOW, 200);
	m_nIterations = 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void AnimationWindow::HA()
{
	TRACE(L"AnimationWindow HA\n");

	KillTimer(TIMER_ANIMATE_WINDOW);
	m_windowOptions.originalWnd.ShowWindow(SW_SHOW);
	CloseWindow();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Message handlers

//////////////////////////////////////////////////////////////////////////////

LRESULT AnimationWindow::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
/*
	// create background DC
	m_dcWindow.CreateCompatibleDC(GetDC());

	// create thumbnail DC
	m_dcThumbnail.CreateCompatibleDC(GetDC());

	// get original window's placement info
	::ZeroMemory(&m_origWindowPlacement, sizeof(WINDOWPLACEMENT));
	m_origWindowPlacement.length = sizeof(WINDOWPLACEMENT);
	m_windowOptions.originalWnd.GetWindowPlacement(&m_origWindowPlacement);

	// set layered attribute for original window if needed
	long lOrigWndExStyle = m_windowOptions.originalWnd.GetWindowLong(GWL_EXSTYLE);

	if ((lOrigWndExStyle & WS_EX_TOPMOST) != WS_EX_TOPMOST)
	{
		m_windowOptions.originalWnd.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		m_windowOptions.originalWnd.RedrawWindow(
										NULL, 
										NULL, 
										RDW_UPDATENOW | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
	else
	{
		m_bOrigWndTopmost = true;
	}

	// now, set rectangles
	CRect rectOrigWnd;

	m_windowOptions.originalWnd.GetWindowRect(&rectOrigWnd);

	m_rectClient.left	= 0;
	m_rectClient.top	= 0;

	if (rectOrigWnd.Width() > rectOrigWnd.Height())
	{
		m_rectClient.right	= m_windowOptions.nWindowSize;
		m_rectClient.bottom	= m_windowOptions.nWindowSize * rectOrigWnd.Height() / rectOrigWnd.Width();
	}
	else
	{
		m_rectClient.bottom	= m_windowOptions.nWindowSize;
		m_rectClient.right	= m_windowOptions.nWindowSize * rectOrigWnd.Width() / rectOrigWnd.Height();
	}

	m_rectWindow		= m_rectClient;

	// if we're drawing border, increase window size
	if (m_windowOptions.bDrawBorder)
	{
		m_rectWindow.right	+= 2;
		m_rectWindow.bottom	+= 2;
	}

	m_rectWindow.OffsetRect(rectOrigWnd.left, rectOrigWnd.top);

	// create offscreen bitmap and select it into HDC
	m_bmpWindow.CreateCompatibleBitmap(GetDC(), m_rectClient.Width(), m_rectClient.Height());
	m_dcWindow.SelectBitmap(m_bmpWindow);

	// create thumbnail bitmap
	m_bmpThumbnail.CreateCompatibleBitmap(GetDC(), m_rectClient.Width(), m_rectClient.Height());
	m_dcThumbnail.SelectBitmap(m_bmpThumbnail);

	// set window border
	if (m_windowOptions.bDrawBorder)
	{
		SetWindowLong(
			GWL_STYLE, 
			GetWindowLong(GWL_STYLE) | WS_BORDER);
	}

	// set window size and position
	SetWindowPos(
		m_windowOptions.hwndPosition, 
		m_rectWindow.left, 
		m_rectWindow.top, 
		m_rectWindow.Width(), 
		m_rectWindow.Height(), 
		0);

	UpdateThumbnail(rectOrigWnd);

	// set window opacity
	if (m_windowOptions.byOpacity < 255)
	{
		SetWindowLong(
			GWL_EXSTYLE, 
			GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYERED);

		::SetLayeredWindowAttributes(
			m_hWnd,
			0, 
 			m_windowOptions.byOpacity, 
			LWA_ALPHA);
	}

	// set window check timer
	SetTimer(TIMER_WINDOW_CHECK, 500);

	SetZOrderCheck();

	// finally, show the window
	ShowWindow(SW_SHOW);
*/
	TRACE(L"AnimationWindow OnCreate\n");

	m_rectWindow.left = 100;
	m_rectWindow.top = 100;
	m_rectWindow.right = 200;
	m_rectWindow.bottom = 200;

	// set window size and position
	SetWindowPos(
		HWND_TOPMOST, 
		m_rectWindow.left, 
		m_rectWindow.top, 
		m_rectWindow.Width(), 
		m_rectWindow.Height(), 
		0);

	// finally, show the window
	ShowWindow(SW_SHOW);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT AnimationWindow::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	TRACE(L"AnimationWindow OnClose\n");

	DestroyWindow();
//	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT AnimationWindow::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// don't do anything here, prevents flicker
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT AnimationWindow::OnNcPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
/*
	HDC hdcNc;
	// 0x10000 is an undocumented flag need for this to work (get a window DC in WM_NCPAINT handler)
	hdcNc = GetDCEx((HRGN)wParam, DCX_WINDOW|DCX_INTERSECTRGN|0x10000);

	// draw border
	RECT rect;
	GetWindowRect(&rect);
	::OffsetRect(&rect, -rect.left, -rect.top);
	::FrameRect(hdcNc, &rect, m_brushBorder);
    
    ReleaseDC(hdcNc);
*/
	return 0;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT AnimationWindow::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	TRACE(L"AnimationWindow OnPaint\n");

	CPaintDC	dc(m_hWnd);

	CBrush brush(::CreateSolidBrush(RGB(0, 0, 0)));

	dc.FillRect(&m_rectWindow, brush);
/*
	CPaintDC	dc(m_hWnd);
	CRect		rect;

	GetClientRect(&rect);

	// draw thumbnail
	m_dcWindow.BitBlt(
				0, 
				0, 
				m_rectClient.Width(), 
				m_rectClient.Height(), 
				m_dcThumbnail, 
				0, 
				0, 
				SRCCOPY);

	dc.BitBlt(
		0, 
		0, 
		rect.right, 
		rect.bottom, 
		m_dcWindow, 
		0, 
		0, 
		SRCCOPY);
*/
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT AnimationWindow::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	TRACE(L"AnimationWindow OnTimer\n");

	if (wParam == TIMER_ANIMATE_WINDOW)
	{
		++m_nIterations;

		if (m_nIterations == 10)
		{
			// TODO: implement here

			m_windowOptions.originalWnd.ShowWindow(SW_HIDE);

			KillTimer(TIMER_ANIMATE_WINDOW);
		}

		Invalidate();
	}
/*
	if (wParam == TIMER_ANIMATE_WINDOW)
	{
		if (!m_windowOptions.originalWnd.IsWindow()) CloseWindow();
	}
*/
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Private operations

//////////////////////////////////////////////////////////////////////////////

void AnimationWindow::UpdateThumbnail(const CRect& rectOrigWnd)
{
/*
	m_dcThumbnail.SetStretchBltMode(HALFTONE);
	
	CDC	dcOriginalWindow(::GetDCEx(m_windowOptions.originalWnd, NULL, DCX_WINDOW));
	m_dcThumbnail.StretchBlt(
					0, 
					0, 
					m_rectClient.Width(), 
					m_rectClient.Height(), 
					dcOriginalWindow,
					0, 
					0, 
					rectOrigWnd.Width(),
					rectOrigWnd.Height(),
					SRCCOPY);
*/
}

//////////////////////////////////////////////////////////////////////////////


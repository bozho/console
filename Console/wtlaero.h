// WtlAero.h
//
// WTL::aero namespace: classes and functions supporting the Vista(r) Aero visual style
//
// Written by Alain Rist (ar@navpoch.com)
// Copyright (c) 2007 Alain Rist.
//
// AR 05.31.2007 CodeProject release
// AR 06.05.2007 CTabView: fixed unthemed environment issues, added DrawMoveMark().
//
// The use and distribution terms for this software are covered by the
// Common Public License 1.0 (http://opensource.org/osi3.0/licenses/cpl1.0.php)
// By using this software in any fashion, you are agreeing to be bound by
// the terms of this license. You must not remove this notice, or
// any other, from this software.
//

#ifndef __WTL_AERO_H__
#define __WTL_AERO_H__

#pragma once

#if !defined _WTL_VER || _WTL_VER < 0x800
	#error WtlAero.h requires the Windows Template Library 8.0
#endif

#if _WIN32_WINNT < 0x0600
	#error WtlAero.h requires _WIN32_WINNT >= 0x0600
#elif !defined(NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_LONGHORN)
	#error WtlAero.h requires the Microsoft® Windows® Software Development Kit for Windows Vista™
#endif

#if !defined _UNICODE && !defined UNICODE
	#error WtlAero.h requires Unicode
#endif

#ifndef __ATLTHEME_H__
	#include "atltheme.h"
#endif

#include <dwmapi.h>

#pragma comment (lib, "dwmapi.lib")


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// aero::CAeroBase<T> - Base class for Aero translucency (when available)
// aero::CAeroImpl<T> - enables Aero translucency (when available) for any window
// aero::CWindowImpl<T, TBase, TWinTraits> - Implements a Aero window
// aero::CDialogImpl<T, TBase> - dialog implementation of Aero translucency (when available)
// aero::CCtrl<TCtrl> - implementation of Aero drawing for system controls
// aero::CTabCtrl - Aero drawing Tab control
// aero::CReBarCtrl - Aero drawing ReBar control
// aero::CToolBarCtrl - Aero drawing ToolBar Control
// aero::CStatusBarCtrl - Aero drawing StatusBar Control
// aero::CStatic - Aero drawing Static control
// aero::CButton - Aero drawing Button control
// aero::CFrameWindowImpl<T, TBase, TWinTraits> - frame implementation of Aero translucency (when available)
// aero::CCtrlImpl<T, TCtrlImpl> - implementation of Aero drawing for user and WTL defined controls
// aero::CPrintPreviewWindow - Aero drawing WTL::CPrintPreviewWindow control
// aero::CCommandBarCtrl - Aero drawing WTL::CCommandBarCtrl control
// aero::CTabView - Aero drawing WTL::CTabView control
// aero::CPaneContainer - Aero drawing WTL::CPaneContainer control
// aero::CSplitterImpl<T, t_bVertical> - Aero drawing splitter implementation for any window
// aero::CSplitterWindowImpl<T, t_bVertical, TBase, TWinTraits> - Implements Aero drawing splitter windows
// aero::CSplitterWindowT<t_bVertical> - Aero drawing splitter windows
// aero::CSplitterWindow - Vertical Aero drawing splitter window to be used as is
// aero::CHorSplitterWindow - Horizontal Aero drawing  splitter window to be used as is
//
// namespace helper functions
//
// aero::IsSupported()
// aero::IsComposing()
// aero::IsOpaqueBlend()
// aero::Subclass()

namespace WTL
{
namespace aero
{

///////////////////////////////////////////////////////////////////////////////
// WTL::aero helper functions

inline bool IsSupported()
{
	return CBufferedPaintBase::IsBufferedPaintSupported();
}

extern bool _bAeroGlassActive;
__declspec(selectany) bool _bAeroGlassActive = true;

inline void SetAeroGlassActive(bool bAeroGlassActive)
{
	_bAeroGlassActive = bAeroGlassActive;
}

inline bool IsAeroGlassActive(void)
{
	return _bAeroGlassActive;
}

inline bool IsComposing()
{
	if( !_bAeroGlassActive ) return false;

	BOOL bEnabled = FALSE;
	return IsSupported() ? SUCCEEDED(DwmIsCompositionEnabled(&bEnabled)) && bEnabled : false;
}

inline bool IsOpaqueBlend()
{
	BOOL bOpaqueBlend = FALSE;
	DWORD dwColor;
	return IsSupported() && SUCCEEDED(DwmGetColorizationColor(&dwColor, &bOpaqueBlend)) ? bOpaqueBlend == TRUE : false;
}

template <class TCtrl>

inline static BOOL Subclass(TCtrl& Ctrl, HWND hCtrl)
{
	ATLASSERT(::IsWindow(hCtrl));
	return IsSupported() ? Ctrl.SubclassWindow(hCtrl) : FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// aero::CAeroBase - Base class for Aero translucency (when available)

template <class T>
class CAeroBase :
	public WTL::CThemeImpl<T>
{
public:
	CAeroBase(LPCWSTR lpstrThemeClassList = L"globals")
	{
		SetThemeClassList(lpstrThemeClassList);
	}

	bool IsTheming() const
	{
		return m_hTheme != 0;
	}

	template <class TCtrl>
	BOOL Subclass(TCtrl& Ctrl, INT idCtrl)
	{
		return aero::Subclass(Ctrl, static_cast<T*>(this)->GetDlgItem(idCtrl));
	}

	bool DrawPartText(HDC dc, int iPartID, int iStateID, LPCTSTR pStr, LPRECT prText, UINT uFormat, DTTOPTS &dto)
	{
		HRESULT hr = S_FALSE;
		if(IsTheming())
			if (IsSupported())
				hr = DrawThemeTextEx (dc, iPartID, iStateID, pStr, -1, uFormat, prText, &dto );
			else
				hr = DrawThemeText(dc, iPartID, iStateID, pStr, -1, uFormat, 0, prText);
		else
			hr = CDCHandle(dc).DrawText(pStr, -1, prText, uFormat) != 0 ? S_OK : S_FALSE;

		return SUCCEEDED(hr);
	}

	 bool DrawPartText(HDC dc, int iPartID, int iStateID, LPCTSTR pStr, LPRECT prText, UINT uFormat,
		DWORD dwFlags = DTT_COMPOSITED, int iGlowSize = 0)
	{
		DTTOPTS dto = {sizeof(DTTOPTS)};
		dto.dwFlags = dwFlags;
		dto.iGlowSize = iGlowSize;
		return DrawPartText(dc, iPartID, iStateID, pStr, prText, uFormat, dto);
	}

	bool DrawText(HDC dc, LPCTSTR pStr, LPRECT prText, UINT uFormat, DTTOPTS &dto)
	{
		return DrawPartText(dc, 1, 1, pStr, prText, uFormat, dto);
	}

	bool DrawText(HDC dc, LPCTSTR pStr, LPRECT prText, UINT uFormat, DWORD dwFlags = DTT_COMPOSITED, int iGlowSize = 0)
	{
		return DrawPartText(dc, 1, 1, pStr, prText, uFormat, dwFlags, iGlowSize);
	}

};

///////////////////////////////////////////////////////////////////////////////
// aero::CAeroImpl - implementation of Aero translucency (when available)

template <class T>
class CAeroImpl :
	public WTL::CBufferedPaintImpl<T>,
	public CAeroBase<T>

{
public:
	CAeroImpl(LPCWSTR lpstrThemeClassList = L"globals") : CAeroBase<T>(lpstrThemeClassList)
	{
		m_PaintParams.dwFlags = BPPF_ERASE;
		MARGINS m = {-1};
		m_Margins = m;
	}

	MARGINS m_Margins;

	bool SetMargins(MARGINS& m)
	{
		m_Margins = m;
		T* pT = static_cast<T*>(this);
		return pT->IsWindow() && IsComposing() ? SUCCEEDED(DwmExtendFrameIntoClientArea(pT->m_hWnd, &m_Margins)) : true;
	}

	bool SetOpaque(bool bOpaque = true)
	{
		MARGINS m = {bOpaque - 1};
		return SetMargins(m);
	}

	bool SetOpaque(RECT &rOpaque)
	{
		T* pT = static_cast<T*>(this);
		RECT rClient;
		pT->GetClientRect(&rClient);
		MARGINS m = {rOpaque.left, rClient.right - rOpaque.right, rOpaque.top, rClient.bottom - rOpaque.bottom};
		return SetMargins(m);
	}

	bool SetOpaqueUnder(ATL::CWindow wChild)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(wChild.IsWindow());
		ATLASSERT(pT->IsChild(wChild));

		RECT rChild;
		wChild.GetWindowRect(&rChild);
		pT->ScreenToClient(&rChild);

		return SetOpaque(rChild);
	}

	bool SetOpaqueUnder(UINT uID)
	{
		return SetOpaqueUnder(static_cast<T*>(this)->GetDlgItem(uID));
	}

// implementation
	void DoPaint(CDCHandle dc, RECT& rDest)
	{
		T* pT = static_cast<T*>(this);

		RECT rClient;
		pT->GetClientRect(&rClient);

		RECT rView = {rClient.left + m_Margins.cxLeftWidth, rClient.top + m_Margins.cyTopHeight,
			rClient.right - m_Margins.cxRightWidth, rClient.bottom - m_Margins.cyBottomHeight};

		if (!IsComposing())
			if (IsTheming())
				pT->DrawThemeBackground(dc, WP_FRAMEBOTTOM, pT->m_hWnd == GetFocus() ? FS_ACTIVE : FS_INACTIVE, &rClient, &rDest);
			else
				dc.FillSolidRect(&rClient, ::GetSysColor(COLOR_MENUBAR));

		if ((m_Margins.cxLeftWidth != -1) && !::IsRectEmpty(&rView))
		{
			dc.FillSolidRect(&rView, ::GetSysColor(COLOR_WINDOW));
			if (!m_BufferedPaint.IsNull())
				m_BufferedPaint.MakeOpaque(&rView);
		}
		else
			::SetRectEmpty(&rView);

		pT->Paint(dc, rClient, rView, rDest);
	}

	// Overrideables
	void Paint(CDCHandle /*dc*/, RECT& /*rClient*/, RECT& /*rView*/, RECT& /*rDest*/)
	{}

	void OnComposition()
	{}

	void OnColorization()
	{}

	BEGIN_MSG_MAP(CAeroImpl)
		CHAIN_MSG_MAP(CThemeImpl<T>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        MESSAGE_HANDLER(WM_DWMCOMPOSITIONCHANGED, OnCompositionChanged)
        MESSAGE_HANDLER(WM_DWMCOLORIZATIONCOLORCHANGED, OnColorizationChanged)
		CHAIN_MSG_MAP(CBufferedPaintImpl<T>)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (IsThemingSupported())
			OpenThemeData();

		if (IsComposing())
			::DwmExtendFrameIntoClientArea(static_cast<T*>(this)->m_hWnd, &m_Margins);
		return bHandled = FALSE;
	}

	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (!IsComposing() && IsTheming())
			static_cast<T*>(this)->Invalidate(FALSE);
		return bHandled = FALSE;
	}

	LRESULT OnCompositionChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (IsComposing())
			SetMargins(m_Margins);
		static_cast<T*>(this)->OnComposition();
		return bHandled = FALSE;
	}

	LRESULT OnColorizationChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		static_cast<T*>(this)->OnColorization();
		return bHandled = FALSE;
	}
};

///////////////////////////////////////////////////////////////////////////////
//  aero::CWindowImpl - Implements a Aero window

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>

class ATL_NO_VTABLE CWindowImpl :
	public ATL::CWindowImpl< T, TBase, TWinTraits >,
	public CAeroImpl<T>
{
public:
	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS, -1)

	CWindowImpl(LPCWSTR lpstrThemeClassList = L"window") : CAeroImpl<T>(lpstrThemeClassList)
	{}

	typedef CAeroImpl<T> _baseAero;

	void Paint(CDCHandle /*dc*/, RECT& /*rClient*/, RECT& /*rView*/, RECT& /*rDest*/)
	{}

	BEGIN_MSG_MAP(CWindowImpl)
		CHAIN_MSG_MAP(_baseAero)
	END_MSG_MAP()
};

///////////////////////////////////////////////////////////////////////////////
// aero::CDialogImpl - dialog implementation of Aero translucency (when available)

template <class T, class TBase  = ATL::CWindow>
class ATL_NO_VTABLE CDialogImpl :
	public ATL::CDialogImpl<T, TBase>,
	public CAeroImpl<T>
{
public:
	CDialogImpl(LPCWSTR lpstrThemeClassList = L"dialog") : CAeroImpl(lpstrThemeClassList)
	{}

	void Paint(CDCHandle dc, RECT& /*rClient*/, RECT& rView, RECT& rDest)
	{
		if (!::IsRectEmpty(&rView))
		{
			if (IsTheming())
				DrawThemeBackground(dc, WP_DIALOG, 1, &rView, &rDest);
			else
				dc.FillSolidRect(&rView, GetSysColor(COLOR_BTNFACE));
		}
	}

	BEGIN_MSG_MAP(CDialogImpl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CAeroImpl<T>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		SendMessage(WM_CREATE);
		if (IsThemingSupported())
			EnableThemeDialogTexture(ETDT_ENABLE);
		return bHandled = FALSE;
	}

};

#ifdef __ATLCTRLS_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CCtrl -  implementation of Aero drawing for system controls
// Note: This class is intended for system themed control specializations

template <class TBase>

class CCtrl :
	public WTL::CBufferedPaintWindowImpl<CCtrl<TBase>, TBase>,
	public CAeroBase<CCtrl<TBase> >
{
public:
	typedef CAeroBase<CCtrl<TBase> > baseAero;
	typedef WTL::CBufferedPaintWindowImpl<CCtrl<TBase>, TBase> baseWindow;

	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	// creation and initilization
	CCtrl(LPCWSTR lpstrThemeClassList = GetThemeName()) : baseAero(lpstrThemeClassList)
	{
		m_PaintParams.dwFlags = BPPF_ERASE;
	}

	CCtrl<TBase>& operator =(HWND hWnd)
	{
		TBase::m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		TBase baseCtrl;
		if (baseCtrl.Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam) != NULL)
			SubclassWindow(baseCtrl.m_hWnd);
		return m_hWnd;
	}

	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(IsSupported());
		if(baseWindow::SubclassWindow(hWnd))
			OpenThemeData();
		return m_hWnd != NULL;
	}

	// specializables
	static LPCWSTR GetThemeName()
	{
		return TBase::GetWndClassName();
	}

	void CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
	{
		DefCtrlPaint(hdc, rPaint);
	}

	// operations
	void DefCtrlPaint(HDC hdc, RECT& rPaint, bool bEraseBkGnd = false)
	{
		if (bEraseBkGnd)
			DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, NULL);
		DefWindowProc(WM_PAINT, (WPARAM)hdc, 0);
		m_BufferedPaint.MakeOpaque(&rPaint);
	}

	BOOL DrawCtrlBackground(HDC hdc, int nPartID, int nStateID, RECT &rCtrl, RECT &rPaint)
	{
		return SUCCEEDED(DrawThemeBackground(hdc, nPartID, nStateID, &rCtrl, &rPaint));
	}

	BOOL DrawCtrlEdge(HDC hdc, int nPartID, int nStateID, RECT &rCtrl, UINT uEdge = EDGE_ETCHED, UINT uFlags = BF_RECT, LPRECT pContentRect = NULL)
	{
		return SUCCEEDED(DrawThemeEdge(hdc, nPartID, nStateID, &rCtrl, uEdge, uFlags, pContentRect));
	}

	BOOL DrawCtrlText(CDCHandle dc, int nPartID, int nStateID, UINT uFormat, RECT &rCtrl, HFONT hFont = 0,
		DWORD dwFlags = DTT_COMPOSITED, int iGlowSize = 0)
	{
		HRESULT hr;
		RECT rText;
		hr = GetThemeBackgroundContentRect(dc, nPartID, nStateID, &rCtrl, &rText);
		MARGINS m = {0};
		hr = GetThemeMargins(dc, nPartID, nStateID, TMT_CONTENTMARGINS, &rText, &m);
		rText.left += m.cxLeftWidth;
		rText.right -= m.cxRightWidth;
		int iLength = GetWindowTextLength();
		if (iLength > 0)
		{
			CTempBuffer<WCHAR> sText(++iLength);
			GetWindowText(sText, iLength);

			HFONT hf = dc.SelectFont(hFont == 0 ? GetFont() : hFont);
			hr = DrawPartText(dc, nPartID, nStateID, sText,  &rText , uFormat, dwFlags, iGlowSize);
			dc.SelectFont(hf);
		}
		return SUCCEEDED(hr) && iLength > 0;
	}

	// implementation
	void DoBufferedPaint(HDC hdc, RECT& rPaint)
	{
		HDC hDCPaint = NULL;
		RECT rCtrl;
		GetClientRect(&rCtrl);
		m_BufferedPaint.Begin(hdc, &rCtrl, m_dwFormat, &m_PaintParams, &hDCPaint);
		ATLASSERT(hDCPaint != NULL);
		CtrlPaint(hDCPaint, rCtrl, rPaint);
		m_BufferedPaint.End();
	}

	void DoPaint(HDC /*hdc*/, RECT& /*rCtrl*/)
	{
		DefWindowProc();
	}

	BEGIN_MSG_MAP(CCtrl)
		MESSAGE_HANDLER(WM_PAINT, OnPaintMsg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnPaintMsg)
		CHAIN_MSG_MAP(baseAero)
		CHAIN_MSG_MAP(baseWindow)
	END_MSG_MAP()

	LRESULT OnPaintMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(!IsComposing())
			return DefWindowProc();
		else
			return bHandled = FALSE;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Macro declaring and subclassing a control (static declaration to use in OnCreate or OnInitDialog members)

#define AERO_CONTROL(type, name, id)\
	static aero::type name;\
	Subclass(name, id);

///////////////////////////////////////////////////////////////////////////////
// aero::CEdit - Aero drawing Edit control

class CEdit : public CCtrl<WTL::CEdit>

{
public:
	BEGIN_MSG_MAP(CEdit)
		MESSAGE_HANDLER(EM_SETSEL, OnRedraw)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnRedraw)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnRedraw)
		MESSAGE_HANDLER(WM_KEYDOWN, OnRedraw)
		MESSAGE_HANDLER(WM_PASTE, OnRedraw)
		CHAIN_MSG_MAP(CCtrl<WTL::CEdit>)
	END_MSG_MAP()

	LRESULT OnRedraw(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DefWindowProc();
		Invalidate(FALSE);
		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CTabCtrl - Aero drawing Tab control

typedef CCtrl<WTL::CTabCtrl> CTabCtrl;

inline LPCWSTR CTabCtrl::GetThemeName()
{
	return L"TAB";
};

inline void CTabCtrl::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& /*rPaint*/)
{
	DefWindowProc(WM_PAINT, (WPARAM)hdc, NULL);
	RECT rTab;
	for (int nTab = 0; GetItemRect(nTab, &rTab); nTab++)
	{
		rTab.right -= 3;
		rTab.bottom -= 3;
		m_BufferedPaint.MakeOpaque(&rTab);
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CReBarCtrl - Aero drawing ReBar control

typedef CCtrl<WTL::CReBarCtrl> CReBarCtrl;

inline LPCWSTR CReBarCtrl::GetThemeName()
{
	return L"REBAR";
};

inline void CReBarCtrl::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	m_BufferedPaint.Clear(&rPaint);

	CRect rect;
	GetWindowRect(rect);

	int nBandCount = GetBandCount();
	for(int i =0; i < nBandCount; i++)
	{
		REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO() };
		rbbi.fMask = RBBIM_STYLE | RBBIM_HEADERSIZE | RBBIM_CHILD;
		BOOL bRet = GetBandInfo(i, &rbbi);
		ATLASSERT(bRet);

		CWindow band(rbbi.hwndChild);

		if( band.IsWindowVisible() )
		{
			if( (rbbi.fStyle & RBBS_NOGRIPPER) == 0 )
			{
				CRect rectBand;
				band.GetWindowRect(rectBand);

				SIZE size = {0};
				GetThemePartSize(hdc, RP_GRIPPER, 0, NULL, TS_TRUE, &size);

				CRect rectGripper(
					rectBand.left - rect.left - rbbi.cxHeader,
					rectBand.top - rect.top,
					rectBand.left - rect.left - rbbi.cxHeader + size.cx,
					rectBand.bottom - rect.top);
				DrawThemeBackground(hdc, RP_GRIPPER, 0, rectGripper, &rPaint);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// aero::CToolBarCtrl - Aero translucent ToolBar Control

typedef CCtrl<WTL::CToolBarCtrl> CToolBarCtrl;

inline LPCWSTR CToolBarCtrl::GetThemeName()
{
	return L"TOOLBAR";
};

inline void CToolBarCtrl::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	int nBtn = GetButtonCount();
	int iHot = GetHotItem();

	CImageList img = GetImageList();
	CDCHandle dcPaint(hdc);

	int cx, cy;
	img.GetIconSize(cx, cy);

	for (int i = 0; i < nBtn; i++)
	{
		TBBUTTONINFO tbbi = {sizeof(TBBUTTONINFO), TBIF_BYINDEX | TBIF_STATE | TBIF_STYLE | TBIF_IMAGE};
		GetButtonInfo(i, &tbbi);

		RECT rBtn;
		GetItemRect(i, &rBtn);

		if( tbbi.fsStyle & BTNS_SEP )
		{
			DrawThemeBackground(hdc, TP_SEPARATOR, 0, &rBtn, &rPaint);
		}
		else
		{
			TOOLBARSTYLESTATES state = tbbi.fsState & TBSTATE_ENABLED ? tbbi.fsState & TBSTATE_CHECKED ? TS_CHECKED : TS_NORMAL : TS_DISABLED;
			if( state == TS_NORMAL && i == iHot )
				state = tbbi.fsState & TBSTATE_PRESSED ? TS_PRESSED : TS_HOT;

			BOOL hasDropDownArrow = (tbbi.fsStyle & BTNS_DROPDOWN) || (tbbi.fsStyle & BTNS_WHOLEDROPDOWN);
			BOOL drawSepDropDownArrow = hasDropDownArrow && (~tbbi.fsStyle & BTNS_WHOLEDROPDOWN);

			RECT rbmp;

			if( hasDropDownArrow )
			{
				int nDropDownArrowWidth = ::GetSystemMetrics(SM_CXMENUCHECK);

				if( drawSepDropDownArrow )
				{
					RECT rBtn2 = rBtn;
					rBtn2.right -= nDropDownArrowWidth;

					DrawThemeBackground(hdc, TP_SPLITBUTTON, state, &rBtn2, &rPaint);
					GetThemeBackgroundContentRect(hdc, TP_SPLITBUTTON, state, &rBtn2, &rbmp);

					rBtn2 = rBtn;
					rBtn2.left = rBtn2.right - nDropDownArrowWidth;

					DrawThemeBackground(hdc, TP_SPLITBUTTONDROPDOWN, state, &rBtn2, &rPaint);
				}
				else
				{
					RECT rBtn2 = rBtn;
					rBtn2.right = rBtn2.left + (rBtn2.bottom - rBtn2.top);

					DrawThemeBackground(hdc, TP_SPLITBUTTON, state, &rBtn, &rPaint);
					GetThemeBackgroundContentRect(hdc, TP_BUTTON, state, &rBtn2, &rbmp);

					rBtn2 = rBtn;
					rBtn2.left = rBtn2.right - nDropDownArrowWidth;

					DrawThemeBackground(hdc, TP_DROPDOWNBUTTONGLYPH, state, &rBtn2, &rPaint);
				}
			}
			else
			{
				DrawThemeBackground(hdc, TP_BUTTON, state, &rBtn, &rPaint);
				GetThemeBackgroundContentRect(hdc, TP_BUTTON, state, &rBtn, &rbmp);
			}

			int x = rbmp.left + (rbmp.right  - rbmp.left - cx) / 2;
			int y = rbmp.top  + (rbmp.bottom - rbmp.top  - cy) / 2;

			if( (tbbi.fsState & TBSTATE_PRESSED) || (tbbi.fsState & TBSTATE_CHECKED) ) x += 1;

			img.DrawEx(tbbi.iImage, hdc, x, y, cx, cy, CLR_NONE, CLR_NONE, (state == TS_DISABLED)? ILS_SATURATE : ILS_NORMAL);
		}

	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CStatusBarCtrl - Aero drawing StatusBar Control

typedef CCtrl<WTL::CStatusBarCtrl> CStatusBarCtrl;

inline LPCWSTR CStatusBarCtrl::GetThemeName()
{
	return L"STATUS";
};

inline void CStatusBarCtrl::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	DefCtrlPaint(hdc, rPaint, true);
}

///////////////////////////////////////////////////////////////////////////////
// aero::CListBox - Aero drawing ListBox control

typedef CCtrl<WTL::CListBox> CListBox;

inline void CListBox::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	DefCtrlPaint(hdc, rPaint, true);
}

///////////////////////////////////////////////////////////////////////////////
// aero::CComboBox - Aero drawing ComboBox control

inline void CCtrl<WTL::CComboBox>::CtrlPaint(HDC hdc, RECT& /*rCtrl*/, RECT& rPaint)
{
	DefCtrlPaint(hdc, rPaint, true);
}

class CComboBox : public CCtrl<WTL::CComboBox>

{
public:
	aero::CEdit m_AE;

	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(IsSupported());
		if(baseWindow::SubclassWindow(hWnd))
		{
			OpenThemeData();
			COMBOBOXINFO cbi = {sizeof(COMBOBOXINFO)};
			GetComboBoxInfo(&cbi);
			if (cbi.hwndItem != NULL)
				ATLVERIFY(aero::Subclass(m_AE, cbi.hwndItem));
		}
		return m_hWnd != NULL;
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CStatic - Aero drawing Static control

typedef CCtrl<WTL::CStatic> CStatic;

inline LPCWSTR CStatic::GetThemeName()
{
	return L"static::globals";
};

inline void CStatic::CtrlPaint(HDC hdc, RECT& rCtrl, RECT& rPaint)
{
	DWORD dwStyle = GetStyle();

	UINT uFormat = dwStyle & 0x3L;
	if ((dwStyle | SS_SIMPLE) == SS_SIMPLE)
		uFormat |= DT_SINGLELINE;
	else
		uFormat |= DT_WORDBREAK;


	switch (dwStyle & 0xfL)
	{
	case SS_ICON:
	case SS_BITMAP:
	case SS_ENHMETAFILE:
	case SS_BLACKRECT:
	case SS_GRAYRECT:
	case SS_WHITERECT:
	case SS_BLACKFRAME:
	case SS_GRAYFRAME:
	case SS_WHITEFRAME:
	case SS_OWNERDRAW:
		DefCtrlPaint(hdc, rPaint);
		break;
	default:
		DrawCtrlText(hdc, 1, 1, uFormat, rCtrl, 0, DTT_COMPOSITED | DTT_GLOWSIZE, 10);
	}
}

///////////////////////////////////////////////////////////////////////////////
// aero::CButton - Aero drawing Button control

class CButtonThemeHelper
{
public:
	CButtonThemeHelper(DWORD dwStyle, DWORD dwState, BOOL bEnabled)
	{
		m_bp = _GetPart(dwStyle);
		m_bs = _GetState(m_bp, dwState, bEnabled);
		m_fmt = _GetFormat(m_bp, dwStyle);
	}

	enum BUTTONPARTS m_bp;
	int m_bs;
	UINT m_fmt;

	static int _GetFormat(enum BUTTONPARTS bp, DWORD dwStyle)
	{
		int fmt = dwStyle & BS_MULTILINE ? DT_WORDBREAK : DT_SINGLELINE;
		switch (dwStyle & BS_CENTER)
		{
		case BS_LEFT:
			fmt |= DT_LEFT;
			break;
		case BS_RIGHT:
			fmt |= DT_RIGHT;
			break;
		case BS_CENTER:
			fmt |= DT_CENTER;
			break;
		default:
			fmt |= bp == BP_PUSHBUTTON ? DT_CENTER : DT_LEFT;
		}

		switch (dwStyle & BS_VCENTER)
		{
		case BS_TOP:
			fmt |= DT_TOP;
			break;
		case BS_BOTTOM:
			fmt |= DT_BOTTOM;
			break;
		case BS_VCENTER:
			fmt |= DT_VCENTER;
			break;
		default:
			if (fmt ^ BS_MULTILINE)
				fmt |= bp == BP_GROUPBOX ? DT_TOP : DT_VCENTER;
		}

		return fmt;
	}

	static int _GetBaseButtonState(DWORD dwState, BOOL bEnabled)
	{
		if (!bEnabled)
			return PBS_DISABLED;
		if (dwState & BST_PUSHED)
			return PBS_PRESSED;
		if (dwState & BST_HOT)
			return PBS_HOT;
		return PBS_NORMAL;
	}

	static int _GetState(enum BUTTONPARTS bp, DWORD dwState, BOOL bEnabled)
	{
		int bs = _GetBaseButtonState(dwState, bEnabled);
		switch (bp)
		{
		case BP_PUSHBUTTON:
		case BP_COMMANDLINK:
			if ((bs == PBS_NORMAL) && (dwState & BST_FOCUS))
				bs = PBS_DEFAULTED_ANIMATING;
			break;
		case BP_CHECKBOX:
			if (dwState & BST_INDETERMINATE)
			{
				bs += 8;
				break;
			}
		case BP_RADIOBUTTON:
			if (dwState & BST_CHECKED)
				bs += 4;
		}
		return bs;
	}

	static enum BUTTONPARTS _GetPart(DWORD dwStyle)
	{
		switch(dwStyle & 0xf)
		{
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
		case BS_3STATE:
		case BS_AUTO3STATE:
			return BP_CHECKBOX;
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
			return BP_RADIOBUTTON;
		case BS_GROUPBOX:
			return BP_GROUPBOX;
		case BS_USERBUTTON:
		case BS_OWNERDRAW:
			return BP_USERBUTTON;
		case BS_COMMANDLINK:
		case BS_DEFCOMMANDLINK:
			return BP_COMMANDLINK;
		default:
			return BP_PUSHBUTTON;
		}
	}
};

typedef CCtrl<WTL::CButton> CButton;

inline void CButton::CtrlPaint(HDC hdc, RECT& rCtrl, RECT& rPaint)
{
		HRESULT hr;
		CButtonThemeHelper helper(GetButtonStyle(), GetState(), IsWindowEnabled());
		switch (helper.m_bp)
		{
		case BP_USERBUTTON:
			DefCtrlPaint(hdc, rPaint);
			return;

		case BP_GROUPBOX:
			DefWindowProc(WM_PAINT, (WPARAM)hdc, 0);
			{
				int iLength = GetWindowTextLength();
				if (iLength > 0)
				{
					RECT rText;
					CDCHandle dc(hdc);
					CTempBuffer<WCHAR> sText(++iLength);
					GetWindowText(sText, iLength);
					HFONT hf = dc.SelectFont(GetFont());
					hr = GetThemeTextExtent(dc, helper.m_bp, helper.m_bs, sText, -1, helper.m_fmt, &rCtrl, &rText);
					ATLASSERT(SUCCEEDED(hr));
					OffsetRect(&rText, 10, 0);
					m_BufferedPaint.Clear(&rText);
					hr = DrawPartText(dc, helper.m_bp, helper.m_bs,  (LPCWSTR)sText, &rText , helper.m_fmt, DTT_COMPOSITED | DTT_GLOWSIZE, 10);
					ATLASSERT(SUCCEEDED(hr));
					dc.SelectFont(hf);
				}
			}
			return;

		case BP_RADIOBUTTON:
		case BP_CHECKBOX:
			{
				CBitmapHandle bm;
				SIZE s;
				RECT rBitmap = rCtrl;
				hr = GetThemeBitmap(helper.m_bp, helper.m_bs, 0, GBF_DIRECT, bm.m_hBitmap);
				ATLASSERT(SUCCEEDED(hr));
				bm.GetSize(s);
				if (GetButtonStyle() & BS_RIGHTBUTTON)
					rCtrl.right = rBitmap.left = rBitmap.right - s.cx - s.cx / 2;
				else
					rCtrl.left = rBitmap.right = rBitmap.left + s.cx + s.cx / 2;
				hr = DrawThemeBackground(hdc, helper.m_bp, helper.m_bs, &rBitmap, NULL);
				ATLASSERT(SUCCEEDED(hr));
			}
			break;

		default:
			hr = DrawCtrlBackground(hdc, helper.m_bp, helper.m_bs, rCtrl, rPaint);
			ATLASSERT(SUCCEEDED(hr));
		}
		hr = DrawCtrlText(hdc, helper.m_bp, helper.m_bs, helper.m_fmt, rCtrl, 0);
		ATLASSERT(SUCCEEDED(hr));
}

#ifdef __ATLDLGS_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CPropertySheetImpl - Property Sheet implementation of Aero translucency (when available)

template <class T, class TBase  = WTL::CPropertySheetWindow>

class ATL_NO_VTABLE CPropertySheetImpl :
	public WTL::CPropertySheetImpl<T, TBase>,
	public CAeroImpl<T>
{
	typedef WTL::CPropertySheetImpl<T, TBase> basePS;
public:

	CPropertySheetImpl(ATL::_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL):
		basePS(title, uStartPage, hWndParent) , CAeroImpl(L"window")
	{}

	aero::CTabCtrl m_Atab;
	aero::CButton m_btns[PSBTN_MAX];

	void Paint(CDCHandle dc, RECT& /*rClient*/, RECT& rView, RECT& rDest)
	{
		if (!::IsRectEmpty(&rView))
		{
			if (IsTheming())
				DrawThemeBackground(dc, WP_DIALOG, 1, &rView, &rDest);
			else
				dc.FillSolidRect(&rView, GetSysColor(COLOR_WINDOW));
		}
	}

	void InitPS()
	{
		aero::Subclass(m_Atab, GetTabControl());
		for (int idBtn = 0 ; idBtn < PSBTN_MAX; idBtn++)
			if (HWND hBtn = GetDlgItem(idBtn))
				aero::Subclass(m_btns[idBtn], hBtn);
		SendMessage(WM_CREATE);
		if (IsThemingSupported())
			EnableThemeDialogTexture(ETDT_ENABLE);
	}

// Callback function
	static int CALLBACK PropSheetCallback(HWND hWnd, UINT uMsg, LPARAM lParam)
	{
		lParam;   // avoid level 4 warning
		int nRet = 0;

		if(uMsg == PSCB_INITIALIZED)
		{
			ATLASSERT(hWnd != NULL);
			T* pT = (T*)ModuleHelper::ExtractCreateWndData();
			// subclass the sheet window
			pT->SubclassWindow(hWnd);
			// remove page handles array
			pT->_CleanUpPages();

			pT->InitPS();
			pT->OnSheetInitialized();
		}

		return nRet;
	}

	BEGIN_MSG_MAP(CPropertySheetImpl)
		CHAIN_MSG_MAP(CAeroImpl<T>)
		CHAIN_MSG_MAP(basePS)
	END_MSG_MAP()
};

///////////////////////////////////////////////////////////////////////////////
// aero::CPropertyPageImpl - Property page implementation of Aero translucency (when available)

template <class T, class TBase = WTL::CPropertyPageWindow>
class ATL_NO_VTABLE CPropertyPageImpl :
	public WTL::CPropertyPageImpl<T, TBase>,
	public CAeroImpl<T>

{
public:
	typedef WTL::CPropertyPageImpl<T, TBase> basePP;

	CPropertyPageImpl(ATL::_U_STRINGorID title = (LPCTSTR)NULL) :
		basePP(title),
		CAeroImpl<T>(L"dialog")
	{}

	void Paint(CDCHandle dc, RECT& /*rClient*/, RECT& rView, RECT& rDest)
	{
		if (!::IsRectEmpty(&rView))
		{
			if (IsTheming())
				DrawThemeBackground(dc, WP_DIALOG, 1, &rView, &rDest);
			else
				dc.FillSolidRect(&rView, GetSysColor(COLOR_WINDOW));
		}
	}

	BEGIN_MSG_MAP(CPropertyPageImpl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CAeroImpl<T>)
		CHAIN_MSG_MAP(basePP)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		SendMessage(WM_CREATE);
		if (IsThemingSupported())
			EnableThemeDialogTexture(ETDT_ENABLE);
		return bHandled = FALSE;
	}
};
#endif // __ATLDLGS_H__


#ifdef __ATLFRAME_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CFrameWindowImpl - Aero frame implementation

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CFrameWinTraits>
class ATL_NO_VTABLE CFrameWindowImpl :
	public WTL::CFrameWindowImpl<T, TBase, TWinTraits>,
	public CAeroImpl<T>

{
	typedef WTL::CFrameWindowImpl<T, TBase, TWinTraits> _baseClass;

public:
	CFrameWindowImpl(LPCWSTR lpstrThemeClassList = L"window") : CAeroImpl(lpstrThemeClassList)
	{}

	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
		RECT rect = { 0 };
		GetClientRect(&rect);

		// position margins
		if (m_Margins.cxLeftWidth != -1)
		{
			rect.left += m_Margins.cxLeftWidth;
			rect.top += m_Margins.cyTopHeight;
			rect.right -= m_Margins.cxRightWidth;
			rect.bottom -= m_Margins.cyBottomHeight;
		}

		// position bars and offset their dimensions
		UpdateBarsPosition(rect, bResizeBars);

		// resize client window
		if(m_hWndClient != NULL)
			::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE);

		Invalidate(FALSE);
	}

	void UpdateBarsPosition(RECT& rect, BOOL bResizeBars = TRUE)
	{
		// resize toolbar
		if(m_hWndToolBar != NULL && ((DWORD)::GetWindowLong(m_hWndToolBar, GWL_STYLE) & WS_VISIBLE))
		{
			RECT rectTB = { 0 };
			::GetWindowRect(m_hWndToolBar, &rectTB);
			if(bResizeBars)
			{
        // we move bars 2 pixels left to remove a stupid border drawed by the rebar
				::SetWindowPos(m_hWndToolBar, NULL, rect.left - 2, rect.top,
					rect.right - rect.left + 2, rectTB.bottom - rectTB.top,
					SWP_NOZORDER | SWP_NOACTIVATE);
				::InvalidateRect(m_hWndToolBar, NULL, FALSE);
			}
			rect.top += rectTB.bottom - rectTB.top;
		}

		// resize status bar
		if(m_hWndStatusBar != NULL && ((DWORD)::GetWindowLong(m_hWndStatusBar, GWL_STYLE) & WS_VISIBLE))
		{
			RECT rectSB = { 0 };
			::GetWindowRect(m_hWndStatusBar, &rectSB);
			rect.bottom -= rectSB.bottom - rectSB.top;
			if(bResizeBars)
				::SetWindowPos(m_hWndStatusBar, NULL, rect.left, rect.bottom,
					rect.right - rect.left, rectSB.bottom - rectSB.top,
					SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	aero::CStatusBarCtrl m_ASB;
	aero::CToolBarCtrl m_ATB;

	BOOL CreateSimpleStatusBar(LPCTSTR lpstrText, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP, UINT nID = ATL_IDW_STATUS_BAR)
	{
		ATLASSERT(!::IsWindow(m_hWndStatusBar));
		m_hWndStatusBar = ::CreateStatusWindow(dwStyle | CCS_NOPARENTALIGN , lpstrText, m_hWnd, nID);
		aero::Subclass(m_ASB, m_hWndStatusBar);
		return (m_hWndStatusBar != NULL);
	}

	BOOL CreateSimpleStatusBar(UINT nTextID = ATL_IDS_IDLEMESSAGE, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP, UINT nID = ATL_IDW_STATUS_BAR)
	{
		const int cchMax = 128;   // max text length is 127 for status bars (+1 for null)
		TCHAR szText[cchMax];
		szText[0] = 0;
		::LoadString(ModuleHelper::GetResourceInstance(), nTextID, szText, cchMax);
		return CreateSimpleStatusBar(szText, dwStyle, nID);
	}

	HWND CreateAeroToolBarCtrl(HWND hWndParent, UINT nResourceID, BOOL bInitialSeparator = FALSE,
			DWORD dwStyle = ATL_SIMPLE_TOOLBAR_STYLE, UINT nID = ATL_IDW_TOOLBAR)
	{
		HWND hWndToolBar = _baseClass::CreateSimpleToolBarCtrl(hWndParent, nResourceID, bInitialSeparator, dwStyle, nID);
		if (hWndToolBar != NULL)
			aero::Subclass(m_ATB, hWndToolBar);
		return hWndToolBar;
	}

	HWND CreateSimpleReBarCtrl(HWND hWndParent, DWORD dwStyle = ATL_SIMPLE_REBAR_STYLE, UINT nID = ATL_IDW_TOOLBAR)
	{
		HWND hRB = _baseClass::CreateSimpleReBarCtrl(hWndParent, dwStyle | CCS_NOPARENTALIGN, nID);
		return hRB;
	}

	BOOL CreateSimpleReBar(DWORD dwStyle = ATL_SIMPLE_REBAR_STYLE, UINT nID = ATL_IDW_TOOLBAR)
	{
		ATLASSERT(!::IsWindow(m_hWndToolBar));
		m_hWndToolBar = CreateSimpleReBarCtrl(m_hWnd, dwStyle | CCS_NOPARENTALIGN, nID);
		return (m_hWndToolBar != NULL);
	}

	BEGIN_MSG_MAP(CFrameWindowImpl)
		CHAIN_MSG_MAP(CAeroImpl<T>)
		CHAIN_MSG_MAP(_baseClass)
	END_MSG_MAP()
};
#endif // __ATLFRAME_H__
#endif // __ATLCTRLS_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CCtrlImpl -  implementation of Aero drawing for user and WTL defined controls
// Note: This class is intended for derivation

template <class T, class TCtrlImpl, bool t_bOpaque = false>
class ATL_NO_VTABLE CCtrlImpl :
	public TCtrlImpl,
	public CAeroImpl<T>

{
public:
	DECLARE_WND_SUPERCLASS(NULL, TCtrlImpl::GetWndClassName())

	CCtrlImpl(LPCWSTR lpstrThemeClassList = L"window") : CAeroImpl(lpstrThemeClassList)
	{
		m_PaintParams.dwFlags = BPPF_ERASE;
	}

	void DoPaint(HDC hdc, RECT& rect)
	{
		BOOL bHandled = TRUE;
		TCtrlImpl::OnPaint(WM_PAINT, (WPARAM) hdc, NULL, bHandled);
		if (t_bOpaque)
			m_BufferedPaint.MakeOpaque(&rect);
	}

	BEGIN_MSG_MAP(CCtrlImpl)
		CHAIN_MSG_MAP(CAeroImpl<T>)
		CHAIN_MSG_MAP(TCtrlImpl)
	END_MSG_MAP()

};

#ifdef __ATLCTRLW_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CCommandBarCtrl - Aero drawing WTL::CCommandBarCtrl control

class CCommandBarCtrl :
	public CCtrlImpl<CCommandBarCtrl, WTL::CCommandBarCtrlImpl<CCommandBarCtrl> >
{
public:

	DECLARE_WND_SUPERCLASS(_T("WTL_AeroCommandBar"), GetWndClassName())

	CCommandBarCtrl() : CCtrlImpl(L"MENU")
	{}

	void DoPaint(HDC hdc, RECT&)
	{
		DefWindowProc(WM_PAINT, (WPARAM) hdc, NULL);
	}

	BEGIN_MSG_MAP(CCommandBarCtrl)
		CHAIN_MSG_MAP(CThemeImpl<CCommandBarCtrl>)
		CHAIN_MSG_MAP(CBufferedPaintImpl<CCommandBarCtrl>)
		CHAIN_MSG_MAP(WTL::CCommandBarCtrlImpl<CCommandBarCtrl>)
	ALT_MSG_MAP(1)   // Parent window messages
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnParentCustomDraw)
		CHAIN_MSG_MAP_ALT(WTL::CCommandBarCtrlImpl<CCommandBarCtrl>, 1)
	ALT_MSG_MAP(3)   // Message hook messages
		CHAIN_MSG_MAP_ALT(WTL::CCommandBarCtrlImpl<CCommandBarCtrl>, 3)
		END_MSG_MAP()

	LRESULT OnParentCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		if (!IsTheming())
			return WTL::CCommandBarCtrlImpl<CCommandBarCtrl>::OnParentCustomDraw(idCtrl, pnmh, bHandled);

		LRESULT lRet = CDRF_DODEFAULT;
		bHandled = FALSE;
		if(pnmh->hwndFrom == m_hWnd)
		{
			LPNMTBCUSTOMDRAW lpTBCustomDraw = (LPNMTBCUSTOMDRAW)pnmh;
			CDCHandle dc = lpTBCustomDraw->nmcd.hdc;
			RECT& rc = lpTBCustomDraw->nmcd.rc;

			if(lpTBCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
			{
				DrawThemeBackground(dc, MENU_BARBACKGROUND, m_bParentActive ? MB_ACTIVE : MB_INACTIVE, &rc);
				lRet = CDRF_NOTIFYITEMDRAW;
				bHandled = TRUE;
			}
			else if(lpTBCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
			{
				bool bHot = ((lpTBCustomDraw->nmcd.uItemState & CDIS_HOT) == CDIS_HOT);
				bool bPushed = ((lpTBCustomDraw->nmcd.uItemState & CDIS_SELECTED) == CDIS_SELECTED);
				bool bDisabled = ((lpTBCustomDraw->nmcd.uItemState & CDIS_DISABLED) == CDIS_DISABLED);

				int bis = bPushed ? MBI_PUSHED : bHot ? MBI_HOT : MBI_NORMAL;

				if(m_bFlatMenus)
				{
					if (bDisabled || (!m_bParentActive && !bPushed && !bHot))
						bis += 3;
					if ((bHot || bPushed) && !bDisabled)
						DrawThemeBackground(dc, MENU_POPUPITEM, MBI_HOT, &rc);
				}
				else
				{
					if (!m_bParentActive ||  bDisabled)
						bis += 3;
					DrawThemeBackground(dc, MENU_BARITEM, bis, &rc);
				}

				CLogFont lf;
				HRESULT hr = GetThemeSysFont(TMT_MENUFONT, &lf);
				CFont hFont = (hr == S_OK) ? lf.CreateFontIndirect() : GetFont();
				HFONT hFontOld = NULL;
				if(hFont != NULL)
					hFontOld = dc.SelectFont(hFont);

				CTempBuffer<WCHAR> szText(200);
				TBBUTTONINFO tbbi = {sizeof(TBBUTTONINFO), TBIF_TEXT};
				tbbi.pszText = szText;
				tbbi.cchText = 200;
				GetButtonInfo((int)lpTBCustomDraw->nmcd.dwItemSpec, &tbbi);

				hr = DrawPartText(dc, MENU_BARITEM, bis, szText, &rc,
					DT_SINGLELINE | DT_CENTER | DT_VCENTER | (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX));

				if(hFont != NULL)
					dc.SelectFont(hFontOld);
				lRet = CDRF_SKIPDEFAULT;
				bHandled = TRUE;
			}
		}
		return lRet;
	}

};
#endif // __ATLCTRLW_H__

#ifdef __ATLPRINT_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CPrintPreviewWindow - Aero drawing WTL::CPrintPreviewWindow control

class CPrintPreviewWindow : public aero::CCtrlImpl<CPrintPreviewWindow, WTL::CPrintPreviewWindow>

{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_AeroPrintPreview"), CS_VREDRAW | CS_HREDRAW, -1)

	void DoPaint(CDCHandle dc, RECT& rc)
	{
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);
		RECT rcArea = rcClient;
		::InflateRect(&rcArea, -m_cxOffset, -m_cyOffset);
		if (rcArea.left > rcArea.right)
			rcArea.right = rcArea.left;
		if (rcArea.top > rcArea.bottom)
			rcArea.bottom = rcArea.top;
		GetPageRect(rcArea, &rc);
		if (!aero::IsComposing())
		{
			CRgn rgn1, rgn2;
			rgn1.CreateRectRgnIndirect(&rc);
			rgn2.CreateRectRgnIndirect(&rcClient);
			rgn2.CombineRgn(rgn1, RGN_DIFF);
			dc.SelectClipRgn(rgn2);
			if (IsTheming())
				DrawThemeBackground(dc, WP_FRAMEBOTTOM, m_hWnd == GetFocus() ? FS_ACTIVE : FS_INACTIVE,
					&rcClient);
			else
				dc.FillRect(&rcClient, ::GetSysColor(COLOR_BTNSHADOW));
			dc.SelectClipRgn(NULL);
		}
		dc.FillRect(&rc, (HBRUSH)::GetStockObject(WHITE_BRUSH));

		WTL::CPrintPreviewWindow::DoPaint(dc, rc);
		m_BufferedPaint.MakeOpaque(&rc);
	}

};

#endif // __ATLPRINT_H__

//#ifdef __ATLCTRLX_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CTabView - Aero drawing WTL::CTabView control

class CTabView :
	public CCtrlImpl<CTabView, WTL::CTabViewImpl<CTabView> >
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_AeroTabView"), 0, COLOR_APPWORKSPACE)

	aero::CTabCtrl m_Atab;

	bool CreateTabControl()
	{
		if (aero::IsSupported())
		{
			m_Atab.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_TOOLTIPS, 0, m_nTabID);
			m_tab.SubclassWindow(m_Atab);
		}
		else
			m_tab.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_TOOLTIPS, 0, m_nTabID);

		ATLASSERT(m_tab.m_hWnd != NULL);
		if(m_tab.m_hWnd == NULL)
			return false;

		m_tab.SetFont(AtlGetDefaultGuiFont());
		m_tab.SetItemExtra(sizeof(TABVIEWPAGE));
		m_cyTabHeight = CalcTabHeight();

		return true;
	}

	void GetMoveMarkRect(RECT& rect) const
	{
		m_tab.GetClientRect(&rect);

		RECT rcItem = { 0 };
		m_tab.GetItemRect(m_nInsertItem, &rcItem);

		int cxMoveMark = IsComposing() ? 0 : m_cxMoveMark / 2;

		if(m_nInsertItem <= m_nActivePage)
		{
			rect.left = rcItem.left - cxMoveMark - 1;
			rect.right = rcItem.left + cxMoveMark + 1;
		}
		else
		{
			rect.left = rcItem.right - cxMoveMark - 1;
			rect.right = rcItem.right + cxMoveMark + 1;
		}
	}

	void DrawMoveMark(int nItem)
	{
		if (!IsComposing())
			return WTL::CTabViewImpl<CTabView>::DrawMoveMark(nItem);

		RECT rect = { 0 };
		if(m_nInsertItem != -1)
		{
			GetMoveMarkRect(rect);
			m_tab.InvalidateRect(&rect);
		}

		m_nInsertItem = nItem;

		if(m_nInsertItem != -1)
		{
			GetMoveMarkRect(rect);

			CClientDC dcTab(m_tab.m_hWnd);
			HDC hDCPaint = NULL;
			m_BufferedPaint.Begin(dcTab, &rect, m_dwFormat, &m_PaintParams, &hDCPaint);
			ATLASSERT(hDCPaint != NULL);
			CDC dc(hDCPaint);

			CPen pen;
			pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_HOTLIGHT));
			CBrush brush;
			brush.CreateSolidBrush(::GetSysColor(COLOR_HOTLIGHT));

			HPEN hPenOld = dc.SelectPen(pen);
			HBRUSH hBrushOld = dc.SelectBrush(brush);

			dc.Rectangle(&rect);

			dc.SelectPen(hPenOld);
			dc.SelectBrush(hBrushOld);

			m_BufferedPaint.MakeOpaque(&rect);
			m_BufferedPaint.End();
		}
	}

	void DoPaint(HDC hdc, RECT& rDest)
	{
		if (!IsComposing())
		{
			RECT rClient;
			GetClientRect(&rClient);
			if (IsTheming())
				DrawThemeBackground(hdc, WP_FRAMEBOTTOM, FS_INACTIVE, &rClient, &rDest);
			else
			{
				DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, NULL);
				if (!m_BufferedPaint.IsNull())
					m_BufferedPaint.MakeOpaque(&rDest);
			}
		}

		DefWindowProc(WM_PAINT, (WPARAM)hdc, NULL);
	}

// Message map and handlers
	BEGIN_MSG_MAP(CTabView)
		CHAIN_MSG_MAP(CAeroImpl<CTabView>)
		CHAIN_MSG_MAP(WTL::CTabViewImpl<CTabView>)
	ALT_MSG_MAP(1)   // tab control
		CHAIN_MSG_MAP_ALT(WTL::CTabViewImpl<CTabView>, 1)
	END_MSG_MAP()

};

///////////////////////////////////////////////////////////////////////////////
// aero::CPaneContainer - Aero drawing WTL::CPaneContainer control

class CPaneContainer :
	public CCtrlImpl<CPaneContainer, WTL::CPaneContainerImpl<CPaneContainer> >
{
	// not using aero::CToolBarCtrl
	class CPaneToolBar : public WTL::CToolBarCtrl
	{};
	CCtrl<CPaneToolBar> m_Atb;

public:
	void CreateCloseButton()
	{
		WTL::CPaneContainerImpl<CPaneContainer>::CreateCloseButton();
		aero::Subclass(m_Atb, m_tb.m_hWnd);
	}

	void DrawPaneTitle(CDCHandle dc)
	{
		WTL::CPaneContainerImpl<CPaneContainer>::DrawPaneTitle(dc);

		if (IsComposing() && !m_BufferedPaint.IsNull())
		{
			RECT rect = { 0 };
			GetClientRect(&rect);
			if(IsVertical())
				rect.right = rect.left + m_cxyHeader;
			else
				rect.bottom = rect.top + m_cxyHeader;

			m_BufferedPaint.MakeOpaque(&rect);
		}
	}

	// called only if pane is empty
	void DrawPane(CDCHandle dc)
	{
		RECT rect = { 0 };
		GetClientRect(&rect);
		if(IsVertical())
			rect.left += m_cxyHeader;
		else
			rect.top += m_cxyHeader;

		if((GetExStyle() & WS_EX_CLIENTEDGE) == 0)
				dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
		if (!IsComposing())
				dc.FillRect(&rect, COLOR_APPWORKSPACE);
	}

};

//#endif // __ATLCTRLX_H__

#ifdef __ATLSPLIT_H__

///////////////////////////////////////////////////////////////////////////////
// aero::CSplitterImpl<T, t_bVertical> - Provides Aero drawing splitter support to any window

// Aero splitter extended style
#define SPLIT_NONTRANSPARENT		0x00000008

template <class T, bool t_bVertical = true>

class CSplitterImpl :
	public WTL::CSplitterImpl<T, t_bVertical>
{
public:
	typedef WTL::CSplitterImpl<T, t_bVertical> baseSplit;

	// called only if pane is empty
	void DrawSplitterPane(CDCHandle dc, int nPane)
	{
		T* pT = static_cast<T*>(this);
		RECT rect;
		if(GetSplitterPaneRect(nPane, &rect))
		{
			if((pT->GetExStyle() & WS_EX_CLIENTEDGE) == 0)
				dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);

			if (!aero::IsComposing())
				dc.FillRect(&rect, COLOR_APPWORKSPACE);
		}
	}

	void DrawSplitterBar(CDCHandle dc)
	{
		T* pT = static_cast<T*>(this);
		if (aero::IsComposing())
		{
			if (GetSplitterExtendedStyle() & SPLIT_NONTRANSPARENT)
			{
				RECT rect;
				GetSplitterBarRect(&rect);
				pT->DrawThemeBackground(dc, WP_FRAMEBOTTOM, pT->m_hWnd == GetFocus() ? FS_ACTIVE : FS_INACTIVE, &rect) ;
			}
		}
		else
			baseSplit::DrawSplitterBar(dc);
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CSplitterWindowImpl - Implements an Aero drawing splitter window

template <class T, bool t_bVertical = true, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CSplitterWindowImpl :
	public aero::CWindowImpl<T, TBase, TWinTraits>,
	public aero::CSplitterImpl<T, t_bVertical>

{
public:
	typedef aero::CWindowImpl<T, TBase, TWinTraits> baseClass;

	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS, COLOR_WINDOW)

	void DoPaint(HDC hdc, RECT&)
	{
		if(m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			SetSplitterPos();
		DrawSplitter(hdc);
	}

	BEGIN_MSG_MAP(CSplitterWindowImpl)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseSplit)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SetSplitterRect();

		return bHandled = FALSE;
	}
};

///////////////////////////////////////////////////////////////////////////////
// aero::CSplitterWindow - Implements an Aero drawing splitter window to be used as is

template <bool t_bVertical = true>
class CSplitterWindowT : public aero::CSplitterWindowImpl<CSplitterWindowT<t_bVertical>, t_bVertical>
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_AeroSplitterWindow"), CS_DBLCLKS, COLOR_WINDOW)
};

typedef CSplitterWindowT<true>    CSplitterWindow;
typedef CSplitterWindowT<false>   CHorSplitterWindow;

#endif // __ATLSPLIT_H__

}; // namespace aero
}; // namespace WTL

#endif // __WTL_AERO_H__


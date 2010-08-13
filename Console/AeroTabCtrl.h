#pragma once

template<typename T, typename TItem = CCustomTabItem, class TBase = ATL::CWindow, class TWinTraits = CCustomTabCtrlWinTraits>
class CAeroTabCtrlImpl :
	public CDotNetTabCtrlImpl<T, TItem, TBase, TWinTraits>
{
protected:
	typedef CAeroTabCtrlImpl<T, TItem, TBase, TWinTraits> thisClass;
	typedef CDotNetTabCtrlImpl<T, TItem, TBase, TWinTraits> baseClass;

  signed char m_iMargin;
  signed char m_iLeftSpacing;
  signed char m_iTopMargin;
  signed char m_iRadius;

// Constructor
public:

	CAeroTabCtrlImpl()
	{
		// We can't use a member initialization list to initialize
		// members of our base class, so do it explictly by assignment here.
		m_clrTextInactiveTab = /*RGB(255,255,255); */::GetSysColor(COLOR_BTNTEXT);
		m_clrSelectedTab = ::GetSysColor(COLOR_WINDOW);
	}

// Message Handling
public:
	DECLARE_WND_CLASS_EX(_T("WTL_CAeroTabCtrl"), CS_DBLCLKS, COLOR_WINDOW)

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSettingChange)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DWORD dwStyle = this->GetStyle();

		// Initialize/Reinitialize font
		// Visual Studio.Net seems to use the "icon" font for the tabs
		LOGFONT lfIcon = { 0 };
		::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfIcon), &lfIcon, 0);

		bool bResetFont = true;
		if(!m_font.IsNull())
		{
			LOGFONT lf = {0};
			if(m_font.GetLogFont(&lf))
			{
				if(lstrcmpi(lf.lfFaceName, lfIcon.lfFaceName) == 0 &&
					lf.lfHeight == lfIcon.lfHeight)
				{
					bResetFont = false;
				}
			}
		}

		if(bResetFont)
		{
			if(!m_font.IsNull()) m_font.DeleteObject();
			if(!m_fontSel.IsNull()) m_fontSel.DeleteObject();

			HFONT font = m_font.CreateFontIndirect(&lfIcon);
			if(font==NULL)
			{
				m_font.Attach(AtlGetDefaultGuiFont());
			}

			if(CTCS_BOLDSELECTEDTAB == (dwStyle & CTCS_BOLDSELECTEDTAB))
			{
				lfIcon.lfWeight = FW_BOLD;
			}

			font = m_fontSel.CreateFontIndirect(&lfIcon);
			if(font==NULL)
			{
				m_fontSel.Attach(AtlGetDefaultGuiFont());
			}
		}

		// Background brush
		if(!m_hbrBackground.IsNull() ) m_hbrBackground.DeleteObject();

		m_hbrBackground.CreateSysColorBrush(COLOR_BTNFACE);

    m_iMargin = 6;
    m_iLeftSpacing = 2;
    m_iTopMargin = 4;
    m_iRadius = 3;
		m_settings.iIndent = 5;
		m_settings.iPadding = 4;
		m_settings.iMargin = m_iMargin + m_iLeftSpacing / 2;
		m_settings.iSelMargin = 6;

		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();
		pT->Invalidate();
		return 0;
	}

// Overrides for painting from CDotNetTabCtrlImpl
public:

	void DrawBackground(RECT /*rcClient*/, LPNMCTCCUSTOMDRAW lpNMCustomDraw)
	{
    Gdiplus::Graphics g(lpNMCustomDraw->nmcd.hdc);
    g.Clear(Gdiplus::Color(0,0,0,0));
	}

  void DrawTab(RECT& rcTab, Gdiplus::Graphics& g, Gdiplus::Color& color)
  {
    Gdiplus::SolidBrush brush(color);

    Gdiplus::GraphicsPath gp;

    INT radius = m_iRadius;
    INT X      = rcTab.left + m_iLeftSpacing;
    INT Y      = rcTab.top + m_iTopMargin;
    INT width  = rcTab.right  - rcTab.left - m_iLeftSpacing - 2;
    INT height = rcTab.bottom - rcTab.top  - m_iTopMargin;

    /*
    gp.AddLine(X + radius, Y, X + width - (radius * 2), Y);
    gp.AddArc(X + width - (radius * 2), Y, radius * 2, radius * 2, 270, 90);
    gp.AddLine(X + width, Y + radius, X + width, Y + height);
    gp.AddLine(X + width, Y + height, X, Y + height);
    gp.AddLine(X, Y + height, X, Y + radius);
    gp.AddArc(X, Y, radius * 2, radius * 2, 180, 90);
    */
    gp.AddLine(X + radius, Y, X + width - radius, Y);
    gp.AddLine(X + width - radius, Y, X + width, Y + radius);
    gp.AddLine(X + width, Y + radius, X + width, Y + height);
    gp.AddLine(X + width, Y + height, X, Y + height);
    gp.AddLine(X, Y + height, X, Y + radius);
    gp.AddLine(X, Y + radius, X + radius, Y);

    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    Gdiplus::Pen pen(color,1.0);
    g.DrawPath(&pen, &gp);

    g.SetSmoothingMode(Gdiplus::SmoothingModeNone);
    g.FillPath(&brush, &gp);

    radius += 2;
    X      -= 1;
    Y      -= 1;
    width  += 2;
    height += 2;

    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    Gdiplus::Pen pen2(Gdiplus::Color(64, 0, 0,0),1.0);

    g.DrawLine(&pen2, X, Y + radius, X + radius, Y);
    g.DrawLine(&pen2, X, Y + height, X, Y + radius);
    g.DrawLine(&pen2, X + radius, Y, X + width - radius, Y);
    g.DrawLine(&pen2, X + width - radius, Y, X + width, Y + radius);
    g.DrawLine(&pen2, X + width, Y + radius, X + width, Y + height);
  }

	void DrawItem_InitBounds(DWORD /*dwStyle*/, RECT /*rcItem*/, RECT& rcTab, RECT& rcText, int& nIconVerticalCenter)
	{
    rcText.top += m_iTopMargin;
		rcText.left += (m_iLeftSpacing + m_iMargin);
		rcText.right -= m_iMargin;
		nIconVerticalCenter = (rcTab.bottom + rcTab.top - m_iTopMargin) / 2 + m_iTopMargin;
	}

	void DrawItem_TabInactive(DWORD /*dwStyle*/, LPNMCTCCUSTOMDRAW lpNMCustomDraw, RECT& rcTab)
	{
	}

	void DrawItem_TabSelected(DWORD /*dwStyle*/, LPNMCTCCUSTOMDRAW lpNMCustomDraw, RECT& rcTab)
	{
	}

  void DrawItem_ImageAndText(DWORD /*dwStyle*/, LPNMCTCCUSTOMDRAW lpNMCustomDraw, int nIconVerticalCenter, RECT& rcTab, RECT& rcText)
	{
    HDC targetDC = lpNMCustomDraw->nmcd.hdc;
    HDC bufferedDC = NULL;
    BP_PAINTPARAMS m_PaintParams = { sizeof(BP_PAINTPARAMS) };
    HPAINTBUFFER pb = BeginBufferedPaint(targetDC, &rcTab, BPBF_TOPDOWNDIB, &m_PaintParams, &bufferedDC);
    BufferedPaintClear(pb, &rcTab);
    Gdiplus::Graphics g(bufferedDC);

		bool bHighlighted = (CDIS_MARKED == (lpNMCustomDraw->nmcd.uItemState & CDIS_MARKED));
		bool bSelected = (CDIS_SELECTED == (lpNMCustomDraw->nmcd.uItemState & CDIS_SELECTED));
		bool bHot = (CDIS_HOT == (lpNMCustomDraw->nmcd.uItemState & CDIS_HOT));
		int nItem = (int)lpNMCustomDraw->nmcd.dwItemSpec;

		TItem* pItem = this->GetItem(nItem);

    ::SelectObject(
      bufferedDC,
      ( bSelected )?
        lpNMCustomDraw->hFontSelected :
        lpNMCustomDraw->hFontInactive);

    COLORREF txtcolorref = 0;
    COLORREF tabcolorref = 0;
    BYTE byteAlpha = 0;
		if(bSelected)
		{
			txtcolorref = lpNMCustomDraw->clrTextSelected;
      tabcolorref = lpNMCustomDraw->clrSelectedTab;
      byteAlpha   = 255;
		}
		else if(bHighlighted)
		{
      txtcolorref = lpNMCustomDraw->clrHighlightText;
      if( bHot )
      {
        tabcolorref = lpNMCustomDraw->clrHighlightHotTrack;
        byteAlpha   = 96;
      }
      else
      {
        tabcolorref = lpNMCustomDraw->clrHighlight;
        byteAlpha   = 64;
      }
		}
		else if(bHot)
		{
      txtcolorref = lpNMCustomDraw->clrTextInactive;
      tabcolorref = lpNMCustomDraw->clrSelectedTab;
      byteAlpha   = 96;
		}
		else
		{
			txtcolorref = lpNMCustomDraw->clrTextInactive;
      tabcolorref = lpNMCustomDraw->clrSelectedTab;
      byteAlpha   = 64;
		}
    Gdiplus::Color tabcolor;
    tabcolor.SetFromCOLORREF(tabcolorref);
    tabcolor.SetValue(Gdiplus::Color::MakeARGB(byteAlpha, tabcolor.GetR(), tabcolor.GetG(), tabcolor.GetB()));

    this->DrawTab(rcTab, g, tabcolor);

		//--------------------------------------------
		// This is how CDotNetTabCtrlImpl interprets padding, margin, etc.:
		//
		//  M - Margin
		//  P - Padding
		//  I - Image
		//  Text - Tab Text
		//
		// With image:
		//     __________________________
		//
		//    | M | I | P | Text | P | M |
		//     --------------------------
		//
		// Without image:
		//     ______________________
		//
		//    | M | P | Text | P | M |
		//     ----------------------

		if (pItem->UsingImage() && !m_imageList.IsNull())
		{
			// Draw the image.
			IMAGEINFO ii = {0};
			int nImageIndex = pItem->GetImageIndex();
			m_imageList.GetImageInfo(nImageIndex, &ii);

			if((ii.rcImage.right - ii.rcImage.left) < (rcTab.right - rcTab.left))
			{
				int nImageHalfHeight = (ii.rcImage.bottom - ii.rcImage.top) / 2;
        CIcon icon;
        icon.Attach(m_imageList.ExtractIcon(nImageIndex));
        Gdiplus::Bitmap bitmap(icon.m_hIcon);

        g.DrawImage(
          &bitmap,
          rcText.left,
          nIconVerticalCenter - nImageHalfHeight + m_nFontSizeTextTopOffset);
			}

			// Offset on the right of the image.
			rcText.left += (ii.rcImage.right - ii.rcImage.left);
		}

		if (rcText.left + m_nMinWidthToDisplayText < rcText.right)
		{
			::InflateRect(&rcText, -m_settings.iPadding, 0);

      TCHAR szTitle[256];
      int szTitleLen = _sntprintf_s(szTitle, 256, _TRUNCATE, _T("%d. %s"), nItem + 1, pItem->GetText());

      bool bGlow = true;

      DTTOPTS dtto = { 0 };
        dtto.dwSize = sizeof(DTTOPTS);
        dtto.iGlowSize = 8;
        dtto.crText = txtcolorref;
        dtto.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;

      HTHEME hTheme = OpenThemeData(m_hWnd, VSCLASS_WINDOW);

      DrawThemeTextEx(
        hTheme,
        bufferedDC,
        WP_CAPTION, CS_ACTIVE,
        szTitle,
        szTitleLen,
        DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX,
        &rcText,
        &dtto);

      CloseThemeData(hTheme);
		}

    EndBufferedPaint(pb, TRUE);
	}

	void DrawCloseButton(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
	{
    RECT zone;
    zone.top    = min(m_rcScrollLeft.top, m_rcCloseButton.top);
    zone.left   = m_rcScrollLeft.left;
    zone.bottom = max(m_rcScrollLeft.bottom, m_rcCloseButton.bottom);
    zone.right  = m_rcCloseButton.right;

    HDC targetDC = lpNMCustomDraw->nmcd.hdc;
    HDC bufferedDC = NULL;
    BP_PAINTPARAMS m_PaintParams = { sizeof(BP_PAINTPARAMS) };
    HPAINTBUFFER pb = BeginBufferedPaint(targetDC, &zone, BPBF_TOPDOWNDIB, &m_PaintParams, &bufferedDC);
    BufferedPaintClear(pb, &zone);

    int iStateCloseButton = CBS_NORMAL;
    if( ectcMouseDownL_CloseButton == (m_dwState & ectcMouseDown) )
      iStateCloseButton = CBS_PUSHED;
    else if( ectcMouseOver_CloseButton == (m_dwState & ectcMouseOver) )
      iStateCloseButton = CBS_HOT;

    HTHEME hTheme = OpenThemeData(m_hWnd, VSCLASS_WINDOW);

    DrawThemeBackgroundEx(
      hTheme,
      bufferedDC,
      WP_SMALLCLOSEBUTTON,
      iStateCloseButton,
      &m_rcCloseButton,
      NULL);

    CloseThemeData(hTheme);

    int iStateScrollLeft = NAV_BB_DISABLED;
    if( m_dwState & ectcOverflowLeft )
    {
      iStateScrollLeft = NAV_BB_NORMAL;
      if( ectcMouseDownL_ScrollLeft == (m_dwState & ectcMouseDown) )
        iStateScrollLeft = NAV_BB_PRESSED;
      else if( ectcMouseOver_ScrollLeft == (m_dwState & ectcMouseOver) )
        iStateScrollLeft = NAV_BB_HOT;
    }

    int iStateScrollRight = NAV_BB_DISABLED;
    if( m_dwState & ectcOverflowRight )
    {
      iStateScrollRight = NAV_FB_NORMAL;
      if( ectcMouseDownL_ScrollRight == (m_dwState & ectcMouseDown) )
        iStateScrollRight = NAV_FB_PRESSED;
      else if( ectcMouseOver_ScrollRight == (m_dwState & ectcMouseOver) )
        iStateScrollRight = NAV_FB_HOT;
    }

    hTheme = OpenThemeData(m_hWnd, VSCLASS_NAVIGATION);

    DrawThemeBackgroundEx(
      hTheme,
      bufferedDC,
      NAV_BACKBUTTON,
      iStateScrollLeft,
      &m_rcScrollLeft,
      NULL);

    DrawThemeBackgroundEx(
      hTheme,
      bufferedDC,
      NAV_FORWARDBUTTON,
      iStateScrollRight,
      &m_rcScrollRight,
      NULL);

    CloseThemeData(hTheme);

    EndBufferedPaint(pb, TRUE);
	}

	void DrawScrollButtons(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
	{
	}

  void CalcSize_CloseButton(LPRECT prcTabItemArea)
	{
    SIZE size;

    HTHEME hTheme = OpenThemeData(m_hWnd, VSCLASS_WINDOW);

    GetThemePartSize(
      hTheme,
      NULL,
      WP_SMALLCLOSEBUTTON,
      0,
      NULL,
      TS_TRUE,
      &size
      );

    CloseThemeData(hTheme);

		if( (prcTabItemArea->right - prcTabItemArea->left) < size.cx )
		{
			::SetRectEmpty(&m_rcCloseButton);
			return;
		}

		m_rcCloseButton = *prcTabItemArea;

		DWORD dwStyle = this->GetStyle();

		if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
		{
			m_rcCloseButton.top += 3;
		}
		else
		{
			m_rcCloseButton.top += 1;
			m_rcCloseButton.bottom -= 2;
		}
		m_rcCloseButton.top = (m_rcCloseButton.bottom + m_rcCloseButton.top - size.cy) / 2;
		m_rcCloseButton.bottom = m_rcCloseButton.top + size.cy;

		m_rcCloseButton.left = m_rcCloseButton.right - (size.cx);

		if(m_tooltip.IsWindow())
		{
			m_tooltip.SetToolRect(m_hWnd, (UINT)ectcToolTip_Close, &m_rcCloseButton);
		}

		// Adjust the tab area
		prcTabItemArea->right = m_rcCloseButton.left - 3;
	}

	void CalcSize_ScrollButtons(LPRECT prcTabItemArea)
	{
    SIZE size;

    HTHEME hTheme = OpenThemeData(m_hWnd, VSCLASS_NAVIGATION);

    GetThemePartSize(
      hTheme,
      NULL,
      NAV_BACKBUTTON,
      0,
      NULL,
      TS_TRUE,
      &size
      );

    CloseThemeData(hTheme);

		if((prcTabItemArea->right - prcTabItemArea->left) < size.cx)
		{
			::SetRectEmpty(&m_rcScrollRight);
			::SetRectEmpty(&m_rcScrollLeft);
			return;
		}

		RECT rcScroll = *prcTabItemArea;

		DWORD dwStyle = this->GetStyle();

		if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
		{
			rcScroll.top += 3;
		}
		else
		{
			rcScroll.top += 1;
			rcScroll.bottom -= 2;
		}
		rcScroll.top = (rcScroll.bottom + rcScroll.top - size.cy) / 2;
		rcScroll.bottom = rcScroll.top + size.cy;

		m_rcScrollRight = rcScroll;
		m_rcScrollLeft = rcScroll;

		m_rcScrollRight.left = m_rcScrollRight.right - size.cx;

		m_rcScrollLeft.right = m_rcScrollRight.left - 3;
		m_rcScrollLeft.left = m_rcScrollLeft.right - size.cx;

		if(m_tooltip.IsWindow())
		{
			m_tooltip.SetToolRect(m_hWnd, (UINT)ectcToolTip_ScrollRight, &m_rcScrollRight);
			m_tooltip.SetToolRect(m_hWnd, (UINT)ectcToolTip_ScrollLeft, &m_rcScrollLeft);
		}

		// Adjust the tab area
		prcTabItemArea->right = m_rcScrollLeft.left;
	}

};

template <class TItem = CCustomTabItem>
class CAeroTabCtrl :
	public CAeroTabCtrlImpl<CAeroTabCtrl<TItem>, TItem>
{
protected:
	typedef CAeroTabCtrl<TItem> thisClass;
	typedef CAeroTabCtrlImpl<CAeroTabCtrl<TItem>, TItem> baseClass;

// Constructors:
public:
	CAeroTabCtrl()
	{
	}

public:

	DECLARE_WND_CLASS_EX(_T("WTL_CAeroTabCtrl"), CS_DBLCLKS, COLOR_WINDOW)

	//We have nothing special to add.
	//BEGIN_MSG_MAP(thisClass)
	//	CHAIN_MSG_MAP(baseClass)
	//END_MSG_MAP()
};
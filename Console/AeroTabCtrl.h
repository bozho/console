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
  signed char m_iCloseButtonWidth;
  signed char m_iCloseButtonHeight;

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
		DWORD dwStyle = this->GetStyle();

		if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
      this->DrawTabBottom(rcTab, g, color);
    else
      this->DrawTabTop(rcTab, g, color);
  }

  void DrawTabTop(RECT& rcTab, Gdiplus::Graphics& g, Gdiplus::Color& color)
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

  void DrawTabBottom(RECT& rcTab, Gdiplus::Graphics& g, Gdiplus::Color& color)
  {
    Gdiplus::SolidBrush brush(color);

    Gdiplus::GraphicsPath gp;

    INT radius = m_iRadius;
    INT X      = rcTab.left + m_iLeftSpacing;
    INT Y      = rcTab.bottom - m_iTopMargin;
    INT width  = rcTab.right  - rcTab.left - m_iLeftSpacing - 2;
    INT height = rcTab.bottom - rcTab.top  - m_iTopMargin;

    gp.AddLine(X + radius, Y, X + width - radius, Y);
    gp.AddLine(X + width - radius, Y, X + width, Y - radius);
    gp.AddLine(X + width, Y - radius, X + width, Y - height);
    gp.AddLine(X + width, Y - height, X, Y - height);
    gp.AddLine(X, Y - height, X, Y - radius);
    gp.AddLine(X, Y - radius, X + radius, Y);

    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    Gdiplus::Pen pen(color,1.0);
    g.DrawPath(&pen, &gp);

    g.SetSmoothingMode(Gdiplus::SmoothingModeNone);
    g.FillPath(&brush, &gp);

    radius += 2;
    X      -= 1;
    Y      += 1;
    width  += 2;
    height += 2;

    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    Gdiplus::Pen pen2(Gdiplus::Color(64, 0, 0,0),1.0);

    g.DrawLine(&pen2, X, Y - radius, X + radius, Y);
    g.DrawLine(&pen2, X, Y - height, X, Y - radius);
    g.DrawLine(&pen2, X + radius, Y, X + width - radius, Y);
    g.DrawLine(&pen2, X + width - radius, Y, X + width, Y - radius);
    g.DrawLine(&pen2, X + width, Y - radius, X + width, Y - height);
  }

	void DrawItem_InitBounds(DWORD /*dwStyle*/, RECT /*rcItem*/, RECT& rcTab, RECT& rcText, int& nIconVerticalCenter)
	{
    DWORD dwStyle = this->GetStyle();
    if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
    {
      rcText.bottom -= m_iTopMargin;
      rcText.left += (m_iLeftSpacing + m_iMargin);
      rcText.right -= m_iMargin;
      nIconVerticalCenter = (rcTab.bottom + rcTab.top - m_iTopMargin) / 2;
    }
    else
    {
      rcText.top += m_iTopMargin;
      rcText.left += (m_iLeftSpacing + m_iMargin);
      rcText.right -= m_iMargin;
      nIconVerticalCenter = (rcTab.bottom + rcTab.top - m_iTopMargin) / 2 + m_iTopMargin;
    }

    if (CTCS_CLOSEBUTTON == (dwStyle & CTCS_CLOSEBUTTON))
    {
      rcText.right -= m_iCloseButtonWidth;
    }
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
    // This is how CAeroTabCtrlImpl interprets padding, margin, etc.:
		//
		//  M - Margin
		//  P - Padding
		//  I - Image
    //  C - Close button
		//  Text - Tab Text
		//
    // With image & With close button:
    //    | M | I | P | Text | P | C | M |
		//
    // Without image & With close Button :
    //    | M | P | Text | P | C | M |
		//
    // With image & Without close button:
    //    | M | I | P | Text | P | M |
		//
    // Without image & Without close Button :
		//    | M | P | Text | P | M |

		if (pItem->UsingImage() && !m_imageList.IsNull())
		{
			// Draw the image.
			IMAGEINFO ii = {0};
			int nImageIndex = pItem->GetImageIndex();
			m_imageList.GetImageInfo(nImageIndex, &ii);

			if((ii.rcImage.right - ii.rcImage.left) < (rcTab.right - rcTab.left))
			{
				int nImageHalfHeight = (ii.rcImage.bottom - ii.rcImage.top) / 2;

        CIcon tabSmallIcon(m_imageList.ExtractIcon(nImageIndex));
        if( !tabSmallIcon.IsNull() )
        {
          WTL::CDCHandle dc(bufferedDC);
          dc.DrawIconEx(
            rcText.left, nIconVerticalCenter - nImageHalfHeight + m_nFontSizeTextTopOffset,
            tabSmallIcon.m_hIcon,
            16, 16);
        }
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
    // drawed in the current tab

    HDC targetDC = lpNMCustomDraw->nmcd.hdc;
    HDC bufferedDC = NULL;
    BP_PAINTPARAMS m_PaintParams = { sizeof(BP_PAINTPARAMS) };
    HPAINTBUFFER pb = BeginBufferedPaint(targetDC, &m_rcCloseButton, BPBF_TOPDOWNDIB, &m_PaintParams, &bufferedDC);

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

    EndBufferedPaint(pb, TRUE);
  }

  void DrawScrollButtons(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
  {
    RECT zone;
    zone.top    = m_rcScrollLeft.top;
    zone.left   = m_rcScrollLeft.left;
    zone.bottom = m_rcScrollLeft.bottom;
    zone.right  = m_rcScrollRight.right;

    HDC targetDC = lpNMCustomDraw->nmcd.hdc;
    HDC bufferedDC = NULL;
    BP_PAINTPARAMS m_PaintParams = { sizeof(BP_PAINTPARAMS) };
    HPAINTBUFFER pb = BeginBufferedPaint(targetDC, &zone, BPBF_TOPDOWNDIB, &m_PaintParams, &bufferedDC);
    BufferedPaintClear(pb, &zone);

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

    HTHEME hTheme = OpenThemeData(m_hWnd, VSCLASS_NAVIGATION);

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

  void CalcSize_CloseButton(LPRECT prcTabItemArea)
	{
    DWORD dwStyle = this->GetStyle();

    if (CTCS_CLOSEBUTTON == (dwStyle & CTCS_CLOSEBUTTON) && m_Items.GetCount() > 1)
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

      m_iCloseButtonWidth  = size.cx;
      m_iCloseButtonHeight = size.cy;
		}
		else
		{
      m_iCloseButtonWidth  = 0;
      m_iCloseButtonHeight = 0;
		}
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

  void UpdateLayout_CloseButton(RECT rcItem)
  {
    if( m_Items.GetCount() < 1 )
    {
      ::SetRectEmpty(&m_rcCloseButton);
      return;
    }

    RECT rcItemDP = {0};
    ::CopyRect(&rcItemDP, &rcItem);
    //::OffsetRect(&rcItemDP, m_iScrollOffset, 0);

    int nIconVerticalCenter;
    if (CTCS_BOTTOM == (this->GetStyle() & CTCS_BOTTOM))
    {
      nIconVerticalCenter = (rcItemDP.bottom + rcItemDP.top - m_iTopMargin) / 2;
    }
    else
    {
      nIconVerticalCenter = (rcItemDP.bottom + rcItemDP.top - m_iTopMargin) / 2 + m_iTopMargin;
    }

    // calculate the position of the close button
    m_rcCloseButton.left   = rcItemDP.right;
    m_rcCloseButton.right  = m_rcCloseButton.left + m_iCloseButtonWidth;
    m_rcCloseButton.top    = rcItemDP.top + nIconVerticalCenter - m_iCloseButtonHeight / 2;
    m_rcCloseButton.bottom = m_rcCloseButton.top + m_iCloseButtonHeight;
  }

  void UpdateLayout_Default(RECT rcTabItemArea)
  {
    long nMinInactiveWidth = 0x7FFFFFFF;
    long nMaxInactiveWidth = 0;

    WTL::CClientDC dc(m_hWnd);
    HFONT hOldFont = dc.SelectFont(m_font);

    LONG nTabAreaWidth = (rcTabItemArea.right - rcTabItemArea.left);

    RECT rcItem = rcTabItemArea;
    // rcItem.top and rcItem.bottom aren't really going to change

    // Recalculate tab positions and widths
    // See DrawItem_ImageAndText for a discussion of how CDotNetTabCtrlImpl
    //  interprets margin, padding, etc.
    size_t nCount = m_Items.GetCount();
    int xpos = m_settings.iIndent;
    HFONT hRestoreNormalFont = NULL;
    for( size_t i=0; i<nCount; ++i )
    {
      bool bSelected = ((int)i == m_iCurSel);
      if(bSelected)
      {
        hRestoreNormalFont = dc.SelectFont(m_fontSel);
      }

      TItem* pItem = m_Items[i];
      ATLASSERT(pItem != NULL);
      rcItem.left = rcItem.right = xpos;
      rcItem.right += m_settings.iMargin;
      if(pItem->UsingImage() && !m_imageList.IsNull())
      {
        IMAGEINFO ii = {0};
        int nImageIndex = pItem->GetImageIndex();
        m_imageList.GetImageInfo(nImageIndex, &ii);
        rcItem.right += (ii.rcImage.right - ii.rcImage.left);
      }
      if(pItem->UsingText())
      {
        rcItem.right += pItem->GetTextSize(dc) + (m_settings.iPadding * 2);
      }

      // close button
      if( bSelected )
      {
        UpdateLayout_CloseButton(rcItem);
        ::OffsetRect(&m_rcCloseButton, m_iScrollOffset, 0);
      }
      rcItem.right += m_iCloseButtonWidth;

      rcItem.right += m_settings.iMargin;
      pItem->SetRect(rcItem);
      xpos += (rcItem.right - rcItem.left);

      if(hRestoreNormalFont != NULL)
      {
        dc.SelectFont(hRestoreNormalFont);
        hRestoreNormalFont = NULL;
      }

      if(!bSelected)
      {
        if((rcItem.right - rcItem.left) < nMinInactiveWidth)
        {
          nMinInactiveWidth = (rcItem.right - rcItem.left);
        }
        if((rcItem.right - rcItem.left) > nMaxInactiveWidth)
        {
          nMaxInactiveWidth = (rcItem.right - rcItem.left);
        }
      }
    }
    xpos += m_settings.iIndent;

    if(xpos > nTabAreaWidth && nCount > 0 && m_iCurSel >= 0)
    {
      // Our desired widths are more than the width of the client area.
      // We need to have some or all of the tabs give up some real estate

      // We'll try to let the selected tab have its fully desired width.
      // If it can't, we'll make all the tabs the same width.

      RECT rcSelected = m_Items[m_iCurSel]->GetRect();
      LONG nSelectedWidth = (rcSelected.right - rcSelected.left);

      long cxClientInactiveTabs = nTabAreaWidth - (m_settings.iIndent * 2) - nSelectedWidth;
      long cxDesiredInactiveTabs = xpos - (m_settings.iIndent * 2) - nSelectedWidth;

      double nRatioWithSelectionFullSize = 0.0;
      if(cxDesiredInactiveTabs != 0)
      {
        nRatioWithSelectionFullSize = (double) (cxClientInactiveTabs) / (double)(cxDesiredInactiveTabs);
      }

      long nInactiveSameSizeWidth = (m_nMinWidthToDisplayText + (m_settings.iMargin*2) + (m_settings.iPadding));

      if(cxClientInactiveTabs > (nInactiveSameSizeWidth * (long)(nCount-1)))
      {
        //  There should be enough room to display the entire contents of
        //  the selected tab plus something for the inactive tabs

        bool bMakeInactiveSameSize = ((nMinInactiveWidth * nRatioWithSelectionFullSize) < nInactiveSameSizeWidth);

        xpos = m_settings.iIndent;
        for(size_t i=0; i<nCount; ++i )
        {
          TItem* pItem = m_Items[i];
          ATLASSERT(pItem != NULL);
          RECT rcItemDesired = pItem->GetRect();
          rcItem.left = rcItem.right = xpos;
          if((int)i == m_iCurSel)
          {
            rcItem.right += (rcItemDesired.right - rcItemDesired.left);
          }
          else
          {
            if(bMakeInactiveSameSize && (nCount != 1))
            {
              rcItem.right += (long)((cxClientInactiveTabs / (nCount-1)) + 0.5);
            }
            else
            {
              rcItem.right += (long)(((rcItemDesired.right - rcItemDesired.left) * nRatioWithSelectionFullSize) + 0.5);
            }
          }
          pItem->SetRect(rcItem);
          xpos += (rcItem.right-rcItem.left);
        }
      }
      else
      {
        // We're down pretty small, so just make all the tabs the same width
        int cxItem = (nTabAreaWidth - (m_settings.iIndent*2)) / (int)nCount;

        xpos = m_settings.iIndent;

        for(size_t i=0; i<nCount; ++i)
        {
          rcItem.left = rcItem.right = xpos;
          rcItem.right += cxItem;
          m_Items[i]->SetRect(rcItem);
          xpos += (rcItem.right-rcItem.left);
        }
      }
    }

    dc.SelectFont(hOldFont);
  }

  void UpdateLayout_ScrollToFit(RECT rcTabItemArea)
  {
    // When we scroll to fit, we ignore what's passed in for the
    // tab item area rect, and use the client rect instead
    RECT rcClient;
    this->GetClientRect(&rcClient);

    WTL::CClientDC dc(m_hWnd);
    HFONT hOldFont = dc.SelectFont(m_font);

    RECT rcItem = rcClient;

    // Recalculate tab positions and widths
    // See DrawItem_ImageAndText for a discussion of how CDotNetTabCtrlImpl
    //  interprets margin, padding, etc.
    size_t nCount = m_Items.GetCount();
    int xpos = m_settings.iIndent;
    HFONT hRestoreNormalFont = NULL;
    for( size_t i=0; i<nCount; ++i )
    {
      bool bSelected = ((int)i == m_iCurSel);
      if(bSelected)
      {
        hRestoreNormalFont = dc.SelectFont(m_fontSel);
      }

      TItem* pItem = m_Items[i];
      ATLASSERT(pItem != NULL);
      rcItem.left = rcItem.right = xpos;
      rcItem.right += m_settings.iMargin;
      if(pItem->UsingImage() && !m_imageList.IsNull())
      {
        IMAGEINFO ii = {0};
        int nImageIndex = pItem->GetImageIndex();
        m_imageList.GetImageInfo(nImageIndex, &ii);
        rcItem.right += (ii.rcImage.right - ii.rcImage.left);
      }
      if(pItem->UsingText())
      {
        rcItem.right += pItem->GetTextSize(dc) + (m_settings.iPadding * 2);
      }

      // close button
      if( bSelected )
      {
        UpdateLayout_CloseButton(rcItem);
      }
      rcItem.right += m_iCloseButtonWidth;

      rcItem.right += m_settings.iMargin;
      pItem->SetRect(rcItem);
      xpos += (rcItem.right - rcItem.left);

      if(hRestoreNormalFont != NULL)
      {
        dc.SelectFont(hRestoreNormalFont);
        hRestoreNormalFont = NULL;
      }
    }
    xpos += m_settings.iIndent;

    // If we've been scrolled to the left, and resize so
    // there's more client area to the right, adjust the
    // scroll offset accordingly.
    if((xpos + m_iScrollOffset) < rcTabItemArea.right)
    {
      m_iScrollOffset = (rcTabItemArea.right - xpos);
    }
    this->UpdateScrollOverflowStatus();

    dc.SelectFont(hOldFont);

    ::OffsetRect(&m_rcCloseButton, m_iScrollOffset, 0);
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
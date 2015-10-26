#pragma once

/* Aero default theme drawn an internal border (2 pixels) */
#define AERO_FRAME_BORDER_SIZE 2;

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
  signed long m_iCloseButtonWidth;
  signed long m_iCloseButtonHeight;

  bool        m_bAppActive;

  // Constructor
public:

  CAeroTabCtrlImpl()
    :m_iTopMargin(0)
    ,m_bAppActive(true)
    ,m_iCloseButtonWidth(0)
    ,m_iCloseButtonHeight(0)
    ,m_iMargin(2)
    ,m_iLeftSpacing(0)
  {
    // We can't use a member initialization list to initialize
    // members of our base class, so do it explictly by assignment here.
    m_clrTextInactiveTab = ::GetSysColor(COLOR_BTNTEXT);
    m_clrSelectedTab     = ::GetSysColor(COLOR_WINDOW);
  }

  void SetTopMargin(int nTopMargin)
  {
    m_iTopMargin = static_cast<signed char>(nTopMargin);
  }

  void SetAppActiveStatus(bool bAppActive)
  {
    m_bAppActive = bAppActive;
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

    m_settings.iIndent = 5;
    m_settings.iPadding = 2;
    m_settings.iMargin = m_iMargin + m_iLeftSpacing / 2;
    m_settings.iSelMargin = 0;

    T* pT = static_cast<T*>(this);
    pT->UpdateLayout();
    pT->Invalidate();
    return 0;
  }

  // Overrides for painting from CDotNetTabCtrlImpl
public:

  void DrawBackground(RECT rcClient, LPNMCTCCUSTOMDRAW lpNMCustomDraw)
  {
    Gdiplus::Graphics g(lpNMCustomDraw->nmcd.hdc);

    if( !aero::IsComposing() )
    {
      Gdiplus::Color clr;
      clr.SetFromCOLORREF(lpNMCustomDraw->clrBtnFace);

      g.Clear(clr);
    }

    Gdiplus::Color tabcolor;
    tabcolor.SetFromCOLORREF(lpNMCustomDraw->clrSelectedTab);
    Gdiplus::SolidBrush brush(tabcolor);
    Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(0x72000000)), 1.0);

    DWORD dwStyle = this->GetStyle();

    if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
    {
      //g.FillRectangle(&brush, rcClient.left, rcClient.top + 1, rcClient.right, rcClient.top + 2);
      g.DrawLine(&pen, rcClient.left, rcClient.top + 2, rcClient.right, rcClient.top + 2);
    }
    else
    {
      //g.FillRectangle(&brush, rcClient.left, rcClient.bottom - 2, rcClient.right, rcClient.bottom - 2);
      g.DrawLine(&pen, rcClient.left, rcClient.bottom - 3, rcClient.right, rcClient.bottom - 3);
    }
  }

  void DrawTab(RECT& rcTab, Gdiplus::Graphics& g, Gdiplus::Color& colorTab, Gdiplus::Color& colorProgressBar, bool bSelected, TItem* pItem)
  {
    DWORD dwStyle = this->GetStyle();

    if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
      this->DrawTabBottom(rcTab, g, colorTab, colorProgressBar, bSelected, pItem);
    else
      this->DrawTabTop(rcTab, g, colorTab, colorProgressBar, bSelected, pItem);
  }

  void DrawTabTop(RECT& rcTab, Gdiplus::Graphics& g, Gdiplus::Color& colorTab, Gdiplus::Color& colorProgressBar, bool bSelected, TItem* pItem)
  {
    INT X       = rcTab.left + m_iLeftSpacing;
    INT Y       = rcTab.top + m_iTopMargin;
    INT width   = rcTab.right - rcTab.left - m_iLeftSpacing;
    INT height  = rcTab.bottom - rcTab.top - m_iTopMargin - 1;
    INT height2 = bSelected ? height : (height - 1);

    Gdiplus::SolidBrush brushTab(colorTab);
    Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(0x72000000)), 1.0);

    g.DrawLine(&pen, rcTab.left, rcTab.bottom - 1, rcTab.right, rcTab.bottom - 1);

    if( pItem->GetProgressTotal() )
    {
      Gdiplus::SolidBrush brushProgressBar(colorProgressBar);
      INT widthProgressBar = static_cast<INT>(pItem->GetProgressCompleted() * (width - 2) / pItem->GetProgressTotal());
      INT widthRemaining = width - 2 - widthProgressBar;

      if( widthProgressBar )
        g.FillRectangle(&brushProgressBar, X + 1, Y + 1, widthProgressBar, height2);

      if( widthRemaining )
        g.FillRectangle(&brushTab, X + 1 + widthProgressBar, Y + 1, widthRemaining, height2);
    }
    else
    {
      g.FillRectangle(&brushTab, X + 1, Y + 1, width - 2, height2);
    }

    if( bSelected )
    {
      g.DrawLine(&pen, X, Y, X, rcTab.bottom - 2);
      g.DrawLine(&pen, X + width - 1, Y, X + width - 1, rcTab.bottom - 2);
      g.DrawLine(&pen, X + 1, Y, X + width - 2, Y);
    }

#ifdef _DRAW_TAB_RECT
    {
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Green)));
      g.DrawRectangle(
        &pen,
        X, Y,
        width, height);
    }
#endif //_DRAW_TAB_RECT
  }

  void DrawTabBottom(RECT& rcTab, Gdiplus::Graphics& g, Gdiplus::Color& colorTab, Gdiplus::Color& colorProgressBar, bool bSelected, TItem* pItem)
  {
    INT X       = rcTab.left + m_iLeftSpacing;
    INT Y       = rcTab.top;
    INT Y2      = bSelected ? Y : (Y + 1);
    INT width   = rcTab.right - rcTab.left - m_iLeftSpacing;
    INT height  = rcTab.bottom - rcTab.top - m_iTopMargin - 1;
    INT height2 = bSelected ? height : (height - 1);

    Gdiplus::SolidBrush brushTab(colorTab);
    Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(0x72000000)), 1.0);

    g.DrawLine(&pen, rcTab.left, rcTab.top, rcTab.right, rcTab.top);

    if( pItem->GetProgressTotal() )
    {
      Gdiplus::SolidBrush brushProgressBar(colorProgressBar);
      INT widthProgressBar = static_cast<INT>(pItem->GetProgressCompleted() * (width - 2) / pItem->GetProgressTotal());
      INT widthRemaining = width - 2 - widthProgressBar;

      if( widthProgressBar )
        g.FillRectangle(&brushProgressBar, X + 1, Y2, widthProgressBar, height2);

      if( widthRemaining )
        g.FillRectangle(&brushTab, X + 1 + widthProgressBar, Y2, widthRemaining, height2);
    }
    else
    {
      g.FillRectangle(&brushTab, X + 1, Y2, width - 2, height2);
    }

    if( bSelected )
    {
      g.DrawLine(&pen, X, rcTab.top + 1, X, Y + height);
      g.DrawLine(&pen, X + width - 1, rcTab.top + 1, X + width - 1, Y + height);
      g.DrawLine(&pen, X + 1, Y + height, X + width - 2, Y + height);
    }

#ifdef _DRAW_TAB_RECT
    {
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Green)));
      g.DrawRectangle(
        &pen,
        X, Y - height,
        width, height);
    }
#endif //_DRAW_TAB_RECT
  }

  void DrawItem_InitBounds(DWORD /*dwStyle*/, RECT /*rcItem*/, RECT& rcTab, RECT& rcText, int& nIconVerticalCenter)
  {
    DWORD dwStyle = this->GetStyle();
    if (CTCS_BOTTOM == (dwStyle & CTCS_BOTTOM))
    {
      rcText.top += AERO_FRAME_BORDER_SIZE;
      rcTab.top += AERO_FRAME_BORDER_SIZE;
      rcText.bottom -= m_iTopMargin;
      rcText.left += (m_iLeftSpacing + m_iMargin);
      rcText.right -= m_iMargin;
    }
    else
    {
      rcText.top += m_iTopMargin;
      rcText.bottom -= AERO_FRAME_BORDER_SIZE;
      rcTab.bottom -= AERO_FRAME_BORDER_SIZE;
      rcText.left += (m_iLeftSpacing + m_iMargin);
      rcText.right -= m_iMargin;
    }
    nIconVerticalCenter = (rcText.bottom + rcText.top) / 2;

    if (CTCS_CLOSEBUTTON == (dwStyle & CTCS_CLOSEBUTTON))
    {
      rcText.right -= m_iCloseButtonWidth;
    }
  }

  void DrawItem_TabInactive(DWORD /*dwStyle*/, LPNMCTCCUSTOMDRAW /*lpNMCustomDraw*/, RECT& /*rcTab*/)
  {
  }

  void DrawItem_TabSelected(DWORD /*dwStyle*/, LPNMCTCCUSTOMDRAW /*lpNMCustomDraw*/, RECT& /*rcTab*/)
  {
  }

  void DrawItem_ImageAndText(DWORD /*dwStyle*/, LPNMCTCCUSTOMDRAW lpNMCustomDraw, int nIconVerticalCenter, RECT& rcTab, RECT& rcText)
  {
    CBufferedPaint bufferedPaint;
    HDC hDCPaint = NULL;
    BP_PAINTPARAMS paintParams = { sizeof(BP_PAINTPARAMS), BPPF_ERASE, NULL, NULL };
    bufferedPaint.Begin(lpNMCustomDraw->nmcd.hdc, &rcTab, BPBF_TOPDOWNDIB, &paintParams, &hDCPaint);
    CDCHandle dcPaint(hDCPaint);

    Gdiplus::Graphics g(dcPaint);

    if( !aero::IsComposing() )
    {
      COLORREF clr = lpNMCustomDraw->clrBtnFace;

      g.Clear(
        Gdiplus::Color(
            255,
            GetRValue(clr),
            GetGValue(clr),
            GetBValue(clr)));
    }

    bool bHighlighted = (CDIS_MARKED == (lpNMCustomDraw->nmcd.uItemState & CDIS_MARKED));
    bool bSelected = (CDIS_SELECTED == (lpNMCustomDraw->nmcd.uItemState & CDIS_SELECTED));
    bool bHot = (CDIS_HOT == (lpNMCustomDraw->nmcd.uItemState & CDIS_HOT));
    int nItem = (int)lpNMCustomDraw->nmcd.dwItemSpec;

    TItem* pItem = this->GetItem(nItem);

    ::SelectObject(
      dcPaint,
      ( bSelected )?
      lpNMCustomDraw->hFontSelected :
    lpNMCustomDraw->hFontInactive);

    COLORREF colorrefText        = 0;
    COLORREF colorrefTab         = 0;
    COLORREF colorrefProgressBar = 0;
    BYTE     byteAlpha           = 0;

    if(bSelected)
    {
      colorrefText        = lpNMCustomDraw->clrTextSelected;
      colorrefTab         = lpNMCustomDraw->clrSelectedTab;
      colorrefProgressBar = RGB(0x7f, 0xff, 0x00);
      byteAlpha           = 255;
    }
    else if(bHighlighted)
    {
      colorrefText        = lpNMCustomDraw->clrHighlightText;
      colorrefProgressBar = RGB(0x90, 0xee, 0x90);
      if( bHot )
      {
        colorrefTab       = lpNMCustomDraw->clrHighlightHotTrack;
        byteAlpha         = 160;
      }
      else
      {
        colorrefTab       = lpNMCustomDraw->clrHighlight;
        byteAlpha         = 128;
      }
    }
    else
    {
      colorrefText        = lpNMCustomDraw->clrTextInactive;
      colorrefProgressBar = RGB(0x55, 0x6b, 0x2f);
      if(bHot)
      {
        colorrefTab       = lpNMCustomDraw->clrSelectedTab;
        byteAlpha         = 160;
      }
      else
      {
        colorrefTab       = lpNMCustomDraw->clrBtnShadow;
        byteAlpha         = 128;
      }
    }

    Gdiplus::Color colorTab        (Gdiplus::Color::MakeARGB(byteAlpha,
                                                             GetRValue(colorrefTab),
                                                             GetGValue(colorrefTab),
                                                             GetBValue(colorrefTab)));
    Gdiplus::Color colorProgressBar(Gdiplus::Color::MakeARGB(byteAlpha,
                                                             GetRValue(colorrefProgressBar),
                                                             GetGValue(colorrefProgressBar),
                                                             GetBValue(colorrefProgressBar)));

    this->DrawTab(rcTab, g, colorTab, colorProgressBar, bSelected, pItem);

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
			dcPaint.DrawIconEx(
				rcText.left, nIconVerticalCenter - nImageHalfHeight + m_nFontSizeTextTopOffset,
				tabSmallIcon.m_hIcon,
				ii.rcImage.right - ii.rcImage.left,
				ii.rcImage.right - ii.rcImage.left);
        }

#ifdef _DRAW_TAB_RECT
        {
          Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Cyan)));
          g.DrawRectangle(
            &pen,
            rcText.left, nIconVerticalCenter - nImageHalfHeight + m_nFontSizeTextTopOffset,
            15, 15);
        }
#endif //_DRAW_TAB_RECT
      }

      // Offset on the right of the image.
      rcText.left += (ii.rcImage.right - ii.rcImage.left);
    }

    if (rcText.left + m_nMinWidthToDisplayText < rcText.right)
    {
      ::InflateRect(&rcText, -m_settings.iPadding, 0);

      CString sText = pItem->GetText();

      bool bGlow = true;

      DTTOPTS dtto = { 0 };
      dtto.dwSize = sizeof(DTTOPTS);
      dtto.iGlowSize = 8;
      dtto.crText = colorrefText;
      dtto.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;

      HTHEME hTheme = ::OpenThemeData(m_hWnd, VSCLASS_WINDOW);
      if( hTheme )
      {
        ::DrawThemeTextEx(
          hTheme,
          dcPaint,
          WP_CAPTION, CS_ACTIVE,
          sText, sText.GetLength(),
          DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX,
          &rcText,
          &dtto);

        ::CloseThemeData(hTheme);
      }
      else
      {
        dcPaint.SetBkMode(TRANSPARENT);
        dcPaint.DrawText(
          sText, sText.GetLength(),
          &rcText,
          DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
      }
    }

#ifdef _DRAW_TAB_RECT
    {
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Yellow)));
      g.DrawRectangle(
        &pen,
        rcTab.left, rcTab.top,
        rcTab.right - rcTab.left - 1, rcTab.bottom - rcTab.top - 1);
    }

    {
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Red)));
      g.DrawRectangle(
        &pen,
        rcText.left, rcText.top,
        rcText.right - rcText.left - 1, rcText.bottom - rcText.top - 1);
    }
#endif //_DRAW_TAB_RECT

    bufferedPaint.End();
  }

  void DrawCloseButton(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
  {
    // drawed in the current tab
    if( m_iCloseButtonWidth == 0 )
      return;

    // we want to clip the close button without distorting/shrinking
    CBufferedPaint bufferedPaint;
    HDC hDCPaint = NULL;
    BP_PAINTPARAMS paintParams = { sizeof(BP_PAINTPARAMS), 0, NULL, NULL };
    bufferedPaint.Begin(lpNMCustomDraw->nmcd.hdc, &m_rcCloseButton, BPBF_TOPDOWNDIB, &paintParams, &hDCPaint);
    CDCHandle dcPaint(hDCPaint);

    Gdiplus::Graphics g(dcPaint);

    COLORREF clr = lpNMCustomDraw->clrSelectedTab;

    g.Clear(
      Gdiplus::Color(
          255,
          GetRValue(clr),
          GetGValue(clr),
          GetBValue(clr)));

    RECT rcCloseButton   = m_rcCloseButton;
    rcCloseButton.right  = rcCloseButton.left + m_iCloseButtonWidth;
    rcCloseButton.bottom = rcCloseButton.top  + m_iCloseButtonHeight;

    int iStateCloseButton = SCBS_NORMAL;
    if( ectcMouseDownL_CloseButton == (m_dwState & ectcMouseDown) )
      iStateCloseButton = SCBS_PUSHED;
    else if( ectcMouseOver_CloseButton == (m_dwState & ectcMouseOver) )
      iStateCloseButton = SCBS_HOT;

    HTHEME hTheme = ::OpenThemeData(m_hWnd, VSCLASS_WINDOW);
    if( hTheme )
    {
      ::DrawThemeBackgroundEx(
        hTheme,
        dcPaint,
        WP_SMALLCLOSEBUTTON,
        iStateCloseButton,
        &rcCloseButton,
        NULL);

      ::CloseThemeData(hTheme);
    }
    else
    {
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Red)));
      /*
      g.DrawRectangle(
        &pen,
        rcCloseButton.left, rcCloseButton.top,
        rcCloseButton.right - rcCloseButton.left, rcCloseButton.bottom - rcCloseButton.top);
        */
      g.DrawLine(
        &pen,
        rcCloseButton.left, rcCloseButton.top,
        rcCloseButton.right - 1, rcCloseButton.bottom - 1);
      g.DrawLine(
        &pen,
        rcCloseButton.right - 1, rcCloseButton.top,
        rcCloseButton.left, rcCloseButton.bottom - 1);
    }

#ifdef _DRAW_TAB_RECT
    {
      Gdiplus::Graphics g(dcPaint);
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Cyan)));
      g.DrawRectangle(
        &pen,
        rcCloseButton.left, rcCloseButton.top,
        rcCloseButton.right - rcCloseButton.left, rcCloseButton.bottom - rcCloseButton.top);
    }
#endif //_DRAW_TAB_RECT

    bufferedPaint.End();

  }

  void DrawScrollButtons(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
  {
    RECT zone;
    zone.top    = m_rcScrollLeft.top;
    zone.left   = m_rcScrollLeft.left;
    zone.bottom = m_rcScrollLeft.bottom;
    zone.right  = m_rcScrollRight.right;

    int iStateScrollLeft = ABS_LEFTDISABLED;
    if( m_dwState & ectcOverflowLeft )
    {
      iStateScrollLeft = ABS_LEFTNORMAL;
      if( ectcMouseDownL_ScrollLeft == (m_dwState & ectcMouseDown) )
        iStateScrollLeft = ABS_LEFTPRESSED;
      else if( ectcMouseOver_ScrollLeft == (m_dwState & ectcMouseOver) )
        iStateScrollLeft = ABS_LEFTHOT;
    }

    int iStateScrollRight = ABS_RIGHTDISABLED;
    if( m_dwState & ectcOverflowRight )
    {
      iStateScrollRight = ABS_RIGHTNORMAL;
      if( ectcMouseDownL_ScrollRight == (m_dwState & ectcMouseDown) )
        iStateScrollRight = ABS_RIGHTPRESSED;
      else if( ectcMouseOver_ScrollRight == (m_dwState & ectcMouseOver) )
        iStateScrollRight = ABS_RIGHTHOT;
    }

    HTHEME hTheme = ::OpenThemeData(m_hWnd, VSCLASS_SCROLLBAR /*VSCLASS_TRACKBAR*/);
    if( hTheme )
    {
      ::DrawThemeBackgroundEx(
        hTheme,
        lpNMCustomDraw->nmcd.hdc,
        SBP_ARROWBTN,
        iStateScrollLeft,
        /*TKP_THUMBLEFT,TUVLS_NORMAL,*/
        &m_rcScrollLeft,
        NULL);

      ::DrawThemeBackgroundEx(
        hTheme,
        lpNMCustomDraw->nmcd.hdc,
        SBP_ARROWBTN,
        iStateScrollRight,
        /*TKP_THUMBRIGHT,TUVRS_PRESSED,*/
        &m_rcScrollRight,
        NULL);

      ::CloseThemeData(hTheme);
    }
    else
    {
      Gdiplus::Graphics g(lpNMCustomDraw->nmcd.hdc);
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Navy)));
      int nHalfHeight = (m_rcScrollLeft.bottom - m_rcScrollLeft.top) >> 1;
      g.DrawLine(
        &pen,
        m_rcScrollLeft.right, m_rcScrollLeft.top,
        m_rcScrollLeft.right, m_rcScrollLeft.bottom);
      g.DrawLine(
        &pen,
        m_rcScrollLeft.right, m_rcScrollLeft.top,
        m_rcScrollLeft.left, m_rcScrollLeft.top + nHalfHeight);
      g.DrawLine(
        &pen,
        m_rcScrollLeft.right, m_rcScrollLeft.bottom,
        m_rcScrollLeft.left, m_rcScrollLeft.bottom - nHalfHeight);

      g.DrawLine(
        &pen,
        m_rcScrollRight.left, m_rcScrollRight.top,
        m_rcScrollRight.left, m_rcScrollRight.bottom);
      g.DrawLine(
        &pen,
        m_rcScrollRight.left, m_rcScrollRight.top,
        m_rcScrollRight.right, m_rcScrollRight.top + nHalfHeight);
      g.DrawLine(
        &pen,
        m_rcScrollRight.left, m_rcScrollRight.bottom,
        m_rcScrollRight.right, m_rcScrollRight.bottom - nHalfHeight);
    }

#ifdef _DRAW_TAB_RECT
    {
      Gdiplus::Graphics g(lpNMCustomDraw->nmcd.hdc);
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Lime)));
      g.DrawRectangle(
        &pen,
        m_rcScrollLeft.left, m_rcScrollLeft.top,
        m_rcScrollLeft.right - m_rcScrollLeft.left, m_rcScrollLeft.bottom - m_rcScrollLeft.top);
    }
    {
      Gdiplus::Graphics g(lpNMCustomDraw->nmcd.hdc);
      Gdiplus::Pen pen(Gdiplus::Color(static_cast<Gdiplus::ARGB>(Gdiplus::Color::Magenta)));
      g.DrawRectangle(
        &pen,
        m_rcScrollRight.left, m_rcScrollRight.top,
        m_rcScrollRight.right - m_rcScrollRight.left, m_rcScrollRight.bottom - m_rcScrollRight.top);
    }
#endif //_DRAW_TAB_RECT

  }

  void CalcSize_CloseButton(LPRECT /*prcTabItemArea*/)
  {
    DWORD dwStyle = this->GetStyle();

    if (CTCS_CLOSEBUTTON == (dwStyle & CTCS_CLOSEBUTTON) && ( m_Items.GetCount() > 1 || CTCS_CLOSELASTTAB == (dwStyle & CTCS_CLOSELASTTAB)))
    {
      HTHEME hTheme = ::OpenThemeData(m_hWnd, VSCLASS_WINDOW);
      if( hTheme )
      {
        SIZE size;

        ::GetThemePartSize(
          hTheme,
          NULL,
          WP_SMALLCLOSEBUTTON,
          0,
          NULL,
          TS_TRUE,
          &size
          );

        ::CloseThemeData(hTheme);

        // I cannot find a system metrics for the size of the close button.
        // So I manually resize.
        m_iCloseButtonWidth  = ::MulDiv(size.cx, ::GetSystemMetrics(SM_CXSMICON), 16);
        m_iCloseButtonHeight = ::MulDiv(size.cy, ::GetSystemMetrics(SM_CYSMICON), 16);
      }
      else
      {
        m_iCloseButtonWidth  = 9;
        m_iCloseButtonHeight = 9;
      }
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

    HTHEME hTheme = ::OpenThemeData(m_hWnd, VSCLASS_SCROLLBAR);
    if( hTheme )
    {
      /*
      ::GetThemePartSize(
        hTheme,
        NULL,
        SBP_ARROWBTN,
        ABS_LEFTNORMAL,
        NULL,
        TS_TRUE,
        &size
        );
        */

      // GetThemeSysSize returns size stored in the current visual style
      // (SysMetrics section of the visual style) scaled to the current screen dpi.
      size.cx = ::GetThemeSysSize(hTheme, SM_CXVSCROLL);
      size.cy = ::GetThemeSysSize(hTheme, SM_CYHSCROLL);

      ::CloseThemeData(hTheme);
    }
    else
    {
      size.cx = 16;
      size.cy = 16;
    }

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
      rcItemDP.top += AERO_FRAME_BORDER_SIZE;
      rcItemDP.bottom -= m_iTopMargin;
    }
    else
    {
      rcItemDP.top += m_iTopMargin;
      rcItemDP.bottom -= AERO_FRAME_BORDER_SIZE;
    }
    nIconVerticalCenter = (rcItemDP.bottom + rcItemDP.top) / 2;

    // calculate the position of the close button
    if( m_iCloseButtonWidth && m_iCloseButtonHeight )
    {
      m_rcCloseButton.left   = rcItemDP.right;
      m_rcCloseButton.right  = m_rcCloseButton.left + m_iCloseButtonWidth;
      m_rcCloseButton.top    = nIconVerticalCenter - m_iCloseButtonHeight / 2;
      m_rcCloseButton.bottom = m_rcCloseButton.top + m_iCloseButtonHeight;
    }
    else
    {
      ::SetRectEmpty(&m_rcCloseButton);
    }
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
        LONG lTextSize = pItem->GetTextSize(dc);
        rcItem.right += max(lTextSize, m_nMinWidthToDisplayText) + (m_settings.iPadding * 2);
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
        LONG lTextSize = pItem->GetTextSize(dc);
        rcItem.right += max(lTextSize, m_nMinWidthToDisplayText) + (m_settings.iPadding * 2);
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
    if( m_rcCloseButton.right > rcTabItemArea.right )
      m_rcCloseButton.right = rcTabItemArea.right;
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
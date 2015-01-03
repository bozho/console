// aboutdlg.h : interface of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CAboutDlg :
#ifdef _USE_AERO
  public aero::CDialogImpl<CAboutDlg>
#else
  public CDialogImpl<CAboutDlg>
#endif
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
#ifdef _USE_AERO
    CHAIN_MSG_MAP(aero::CDialogImpl<CAboutDlg>)
#endif
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
#ifdef _USE_AERO
  void Paint(CDCHandle dc, RECT& rClient, RECT& rView, RECT& rDest)
  {
    aero::CDialogImpl<CAboutDlg>::Paint(dc, rClient, rView, rDest);

    CPaintDC(*this);
    Gdiplus::Graphics gr(dc);

    CIcon icon (static_cast<HICON>(
      ::LoadImage(
        ::GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON,
        256,
        256,
        LR_DEFAULTCOLOR)));

    //get the icon info
    ICONINFO ii;
    GetIconInfo(icon, &ii);

    //create a bitmap from the ICONINFO so we can access the bitmapData
    Gdiplus::Bitmap bmpIcon(ii.hbmColor, NULL);
    Gdiplus::Rect rectBounds(0, 0, bmpIcon.GetWidth(), bmpIcon.GetHeight() );

    //get the BitmapData
    Gdiplus::BitmapData bmData;
    bmpIcon.LockBits(&rectBounds, Gdiplus::ImageLockModeRead,
      bmpIcon.GetPixelFormat(), &bmData);

    // create a new 32 bit bitmap using the bitmapData
    Gdiplus::Bitmap bmpAlpha(bmData.Width, bmData.Height, bmData.Stride,
      PixelFormat32bppARGB, (BYTE*)bmData.Scan0);
    bmpIcon.UnlockBits(&bmData);

    CWindow staticMessage(GetDlgItem(IDC_STATIC));
    CRect rectVersion;
    staticMessage.GetWindowRect(rectVersion);
    ScreenToClient(rectVersion);

    INT len = min(rectVersion.Width(), rectVersion.Height());
    Gdiplus::Rect rect2(
      rectVersion.left + (rectVersion.Width() - len) / 2,
      rectVersion.top  + (rectVersion.Height() - len),
      len, len);

    gr.DrawImage(
      &bmpAlpha,
      rect2,
      0, 0,
      256, 256,
      Gdiplus::Unit::UnitPixel);

	INT flatLen = rectVersion.Height() / 2;

    Gdiplus::SolidBrush brush(Gdiplus::Color(140,0,0,0));
	gr.FillRectangle(
		&brush,
		Gdiplus::Rect(
			rectVersion.left, rectVersion.top,
			rectVersion.Width(), flatLen));

	Gdiplus::LinearGradientBrush linGrBrush(
		Gdiplus::Point(0, rectVersion.top + flatLen - 1),
		Gdiplus::Point(0, rectVersion.bottom),
		Gdiplus::Color(140,0,0,0),
		Gdiplus::Color(64,0,0,0));
	gr.FillRectangle(
		&linGrBrush,
		Gdiplus::Rect(
			rectVersion.left, rectVersion.top + flatLen,
			rectVersion.Width(), rectVersion.Height() - flatLen));

    DTTOPTS dtto   = { 0 };
    dtto.dwSize    = sizeof(DTTOPTS);
    dtto.iGlowSize = 16;
    dtto.crText    = RGB(240,240,240);
    dtto.dwFlags   = DTT_COMPOSITED | DTT_TEXTCOLOR | DTT_GLOWSIZE;
   
    CString strMsgVersion;
    strMsgVersion.Format(
      L"\nConsoleZ %i.%i.%i.%i\n"
      L"Copyleft (ↄ)\n"
      L"2011-2015\n"
      L"Christophe Bucher\n"
      L"\n"
      L"a modified version of\n"
      L"Console 2 from\n"
      L"Marko Bozikovic"
      , VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_BUILD2);

    this->DrawTextW(dc, strMsgVersion, rectVersion, DT_CENTER, dtto);
  }
#endif
};

#pragma once

#include "ConsoleView.h"

typedef map<HWND, shared_ptr<ConsoleView> >	ConsoleViewMap;

class TabView
  : public CWindowImpl<TabView>
  , public CMultiSplitImpl<TabView>
{
public:
  DECLARE_WND_CLASS_EX(L"Console_2_TabView", CS_DBLCLKS, COLOR_WINDOW)

  TabView(MainFrame& mainFrame, shared_ptr<TabData> tabData);
  ~TabView();

  BOOL PreTranslateMessage(MSG* pMsg);

  typedef CMultiSplitImpl< TabView > multisplitClass;
  BEGIN_MSG_MAP(TabView)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    MESSAGE_HANDLER (WM_ERASEBKGND, OnEraseBackground)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    CHAIN_MSG_MAP (multisplitClass)
    FORWARD_NOTIFICATIONS ()
  END_MSG_MAP()

  //		Handler prototypes (uncomment arguments if needed):
  //		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  //		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  //		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  LRESULT OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
  LRESULT OnEraseBackground (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
  LRESULT OnSize (UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL & bHandled);

  virtual void OnPaneChanged(void)
  {
    SetAppActiveStatus(true);
  }

  virtual void OnSplitBarMove(HWND hwndPane0, HWND hwndPane1, bool /*boolEnd*/);

  shared_ptr<ConsoleView> GetActiveConsole(const TCHAR*);
  shared_ptr<TabData>     GetTabData() { return m_tabData; }

  void SetTitle(const CString& strTitle);
  const CString& GetTitle() const { return m_strTitle; }
  CIcon& GetIcon(bool bBigIcon = true) { return bBigIcon ? m_bigIcon : m_smallIcon; }
  void SetActive(bool bActive);
  void SetAppActiveStatus(bool bAppActive);
  void SetResizing(bool bResizing);
  void MainframeMoving();
  void Repaint(bool bFullRepaint);
  void InitializeScrollbars();
  void RecreateOffscreenBuffers();
  void AdjustRectAndResize(CRect& clientRect, DWORD dwResizeWindowEdge);
  void GetRect(CRect& clientRect);

  void SplitHorizontally();
  void SplitVertically();
  bool CloseView(HWND hwnd = 0);
  void NextView();
  void PrevView();

  void PostMessageToConsoles(UINT Msg, WPARAM wParam, LPARAM lParam);
  void PasteToConsoles();

  inline bool IsGrouped() const { return m_boolIsGrouped; }
  void Group(bool b);

private:
  HWND CreateNewConsole(void);

private:
  MainFrame&          m_mainFrame;
  ConsoleViewMap      m_views;
  Mutex               m_viewsMutex;
  shared_ptr<TabData> m_tabData;
  CString             m_strTitle;
  CIcon               m_bigIcon;
  CIcon               m_smallIcon;
  bool                m_boolIsGrouped;

  // static members
private:

};

//////////////////////////////////////////////////////////////////////////////

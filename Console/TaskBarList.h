#pragma once

#include "ConsoleView.h"

class TaskBarList
{
public:
  TaskBarList(void);
  ~TaskBarList(void);

  void AddTab         (ConsoleView* pConsoleView, HWND hwndMDI);
  void RefreshTab     (ConsoleView* pConsoleView);
  void UpdateTabTitle (ConsoleView* pConsoleView);
  void RemoveTab      (ConsoleView* pConsoleView);
  void SelectTab      (ConsoleView* pConsoleView, HWND hwndMDI);
//  void DragTab ...

  struct TabProxyInfo
  {
    TabProxyInfo(TaskBarList* pTaskBarList, ConsoleView* pConsoleView, ATOM atom, HICON hIconSm)
      :pTaskBarList(pTaskBarList)
      ,pConsoleView(pConsoleView)
      ,hwndProxyTab(nullptr)
      ,atom        (atom)
      ,hIconSm     (hIconSm)
    {
      TRACE(L"TaskBarList::CreateTabProxy ptpi=%p, pTaskBarList=%p, pConsoleView=%p\n", this, pTaskBarList, pConsoleView);
    }

    ~TabProxyInfo(void)
    {
      BOOL rc;

      if( hwndProxyTab )
      {
        rc = CWindow(this->hwndProxyTab).DestroyWindow();
        ATLASSERT(rc != FALSE);
      }

      rc = ::UnregisterClass(
        reinterpret_cast<wchar_t*>(this->atom),
        ::GetModuleHandle(nullptr));
      ATLASSERT(rc != FALSE);

      CIcon smallIcon(this->hIconSm);
    }

    bool operator==(HWND hwndProxyTab)
    {
      return this->hwndProxyTab == hwndProxyTab;
    }

    bool operator==(ConsoleView * pConsoleView)
    {
      return this->pConsoleView == pConsoleView;
    }

    TaskBarList* pTaskBarList;
    ConsoleView* pConsoleView;
    HWND         hwndProxyTab;
    ATOM         atom;
    HICON        hIconSm;
  };

private:
  HRESULT     RegisterTab   (HWND hwndTab, HWND hwndMDI);
  HRESULT     SetTabActive  (HWND hwndTab, HWND hwndMDI);
  HRESULT     SetTabOrder   (HWND hwndTab, HWND hwndInsertBefore);
  HRESULT     UnregisterTab (HWND hwndTab);

  template<typename T>
  inline bool Call          (T m, const std::function <void (TabProxyInfo*)>& f, bool boolRemove = false);

  static ATOM RegisterTabProxyClass(const wchar_t * szClassName, HICON hIconSm);
  HWND        CreateTabProxy(ConsoleView* pConsoleView);

  static LRESULT CALLBACK TaskBarList::TabProxyWndProc(HWND hwnd,UINT Msg,WPARAM wParam,LPARAM lParam/*,int iTabId*/);

private:
  ATL::CComPtr<ITaskbarList3>              m_taskBarList;
  bool                                     m_boolSupported;
  std::list<std::shared_ptr<TabProxyInfo>> m_listTabProxyInfos;
};


#include "stdafx.h"
#include "TaskBarList.h"


TaskBarList::TaskBarList(void)
{
  HRESULT hr = m_taskBarList.CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER);
  if( hr == S_OK )
  {
    // Initializes the taskbar list object.
    // This method must be called before any other ITaskbarList methods can be called.
    hr = m_taskBarList->HrInit();
  }
  m_boolSupported = hr == S_OK;
}


TaskBarList::~TaskBarList(void)
{
}


void TaskBarList::AddTab(ConsoleView* pConsoleView, HWND hwndMDI)
{
  if( !m_boolSupported )
    return;

  HWND hwndTabProxy = this->CreateTabProxy(pConsoleView);
  ATLASSERT(hwndTabProxy != nullptr);

  this->RegisterTab(hwndTabProxy, hwndMDI);
  this->SetTabOrder(hwndTabProxy, nullptr);
}


void TaskBarList::RefreshTab(ConsoleView* pConsoleView)
{
  if( !m_boolSupported )
    return;

  this->Call(pConsoleView, [](TaskBarList::TabProxyInfo * ptib)
  {
    TRACE(L"******** TaskBarList::RefreshTab called in thread %lu\n", ::GetCurrentThreadId());
#ifdef _DEBUG
    HRESULT hr = 
#endif
    ::DwmInvalidateIconicBitmaps(ptib->hwndProxyTab);
    ATLASSERT(hr == S_OK);
  });
}


void TaskBarList::UpdateTabTitle(ConsoleView* pConsoleView)
{
  if( !m_boolSupported )
    return;

  this->Call(pConsoleView, [](TaskBarList::TabProxyInfo * ptib)
  {
    TRACE(L"******** TaskBarList::UpdateTabTitle(%s) called in thread %lu\n",  ptib->pConsoleView->GetTitle(), ::GetCurrentThreadId());
    ::SetWindowText(ptib->hwndProxyTab, ptib->pConsoleView->GetTitle());
  });
}


void TaskBarList::RemoveTab(ConsoleView* pConsoleView)
{
  if( !m_boolSupported )
    return;

  this->Call(pConsoleView, [](TaskBarList::TabProxyInfo * ptib)
  {
    TRACE(L"******** TaskBarList::RemoveTab called in thread %lu\n", ::GetCurrentThreadId());
    ptib->pTaskBarList->UnregisterTab(ptib->hwndProxyTab);
  },
  true);
}


void TaskBarList::SelectTab(ConsoleView* pConsoleView, HWND hwndMDI)
{
  if( !m_boolSupported )
    return;

  this->Call(pConsoleView, [hwndMDI](TaskBarList::TabProxyInfo * ptib)
  {
    TRACE(L"******** TaskBarList::SelectTab(%s) called in thread %lu\n",  ptib->pConsoleView->GetTitle(), ::GetCurrentThreadId());
    ptib->pTaskBarList->SetTabActive(ptib->hwndProxyTab, hwndMDI);
  });
}


HRESULT TaskBarList::RegisterTab(HWND hwndTab, HWND hwndMDI)
{
  ATLASSERT(::IsWindow(hwndTab));
  ATLASSERT(::IsWindow(hwndMDI));
  HRESULT hr = m_taskBarList->RegisterTab(hwndTab, hwndMDI);
  ATLASSERT(hr == S_OK);
  return hr;
}


HRESULT TaskBarList::SetTabActive(HWND hwndTab, HWND hwndMDI)
{
  ATLASSERT(::IsWindow(hwndTab));
  ATLASSERT(::IsWindow(hwndMDI));
  HRESULT hr = m_taskBarList->SetTabActive(hwndTab, hwndMDI, 0);
  ATLASSERT(hr == S_OK);
  return hr;
}


HRESULT TaskBarList::SetTabOrder(HWND hwndTab, HWND hwndInsertBefore)
{
  ATLASSERT(::IsWindow(hwndTab));
  HRESULT hr = m_taskBarList->SetTabOrder(hwndTab, hwndInsertBefore);
  ATLASSERT(hr == S_OK);
  return hr;
}


HRESULT TaskBarList::UnregisterTab(HWND hwndTab)
{
  ATLASSERT(::IsWindow(hwndTab));
  return m_taskBarList->UnregisterTab(hwndTab);
}


ATOM TaskBarList::RegisterTabProxyClass(const wchar_t * szClassName, HICON hIconSm)
{
  WNDCLASSEX wcex;

  wcex.cbSize        = sizeof(wcex);
  wcex.style         = 0;
  wcex.lpfnWndProc   = &TaskBarList::TabProxyWndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = sizeof(TaskBarList::TabProxyInfo *);
  wcex.hInstance     = ::GetModuleHandle(nullptr);
  wcex.hIcon         = nullptr;
  wcex.hCursor       = ::LoadCursor(nullptr,IDC_ARROW);
  wcex.hbrBackground = nullptr;
  wcex.lpszMenuName  = nullptr;
  wcex.lpszClassName = szClassName;
  wcex.hIconSm       = hIconSm;

  return ::RegisterClassEx(&wcex);
}


HWND TaskBarList::CreateTabProxy(ConsoleView* pConsoleView)
{
  /* class name */
  wchar_t szClassName[32];
  static int iCount = 0;
  _snwprintf_s(
    szClassName, ARRAYSIZE(szClassName),
    _TRUNCATE,
    L"Console2++TabProxy%ld",
    iCount ++);

  /* small icon */
  std::shared_ptr<TabData> tabData = pConsoleView->GetTabData();
  CIcon tabSmallIcon(Helpers::LoadTabIcon(false, tabData->bUseDefaultIcon, tabData->strIcon, tabData->strShell));

  ATOM atom = this->RegisterTabProxyClass(szClassName, tabSmallIcon);

  if( atom == 0 )
    return nullptr;

  std::shared_ptr<TaskBarList::TabProxyInfo> tpi(new TaskBarList::TabProxyInfo(this, pConsoleView, atom, tabSmallIcon.Detach()));
  
  TRACE(L"TaskBarList::CreateTabProxy ptpi=%p\n", tpi.get());

  HWND hwndTabProxy = ::CreateWindow(
    szClassName,
    pConsoleView->GetTitle(),
    WS_OVERLAPPEDWINDOW,
    0, 0,
    0, 0,
    nullptr,
    nullptr,
    ::GetModuleHandle(0),
    tpi.get());

  if( hwndTabProxy )
  {
    HRESULT hr;

    BOOL bHasIconicBitmap = TRUE;
    hr = ::DwmSetWindowAttribute(
      hwndTabProxy,
      DWMWA_HAS_ICONIC_BITMAP,
      &bHasIconicBitmap,
      sizeof(bHasIconicBitmap));
    ATLASSERT(hr == S_OK);

    BOOL bForceIconic = TRUE;
    hr = ::DwmSetWindowAttribute(
      hwndTabProxy,
      DWMWA_FORCE_ICONIC_REPRESENTATION,
      &bForceIconic,
      sizeof(bForceIconic));
    ATLASSERT(hr == S_OK);

    m_listTabProxyInfos.push_back(tpi);
  }

  return hwndTabProxy;
}


LRESULT CALLBACK TaskBarList::TabProxyWndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam/*,int iTabId*/)
{
  TaskBarList::TabProxyInfo* ptpi = reinterpret_cast<TaskBarList::TabProxyInfo*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

  HRESULT hr;

  switch(Msg)
  {
  case WM_CREATE:
    ptpi = reinterpret_cast<TaskBarList::TabProxyInfo*>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
    ptpi->hwndProxyTab = hwnd;
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptpi));
    TRACE(L"TaskBarList::TabProxyWndProc hwnd=%p set ptpi=%p\n", hwnd, ptpi);
    break;

  case WM_ACTIVATE:
    TRACE(L"******** WM_ACTIVATE called in thread %lu\n", ::GetCurrentThreadId());
    ptpi->pConsoleView->Activate();
    break;
  
  case WM_SETFOCUS:
    TRACE(L"******** WM_SETFOCUS called in thread %lu\n", ::GetCurrentThreadId());
    break;

  case WM_SYSCOMMAND:
    TRACE(L"******** WM_SYSCOMMAND called in thread %lu\n", ::GetCurrentThreadId());
    break;

    /* Generate a thumbnail of the current tab. Basic procedure:
    1. Generate a full-scale bitmap of the main window.
    2. Overlay a bitmap of the specified tab onto the main
    window bitmap.
    3. Shrink the resulting bitmap down to the correct thumbnail size.

    A thumbnail will be dynamically generated, provided the main window
    is not currently minimized (as we won't be able to grap a screenshot
    of it). If the main window is minimized, we'll use a cached screenshot
    of the tab (taken before the main window was minimized). */
  case WM_DWMSENDICONICTHUMBNAIL:
    TRACE(L"******** WM_DWMSENDICONICTHUMBNAIL called in thread %lu\n", ::GetCurrentThreadId());
    {
      int nMaxWidth  = HIWORD(lParam);
      int nMaxHeight = LOWORD(lParam);

      CBitmap bmpThumbnail(ptpi->pConsoleView->GetThumbnail(nMaxWidth,nMaxHeight));

      hr = ::DwmSetIconicThumbnail(hwnd, bmpThumbnail, 0);
      ATLASSERT(hr == S_OK);

      return 0;
    }
    break;

  case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
    TRACE(L"******** WM_DWMSENDICONICLIVEPREVIEWBITMAP called in thread %lu\n", ::GetCurrentThreadId());
    {
      POINT   ptClient;
      HBITMAP bmpLivePreview = ptpi->pConsoleView->GetLivePreview(ptClient);

      hr = ::DwmSetIconicLivePreviewBitmap(hwnd, bmpLivePreview, &ptClient, 0);
      ATLASSERT(hr == S_OK);

      return 0;
    }
    break;

  case WM_CLOSE:
    ptpi->pConsoleView->OnConsoleClose();
    return 0;
    break;

  case WM_SETTEXT:
  case WM_ACTIVATEAPP:
    break;

  default:
    TRACE(L"@@@@@@ message %LX %lu\n", Msg, Msg);
    break;

  }

  return ::DefWindowProc(hwnd, Msg, wParam, lParam);
}

template<typename T>
inline bool TaskBarList::Call(T m, const std::function <void (TaskBarList::TabProxyInfo*)>& f, bool boolRemove /*= false*/)
{
  for(auto i = m_listTabProxyInfos.begin(); i != m_listTabProxyInfos.end(); ++i)
  {
    TaskBarList::TabProxyInfo * p = i->get();
    if( *p == m )
    {
      f(p);
      if( boolRemove )
        m_listTabProxyInfos.erase(i);
      return true;
    }
  }

  return false;
}

#include "StdAfx.h"
#include "WallPaper.h"

MyThread::MyThread(void)
:hStopSignalPtr(nullptr)
,hThreadPtr(nullptr)
,dwResult(0)
{
}

MyThread::~MyThread(void)
{
  try
  {
    this->Stop(INFINITE);
  }
  catch(std::exception&)
  {
  }
}

void MyThread::Start(DWORD dwStackSize /*= 0*/)
{
  this->dwResult = 0;

  if( this->hThreadPtr.get() )
    throw std::exception("Thread is already running!");

  this->hStopSignalPtr.reset(::CreateEvent(NULL, FALSE, FALSE, NULL));
  if( this->hStopSignalPtr.get() == nullptr )
    Win32Exception::ThrowFromLastError();

  this->hThreadPtr.reset((HANDLE)_beginthreadex(0, dwStackSize, &_MyThreadFunction, (void*)this, 0, NULL));
  if( this->hThreadPtr.get() == nullptr )
    throw std::exception(_sys_errlist[errno]);
}

DWORD MyThread::Stop(DWORD dwTimeout)
{
  try
  {
    if( this->hStopSignalPtr.get() )
      if( !SetEvent(this->hStopSignalPtr.get()) )
        Win32Exception::ThrowFromLastError();

    if( this->hThreadPtr.get() )
      if( WaitForSingleObject(this->hThreadPtr.get(), dwTimeout) != WAIT_OBJECT_0 )
        Win32Exception::ThrowFromLastError();
  }
  catch(std::exception&)
  {
    this->hStopSignalPtr.reset();
    this->hThreadPtr.reset();

    throw;
  }

  this->hStopSignalPtr.reset();
  this->hThreadPtr.reset();

  return this->dwResult;
}

unsigned int __stdcall _MyThreadFunction(void* arg)
{
  MyThread* pThread = static_cast<MyThread*>(arg);

  try
  {
    pThread->dwResult = pThread->Process(pThread->hStopSignalPtr.get());
  }
  catch(std::exception& e)
  {
    pThread->dwResult = ~(DWORD)0;
  }

  return 0;
}

DWORD WallPaperThread::Process(HANDLE hStopSignal)
{
  HKEY hkey;
  LSTATUS rc = ::RegOpenKeyEx(
    HKEY_CURRENT_USER,
    L"Control Panel\\Desktop",
    0,
    KEY_READ,
    &hkey);

  if( rc != ERROR_SUCCESS )
    Win32Exception::Throw(rc);

  unique_ptr<HKEY__, RegCloseKeyHelper>hkeyPtr(hkey);

  wchar_t szWallpaper[_MAX_PATH];

  std::unique_ptr<void, CloseHandleHelper>regkeyChangeNotification(
    ::CreateEvent(NULL, FALSE, TRUE, NULL));
  if( regkeyChangeNotification.get() == nullptr )
    Win32Exception::ThrowFromLastError();

  unique_ptr<void, FindCloseChangeNotificationHelper>folderChangeNotification(INVALID_HANDLE_VALUE);

  long long llWallPaperFileSize = 0;

  for(bool loop = true; loop;)
  {
    HANDLE h[3] = {hStopSignal, regkeyChangeNotification.get(), folderChangeNotification.get()};
    switch( ::WaitForMultipleObjects(folderChangeNotification.get() == INVALID_HANDLE_VALUE?2:3, h, FALSE, INFINITE) )
    {
    case WAIT_OBJECT_0:
      loop = false;
      break;

    case WAIT_OBJECT_0+1:
      {
        DWORD dwType;
        DWORD dwValueSize = static_cast<DWORD>(sizeof(szWallpaper));

        rc = ::RegQueryValueEx(
          hkeyPtr.get(),
          L"Wallpaper",
          NULL,
          &dwType,
          (LPBYTE)szWallpaper, &dwValueSize);

        if( rc != ERROR_SUCCESS )
          Win32Exception::Throw(rc);

        wchar_t szWallpaperFolder[_MAX_PATH];
        ::memcpy(szWallpaperFolder, szWallpaper, sizeof(szWallpaperFolder));
        ::PathRemoveFileSpec(szWallpaperFolder);

        folderChangeNotification.reset(
          ::FindFirstChangeNotification(
            szWallpaperFolder,
            FALSE,
            FILE_NOTIFY_CHANGE_SIZE));

        if( folderChangeNotification.get() == INVALID_HANDLE_VALUE )
          Win32Exception::ThrowFromLastError();
      }
      break;

    case WAIT_OBJECT_0+2:
      {
        _stat64 theStat;
        if( _wstat64(szWallpaper, &theStat) )
          throw std::exception(_sys_errlist[errno]);

        if( theStat.st_size && llWallPaperFileSize != theStat.st_size )
        {
          llWallPaperFileSize = theStat.st_size;

          // delay the message sending: desktop black / not ready
          ::Sleep(100);
          m_mainFrame.SendMessageW(WM_SETTINGCHANGE, 0, (LPARAM)L"intl");
        }

        if( !::FindNextChangeNotification(folderChangeNotification.get()) )
           Win32Exception::ThrowFromLastError();
      }
      break;
    }
  }

  return 0;
}
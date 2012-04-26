#pragma once

struct LocalFreeHelper
{
  void operator()(void * toFree)
  {
    ::LocalFree(static_cast<HLOCAL>(toFree));
  };
};

struct CloseHandleHelper
{
  void operator()(void * toFree)
  {
    if( toFree && toFree != INVALID_HANDLE_VALUE )
      ::CloseHandle(static_cast<HANDLE>(toFree));
  };
};

struct FindCloseChangeNotificationHelper
{
  void operator()(void * toFree)
  {
    if( toFree != INVALID_HANDLE_VALUE )
      ::FindCloseChangeNotification(static_cast<HANDLE>(toFree));
  };
};

struct RegCloseKeyHelper
{
  void operator()(void * toFree)
  {
    ::RegCloseKey(static_cast<HKEY>(toFree));
  };
};
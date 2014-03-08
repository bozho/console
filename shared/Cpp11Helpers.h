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

#ifdef _INC_USERENV
struct DestroyEnvironmentBlockHelper
{
  void operator()(void * toFree)
  {
    if( toFree )
      ::DestroyEnvironmentBlock(toFree);
  };
};
#endif

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

#ifdef _MUILOAD_H_INCLUDED_
struct FreeMUILibraryHelper
{
	void operator()(HINSTANCE toFree)
	{
		if( toFree != NULL )
			::FreeMUILibrary(toFree);
	};
};
#endif
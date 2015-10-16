#pragma once

struct LocalFreeHelper
{
	void operator()(void * toFree)
	{
		::LocalFree(static_cast<HLOCAL>(toFree));
	};
};

class Win32Exception : public std::exception
{
  DWORD errorCode_;
  mutable std::string buff_;
  std::string api_;

  void FormatMessage() const
  {
		if( !buff_.empty() ) return;

		buff_ = "(";
		buff_ += api_;
		buff_ += ")";

    LPSTR buffer = nullptr;
    DWORD bufferLength = ::FormatMessageA(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      NULL,
      GetErrorCode(),
      0,
      reinterpret_cast<LPSTR>(&buffer),
      0,
      NULL);
    if( bufferLength )
    {
      std::unique_ptr<void, LocalFreeHelper> buffPtr(buffer);
      buff_ += buffer;
    }
  }

public:
  Win32Exception(const char * api, DWORD errorCode) : errorCode_(errorCode), api_(api) {};

  __declspec(noreturn) static void Throw(const char * api, DWORD lastError) { throw Win32Exception(api, lastError); }

  __declspec(noreturn) static void ThrowFromLastError(const char * api) { Throw(api, ::GetLastError()); };

  inline DWORD GetErrorCode() const
  {
    return errorCode_;
  }

  virtual const char* what() const
  {
    FormatMessage();
    return buff_.c_str();
  };
};

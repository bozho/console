#pragma once

class Win32Exception : public std::exception
{
  DWORD errorCode_;
  mutable std::string buff_;

  void FormatMessage() const
  {
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
      buff_ = buffer;
    }
  }

public:
  Win32Exception(DWORD errorCode) : errorCode_(errorCode) {};

  __declspec(noreturn) static void Throw(DWORD lastError) {  throw Win32Exception(lastError); }

  __declspec(noreturn) static void ThrowFromLastError() { Throw(::GetLastError()); };

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

#pragma once

#include <string>

#include "Support.h"

// debug is send to the local USBSerial port
#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#  define Debug(...) do { ::Output(__VA_ARGS__); } while (false)
#else
#  define Debug(...) do {} while (false)
#endif
//#define ENABLE_TRACE
#ifdef ENABLE_TRACE
#  define Trace() Debug("%s\n",__PRETTY_FUNCTION__)
#else
#  define Trace() do {} while (false)
#endif

#define CRLF "\r\n"

namespace Output_internal
{
class Output final
{
public:
  Output();
  Output(const Output &) = delete;
  Output(Output &&) = delete;

  bool Setup();

  template<typename... ARGS>
  void operator()(const char* format, ARGS... args) const
  {
    std::string s = Format(format, args ...);    
    WriteString(s);
  }
  template<typename... ARGS>
  void operator()(const std::string & format, ARGS... args) const
  {
    std::string s = Format(format, args ...);
    WriteString(s);
  }

private:
  void WriteString(const std::string & s) const;  
};
}

extern ::Output_internal::Output Output;

#pragma once

#include <memory>
#include <string>
#include <type_traits>

//
// support functions which do not have dependencies on other parts of the code
// separated to make the rest of the code smaller and only work on concepts
//

//
// turn a hexadecimal character [0-9,a-f,A-F] into a numeric values (0-16).
// returns -1 on failure (out of range)
// 
int HexCharToNible(char hex);

//
// return a lowercase copy of s
//
std::string ToLower(const std::string & s);

//
// find the first occurrance of separator in the input, 
// return [start-separator) as return value and set input to (separator-end]
//
std::string GetFirstPart(std::string & input, const char separator = ' ');

//
// reduce any enum to its underlying type
//
template <typename E>
constexpr auto ToUnderlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

//
// format the string
// 
template<typename... ARGS>
std::string Format(const char* format, ARGS... args)
{
  // this function is not re-entrant. cache the buffer for better performance
  static std::vector<char> buf(16,0);
  int size = 0;
  while(size>=0)
  {
    size = std::snprintf( buf.data(), buf.size(), format, args ... );
    if( size >= buf.size())
    {
      buf.resize(size + 16,0);
    }
    else if (size >= 0)
    {
      return std::string(buf.begin(), buf.begin() + size);
    }
  }
  return "[ERROR]";
}
template<typename... ARGS>
std::string Format(const std::string & format, ARGS... args)
{
  return Format(format.c_str(), args ...);
}

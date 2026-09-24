#pragma once

#include <algorithm> 
#include <cctype>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

//
// support functions which do not have dependencies on other parts of the code
// separated to make the rest of the code smaller and only work on concepts
//

namespace Joost
{

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
// string Trim functions
//
inline void LeftTrim(std::string &s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) 
    {
        return !std::isspace(ch);
    }));
}
inline void RightTrim(std::string &s) 
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) 
    {
        return !std::isspace(ch);
    }).base(), s.end());
}
inline void Trim(std::string &s) 
{
    RightTrim(s);
    LeftTrim(s);
}
inline std::string LeftTrimCopy(std::string s) 
{
    LeftTrim(s);
    return s;
}
inline std::string RightTrimCopy(std::string s) 
{
    RightTrim(s);
    return s;
}
inline std::string TrimCopy(std::string s) 
{
    Trim(s);
    return s;
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

std::string Join(const std::vector<std::string> & v, const std::string & separator);

} // namespace Joost

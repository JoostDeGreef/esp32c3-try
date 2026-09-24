#include <algorithm>
#include <vector>

#include "joost_string.h"

using namespace std;

int Joost::HexCharToNible(char hex)
{
  switch(hex)
  {
    case '0':           return 0;      case '1':           return 1;   
    case '2':           return 2;      case '3':           return 3; 
    case '4':           return 4;      case '5':           return 5;   
    case '6':           return 6;      case '7':           return 7; 
    case '8':           return 8;      case '9':           return 9;   
    case 'a': case 'A': return 10;     case 'b': case 'B': return 11; 
    case 'c': case 'C': return 12;     case 'd': case 'D': return 13;  
    case 'e': case 'E': return 14;     case 'f': case 'F': return 15; 
  }
  return -1;
};

string Joost::GetFirstPart(string & input, const char separator)
{
  auto index = input.find(separator);
  std::string res;
  if(index != std::string::npos)
  {
    res = input.substr(index+1);
    input.resize(index);
  }
  res.swap(input);
  return res;
}

string Joost::ToLower(const string & s)
{
  string res;
  res.resize(s.size());
  transform(s.begin(), s.end(), res.begin(), [](unsigned char c){ return tolower(c); } );
  return res;
}

string Joost::Join(const vector<string> & v, const string & separator)
{
    if(v.empty())
    {
        return "";
    }
    string res = v[0];
    for(size_t i = 1; i < v.size(); ++i)
    {
        res += separator + v[i];
    }
    return res;
}

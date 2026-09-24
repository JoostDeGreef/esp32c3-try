#include <stdint.h>
#include <algorithm>
#include <numeric>

#include <esp_mac.h>  // For the MAC2STR and MACSTR macros
#include <ESP32_NOW.h>
#include <WiFi.h>

#include "MAC.h"
#include "Output.h"
#include "Support.h"

using namespace std;

namespace 
{
  MAC Parse(const string & mac)
  {
    if(mac.size()!=17)
    {
      return MAC();
    }
    for(int i=0;i<5;++i)
    {
      if(mac[2+i*3] != ':')
      {
        return MAC();
      }
    }
    array<uint8_t, 6> input;
    for(int i=0;i<6;++i)
    {
      int a = HexCharToNible(mac[i*3+0]);
      int b = HexCharToNible(mac[i*3+1]);
      if(a<0 || b<0)
      {
        return MAC();
      }
      input[i] = a*16+b;
    }
    return MAC(input.data());
  }
}

MAC::MAC(const uint8_t* source)
{
  std::copy(source, source + mac.size(), mac.begin());
}
MAC::MAC(const MAC & source)
    : mac(source.mac)
{}
MAC::MAC(const string & source)
    : mac(Parse(source).mac)
{}

MAC& MAC::operator=(const uint8_t* source)
{
    return *this = MAC(source);
}
MAC& MAC::operator=(const MAC & source)
{
    return *this = MAC(source);
}
MAC& MAC::operator=(const string & source)
{
    return *this = MAC(source);
}

bool MAC::operator==(const MAC& other) const
{
    return mac == other.mac;
}
bool MAC::operator!=(const MAC& other) const
{
    return mac != other.mac;
}

string MAC::ToString() const
{
    return Format(MACSTR, MAC2STR(mac.data()));
}

const uint8_t *MAC::Data() const
{
    return mac.data();
}
uint8_t *MAC::Data()
{
    return mac.data();
}

bool MAC::IsValid() const
{
    return accumulate(begin(mac), end(mac), (int)0) > 0;
}

size_t MAC::HashFunction::operator()(const MAC& mac) const
{
    auto data = mac.Data();
    return (hash<uint8_t>()(data[0]) <<  0)
         ^ (hash<uint8_t>()(data[1]) <<  5)
         ^ (hash<uint8_t>()(data[2]) << 10)
         ^ (hash<uint8_t>()(data[3]) << 15)
         ^ (hash<uint8_t>()(data[4]) << 20)
         ^ (hash<uint8_t>()(data[5]) << 24);
}

const MAC MAC::BroadCast(ESP_NOW.BROADCAST_ADDR);



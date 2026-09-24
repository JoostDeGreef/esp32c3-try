#include "esp32-hal.h"
#pragma once

#include <string>
#include <queue>

#include <ESP32_NOW.h>

#include "Action.h"
#include "MAC.h"

class Peer final : public ESP_NOW_Peer 
{
public:
  Peer(MAC mac, int channel, wifi_interface_t iface, const uint8_t *lmk) 
    : ESP_NOW_Peer(mac.Data(), channel, iface, lmk) 
  {}

  // Destructor of the class
  ~Peer() 
  {
    remove();
  }

  // Function to register the peer
  bool Setup() 
  {
    if (!add()) 
    {
      Debug("Failed to register the peer (%s)",MAC(addr()).ToString().c_str());
      return false;
    }
    Tick();
    return true;
  }

  bool Send(const Action action)
  {
    return SendMessage(ToString(action));
  }
  bool Send(const std::string & message)
  {
    return SendMessage(message);
  }
  bool Send(const Action action, const std::string & data)
  {
    return SendMessage(ToString(action) + " " + data);
  }
  void onReceive(const uint8_t *data, size_t len, bool broadcast) 
  {
    messages.emplace(reinterpret_cast<const char *>(data),len);
    Tick();
  }
  bool PopMessage(std::string & message)
  {
    if(messages.empty())
    {
      return false;
    }
    message = messages.front();
    messages.pop();
    return true;
  }
  bool IsStale(unsigned long oldAge = 5000) const
  {
    return millis() - last >= oldAge;
  }
private:
  // Function to send a message
  bool SendMessage(const std::string & data)
  {
    return SendMessage(data.c_str(), data.size());
  } 
  bool SendMessage(const void *data, size_t len) 
  {
    if (!send((const uint8_t *)data, len)) 
    {
      Debug("Failed to send message\n");
      return false;
    }
    return true;
  }
  void Tick()
  {
    last = millis();
  }
  std::queue<std::string> messages;
  unsigned long last = millis();
};

#include <stdint.h>

#include <ESP32_NOW.h>
#include "esp_wifi.h"
#include <WiFi.h>

#include "MAC.h"
#include "Output.h"
#include "Support.h"
#include "Timer.h"
#include "Wireless.h"

using namespace std;
using namespace Wireless_internal;

#define ESPNOW_WIFI_CHANNEL 6

bool Wireless::Setup()
{
  WiFi.mode(WIFI_STA);
  if (WiFi.setChannel(ESPNOW_WIFI_CHANNEL) != ESP_OK)
  {
    Debug("Failed to set WiFi channel" CRLF);
    return false;
  }
  if( !WiFi.STA.started() ) 
  {
    Debug("WiFi could not be started" CRLF);
    return false;
  }
  if( !ESP_NOW.begin() )
  {
    Debug("Failed to initialize ESP-NOW" CRLF);
    return false;
  }
  // lower TX power because the devices are close together and we'd overload them otherwise.
  if (!WiFi.setTxPower(WIFI_POWER_5dBm))
  {
    Debug("Failed to set WiFi TX power" CRLF);
    return false;
  }
  // hookup the 'new peer' callback 
  ESP_NOW.onNewPeer(OnNewPeer, this);
  // register maintenance timers
  Timer.AddEvent([&](){BroadcastPing();}, 1000); // broadcastping every second
  Timer.AddEvent([&](){RemoveStalePeers();}, 12000); // remove stale peers every 12 seconds
  // add broadcast peer
  return AddAndSetupPeer(MAC::BroadCast);
}

std::tuple<MAC,std::string> Wireless::ReadInput()
{
  // check the peers for available messages
  std::string message;
  for(auto macPeer:peers)
  {
    if(macPeer.second->PopMessage(message))
    {
      return make_tuple(macPeer.first, message);
    }
  }
  return make_tuple(MAC(),message);
}

string Wireless::GetESPInfo()
{
  uint32_t version;
  esp_now_get_version(&version);
  return Format("ESP_NOW/Wi-Fi parameters:" CRLF)
       + Format("  Mode: STA" CRLF)
       + Format("  MAC Address: %s" CRLF, WiFi.macAddress().c_str())
       + Format("  Channel: %u" CRLF, ESPNOW_WIFI_CHANNEL)
       + Format("  ESP-NOW version: %d" CRLF, ESP_NOW.getVersion())
       + Format("  max data length: %d" CRLF, ESP_NOW.getMaxDataLen());
}

void Wireless::BroadcastPing()
{
  Trace();
  if( !peers[MAC::BroadCast]->Send(Action::Ping) )
  {
    Debug("Failed to broadcast ping!" CRLF);
  }
}

void Wireless::OnNewPeer(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg)
{
  Trace();
  Wireless & wireless = *(Wireless *)arg;
  MAC source(info->src_addr);
  MAC dest(info->des_addr);
  Debug("Unknown peer %s sent a message to %s" CRLF, source.ToString().c_str(), dest.ToString().c_str());
  bool broadcast = dest == MAC::BroadCast;
  if(wireless.AddAndSetupPeer(source))
  {
    wireless.peers[source]->onReceive(data, len, broadcast);
  }
}

bool Wireless::AddAndSetupPeer(MAC mac)
{
  auto res = peers.emplace(mac, new Peer(mac, ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, nullptr));
  if( !res.second || !res.first->second->Setup() )
  {
    Debug("Failed to setup peer %s" CRLF, mac.ToString().c_str());
    return false;
  }
  else
  {
    Debug("Added peer %s" CRLF, mac.ToString().c_str());
  }
  return true;
};

void Wireless::RemoveStalePeers()
{
  for (auto macPeer = peers.begin(); macPeer != peers.end();)
  {
    if( macPeer->second->IsStale() && macPeer->first != MAC::BroadCast )
    {
      Debug("Peer %s turned stale, removing" CRLF, macPeer->first.ToString().c_str());
      macPeer = peers.erase(macPeer);
    }
    else
    {
      ++macPeer;
    }
  }
}

std::string Wireless::ListPeerStatus()
{
  Trace();
  if(peers.size()<=1)
  {
    return "  None" CRLF;
  }
  std::string res;
  for (auto macPeer = peers.begin(); macPeer != peers.end(); ++macPeer)
  {
    if( macPeer->first != MAC::BroadCast )
    {
      res += "  " + macPeer->first.ToString() + (macPeer->second->IsStale() ? " (Stale)" CRLF : CRLF);
    }
  }
  return res;
}

void Wireless::SendMessage(const MAC & dest, const std::string & message)
{
  auto iter = peers.find(dest);
  if( iter == peers.end())
  {
    Debug("Could not find peer %s" CRLF, dest.ToString().c_str());
    return;
  }
  Debug("Sending to (%s): %s" CRLF,dest.ToString().c_str(),message.c_str());
  iter->second->Send(message);
}

void Wireless::SendAction(const MAC & dest, const Action & action)
{
  auto iter = peers.find(dest);
  if( iter == peers.end())
  {
    Debug("Could not find peer %s" CRLF, dest.ToString().c_str());
    return;
  }
  iter->second->Send(action);
}

void Wireless::SendActionData(const MAC & dest, const Action & action, const std::string & data)
{
  auto iter = peers.find(dest);
  if( iter == peers.end())
  {
    Debug("Could not find peer %s" CRLF, dest.ToString().c_str());
    return;
  }
  iter->second->Send(action, data);
}

::Wireless_internal::Wireless Wireless;

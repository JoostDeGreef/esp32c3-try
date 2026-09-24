#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

#include "MAC.h"
#include "Peer.h"

namespace Wireless_internal
{
  class Wireless final
  {
  public:
    bool Setup();

    std::tuple<MAC,std::string> ReadInput();

    std::string GetESPInfo();

    std::string ListPeerStatus();

    void SendMessage(const MAC & dest, const std::string & message);
    void SendAction(const MAC & dest, const Action & action);
    void SendActionData(const MAC & dest, const Action & action, const std::string & data);

  private:
    static void OnNewPeer(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg);

    bool AddAndSetupPeer(MAC mac);
    void RemoveStalePeers();
    void BroadcastPing();

    std::unordered_map<MAC, std::shared_ptr<Peer>> peers;
  };
}

extern Wireless_internal::Wireless Wireless;

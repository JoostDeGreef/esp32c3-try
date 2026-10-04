#pragma once

#include <string>

namespace WiFi
{
    void configure();
    void connect(const char * ssid, const char * password);
    void disconnect();
    bool is_connected();
    const std::string get_ip();
}

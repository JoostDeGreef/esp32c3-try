
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "sdkconfig.h"

#include "clock.h"
#include "console.h"
#include "serial.h"
#include "wifi.h"
#include "joost_onboardled.h"

// this include file defines things which should not go to github, like wifi ssid and password
#include "wifi_secrets.h"

extern "C" void app_main()
{
    Console::write("Starting up\n");   

    Serial::configure();
  
    WiFi::configure();
    WiFi::connect(WIFI_SSID, WIFI_PASSWORD);

    Clock::start();

    OnboardLed::flash();

    // switch to slient operation, not to mess up the console
    esp_log_level_set("ROAM", ESP_LOG_WARN);
    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("wpa", ESP_LOG_WARN);

    Console::mainLoop();
}

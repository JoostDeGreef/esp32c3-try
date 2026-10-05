#include <string.h>

#include "wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;
#define WIFI_MAXIMUM_RETRY 5

static EventGroupHandle_t s_wifi_event_group;

static std::string wifi_ip = "";

static constexpr uint16_t WIFI_SCAN_MAX_RECORDS = 32;
static wifi_ap_record_t wifi_scan_records[WIFI_SCAN_MAX_RECORDS];

static void print_wifi_status(const char* where)
{
    wifi_ap_record_t ap_info = {};
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);

    if (ret == ESP_OK)
    {
        printf("%s: ASSOCIATED BSSID %02X:%02X:%02X:%02X:%02X:%02X channel %u RSSI %d\n",
               where,
               ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2],
               ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5],
               ap_info.primary,
               ap_info.rssi);
    }
    else
    {
        printf("%s: NOT ASSOCIATED (%s)\n", where, esp_err_to_name(ret));
    }
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        wifi_scan_config_t scan_config = {};
        scan_config.show_hidden = true;

        ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

        uint16_t ap_count = 0;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

        printf("Found %u access points:\n", ap_count);

        uint16_t record_count = ap_count > WIFI_SCAN_MAX_RECORDS
                              ? WIFI_SCAN_MAX_RECORDS
                              : ap_count;

        if (record_count > 0)
        {
            ESP_ERROR_CHECK(
                esp_wifi_scan_get_ap_records(&record_count, wifi_scan_records));

            for (uint16_t i = 0; i < record_count; ++i)
            {
                printf("  %-32s  channel %2u  RSSI %4d  auth %d\n",
                       reinterpret_cast<char*>(wifi_scan_records[i].ssid),
                       wifi_scan_records[i].primary,
                       wifi_scan_records[i].rssi,
                       wifi_scan_records[i].authmode);
            }
        }

        printf("Wi-Fi scan finished.\n");

        esp_err_t ret = esp_wifi_connect();
        printf("esp_wifi_connect(): %s\n", esp_err_to_name(ret));
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        printf("===> for Mira: %i\n", event->reason);

        print_wifi_status("after disconnect");

        wifi_ip = "";
        if (s_retry_num < WIFI_MAXIMUM_RETRY) 
        {
            esp_err_t ret = esp_wifi_connect();
            printf("retry esp_wifi_connect(): %s\n", esp_err_to_name(ret));

            s_retry_num++;
            printf("retry to connect to the AP\n");
        } 
        else 
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        printf("connect to the AP fail\n");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        char buf[16];
        wifi_ip = std::string(esp_ip4addr_ntoa(&event->ip_info.ip, buf, sizeof(buf)));
        printf("got ip:" IPSTR "\n", IP2STR(&event->ip_info.ip));

        print_wifi_status("got IP");

        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

namespace WiFi
{
    void configure()
    {
        printf("ESP_WIFI_MODE_STA\n");

        // esp_log_level_set("wifi", ESP_LOG_DEBUG);
        // esp_log_level_set("wpa", ESP_LOG_DEBUG);
        esp_log_level_set("wifi", ESP_LOG_VERBOSE);
        esp_log_level_set("wpa", ESP_LOG_VERBOSE);

        s_wifi_event_group = xEventGroupCreate();

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &event_handler,
                                                            NULL,
                                                            &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &event_handler,
                                                            NULL,
                                                            &instance_got_ip));
    }

    void connect(const char * ssid, const char * password)
    {
        wifi_config_t sta_config = {};

        strcpy((char*)sta_config.sta.ssid, ssid);
        strcpy((char*)sta_config.sta.password, password);

        sta_config.sta.bssid_set = false;

        // when debugging this can be used to connect to a specific access point, but it is not needed for normal operation
        // static const uint8_t test_bssid[6] = {
        //     0x14, 0x49, 0xBC, 0x8C, 0x61, 0xA2
        // };
        // sta_config.sta.bssid_set = true;
        // memcpy(sta_config.sta.bssid, test_bssid, sizeof(test_bssid));

        // For this diagnostic run, accept only WPA2-PSK or stronger.
        sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        // physical layout issue of the board requires this:
        ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(34));

        printf("wifi_init_sta finished.\n");

        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT) 
        {
            printf("connected to ap SSID:%s\n", ssid);
        } 
        else if (bits & WIFI_FAIL_BIT) 
        {
            printf("Failed to connect to SSID:%s\n", ssid);
        } 
        else 
        {
            printf("UNEXPECTED EVENT\\n");
        }
    }

    void disconnect()
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }

    bool is_connected()
    {
        return !wifi_ip.empty();
    }

    const std::string get_ip()
    {
        return wifi_ip;
    }
}

#include <string.h>

#include "wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* number or retries. make this a member variable */
static int s_retry_num = 0;
#define WIFI_MAXIMUM_RETRY 5

/* FreeRTOS event group to signal when we are connected. make this a member variable */
static EventGroupHandle_t s_wifi_event_group;

static std::string wifi_ip = "";

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        wifi_ip = "";
        if (s_retry_num < WIFI_MAXIMUM_RETRY) 
        {
            esp_wifi_connect();
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
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

namespace WiFi
{
    void configure()
    {
        printf("ESP_WIFI_MODE_STA\n");
        s_wifi_event_group = xEventGroupCreate();

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
        //Allocate storage for the struct
        wifi_config_t sta_config = {};

        //Assign ssid & password strings
        strcpy((char*)sta_config.sta.ssid, ssid);
        strcpy((char*)sta_config.sta.password, password);
        sta_config.sta.bssid_set = false;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
        ESP_ERROR_CHECK(esp_wifi_start() );

        printf("wifi_init_sta finished.\n");

        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
         * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY);

        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
         * happened. */
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
            printf("UNEXPECTED EVENT\n");
        }
    }

    void disconnect()
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect() );
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


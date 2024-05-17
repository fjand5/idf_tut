#include <stdio.h>
#include "wifi.h"
void startWifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = false;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t staCfg = {};
    strcpy((char *)staCfg.sta.ssid, "Vong Cat-Hide");
    strcpy((char *)staCfg.sta.password, "78787878");
    staCfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &staCfg));

    static auto eventHandle =
        [](void *event_handler_arg,
           esp_event_base_t event_base,
           int32_t event_id,
           void *event_data)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            ESP_ERROR_CHECK(esp_wifi_connect());
        }
    };
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        eventHandle, NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
}

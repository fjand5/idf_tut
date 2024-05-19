#include <stdio.h>
#include "wifi/include/wifi.h"
#include "webserver/include/webserver.h"
#include "update/include/update.h"
#include "led/include/led.h"
#include "dac/include/dac.h"
#include "rmt/include/rmt.h"

#include "esp_log.h"
const static char *TAG = "__main";
extern "C"
{
    void app_main(void)
    {

        startRMT();
        // startWifi();
        // ESP_ERROR_CHECK(esp_event_handler_instance_register(
        //     WIFI_EVENT,
        //     ESP_EVENT_ANY_ID,
        //     [](void *event_handler_arg,
        //        esp_event_base_t event_base,
        //        int32_t event_id,
        //        void *event_data)
        //     {
        //         if (event_id == WIFI_EVENT_STA_CONNECTED)
        //         {
        //             httpd_handle_t webserver = startWebserver();
        //             startUpdate(webserver);
        //             startLed(webserver);
        //             startDAC(webserver);
        //         }
        //     },
        //     NULL, NULL));

        //     ESP_LOGW("__MAIN","Da thay doi client update");
    }
}
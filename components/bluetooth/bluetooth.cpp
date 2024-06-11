#include <stdio.h>
#include "bluetooth.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "esp_hidd_api.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "algorithm"
#include "esp_console.h"
#include "argtable3/argtable3.h"

auto typeMessage = xMessageBufferCreate(sizeof(esp_hidd_status_t) + sizeof(size_t));
void startBluetooth(void)
{
  if (nvs_flash_init() != ESP_OK)
  {
    nvs_flash_erase();
    nvs_flash_init();
  }
  esp_bt_controller_config_t *cfg = new esp_bt_controller_config_t BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  cfg->mode = ESP_BT_MODE_CLASSIC_BT;
  esp_bt_controller_init(cfg);
  esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);

  esp_bluedroid_init_with_cfg(new esp_bluedroid_config_t{{false}});
  esp_bluedroid_enable();

  esp_bt_dev_set_device_name("myBluetooth");
  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

  auto hidInitCallback = [](esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param)
  {
    static uint8_t desc_list[] = {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x06, // USAGE (Keyboard)
        0xa1, 0x01, // COLLECTION (Application)
        0x05, 0x07, // USAGE_PAGE (Keyboard)
        0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated)) 0 a=4
        0x29, 0x1d, // USAGE_MAXIMUM (Keyboard z and Z)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0x1d, // LOGICAL_MAXIMUM (29)
        0x75, 0x08, // REPORT_SIZE (8)
        0x95, 0x01, // REPORT_COUNT (1)
        0x81, 0x00, // INPUT (Data,Ary,Abs)
        0xc0        // END_COLLECTION
    };
    switch (event)
    {
    case ESP_HIDD_INIT_EVT:

      esp_bt_hid_device_register_app(
          new esp_hidd_app_param_t{
              "Keyboard",
              "Keyboard",
              "esp32",
              ESP_HID_CLASS_KBD,
              desc_list,
              sizeof(desc_list)},
          new esp_hidd_qos_param_t{},
          new esp_hidd_qos_param_t{});
      break;
    case ESP_HIDD_SEND_REPORT_EVT:
      xMessageBufferSend(typeMessage, &(param->send_report.status), sizeof(esp_hidd_status_t), 0);
      break;
    case ESP_HIDD_OPEN_EVT:
      // xTaskCreate(
      //     [](void *p)
      //     {
      //       uint8_t data = 0;
      //       while (1)
      //       {
      //         data = 0x05; //b
      //         esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0, 1, &data);
      //         data = 0x0; // nhả ra
      //         vTaskDelay(pdMS_TO_TICKS(20));
      //         esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0, 1, &data);
      //         vTaskDelay(pdMS_TO_TICKS(1000));
      //       }
      //     },
      //     "noname", 2000, nullptr, 1, nullptr);
      break;

    default:
      break;
    }
  };
  esp_bt_hid_device_register_callback(hidInitCallback);

  esp_bt_hid_device_init();

  //   esp_spp_register_callback(
  //       [](esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
  //       {
  //       switch (event)
  //       {
  //       case ESP_SPP_INIT_EVT:
  //       {
  //         esp_spp_start_srv(ESP_SPP_SEC_NONE,ESP_SPP_ROLE_SLAVE,0,"");
  //         break;
  //         }
  //       case ESP_SPP_DATA_IND_EVT:
  //       {

  //         char* data = new char[param->data_ind.len+1]{0};
  //         std::copy(param->data_ind.data,param->data_ind.data+param->data_ind.len,data);
  //         ESP_LOGI("BLT","data: %s",data);
  //         delete[] data;
  //         break;
  //       }

  //       default:
  //         break;
  // } });

  //   esp_spp_enhanced_init(new esp_spp_cfg_t{ESP_SPP_MODE_CB, false, 0});

  esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 4, new esp_bt_pin_code_t{'1', '2', '3', '4'});

  esp_console_repl_config_t replConfig = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  esp_console_dev_uart_config_t uartConfig = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  esp_console_repl_t *repl = nullptr;
  esp_console_new_repl_uart(&uartConfig, &replConfig, &repl);
  esp_console_start_repl(repl);

  esp_console_cmd_t typeCMD;
  typeCMD.command = "type"; // type textksdjfbskdsdfsdftextksdjfbskdsdfsdftextksdjfbskdsdfsdftextksdjfbskdsdfsdftextksdjfbskdsdfsdftextksdjfbskdsdfsdftextksdjfbskdsdfsdf
  typeCMD.func = [](int argc, char **argv)
  {
    auto type = [](uint8_t c)
    {
      esp_hidd_status_t status;
      uint8_t data = c - 'a' + 0x04; // b
    retry1:
      esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0, 1, &data);
      xMessageBufferReceive(typeMessage, &status, sizeof(esp_hidd_status_t), portMAX_DELAY);
      if (status != ESP_HIDD_SUCCESS)
        goto retry1;
      data = 0x0; // nhả ra
    retry2:
      esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0, 1, &data);
      xMessageBufferReceive(typeMessage, &status, sizeof(esp_hidd_status_t), portMAX_DELAY);
      if (status != ESP_HIDD_SUCCESS)
        goto retry2;
    };
     uint16_t i = 0;
    while (1)
    {
      uint8_t c = argv[1][i];
      if (c == 0)
        break;
      type(c);
      i++;

    }

    return 0;
  };
  typeCMD.help = "go ban phim";
  typeCMD.argtable = nullptr;

  esp_console_cmd_register(&typeCMD);
}

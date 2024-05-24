#include "remote_command.h"
#include "esp_log.h"
NECFrame powerButtonFrame;
MessageBufferHandle_t remoteMessage;
nvs_handle_t remoteNVS;

void startRemoteCommand(httpd_handle_t webserver)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(nvs_open("remote", NVS_READWRITE, &remoteNVS));

  size_t length;
  nvs_get_blob(remoteNVS, "powerButton", nullptr, &length);
  nvs_get_blob(remoteNVS, "powerButton", &powerButtonFrame, &length);

  remoteMessage = xMessageBufferCreate(sizeof(NECFrame) + sizeof(size_t));
  xMessageBufferReset(remoteMessage);

  setOnReceiveData(
      [](NECFrame frame)
      {
        ESP_LOGI("__remote", "frame: %04x %04x", frame.address, frame.command);
        if (frame == powerButtonFrame)
        {
          if (getBrightness() != 255)
            setBrightness(255);
          else
            setBrightness(0);
        };
        xMessageBufferSend(remoteMessage, &frame, sizeof(NECFrame), 0);
      });
  startRMT();
  startLed(webserver);

  httpd_uri_t powerUri = {};
  powerUri.uri = "/setPowerButton";
  powerUri.method = HTTP_GET;
  powerUri.handler = [](httpd_req_t *req)
  {
    xMessageBufferReset(remoteMessage);
    NECFrame frame;
    ESP_LOGI("__remote", "bat dau");

    xMessageBufferReceive(remoteMessage, &frame, sizeof(NECFrame), portMAX_DELAY);
    ESP_LOGI("__remote", "ket thuc");

    char tmp[9];
    sprintf(tmp, "%04x%04x", frame.address, frame.command);
    ESP_ERROR_CHECK(nvs_set_blob(remoteNVS, "powerButton", &frame, sizeof(NECFrame)));
    powerButtonFrame = frame;
    xMessageBufferReset(remoteMessage);
    httpd_resp_send(req, tmp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
  };
  httpd_register_uri_handler(webserver, &powerUri);
};
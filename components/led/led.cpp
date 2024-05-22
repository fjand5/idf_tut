#include <stdio.h>
#include "cJSON.h"
#include "led.h"

void setBrightness(uint8_t brightness)
{

  ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness, 2000);
  ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);
};
uint8_t getBrightness()
{
  return ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
};

void startLed(httpd_handle_t webserver)
{
  ledc_timer_config_t timerConfig = {};
  timerConfig.clk_cfg = LEDC_AUTO_CLK;
  timerConfig.duty_resolution = LEDC_TIMER_8_BIT;
  timerConfig.freq_hz = 1000;
  timerConfig.speed_mode = LEDC_LOW_SPEED_MODE;
  timerConfig.timer_num = LEDC_TIMER_0;
  ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));

  ledc_channel_config_t ledCConfig = {};
  ledCConfig.channel = LEDC_CHANNEL_0;
  ledCConfig.duty = 0;
  ledCConfig.gpio_num = 4;
  ledCConfig.speed_mode = LEDC_LOW_SPEED_MODE;
  ledCConfig.timer_sel = LEDC_TIMER_0;
  ESP_ERROR_CHECK(ledc_channel_config(&ledCConfig));
  ESP_ERROR_CHECK(ledc_fade_func_install(ESP_INTR_FLAG_LEVEL1));
  httpd_uri_t controlLed = {};
  controlLed.uri = "/controlLed";
  controlLed.method = HTTP_POST;
  controlLed.handler = [](httpd_req_t *req)
  {
    size_t contentLenght = req->content_len + 1;
    char *buf = new char[contentLenght]{0};
    httpd_req_recv(req, buf, contentLenght);
    cJSON *root = cJSON_Parse(buf);

    cJSON *dutyItem = cJSON_GetObjectItem(root, "duty");
    uint8_t duty = cJSON_GetNumberValue(dutyItem);

    // ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    // ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 2000);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);

    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);

    delete[] buf;
    cJSON_free(root);
    return ESP_OK;
  };
  if (webserver)
    httpd_register_uri_handler(webserver, &controlLed);
}

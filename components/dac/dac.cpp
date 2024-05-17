#include <stdio.h>
#include "cJSON.h"
#include "driver/dac_oneshot.h"
#include "dac.h"

void startDAC(httpd_handle_t webserver)
{
  dac_oneshot_config_t oneshotConfig = {};
  oneshotConfig.chan_id = DAC_CHAN_0;
  static dac_oneshot_handle_t oneshotHandle;
  dac_oneshot_new_channel(&oneshotConfig, &oneshotHandle);
  httpd_uri_t controlDAC = {};
  controlDAC.uri = "/controlDAC";
  controlDAC.method = HTTP_POST;
  controlDAC.user_ctx = &oneshotHandle;
  controlDAC.handler = [](httpd_req_t *req)
  {
    dac_oneshot_handle_t *oneshotHandle = (dac_oneshot_handle_t *)req->user_ctx;
    size_t contentLenght = req->content_len + 1;
    char *buf = new char[contentLenght]{0};
    httpd_req_recv(req, buf, contentLenght);
    cJSON *root = cJSON_Parse(buf);

    cJSON *dacItem = cJSON_GetObjectItem(root, "dac");
    uint8_t dac = cJSON_GetNumberValue(dacItem);
    dac_oneshot_output_voltage(*oneshotHandle, dac);
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);

    delete[] buf;
    cJSON_free(root);
    return ESP_OK;
  };
  httpd_register_uri_handler(webserver, &controlDAC);
}

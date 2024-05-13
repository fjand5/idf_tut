#include <stdio.h>
#include "esp_ota_ops.h"
#include "update.h"
#include "esp_log.h"
#include "string"

#define BUFFER_SIZE 1024
void updateBinary(httpd_req_t *req, esp_ota_handle_t otaHandle)
{
  size_t contentLenght = req->content_len;
  size_t remain = contentLenght;
  char *buf = new char[BUFFER_SIZE];

  while (remain > 0)
  {

    size_t ret = httpd_req_recv(req, buf, remain > BUFFER_SIZE ? BUFFER_SIZE : remain);
    remain -= ret;
    esp_ota_write(otaHandle, buf, ret);
  }
  delete[] buf;
}
void updateForm(httpd_req_t *req, esp_ota_handle_t otaHandle)
{
  size_t contentLenght = req->content_len;
  size_t remain = contentLenght;
  char *buf = new char[BUFFER_SIZE];

  std::string boundary;
  while (remain > 0)
  {
    char tmp;
    size_t ret = httpd_req_recv(req, &tmp, 1);
    remain -= ret;
    boundary += tmp;
    if (boundary.ends_with("\r\n"))
    {
      boundary = boundary.substr(0, boundary.size() - 2);
      break;
    }
  }
  ESP_LOGI("__UPDATE","boundary: %s",boundary.c_str());
  std::string dataDesc;
  while (remain > 0)
  {
    char tmp;
    size_t ret = httpd_req_recv(req, &tmp, 1);
    remain -= ret;
    dataDesc += tmp;
    if (dataDesc.ends_with("\r\n\r\n"))
    {
      dataDesc = dataDesc.substr(0, dataDesc.size() - 4);
      break;
    }
  }
  ESP_LOGI("__UPDATE","dataDesc: %s",dataDesc.c_str());

  while (remain > 0)
  {

    size_t ret = httpd_req_recv(req, buf, remain > BUFFER_SIZE ? BUFFER_SIZE : remain);
    remain -= ret;
    esp_ota_write(otaHandle, buf, ret);
  }
  delete[] buf;
}
void startUpdate(httpd_handle_t webserver)
{
  httpd_uri_t updateUri = {};
  updateUri.uri = "/update";
  updateUri.method = HTTP_POST;
  updateUri.handler = [](httpd_req_t *req)
  {
    const esp_partition_t *currentPartition = esp_ota_get_running_partition();
    const esp_partition_t *updatePartition;
    if (currentPartition->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_1)
    {
      updatePartition = esp_partition_find_first(
          ESP_PARTITION_TYPE_APP,
          ESP_PARTITION_SUBTYPE_APP_OTA_0,
          "ota_0");
    }
    else
    {
      updatePartition = esp_partition_find_first(
          ESP_PARTITION_TYPE_APP,
          ESP_PARTITION_SUBTYPE_APP_OTA_1,
          "ota_1");
    }
    if (!updatePartition)
    {

      ESP_LOGI("__UPDATE", "no have  updatePartition");
      httpd_resp_send_500(req);
      return ESP_OK;
    }
    esp_ota_handle_t otaHandle;
    esp_err_t err = esp_ota_begin(updatePartition, OTA_SIZE_UNKNOWN, &otaHandle);
    if (err != ESP_OK)
    {
      ESP_LOGI("__UPDATE", "esp_ota_begin err: %d", err);
      httpd_resp_send_500(req);
      return ESP_OK;
    };

    size_t contentTypeLength = httpd_req_get_hdr_value_len(req, "content-type");
    char *contentType = new char[contentTypeLength + 1]{0};
    httpd_req_get_hdr_value_str(req, "content-type", contentType, contentTypeLength + 1);
    if (strcmp(contentType, "application/octet-stream") == 0)
    {
      updateBinary(req, otaHandle);
    }
    else
    {
      updateForm(req, otaHandle);
    }
    delete[] contentType;
    if (esp_ota_end(otaHandle) == ESP_OK)
    {
      esp_ota_set_boot_partition(updatePartition);
      httpd_resp_send(req, "ok", HTTPD_RESP_USE_STRLEN);

      vTaskDelay(pdMS_TO_TICKS(555));
      esp_restart();
    }
    else
    {
      ESP_LOGI("__UPDATE", "esp_ota_end err");

      httpd_resp_send_500(req);
    }
    return ESP_OK;

    // FILE *file = fopen("/store/file.txt", "w");

    // size_t contentLenght = req->content_len;
    // size_t remain = contentLenght;
    // char *buf = new char[BUFFER_SIZE];

    // while (remain > 0)
    // {

    //   size_t ret = httpd_req_recv(req, buf, remain > BUFFER_SIZE ? BUFFER_SIZE : remain);
    //   remain -= ret;
    //   fwrite(buf, 1, ret, file);
    // }

    // httpd_resp_send(req, "ok", HTTPD_RESP_USE_STRLEN);
    // fclose(file);
    // delete[] buf;
    return ESP_OK;
  };
  httpd_register_uri_handler(webserver, &updateUri);
}

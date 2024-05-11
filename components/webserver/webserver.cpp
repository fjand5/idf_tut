#include <stdio.h>
#include "webserver.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#define BUFFER_SIZE 1024
void startWebserver(void)
{
    esp_vfs_spiffs_conf_t conf = {};
    conf.base_path = "/store";
    conf.format_if_mount_failed = true;
    conf.max_files = 1;
    conf.partition_label = NULL;
    esp_vfs_spiffs_register(&conf);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t handle;

    httpd_uri_t homeUri = {};
    homeUri.uri = "/";
    homeUri.method = HTTP_GET;
    homeUri.handler = [](httpd_req_t *req)
    {
        extern const char index_html_start[] asm("_binary_index_html_start");
        extern const char index_html_end[] asm("_binary_index_html_end");
        uint32_t length = index_html_end - index_html_start;
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, index_html_start, length);
        return ESP_OK;
    };

    httpd_uri_t queryUri = {};
    queryUri.uri = "/query";
    queryUri.method = HTTP_GET;
    queryUri.handler = [](httpd_req_t *req)
    {
        if (nvs_flash_init() != ESP_OK)
        {
            httpd_resp_send_500(req);
            return ESP_OK;
        }
        nvs_handle_t webserverNVS;
        if (nvs_open("webserver", NVS_READWRITE, &webserverNVS) != ESP_OK)
        {
            httpd_resp_send_500(req);
            return ESP_OK;
        }
        size_t length;
        nvs_get_str(webserverNVS, "body", NULL, &length);

        char *buf = new char[length]{0};
        nvs_get_str(webserverNVS, "body", buf, &length);

        httpd_resp_send(req, buf, length);
        return ESP_OK;

        // size_t queryLength = httpd_req_get_url_query_len(req) + 1;
        // if (queryLength > 0)
        // {
        //     char *buf = new char[queryLength]{0};
        //     if (httpd_req_get_url_query_str(req, buf, queryLength) == ESP_OK)
        //     {
        //         char *val = new char[64 + 1]{0};
        //         if (httpd_query_key_value(buf, "key", val, 64 + 1) == ESP_OK)
        //         {
        //             ESP_LOGI("__WEBSERVER", "key: %s", val);
        //         };
        //         delete[] val;
        //     };
        //     delete[] buf;
        // }
        // httpd_resp_send(req, "query page", HTTPD_RESP_USE_STRLEN);
        // return ESP_OK;
    };

    httpd_uri_t bodyUri = {};
    bodyUri.uri = "/body";
    bodyUri.method = HTTP_POST;
    bodyUri.handler = [](httpd_req_t *req)
    {
        if (nvs_flash_init() != ESP_OK)
        {
            httpd_resp_send_500(req);
            return ESP_OK;
        }
        nvs_handle_t webserverNVS;
        if (nvs_open("webserver", NVS_READWRITE, &webserverNVS) != ESP_OK)
        {
            httpd_resp_send_500(req);
            return ESP_OK;
        }

        size_t contentLenght = req->content_len + 1;
        char *buf = new char[contentLenght]{0};
        httpd_req_recv(req, buf, contentLenght);

        if (nvs_set_str(webserverNVS, "body", buf) != ESP_OK)
        {
            httpd_resp_send_500(req);
            return ESP_OK;
        }
        if (nvs_commit(webserverNVS))
        {
            nvs_commit(webserverNVS);
            return ESP_OK;
        }

        ESP_LOGI("__WEBSERVER", "buf: %s", buf);
        httpd_resp_send(req, "ok", HTTPD_RESP_USE_STRLEN);

        return ESP_OK;
    };

    httpd_uri_t readUri = {};
    readUri.uri = "/read";
    readUri.method = HTTP_GET;
    readUri.handler = [](httpd_req_t *req)
    {
        FILE *file = fopen("/store/file.txt", "r");

        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);

        size_t remain = size;
        char *buf = new char[BUFFER_SIZE];

        httpd_resp_set_type(req,"image/jpeg");
        while (remain > 0)
        {
            size_t ret = fread(buf, 1, remain > BUFFER_SIZE ? BUFFER_SIZE : remain, file);
            remain -= ret;
            httpd_resp_send_chunk(req, buf, ret);
        }

        httpd_resp_send_chunk(req, nullptr, 0);
        fclose(file);
        delete[] buf;
        return ESP_OK;
    };
    httpd_uri_t writeUri = {};
    writeUri.uri = "/write";
    writeUri.method = HTTP_POST;
    writeUri.handler = [](httpd_req_t *req)
    {
        FILE *file = fopen("/store/file.txt", "w");

        size_t contentLenght = req->content_len;
        size_t remain = contentLenght;
        char *buf = new char[BUFFER_SIZE];

        while (remain > 0)
        {

            size_t ret = httpd_req_recv(req, buf, remain > BUFFER_SIZE ? BUFFER_SIZE : remain);
            remain -= ret;
            fwrite(buf, 1, ret, file);
        }

        httpd_resp_send(req, "ok", HTTPD_RESP_USE_STRLEN);
        fclose(file);
        delete[] buf;
        return ESP_OK;
    };
    if (httpd_start(&handle, &config) == ESP_OK)
    {
        httpd_register_uri_handler(handle, &homeUri);
        httpd_register_uri_handler(handle, &queryUri);
        httpd_register_uri_handler(handle, &bodyUri);
        httpd_register_uri_handler(handle, &readUri);
        httpd_register_uri_handler(handle, &writeUri);
    };
}

#include <stdio.h>
#include "webserver.h"
#include "esp_log.h"

void startWebserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t handle;

    httpd_uri_t homeUri = {};
    homeUri.uri = "/";
    homeUri.method = HTTP_GET;
    homeUri.handler = [](httpd_req_t *req)
    {
        httpd_resp_send(req, "home page", strlen("home page"));
        return ESP_OK;
    };

    httpd_uri_t queryUri = {};
    queryUri.uri = "/query";
    queryUri.method = HTTP_GET;
    queryUri.handler = [](httpd_req_t *req)
    {
        size_t queryLength = httpd_req_get_url_query_len(req) + 1;
        if (queryLength > 0)
        {
            char *buf = new char[queryLength]{0};
            if (httpd_req_get_url_query_str(req, buf, queryLength) == ESP_OK)
            {
                char *val = new char[64 + 1]{0};
                if (httpd_query_key_value(buf, "key", val, 64 + 1) == ESP_OK)
                {
                    ESP_LOGI("__WEBSERVER", "key: %s", val);
                };
            };
        }
        httpd_resp_send(req, "query page", strlen("query page"));
        return ESP_OK;
    };
    if (httpd_start(&handle, &config) == ESP_OK)
    {
        httpd_register_uri_handler(handle, &homeUri);
        httpd_register_uri_handler(handle, &queryUri);
    };
}

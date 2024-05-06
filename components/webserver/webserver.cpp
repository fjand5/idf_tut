#include <stdio.h>
#include "webserver.h"

void startWebserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t handle;

    httpd_uri_t homeUri = {};
    homeUri.uri = "/";
    homeUri.method = HTTP_GET;
    homeUri.handler = [](httpd_req_t *req)
    {
        httpd_resp_send(req,"home page",strlen("home page"));
        return ESP_OK;
    };
    if (httpd_start(&handle, &config) == ESP_OK)
    {
        httpd_register_uri_handler(handle, &homeUri);
    };
}

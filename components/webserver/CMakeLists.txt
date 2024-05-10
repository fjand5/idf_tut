idf_component_register(SRCS "webserver.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_server nvs_flash spiffs
                    EMBED_FILES index.html
                    )

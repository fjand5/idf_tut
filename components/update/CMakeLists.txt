idf_component_register(SRCS "update.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_server esp_http_client app_update
                    )

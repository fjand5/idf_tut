idf_component_register(SRCS "webserver.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_server
                    EMBED_FILES index.html
                    )

idf_component_register(
    SRCS
        "src/camera_stream.c"
        "src/camera.c"
        "src/route_stream.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "src"
    PRIV_REQUIRES
        esp_http_server
        esp_timer
)

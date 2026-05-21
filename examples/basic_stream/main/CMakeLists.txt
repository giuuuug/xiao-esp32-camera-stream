idf_component_register(
    SRCS "main.c"
    PRIV_REQUIRES camera-stream esp_wifi esp_event esp_netif nvs_flash
)

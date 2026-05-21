#include "camera_stream.h"
#include "camera.h"
#include "route_stream.h"
#include "sdkconfig.h"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "camera_stream";

static httpd_handle_t s_server = NULL;

esp_err_t camera_stream_init(void)
{
    return camera_init();
}

esp_err_t camera_stream_start(void)
{
    if (s_server)
    {
        ESP_LOGW(TAG, "Server already running");
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = CONFIG_CMSTE_SERVER_PORT;
    config.lru_purge_enable = true;
    config.stack_size = 8192;

    esp_err_t ret = httpd_start(&s_server, &config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_ERROR_CHECK(route_stream_register(s_server));

    ESP_LOGI(TAG, "HTTP server started on port %d", CONFIG_CMSTE_SERVER_PORT);
    ESP_LOGI(TAG, "  GET /stream");
    return ESP_OK;
}

esp_err_t camera_stream_stop(void)
{
    if (!s_server)
        return ESP_OK;

    esp_err_t ret = httpd_stop(s_server);
    if (ret == ESP_OK)
    {
        s_server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
    }
    return ret;
}

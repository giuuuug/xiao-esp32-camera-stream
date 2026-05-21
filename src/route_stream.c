#include "route_stream.h"
#include "camera.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>

static const char *TAG = "stream";

#define BOUNDARY           "gc0p4Jq0M2Yt08jU534c0p"
#define CONTENT_TYPE       "multipart/x-mixed-replace;boundary=" BOUNDARY
/* Each MJPEG part starts with this header; Content-Length lets the browser
 * know the exact frame size without needing a closing boundary first. */
#define PART_HEADER        "\r\n--" BOUNDARY "\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n"
#define PART_HEADER_MAXLEN 128

static esp_err_t stream_handler(httpd_req_t *req)
{
    esp_err_t res = httpd_resp_set_type(req, CONTENT_TYPE);
    if (res != ESP_OK) return res;

    /* Prevent the HTTP server from closing the socket after its send timeout;
     * the stream is intentionally infinite. */
    httpd_resp_set_hdr(req, "Connection", "keep-alive");

    char    part_header[PART_HEADER_MAXLEN];
    int64_t last_frame_us = 0;

    while (true) {
        camera_fb_t *fb = camera_capture();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }

        int64_t now = esp_timer_get_time();
        ESP_LOGD(TAG, "Frame %zu B  dt=%lld ms", fb->len, (now - last_frame_us) / 1000);
        last_frame_us = now;

        size_t header_len = snprintf(part_header, sizeof(part_header), PART_HEADER, fb->len);
        res = httpd_resp_send_chunk(req, part_header, header_len);

        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);

        camera_fb_return(fb);

        if (res != ESP_OK) {
            /* Normal on browser tab close or network drop — not an error. */
            ESP_LOGI(TAG, "Client disconnected");
            break;
        }
    }

    /* Zero-length chunk signals end of chunked transfer to the HTTP layer. */
    httpd_resp_send_chunk(req, NULL, 0);
    return res;
}

static const httpd_uri_t uri = {
    .uri     = "/stream",
    .method  = HTTP_GET,
    .handler = stream_handler,
};

esp_err_t route_stream_register(httpd_handle_t server)
{
    return httpd_register_uri_handler(server, &uri);
}

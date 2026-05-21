#pragma once

#include "esp_err.h"

/**
 * @file camera_stream.h
 * @brief Public API for the camera-stream component.
 *
 * Typical usage:
 *   1. camera_stream_init()   — power up the camera
 *   2. <connect to WiFi>
 *   3. camera_stream_start()  — start serving GET /stream
 */

/**
 * @brief Initialise the camera hardware.
 *
 * Reads pin mapping and image settings from menuconfig
 * (Camera Stream → Board profile / Image quality / Image orientation).
 * Must be called once before camera_stream_start().
 *
 * @return ESP_OK on success, or an esp_camera error code on failure.
 */
esp_err_t camera_stream_init(void);

/**
 * @brief Start the HTTP server and begin streaming.
 *
 * Exposes a single route:
 *   GET /stream  — MJPEG live video (multipart/x-mixed-replace)
 *
 * The network interface must be up before calling this function.
 * Calling it a second time while the server is already running is a no-op.
 *
 * @return ESP_OK on success, or an esp_httpd error code on failure.
 */
esp_err_t camera_stream_start(void);

/**
 * @brief Stop the HTTP server gracefully.
 *
 * Active client connections are closed. The camera hardware remains
 * initialised; call camera_stream_start() again to resume.
 *
 * @return ESP_OK on success, or an esp_httpd error code on failure.
 */
esp_err_t camera_stream_stop(void);

#pragma once

/**
 * @file route_stream.h
 * @brief HTTP route: GET /stream (MJPEG over HTTP).
 *
 * Private to the component; registered by camera_stream_start().
 */

#include "esp_http_server.h"

/** Register the /stream route on an already-started httpd server. */
esp_err_t route_stream_register(httpd_handle_t server);

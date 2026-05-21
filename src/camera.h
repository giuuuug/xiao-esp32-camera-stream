#pragma once

/**
 * @file camera.h
 * @brief Internal camera HAL — thin wrapper around espressif/esp32-camera.
 *
 * Pin mapping and image settings are resolved at compile time from the
 * board profile selected in menuconfig (CMSTE_BOARD_* symbols).
 * This header is private to the component; consumers use camera_stream.h.
 */

#include "esp_err.h"
#include "esp_camera.h"

/** Initialise esp_camera with the board-specific pin mapping and settings. */
esp_err_t camera_init(void);

/** Capture one JPEG frame. Must call camera_fb_return() when done. */
camera_fb_t *camera_capture(void);

/** Return a frame buffer to the driver so it can be reused. */
void camera_fb_return(camera_fb_t *fb);

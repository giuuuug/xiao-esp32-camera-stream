#include "camera.h"
#include "sdkconfig.h"
#include "esp_log.h"

static const char *TAG = "camera";

/* ── Pin mapping per board profile ─────────────────────────────────────────── */

#if defined(CONFIG_CMSTE_BOARD_XIAO_ESP32S3_SENSE)
    #define PIN_PWDN   -1
    #define PIN_RESET  -1
    #define PIN_XCLK   10
    #define PIN_SIOD   40
    #define PIN_SIOC   39
    #define PIN_D7     48
    #define PIN_D6     11
    #define PIN_D5     12
    #define PIN_D4     14
    #define PIN_D3     16
    #define PIN_D2     18
    #define PIN_D1     17
    #define PIN_D0     15
    #define PIN_VSYNC  38
    #define PIN_HREF   47
    #define PIN_PCLK   13

#elif defined(CONFIG_CMSTE_BOARD_AITHINKER_ESP32_CAM)
    #define PIN_PWDN   32
    #define PIN_RESET  -1
    #define PIN_XCLK    0
    #define PIN_SIOD   26
    #define PIN_SIOC   27
    #define PIN_D7     35
    #define PIN_D6     34
    #define PIN_D5     39
    #define PIN_D4     36
    #define PIN_D3     21
    #define PIN_D2     19
    #define PIN_D1     18
    #define PIN_D0      5
    #define PIN_VSYNC  25
    #define PIN_HREF   23
    #define PIN_PCLK   22

#elif defined(CONFIG_CMSTE_BOARD_WROVER_KIT)
    #define PIN_PWDN   -1
    #define PIN_RESET  -1
    #define PIN_XCLK   21
    #define PIN_SIOD   26
    #define PIN_SIOC   27
    #define PIN_D7     35
    #define PIN_D6     34
    #define PIN_D5     39
    #define PIN_D4     36
    #define PIN_D3     19
    #define PIN_D2     18
    #define PIN_D1      5
    #define PIN_D0      4
    #define PIN_VSYNC  25
    #define PIN_HREF   23
    #define PIN_PCLK   22

#else /* CMSTE_BOARD_CUSTOM — pins come from menuconfig */
    #define PIN_PWDN   CONFIG_CMSTE_PIN_PWDN
    #define PIN_RESET  CONFIG_CMSTE_PIN_RESET
    #define PIN_XCLK   CONFIG_CMSTE_PIN_XCLK
    #define PIN_SIOD   CONFIG_CMSTE_PIN_SIOD
    #define PIN_SIOC   CONFIG_CMSTE_PIN_SIOC
    #define PIN_D7     CONFIG_CMSTE_PIN_Y9
    #define PIN_D6     CONFIG_CMSTE_PIN_Y8
    #define PIN_D5     CONFIG_CMSTE_PIN_Y7
    #define PIN_D4     CONFIG_CMSTE_PIN_Y6
    #define PIN_D3     CONFIG_CMSTE_PIN_Y5
    #define PIN_D2     CONFIG_CMSTE_PIN_Y4
    #define PIN_D1     CONFIG_CMSTE_PIN_Y3
    #define PIN_D0     CONFIG_CMSTE_PIN_Y2
    #define PIN_VSYNC  CONFIG_CMSTE_PIN_VSYNC
    #define PIN_HREF   CONFIG_CMSTE_PIN_HREF
    #define PIN_PCLK   CONFIG_CMSTE_PIN_PCLK
#endif

/* ── Init ───────────────────────────────────────────────────────────────────── */

esp_err_t camera_init(void)
{
    camera_config_t config = {
        .pin_pwdn     = PIN_PWDN,
        .pin_reset    = PIN_RESET,
        .pin_xclk     = PIN_XCLK,
        .pin_sccb_sda = PIN_SIOD,
        .pin_sccb_scl = PIN_SIOC,

        .pin_d7 = PIN_D7,
        .pin_d6 = PIN_D6,
        .pin_d5 = PIN_D5,
        .pin_d4 = PIN_D4,
        .pin_d3 = PIN_D3,
        .pin_d2 = PIN_D2,
        .pin_d1 = PIN_D1,
        .pin_d0 = PIN_D0,

        .pin_vsync = PIN_VSYNC,
        .pin_href  = PIN_HREF,
        .pin_pclk  = PIN_PCLK,

        .xclk_freq_hz = CONFIG_CMSTE_XCLK_FREQ_HZ,
        /* LEDC timer/channel 0 are unused by any other peripheral on camera boards */
        .ledc_timer   = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,
        .frame_size   = (framesize_t)CONFIG_CMSTE_FRAME_SIZE,
        .jpeg_quality = CONFIG_CMSTE_JPEG_QUALITY,
        .fb_count     = CONFIG_CMSTE_FB_COUNT,
        .fb_location  = CONFIG_CMSTE_FB_IN_PSRAM ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM,
        /* GRAB_WHEN_EMPTY avoids serving a stale frame when the client is slow */
        .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
    };

    esp_err_t ret = esp_camera_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Orientation correction — applied after init because set_vflip/set_hmirror
     * require the sensor to be powered and its registers accessible. */
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor) {
#if CONFIG_CMSTE_VFLIP
        sensor->set_vflip(sensor, 1);
#endif
#if CONFIG_CMSTE_HMIRROR
        sensor->set_hmirror(sensor, 1);
#endif
    }

    return ESP_OK;
}

camera_fb_t *camera_capture(void)
{
    return esp_camera_fb_get();
}

void camera_fb_return(camera_fb_t *fb)
{
    esp_camera_fb_return(fb);
}

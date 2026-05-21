# xiao-esp32-camera-stream

An ESP-IDF component that exposes a live MJPEG camera stream over HTTP with minimal boilerplate.

## Why this component?

The [XIAO ESP32S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) by Seeed Studio is a compact, powerful board with an integrated camera connector (OV2640 / OV3660). Setting it up for live video streaming requires wiring together three independent parts — the camera driver, the HTTP server, and the WiFi stack — with a lot of trial and error on pin configuration, PSRAM settings, and chunked HTTP encoding.

This component packages all of that into three function calls. The default board profile targets the XIAO ESP32S3 Sense, but built-in profiles are provided for other popular boards, and every pin is manually overridable via `menuconfig`.

## Features

- `GET /stream` — MJPEG live stream. Open it in any browser or VLC, no plugin needed.
- **Board profiles** in menuconfig — zero pin configuration for supported boards.
- Sensor-agnostic: OV2640, OV3660, and any sensor supported by `esp_camera` work without code changes.
- Tested on ESP32S3. Compatible with ESP32, ESP32S2, ESP32P4.

## Requirements

- ESP-IDF **v6.0.1** or later (FreeRTOS-based — no other RTOS supported)
- A board with a camera module (OV2640, OV3660, or compatible)
- PSRAM recommended for VGA or higher resolution (the XIAO ESP32S3 Sense has 8 MB PSRAM)

## Development environment

This component was developed and tested with:

- **VS Code** + [ESP-IDF Extension](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) (recommended setup)
- ESP-IDF v6.0.1 toolchain

The extension handles toolchain installation, `idf.py` commands, flashing, and serial monitoring from within VS Code. See the [ESP-IDF Extension docs](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md) to get started.

## Installation

### Option A — ESP Component Registry (recommended)

Run this command from your project root:

```bash
idf.py add-dependency "giuuuug/xiao-esp32-camera-stream^1.0.0"
```

This adds the dependency to your `idf_component.yml` and downloads the component automatically on the next build.

### Option B — Copy into your project

Clone or download this repo and place the `camera-stream` folder inside your project's `components/` directory:

```
your_project/
  components/
    camera-stream/   ← here
  main/
  CMakeLists.txt
```

No additional `CMakeLists.txt` changes needed — ESP-IDF discovers components in `components/` automatically.

### Option C — External path (shared across projects)

In your top-level `CMakeLists.txt`, before `include(...)`:

```cmake
set(EXTRA_COMPONENT_DIRS "/path/to/camera-stream")
```

## Usage

```c
#include "camera_stream.h"

void app_main(void)
{
    // 1. Init the camera hardware (reads board profile from menuconfig)
    ESP_ERROR_CHECK(camera_stream_init());

    // 2. Connect to WiFi (or start your AP) — your responsibility
    //    This component does NOT manage WiFi.
    my_wifi_connect();

    // 3. Start the HTTP server
    ESP_ERROR_CHECK(camera_stream_start());

    // Open http://<device-ip>/stream in your browser
}
```

> **Note:** WiFi initialization is intentionally outside this component.
> Use the ESP-IDF `esp_wifi` API directly, or pair it with a WiFi manager component.
> A complete working example including WiFi is available in [`examples/basic_stream`](examples/basic_stream).

## Configuration (menuconfig)

Run `idf.py menuconfig` and navigate to **Camera Stream (component)**.

### Board profile

Select your board — pin mapping is applied automatically with no further configuration needed.

| Profile | Board |
|---|---|
| **XIAO ESP32S3 Sense** *(default)* | Seeed Studio XIAO ESP32S3 Sense |
| **AI-Thinker ESP32-CAM** | AI-Thinker ESP32-CAM module |
| **ESP-WROVER-KIT v4.1** | Espressif official devkit |
| **Custom** | Any board — configure pins manually |

### Image quality

| Kconfig symbol | Default | Description |
|---|---|---|
| `CMSTE_SERVER_PORT` | `80` | HTTP server port |
| `CMSTE_XCLK_FREQ_HZ` | `20000000` | XCLK frequency in Hz |
| `CMSTE_FRAME_SIZE` | `8` (VGA) | Resolution — see `framesize_t` in `sensor.h` |
| `CMSTE_JPEG_QUALITY` | `12` | 0 = best quality, 63 = worst |
| `CMSTE_FB_COUNT` | `2` | Frame buffer count (2 recommended for smooth streaming) |
| `CMSTE_FB_IN_PSRAM` | enabled | Store frame buffers in PSRAM — disable only if your board has no PSRAM |

### Image orientation

| Kconfig symbol | Default | Description |
|---|---|---|
| `CMSTE_VFLIP` | enabled | Flip image vertically |
| `CMSTE_HMIRROR` | disabled | Mirror image horizontally |

Both enabled together give a 180° rotation — useful when the camera module is mounted upside-down (default on the XIAO ESP32S3 Sense). These can also be set in `sdkconfig.defaults` without going through menuconfig:

```
CONFIG_CMSTE_VFLIP=y
CONFIG_CMSTE_HMIRROR=n
```

### Built-in pin mappings

<details>
<summary>XIAO ESP32S3 Sense (default)</summary>

| Signal | GPIO |
|---|---|
| XCLK | 10 |
| SDA | 40 |
| SCL | 39 |
| D0–D7 | 15, 17, 18, 16, 14, 12, 11, 48 |
| VSYNC | 38 |
| HREF | 47 |
| PCLK | 13 |

</details>

<details>
<summary>AI-Thinker ESP32-CAM</summary>

| Signal | GPIO |
|---|---|
| PWDN | 32 |
| XCLK | 0 |
| SDA | 26 |
| SCL | 27 |
| D0–D7 | 5, 18, 19, 21, 36, 39, 34, 35 |
| VSYNC | 25 |
| HREF | 23 |
| PCLK | 22 |

</details>

<details>
<summary>ESP-WROVER-KIT v4.1</summary>

| Signal | GPIO |
|---|---|
| XCLK | 21 |
| SDA | 26 |
| SCL | 27 |
| D0–D7 | 4, 5, 18, 19, 36, 39, 34, 35 |
| VSYNC | 25 |
| HREF | 23 |
| PCLK | 22 |

</details>

### Custom board

Select **Custom** in the board profile and configure each pin individually in menuconfig.

## API

```c
// Initialize camera hardware. Call once before start().
esp_err_t camera_stream_init(void);

// Start the HTTP server. Exposes GET /stream.
esp_err_t camera_stream_start(void);

// Stop the HTTP server. Camera stays initialized.
esp_err_t camera_stream_stop(void);
```

## How the stream works

`GET /stream` uses **MJPEG over HTTP** with chunked transfer encoding:

```
HTTP/1.1 200 OK
Content-Type: multipart/x-mixed-replace; boundary=<boundary>

--<boundary>
Content-Type: image/jpeg
Content-Length: <size>

<raw JPEG frame>
--<boundary>
...
```

The connection stays open indefinitely. The server captures a frame, sends it, and immediately captures the next one. The client renders each frame as it arrives — no buffering, no latency.

Compatible with: Chrome, Firefox, Safari, VLC, ffmpeg, and any HTTP client that supports multipart responses.

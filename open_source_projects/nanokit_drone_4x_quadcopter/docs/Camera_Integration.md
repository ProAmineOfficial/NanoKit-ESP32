# Camera Integration - Arducam Mega 5 MP

**Developed by Amine Saoud ibn al-Bashir.**

## Architecture Decision

The Arducam Mega 5 MP SPI camera is a payload and observation system. It is not part of the flight safety loop. This repository therefore uses two roles:

| Role | Hardware | Network responsibility | Flight authority |
|---|---|---|---|
| Flight controller | NanoKit Integrated ESP32 + MPU6050 + ESCs | BLE control/telemetry | Full, with local failsafes |
| Camera node | Separate ESP32 + Arducam Mega 5 MP | Wi-Fi image/MJPEG endpoint | None |

This separation is deliberate. The compiled NanoKit flight firmware already consumes significant flash with BLE and safety features. Camera image buffers and Wi-Fi streaming must not be allowed to delay the 250 Hz attitude loop or the command-timeout failsafe.

## Arducam Mega SPI Wiring

| Arducam Mega pin | ESP32 camera-node GPIO | NanoKit pin when a second NanoKit is used |
|---|---:|---:|
| VCC | 3.3 V | 13 or 14 |
| GND | GND | 12 or 31 |
| SCK | GPIO18 | 38 |
| MISO | GPIO19 | 16 |
| MOSI | GPIO23 | 24 |
| CS | GPIO17 | 39 |

The mapping follows the supplied Arducam ESP32 reference. GPIO18/19/23 are ESP32 VSPI signals and GPIO17 is a dedicated chip-select output. Keep SPI wires short and power the camera node from a regulated 3.3 V rail.

## Camera Software Boundary

1. Start with the upstream [Arducam Mega library](https://github.com/ArduCAM/Arducam_Mega), which Arducam documents for Arduino ESP32 platforms.
2. Run the upstream camera bring-up example on the dedicated node before integrating it with the drone.
3. Expose a local Wi-Fi endpoint such as `http://nanokit-camera.local/stream` that returns MJPEG or a periodically refreshed JPEG.
4. In the Flight Deck, open the camera settings button, enter that endpoint, and select **Load Camera**.
5. Keep camera node Wi-Fi failures independent: loss of video must never disarm or re-arm the flight controller.

## Security And Range

Use a protected private Wi-Fi network for the camera stream. Treat camera video as observational only; do not use it as the sole means of control or as a replacement for visual line of sight. The Flight Deck has no authenticated camera-stream protocol in this initial reference, so do not expose the endpoint to the public internet.

## Attribution

The camera module's protocol and library remain upstream Arducam work. This repository documents a separate integration architecture and does not copy vendor images or proprietary application code. See the [Arducam Mega Quick Start Guide](https://docs.arducam.com/Arduino-SPI-camera/MEGA-SPI/Quick-Start-Guide/) and the upstream repository before reusing code.

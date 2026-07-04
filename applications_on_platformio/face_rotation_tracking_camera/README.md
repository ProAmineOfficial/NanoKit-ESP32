# Face Rotation Tracking Camera

A real-world NanoKit Integrated ESP32 application for rotating a pan/tilt camera platform based on face coordinates produced by a companion vision process.

## Goal

Combine a vision-side face detector with ESP32 firmware that receives `FACE x y confidence` messages and moves pan/tilt servos toward the detected face.

## System Parts

| Folder | Purpose |
|---|---|
| `esp32/` | PlatformIO firmware for servo control and tracking status |
| `vision/` | Companion computer vision experiments and model notes |
| `docs/` | Application architecture, wiring, testing, and PDF-ready documentation |
| `images/` | Diagram sources and image assets |
| `assets/` | Shared runtime or documentation assets |

## Firmware Commands

The ESP32 firmware expects one message per line:

```text
FACE 160 120 0.90
NOFACE
```

`FACE x y confidence` moves the camera when confidence is high enough. `NOFACE` holds position and disables active tracking status.

## NanoKit Pin Mapping

| NanoKit Pin | GPIO | Component Pin | Direction | Reason | Electrical Notes |
|---:|---|---|---|---|---|
| 37 | GPIO26 | Pan servo signal | Output | PWM-capable general GPIO | Signal only; servo needs external 5 V |
| 3 | GPIO25 | Tilt servo signal | Output | PWM-capable general GPIO | Signal only; servo needs external 5 V |
| 17 | GPIO2 | Status LED | Output | Onboard LED / safe output | Indicates active tracking |
| 12 or 31 | GND | Servo supply GND | Power | Common reference | Required for stable PWM control |

## Build and Upload

```bash
cd esp32
pio run
pio run --target upload
pio device monitor -b 115200
```

## Documentation

- [ESP32 Firmware Wiring](esp32/docs/Wiring.md)
- [ESP32 Firmware Implementation](esp32/docs/Implementation_Guide.md)
- [Application Architecture](docs/README.md)
- [Connection Diagram Source](images/connection-diagram.md)

## Safety Notes

Do not power servos directly from NanoKit GPIO pins. Use a separate servo supply and connect grounds together.

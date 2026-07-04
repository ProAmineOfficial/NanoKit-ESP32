# Camera Tracking

A complete PlatformIO example that demonstrates pan/tilt tracking logic using two servo PWM outputs and simple target coordinates.

## Goal

Receive target coordinates over Serial, calculate error from the image center, and move pan/tilt servos smoothly toward the target.

## What You Will Learn

- How computer vision coordinates become servo movement.
- How ESP32 LEDC PWM can drive hobby servo control signals.
- Why target error is multiplied by a small gain instead of jumping directly.
- How to test tracking firmware without a real camera by using simulated target data.

## Hardware Required

| Item | Quantity | Notes |
|---|---:|---|
| NanoKit Integrated ESP32 | 1 | Main controller board |
| Pan servo | 1 | Horizontal camera movement |
| Tilt servo | 1 | Vertical camera movement |
| External 5 V supply | 1 | Recommended for servo power |
| Jumper wires | 5+ | Two signals, power, and ground |

## NanoKit Pin Mapping

| NanoKit Pin | GPIO | Component Pin | Direction | Reason | Electrical Notes |
|---:|---|---|---|---|---|
| 17 | GPIO2 | Pan servo signal | Output | PWM-capable and visible onboard LED feedback may also show activity | Servo signal only |
| 37 | GPIO26 | Tilt servo signal | Output | PWM-capable general GPIO | Servo signal only |
| 12 or 31 | GND | Servo GND | Power | Common reference | Must share ground with servo supply |

## Serial Input Format

```text
X=160 Y=120
```

`X=160 Y=120` is the center of the example 320x240 frame. Larger or smaller coordinates move the simulated camera.

## Build and Upload

```bash
pio run
pio run --target upload
pio device monitor -b 115200
```

## Documentation

- [Wiring Guide](docs/Wiring.md)
- [Implementation Guide](docs/Implementation_Guide.md)
- [Connection Diagram Source](images/connection-diagram.md)

## Safety Notes

Use an external 5 V supply for servos. Do not power multiple servos from the ESP32 3.3 V pin.

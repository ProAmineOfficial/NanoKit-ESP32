# Wiring Guide - Face Rotation Tracking Camera Firmware

## Connection Table

| NanoKit Pin | GPIO | Device Pin | Description |
|---:|---|---|---|
| 37 | GPIO26 | Pan servo signal | Horizontal camera rotation PWM |
| 3 | GPIO25 | Tilt servo signal | Vertical camera rotation PWM |
| 17 | GPIO2 | Onboard LED | Tracking status output |
| 12 or 31 | GND | Servo GND | Common ground with servo supply |
| External 5 V | - | Servo VCC | Servo power supply |

## Electrical Notes

Servo current must not come from ESP32 GPIO pins. Use an external servo supply and connect the ground of that supply to NanoKit GND.

## Vision Link

The companion vision process sends one line over Serial:

```text
FACE 160 120 0.90
NOFACE
```

The firmware only moves when confidence is high enough.

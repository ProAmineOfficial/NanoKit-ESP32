# Wiring Guide - Face Rotation Tracking Camera Firmware

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table


<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 37 | GPIO26 | 0x8A | Pan servo | Signal | Output | Confirmed | PWM output for horizontal camera rotation |
| 3 | GPIO25 | 0x71 | Tilt servo | Signal | Output | Confirmed | PWM output for vertical camera rotation |
| 17 | GPIO2 | 0xC0 | Onboard status LED | LED | Output | Confirmed | Onboard LED indicates active tracking |
| 12 or 31 | GND | 0xD0 | Pan servo | GND | Power | Confirmed | Common ground with the servo supply |
| 12 or 31 | GND | 0xD0 | Tilt servo | GND | Power | Confirmed | Common ground with the servo supply |
| - | External | - | Pan servo | VCC | Power | Confirmed | External 5 V servo supply |
| - | External | - | Tilt servo | VCC | Power | Confirmed | External 5 V servo supply |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../../nanokit.yaml`](../../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

Servo current must not come from ESP32 GPIO pins. Use an external servo supply and connect the ground of that supply to NanoKit GND.

## Vision Link

The companion vision process sends one line over Serial:

```text
FACE 160 120 0.90
NOFACE
```

The firmware only moves when confidence is high enough.

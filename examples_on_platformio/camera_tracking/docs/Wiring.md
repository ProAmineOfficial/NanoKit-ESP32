# Wiring Guide - Camera Tracking

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table


<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 17 | GPIO2 | 0xC0 | Pan servo | Signal | Output | Confirmed | PWM-capable output; the onboard LED also shows activity |
| 37 | GPIO26 | 0x8A | Tilt servo | Signal | Output | Confirmed | PWM-capable general GPIO |
| 12 or 31 | GND | 0xD0 | Pan servo | GND | Power | Confirmed | Common ground with the servo supply |
| 12 or 31 | GND | 0xD0 | Tilt servo | GND | Power | Confirmed | Common ground with the servo supply |
| - | External | - | Pan servo | VCC | Power | Confirmed | External 5 V servo supply |
| - | External | - | Tilt servo | VCC | Power | Confirmed | External 5 V servo supply |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

Servo signal lines can be driven from ESP32 GPIO, but servo power should come from a separate 5 V supply. Always connect NanoKit GND and servo supply GND together.

## Wiring Order

1. Connect grounds together first.
2. Connect pan signal to NanoKit Pin 17 / GPIO2.
3. Connect tilt signal to NanoKit Pin 37 / GPIO26.
4. Power servos from an external 5 V supply.
5. Upload firmware and test with Serial commands such as `X=160 Y=120`.

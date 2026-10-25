# Wiring Guide - Stepper Motor Control

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table


<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 3 | GPIO25 | 0x71 | ULN2003 driver board | IN1 | Output | Confirmed | First coil control input, general GPIO safe output |
| 37 | GPIO26 | 0x8A | ULN2003 driver board | IN2 | Output | Confirmed | Second coil control input, general GPIO safe output |
| 19 | GPIO27 | 0x82 | ULN2003 driver board | IN3 | Output | Confirmed | Third coil control input, general GPIO safe output |
| 7 | GPIO32 | 0x54 | ULN2003 driver board | IN4 | Output | Confirmed | Fourth coil control input, general GPIO safe output |
| 12 or 31 | GND | 0xD0 | ULN2003 driver board | GND | Power | Confirmed | Common ground between NanoKit and the driver |
| - | External | - | ULN2003 driver board | VCC | Power | Confirmed | 5 V motor supply for the driver |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

GPIO pins do not power motor coils. They only send logic signals to the ULN2003 driver. Use an external motor supply and connect grounds together.

## Wiring Order

1. Connect NanoKit GND to driver GND.
2. Connect driver VCC to a suitable 5 V motor supply.
3. Connect GPIO25, GPIO26, GPIO27, and GPIO32 to IN1-IN4.
4. Connect the stepper motor cable to the driver module.
5. Upload firmware and open Serial Monitor at 115200 baud.

# Wiring Guide - Ultrasonic Distance

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 36 | GPIO5 | 0x17 | HC-SR04 ultrasonic sensor | TRIG | Output | Confirmed | Safe digital output for the 10 us trigger pulse |
| 2 | GPIO34 | 0x56 | HC-SR04 ultrasonic sensor | ECHO | Input | Confirmed | Input-only pin is ideal for the echo pulse |
| 12 or 31 | GND | 0xD0 | HC-SR04 ultrasonic sensor | GND | Power | Confirmed | Common ground reference, required for correct timing |
| 13 or 14 | 3V3 | - | HC-SR04 ultrasonic sensor | VCC | Power | Confirmed | Preferred for 3.3 V compatible modules |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

GPIO34 is input-only, which matches the echo signal perfectly. If the ultrasonic module outputs 5 V on ECHO, add a voltage divider or logic level shifter before GPIO34.

## Wiring Order

1. Connect GND first so both boards share a reference.
2. Connect VCC to 3.3 V for compatible modules.
3. Connect NanoKit Pin 36 / GPIO5 to TRIG.
4. Connect NanoKit Pin 2 / GPIO34 to ECHO through level shifting if required.
5. Upload the firmware and open Serial Monitor at 115200 baud.

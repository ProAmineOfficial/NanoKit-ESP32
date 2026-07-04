# Wiring Guide - Ultrasonic Distance

## Connection Table

| NanoKit Pin | GPIO | Sensor Pin | Description |
|---:|---|---|---|
| 36 | GPIO5 | TRIG | ESP32 output sends a 10 us trigger pulse |
| 2 | GPIO34 | ECHO | ESP32 input measures echo pulse duration |
| 12 or 31 | GND | GND | Shared ground reference |
| 13 or 14 | 3V3 | VCC | Power for 3.3 V compatible ultrasonic modules |

## Electrical Notes

GPIO34 is input-only, which matches the echo signal perfectly. If the ultrasonic module outputs 5 V on ECHO, add a voltage divider or logic level shifter before GPIO34.

## Wiring Order

1. Connect GND first so both boards share a reference.
2. Connect VCC to 3.3 V for compatible modules.
3. Connect NanoKit Pin 36 / GPIO5 to TRIG.
4. Connect NanoKit Pin 2 / GPIO34 to ECHO through level shifting if required.
5. Upload the firmware and open Serial Monitor at 115200 baud.

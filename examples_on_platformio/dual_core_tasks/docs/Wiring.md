# Wiring Guide - Dual Core Tasks

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 37 | GPIO26 | 0x8A | External LED + 220 ohm resistor | Anode via 220 ohm | Output | Confirmed | Safe general-purpose output toggled by the core 0 task |
| 17 | GPIO2 | 0xC0 | Onboard LED | LED | Output | Confirmed | Onboard LED on the PWM_OUT0 pin toggled by the core 1 task |
| 12 or 31 | GND | 0xD0 | External LED + 220 ohm resistor | Cathode | Power | Confirmed | Return path for the LED current |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- GPIO26 (NanoKit Pin 37) sources the external LED current. A 220 ohm resistor with a red LED gives about (3.3 V - 1.9 V) / 220 ohm = 6.4 mA, far below the 12 mA recommended per ESP32 pin.
- GPIO2 (NanoKit Pin 17) already has the onboard LED and its resistor on the board; nothing is wired to it. It is a minor strapping pin, which is why only an LED or a light signal load belongs there.
- Which core toggles a pin makes no electrical difference: both cores share the same GPIO matrix. The two LEDs are only there to make the two independent rhythms visible.
- Never bring 5 V (NanoKit Pin 11 or Pin 32) to any GPIO.

## Wiring Order

1. Connect the LED anode (long leg) to one end of the 220 ohm resistor and the other end of the resistor to NanoKit Pin 37 / GPIO26.
2. Connect the LED cathode (short leg, flat side) to NanoKit Pin 31 / GND.
3. Plug in USB, upload the firmware, and open the Serial Monitor at 115200 baud.
4. The external LED blinks slowly (1 s on, 1 s off) and the onboard LED blinks fast (300 ms); the Serial Monitor shows which core runs which task.

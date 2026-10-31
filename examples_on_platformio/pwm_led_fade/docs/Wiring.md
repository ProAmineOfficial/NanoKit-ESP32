# Wiring Guide - PWM LED Fade

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 37 | GPIO26 | 0x8A | External LED + 220 ohm resistor | Anode via 220 ohm | Output | Confirmed | LEDC channel 0 output sets the LED brightness |
| 17 | GPIO2 | 0xC0 | Onboard LED | LED | Output | Confirmed | Onboard LED on the PWM_OUT0 pin |
| 12 or 31 | GND | 0xD0 | External LED + 220 ohm resistor | Cathode | Power | Confirmed | Return path for the LED current |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- GPIO26 (NanoKit Pin 37) is a safe general-purpose output. The ESP32 GPIO matrix can route any of the 16 LEDC channels to almost any output-capable pin, so there is no dedicated "PWM pin" to look for.
- PWM switches the LED fully on and fully off. During each "on" slice the current is the same as with a plain `digitalWrite(HIGH)`: with a red LED and 220 ohm, about (3.3 V - 1.9 V) / 220 ohm = 6.4 mA, far below the 12 mA recommended per ESP32 pin. Dimming lowers the *average* current, never the peak, so the resistor is always required.
- GPIO2 (NanoKit Pin 17) already has the onboard LED and its resistor on the board; nothing is wired to it.
- Never bring 5 V (NanoKit Pin 11 or Pin 32) to any GPIO.

## Wiring Order

1. Connect the LED anode (long leg) to one end of the 220 ohm resistor and the other end of the resistor to NanoKit Pin 37 / GPIO26.
2. Connect the LED cathode (short leg, flat side) to NanoKit Pin 31 / GND.
3. Plug in USB, upload the firmware, and open the Serial Monitor at 115200 baud.
4. Watch the external LED and the onboard LED fade up and down together.

# Wiring Guide - Digital Input Output

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 10 | GPIO4 | 0x81 | Push button | Leg 1 | Input | Confirmed | Internal pull-up keeps the pin HIGH; pressing reads LOW |
| 37 | GPIO26 | 0x8A | External LED + 220 ohm resistor | Anode via 220 ohm | Output | Confirmed | Safe general-purpose output drives the LED HIGH = on |
| 17 | GPIO2 | 0xC0 | Onboard LED | LED | Output | Confirmed | Onboard LED on the PWM_OUT0 pin |
| 12 or 31 | GND | 0xD0 | Push button | Leg 2 | Power | Confirmed | Pressing connects the GPIO to ground |
| 12 or 31 | GND | 0xD0 | External LED + 220 ohm resistor | Cathode | Power | Confirmed | Return path for the LED current |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- GPIO4 (NanoKit Pin 10) is a general GPIO with an internal pull-up of roughly 45 kOhm, enabled by `INPUT_PULLUP`. The button therefore needs no external resistor.
- GPIO26 (NanoKit Pin 37) sources the LED current. A 220 ohm resistor with a red LED gives about (3.3 V - 1.9 V) / 220 ohm = 6.4 mA, far below the 12 mA recommended per ESP32 pin.
- GPIO2 (NanoKit Pin 17) already has the onboard LED and its resistor on the board; nothing is wired to it.
- Keep the button wired to GND only. Never bring 5 V (NanoKit Pin 11 or Pin 32) to any GPIO.

## Wiring Order

1. Connect one button leg to NanoKit Pin 10 / GPIO4 and the other leg to NanoKit Pin 12 / GND.
2. Connect the LED anode (long leg) to one end of the 220 ohm resistor and the other end of the resistor to NanoKit Pin 37 / GPIO26.
3. Connect the LED cathode (short leg, flat side) to NanoKit Pin 31 / GND.
4. Plug in USB, upload the firmware, and open the Serial Monitor at 115200 baud.
5. Press the button: the external LED and the onboard LED toggle together.

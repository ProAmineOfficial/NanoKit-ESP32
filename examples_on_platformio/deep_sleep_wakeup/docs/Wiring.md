# Wiring Guide - Deep Sleep Wakeup

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 20 | GPIO33 | 0x83 | Wake button + 10 kOhm pull-down | Button leg 1 | Input | Confirmed | RTC GPIO used as the ext0 wake-up source; the pull-down keeps it LOW, pressing drives it HIGH |
| 17 | GPIO2 | 0xC0 | Onboard LED | LED | Output | Confirmed | Onboard LED on the PWM_OUT0 pin |
| 12 or 31 | GND | 0xD0 | Wake button + 10 kOhm pull-down | Pull-down resistor | Power | Confirmed | 10 kOhm from the GPIO to ground so the pin never floats |
| 13 or 14 | 3V3 | - | Wake button + 10 kOhm pull-down | Button leg 2 | Power | Confirmed | Pressing connects 3.3 V to the GPIO (never 5 V) |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- GPIO33 (NanoKit Pin 20) belongs to the RTC domain, the part of the chip that stays powered in deep sleep. The ext0 wake-up source watches this pin while everything else is off, which is why a non-RTC GPIO such as GPIO18 (NanoKit Pin 38) could not do this job.
- The button sits between 3.3 V (NanoKit Pin 13 or 14) and GPIO33. The 10 kOhm resistor from GPIO33 to GND (NanoKit Pin 12 or 31) defines the level while the button is open: about 0.33 mA flows through it only while the button is pressed, which is negligible.
- An external pull-down is used instead of the internal one because the normal GPIO pull-ups and pull-downs are part of the digital domain and are switched off in deep sleep. A pin without a defined level can wake the chip at random.
- GPIO2 (NanoKit Pin 17) already has the onboard LED and its resistor on the board; nothing is wired to it.
- Never bring 5 V (NanoKit Pin 11 or Pin 32) to GPIO33 or to any other GPIO. The ESP32 is not 5 V tolerant.

## Wiring Order

1. Connect one button leg to NanoKit Pin 20 / GPIO33 and the other leg to NanoKit Pin 13 / 3V3.
2. Connect one end of the 10 kOhm resistor to NanoKit Pin 20 / GPIO33 (same row as the button leg) and the other end to NanoKit Pin 12 / GND.
3. Plug in USB, upload the firmware, and open the Serial Monitor at 115200 baud.
4. Check the boot line "Wake button ... reads LOW (released, pull-down OK)". If it reads HIGH with the button released, the resistor is not connected.
5. Wait 10 s: the chip wakes on the timer, blinks three times, and prints a higher boot number.
6. Press the button during the sleep period: the chip wakes immediately and reports the ext0 cause.

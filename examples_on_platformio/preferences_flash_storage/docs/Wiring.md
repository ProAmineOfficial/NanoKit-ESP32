# Wiring Guide - Preferences Flash Storage

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 10 | GPIO4 | 0x81 | Push button | Leg 1 | Input | Confirmed | Internal pull-up keeps the pin HIGH; pressing reads LOW |
| 17 | GPIO2 | 0xC0 | Onboard LED | LED | Output | Confirmed | Onboard LED on the PWM_OUT0 pin shows the saved state |
| 12 or 31 | GND | 0xD0 | Push button | Leg 2 | Power | Confirmed | Pressing connects the GPIO to ground |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- GPIO4 (NanoKit Pin 10) is a general GPIO with an internal pull-up of roughly 45 kOhm, enabled by `INPUT_PULLUP`. The button therefore needs no external resistor.
- GPIO2 (NanoKit Pin 17) already has the onboard LED and its resistor on the board; nothing is wired to it.
- The NVS data lives in the flash chip that is already on the NanoKit module (the same chip that holds the firmware), so no external memory is needed. The flash pins GPIO6 to GPIO11 (NanoKit Pin 21, Pin 22, Pin 27 to Pin 30) must stay untouched; wiring anything to them stops the board from running.
- Keep the button wired to GND only. Never bring 5 V (NanoKit Pin 11 or Pin 32) to any GPIO.

## Wiring Order

1. Connect one button leg to NanoKit Pin 10 / GPIO4 and the other leg to NanoKit Pin 12 / GND.
2. Plug in USB, upload the firmware, and open the Serial Monitor at 115200 baud.
3. Press the button a few times, then press the board's EN (reset) button: the onboard LED comes back in the state it had before the reset, and the boot counter has grown by one.
4. Unplug USB for a moment and plug it back in: the state survives the power loss as well.

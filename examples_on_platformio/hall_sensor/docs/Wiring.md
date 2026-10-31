# Wiring Guide - Hall Sensor

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 17 | GPIO2 | 0xC0 | Onboard LED | LED | Output | Confirmed | Onboard LED on the PWM_OUT0 pin |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- Nothing is wired in this example. The Hall-effect sensor is inside the ESP32 chip, under the metal shield of the NanoKit-iM module, and the indicator is the onboard LED on NanoKit Pin 17 / GPIO2, which already has its resistor on the board.
- `hallRead()` measures the sensor through ADC1 channel 0 and ADC1 channel 3, which are NanoKit Pin 5 / GPIO36 and NanoKit Pin 4 / GPIO39. These two pins must be left free: a wire, a sensor, or even a finger on them changes the ADC input and corrupts the reading.
- GPIO36 and GPIO39 are input-only pins with no internal pull resistors, so an unconnected pin is exactly what the sensor expects.
- The sensor shares the chip's ADC with everything else that uses it. Do not call `analogRead()` on NanoKit Pin 4 or Pin 5 in the same program.

## Wiring Order

1. Connect nothing to the header. Confirm that NanoKit Pin 4 / GPIO39 and NanoKit Pin 5 / GPIO36 are empty.
2. Plug in USB, upload the firmware, and open the Serial Monitor at 115200 baud. Keep magnets at least 20 cm away until the "Baseline:" line appears.
3. Bring a magnet slowly towards the metal shield of the NanoKit-iM module: the delta grows, the polarity word appears, and the onboard LED on NanoKit Pin 17 / GPIO2 lights.
4. Flip the magnet over and approach again: the delta changes sign.

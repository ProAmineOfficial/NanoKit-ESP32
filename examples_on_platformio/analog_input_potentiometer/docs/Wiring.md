# Wiring Guide - Analog Input Potentiometer

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 2 | GPIO34 | 0x56 | 10 kOhm potentiometer | Wiper (middle leg) | Input | Confirmed | ADC1_CH6 on an input-only pin; 11 dB attenuation covers the full 0-3.3 V range |
| 17 | GPIO2 | 0xC0 | Onboard LED | LED | Output | Confirmed | Onboard LED on the PWM_OUT0 pin |
| 12 or 31 | GND | 0xD0 | 10 kOhm potentiometer | Outer leg B | Power | Confirmed | Bottom of the voltage divider |
| 13 or 14 | 3V3 | - | 10 kOhm potentiometer | Outer leg A | Power | Confirmed | Top of the voltage divider |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- GPIO34 (NanoKit Pin 2) is ADC1 channel 6 and is input-only: it has no output driver and no internal pull-up or pull-down. That is fine here because the potentiometer always drives the pin to a defined voltage; the pin never floats.
- The potentiometer draws 3.3 V / 10 kOhm = 0.33 mA from the 3V3 rail through its track, regardless of the knob position. The wiper itself feeds a very high-impedance ADC input, so almost no current flows into GPIO34.
- The ESP32 ADC with 11 dB attenuation accepts inputs up to about 3.3 V. Espressif documents the accurate span for this setting as roughly 150 mV to 2450 mV; outside it the reading still moves but the curve bends, so the first few percent read as 0 and the top saturates before 3.3 V.
- Power the potentiometer only from 3V3 (NanoKit Pin 13 or Pin 14). Never from 5 V (NanoKit Pin 11 or Pin 32): the wiper would then exceed the 3.3 V GPIO limit.
- GPIO2 (NanoKit Pin 17) already has the onboard LED and its resistor on the board; nothing is wired to it.

## Wiring Order

1. Connect one outer leg of the potentiometer to NanoKit Pin 13 / 3V3.
2. Connect the other outer leg to NanoKit Pin 12 / GND.
3. Connect the middle leg (wiper) to NanoKit Pin 2 / GPIO34.
4. Plug in USB, upload the firmware, and open the Serial Monitor at 115200 baud.
5. Turn the knob slowly from one end to the other: the bar fills, the millivolt value climbs toward 3.3 V, and the onboard LED brightens.

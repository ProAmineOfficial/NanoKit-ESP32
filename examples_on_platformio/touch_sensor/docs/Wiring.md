# Wiring Guide - Touch Sensor

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 10 | GPIO4 | 0x81 | Touch pad (jumper wire / foil) | Pad wire | Input | Confirmed | Touch channel T0; the count drops when a finger adds capacitance to the wire |
| 37 | GPIO26 | 0x8A | External LED + 220 ohm resistor | Anode via 220 ohm | Output | Confirmed | Safe general-purpose output lights the LED while the pad is touched |
| 12 or 31 | GND | 0xD0 | External LED + 220 ohm resistor | Cathode | Power | Confirmed | Return path for the LED current |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- GPIO4 (NanoKit Pin 10) is touch channel T0. The touch block charges and discharges the pin repeatedly and counts how many cycles fit into a fixed window; the wire plugged into the pin is the whole sensor. Nothing else may be connected to NanoKit Pin 10 while this example runs.
- A longer wire or a piece of foil increases the pad capacitance and lowers the untouched reading. That is why the baseline is measured at start-up instead of being written into the code.
- GPIO26 (NanoKit Pin 37) sources the LED current. A 220 ohm resistor with a red LED gives about (3.3 V - 1.9 V) / 220 ohm = 6.4 mA, far below the 12 mA recommended per ESP32 pin.
- Keep the touch wire away from the 5 V rail (NanoKit Pin 11 or Pin 32) and from the 3.3 V rail. A touch pin is an input; a wire that accidentally touches a supply rail would force a voltage into it.

## Wiring Order

1. Plug one end of a jumper wire into NanoKit Pin 10 / GPIO4 and leave the other end in the air. Optionally wrap a small piece of aluminium foil around the free end to make a pad.
2. Connect the LED anode (long leg) to one end of the 220 ohm resistor and the other end of the resistor to NanoKit Pin 37 / GPIO26.
3. Connect the LED cathode (short leg, flat side) to NanoKit Pin 31 / GND.
4. Plug in USB, upload the firmware, and open the Serial Monitor at 115200 baud. Keep your hands away from the wire until the "Baseline:" line appears.
5. Touch the free end of the wire or the foil: the LED lights and the status line shows TOUCHED.

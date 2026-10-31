# Wiring Guide - I2C Scanner

**Developed by Amine Saoud ibn al-Bashir.**

## Connection Table

<!-- GENIUS:CONNECTION_TABLE:START -->
| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |
|---:|---|---|---|---|---|---|---|
| 23 | GPIO21 | 0x30 | I2C bus i2c0 | SDA | Bidirectional | Confirmed | NanoKit I2C0 data line; open-drain, so it needs a pull-up to 3.3 V |
| 18 | GPIO22 | 0x31 | I2C bus i2c0 | SCL | Bidirectional | Confirmed | NanoKit I2C0 clock line; open-drain, so it needs a pull-up to 3.3 V |
| 12 or 31 | GND | 0xD0 | Any 3.3 V I2C module (OLED, BME280, MPU-6050, ...) | GND | Power | Proposed | Common ground with NanoKit |
| 13 or 14 | 3V3 | - | Any 3.3 V I2C module (OLED, BME280, MPU-6050, ...) | VCC | Power | Proposed | Power the module from the 3.3 V rail only |
<!-- GENIUS:CONNECTION_TABLE:END -->

Generated from [`../nanokit.yaml`](../nanokit.yaml) and `registry/nanokit-esp32.yaml` by `python tools/genius/generate.py`.

## Electrical Notes

- GPIO21 (NanoKit Pin 23) and GPIO22 (NanoKit Pin 18) are the board's I2C0 pins and the Arduino core defaults, so any I2C library example written for "ESP32 SDA 21 / SCL 22" works on NanoKit without changes.
- I2C lines are open-drain: a device can only pull a line LOW, never drive it HIGH. A pull-up resistor to 3.3 V is what returns the line to HIGH. Almost every breakout module carries its own 4.7 kOhm to 10 kOhm pull-ups; if yours does not (check the board for two small resistors near SDA/SCL, or the schematic), add one 4.7 kOhm resistor from SDA to 3V3 and one from SCL to 3V3.
- The ESP32 also enables weak internal pull-ups (roughly 45 kOhm) when `Wire.begin` is called. They are enough for a very short wire to a single module on the bench, but not a substitute for real pull-ups in a finished project.
- Several modules can share the bus as long as their addresses differ. Each module adds its own pull-ups in parallel; with four or more modules the combined pull-up can become too strong (below about 1 kOhm), so remove the pull-ups from all but one module in a large setup.
- Keep the wires short (under 20 cm on a breadboard). Long wires add capacitance, which rounds the signal edges; the 100 kHz clock used here is the most tolerant of that.
- A sensor or small OLED draws a few tens of milliamps at most, which the 3V3 rail provides comfortably; how much current remains for other loads depends on the board regulator and the USB source.
- Never bring 5 V (NanoKit Pin 11 or Pin 32) to SDA, SCL, or any other GPIO.

## Wiring Order

1. With USB unplugged, connect the module GND to NanoKit Pin 12 / GND.
2. Connect the module VCC (sometimes labelled VIN or 3V3) to NanoKit Pin 13 / 3V3.
3. Connect the module SDA to NanoKit Pin 23 / GPIO21.
4. Connect the module SCL to NanoKit Pin 18 / GPIO22.
5. Check the four wires once more, especially that VCC and GND are not swapped, then plug in USB.
6. Upload the firmware and open the Serial Monitor at 115200 baud. The first scan runs immediately and repeats every 5 s.

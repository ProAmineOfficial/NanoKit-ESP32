# Wiring Guide - Stepper Motor Control

## Connection Table

| NanoKit Pin | GPIO | Driver Pin | Description |
|---:|---|---|---|
| 3 | GPIO25 | IN1 | First coil control input |
| 37 | GPIO26 | IN2 | Second coil control input |
| 19 | GPIO27 | IN3 | Third coil control input |
| 7 | GPIO32 | IN4 | Fourth coil control input |
| 12 or 31 | GND | GND | Common ground between NanoKit and driver |
| External 5 V | - | VCC | Motor power input for the driver |

## Electrical Notes

GPIO pins do not power motor coils. They only send logic signals to the ULN2003 driver. Use an external motor supply and connect grounds together.

## Wiring Order

1. Connect NanoKit GND to driver GND.
2. Connect driver VCC to a suitable 5 V motor supply.
3. Connect GPIO25, GPIO26, GPIO27, and GPIO32 to IN1-IN4.
4. Connect the stepper motor cable to the driver module.
5. Upload firmware and open Serial Monitor at 115200 baud.

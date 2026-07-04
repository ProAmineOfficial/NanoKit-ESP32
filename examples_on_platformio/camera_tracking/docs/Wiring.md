# Wiring Guide - Camera Tracking

## Connection Table

| NanoKit Pin | GPIO | Device Pin | Description |
|---:|---|---|---|
| 17 | GPIO2 | Pan servo signal | Horizontal servo PWM |
| 37 | GPIO26 | Tilt servo signal | Vertical servo PWM |
| 12 or 31 | GND | Servo GND | Common ground with servo supply |
| External 5 V | - | Servo VCC | Servo power supply |

## Electrical Notes

Servo signal lines can be driven from ESP32 GPIO, but servo power should come from a separate 5 V supply. Always connect NanoKit GND and servo supply GND together.

## Wiring Order

1. Connect grounds together first.
2. Connect pan signal to NanoKit Pin 17 / GPIO2.
3. Connect tilt signal to NanoKit Pin 37 / GPIO26.
4. Power servos from an external 5 V supply.
5. Upload firmware and test with Serial commands such as `X=160 Y=120`.

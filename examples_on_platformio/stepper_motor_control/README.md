# Stepper Motor Control

A complete PlatformIO example for controlling a small four-wire stepper motor through a ULN2003-style driver board.

## Goal

Drive a stepper motor with a half-step sequence and control direction, speed, and movement length from the Serial Monitor.

## What You Will Learn

- How a stepper motor advances through coil activation patterns.
- Why a driver board is required between ESP32 GPIO and motor coils.
- How non-blocking timing keeps Serial commands responsive.
- How NanoKit pins map to GPIO outputs for motor control.

## Hardware Required

| Item | Quantity | Notes |
|---|---:|---|
| NanoKit Integrated ESP32 | 1 | Main controller board |
| 28BYJ-48 stepper motor | 1 | Common educational stepper motor |
| ULN2003 driver module | 1 | Required current driver for the motor coils |
| External 5 V supply | 1 | Recommended for the motor driver |
| Jumper wires | 6+ | Four control lines plus power and ground |

## NanoKit Pin Mapping

| NanoKit Pin | GPIO | Component Pin | Direction | Reason | Electrical Notes |
|---:|---|---|---|---|---|
| 3 | GPIO25 | IN1 | Output | General GPIO, safe output | Logic signal only, not motor power |
| 37 | GPIO26 | IN2 | Output | General GPIO, safe output | Logic signal only, not motor power |
| 19 | GPIO27 | IN3 | Output | General GPIO, safe output | Logic signal only, not motor power |
| 7 | GPIO32 | IN4 | Output | General GPIO, safe output | Logic signal only, not motor power |
| 12 or 31 | GND | Driver GND | Power | Shared reference | Must share ground with external supply |

## Serial Commands

```text
HELP
CW 2048
CCW 2048
SPEED 3
STOP
```

## Build and Upload

```bash
pio run
pio run --target upload
pio device monitor -b 115200
```

## Documentation

- [Wiring Guide](docs/Wiring.md)
- [Implementation Guide](docs/Implementation_Guide.md)
- [Connection Diagram Source](images/connection-diagram.md)

## Safety Notes

Never power the stepper motor directly from ESP32 GPIO pins. GPIO pins only command the driver inputs; the motor current must come from the driver power input.

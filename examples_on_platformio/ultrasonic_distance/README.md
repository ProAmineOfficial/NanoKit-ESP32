# Ultrasonic Distance

A complete PlatformIO example that measures distance with an HC-SR04-style ultrasonic sensor on NanoKit Integrated ESP32.

## Goal

Measure object distance, filter noisy readings, and print stable centimeter values to the PlatformIO Serial Monitor.

## What You Will Learn

- How ultrasonic time-of-flight distance measurement works.
- Why GPIO34 is a good echo input because it is input-only.
- Why GPIO5 is a safe trigger output on NanoKit Pin 36.
- How median filtering improves beginner sensor projects.
- How to avoid blocking forever when a sensor is disconnected.

## Hardware Required

| Item | Quantity | Notes |
|---|---:|---|
| NanoKit Integrated ESP32 | 1 | Main controller board |
| HC-SR04-compatible ultrasonic sensor | 1 | Use a 3.3 V compatible module or level-shift ECHO |
| Jumper wires | 4 | VCC, GND, TRIG, ECHO |
| Breadboard | 1 | Optional but recommended |

## NanoKit Pin Mapping

| NanoKit Pin | GPIO | Component Pin | Direction | Reason | Electrical Notes |
|---:|---|---|---|---|---|
| 36 | GPIO5 | TRIG | Output | Safe digital output for trigger pulses | 3.3 V logic output |
| 2 | GPIO34 | ECHO | Input | Input-only pin is ideal for sensor echo | Level-shift if module outputs 5 V |
| 12 or 31 | GND | GND | Power | Common ground reference | Required for correct timing |
| 13 or 14 | 3V3 | VCC | Power | Preferred for 3.3 V compatible modules | Use 5 V only with safe echo level shifting |

## Build and Upload

```bash
pio run
pio run --target upload
pio device monitor -b 115200
```

## Expected Output

```text
NanoKit ultrasonic distance example ready.
Trigger: NanoKit Pin 36 / GPIO5, Echo: NanoKit Pin 2 / GPIO34.
Distance: 24.8 cm
Distance: 24.7 cm
```

## Documentation

- [Wiring Guide](docs/Wiring.md)
- [Implementation Guide](docs/Implementation_Guide.md)
- [Connection Diagram Source](images/connection-diagram.md)

## Safety Notes

Do not connect a 5 V echo signal directly to GPIO34 unless your sensor module is confirmed 3.3 V safe. Use a voltage divider or level shifter for 5 V echo outputs.

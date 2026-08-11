# NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

NanoKit Drone 4X is an open-source, safety-first Quad-X engineering reference for the NanoKit Integrated ESP32. This revision separates deterministic flight control, the browser Flight Deck, and the optional camera/audio payload into explicit modules.

> **Current state:** bench-development firmware only. Propellers must remain removed. Motor output is locked at the safe minimum because analogue PWM acceptance by the exact 4-in-1 ESC is not yet confirmed.

## System Overview

| System | Responsibility | Authority |
|---|---|---|
| NanoKit flight controller | 250 Hz control loop, safety state, command validation, telemetry, future sensor fusion | Sole motor authority |
| Flight Deck | Wi-Fi/WebSocket pilot commands, safety controls, telemetry display, mission planning UI | Commands only |
| Camera/audio node | Future OV2640, audio, storage, and camera-servo services | No motor authority |

The flight controller creates the `NanoKit-Drone-4X` Wi-Fi access point, serves the Flight Deck from LittleFS at `http://192.168.4.1`, and accepts protocol-v3 commands on WebSocket port `81`.

## Confirmed Interfaces

Only these interfaces are considered confirmed in this revision:

| Function | ESP32 GPIO |
|---|---|
| I2C SDA | GPIO21 |
| I2C SCL | GPIO22 |
| Motor M1 signal | GPIO25 |
| Motor M2 signal | GPIO26 |
| Motor M3 signal | GPIO27 |
| Motor M4 signal | GPIO32 |

No other peripheral pin is assigned. ICM-20948, GNSS, PMW3901, HC-SR04, LEDs, buzzer, camera, audio, microSD, and servo interfaces remain disabled until their wiring and electrical limits are verified.

## Safety Gates

The firmware will not arm unless all mandatory gates are true:

1. A real IMU driver reports healthy, calibrated, valid attitude data.
2. A current Flight Deck command is arriving over Wi-Fi/WebSocket.
3. Throttle is zero and ARM has been released before a new arm attempt.
4. Emergency stop is not latched.
5. `NANOKIT_ESC_ANALOG_PWM_CONFIRMED` is explicitly changed only after the exact ESC accepts 1000-2000 us analogue PWM on a propeller-free bench.

With the repository defaults, the IMU and ESC gates are false. The UI displays unavailable telemetry instead of invented sensor values, and all motor commands remain at 1000 us.

## Project Layout

| Path | Purpose |
|---|---|
| `firmware/` | Modular PlatformIO firmware for the NanoKit flight controller. |
| `web_controller/` | Responsive Ground Control Station served by the flight controller. |
| `camera_node/` | Separate PlatformIO payload node with runtime PSRAM validation. |
| `docs/` | Architecture, safety, protocol, wiring, integration, and test gates. |
| `images/` | Mermaid source diagrams that match the current safety architecture. |
| `assets/` | Project-specific reference assets and notes. |

## Build The Flight Controller

```powershell
cd firmware
pio run
pio run -t buildfs
pio run -t upload
pio run -t uploadfs
pio device monitor
```

The web assets are mapped into LittleFS by `firmware/platformio.ini`. Upload both firmware and filesystem before opening `http://192.168.4.1`.

## Build The Camera/Audio Node

```powershell
cd camera_node
pio run
pio run -t upload
pio device monitor
```

The node refuses camera startup unless 8 MB PSRAM is detected at runtime. Every payload peripheral remains behind a disabled feature flag until its pin map is confirmed.

## Required Verification Before Expansion

- Confirm NanoKit #2 physically contains and detects 8 MB PSRAM before OV2640, audio, and microSD integration.
- Confirm the selected 4-in-1 ESC accepts analogue 1000-2000 us PWM before motor testing.
- Protect HC-SR04 ECHO with a divider or level shifter if the module outputs 5 V.
- Verify BME280 and DPS310 addresses; separate them through TCA9548A when their configured addresses conflict.
- Place each of the six VL53L1CX sensors on a separate TCA9548A channel because they share the same default address.

## Documentation

- [Architecture](docs/Architecture.md)
- [Wi-Fi/WebSocket Protocol](docs/WiFi_WebSocket_Protocol.md)
- [Sensor Integration](docs/Sensor_Integration.md)
- [Camera and Audio Node](docs/Camera_Audio_Node.md)
- [Wiring](docs/Wiring.md)
- [Safety](docs/Safety.md)
- [Calibration](docs/Calibration.md)
- [Testing](docs/Testing.md)
- [PID Tuning](docs/PID_Tuning.md)
- [Bill of Materials](docs/Bill_of_Materials.md)

## Development Credit

**Developed by Amine Saoud ibn al-Bashir.**

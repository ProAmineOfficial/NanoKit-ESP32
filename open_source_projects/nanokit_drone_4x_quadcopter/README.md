# NanoKit Drone 4X (Quadcopter)

An open-source, educational Quad-X flight-controller reference for the NanoKit Integrated ESP32. It is modelled around the compact four-arm drone form in the supplied reference image: four brushless motors, four ESCs, an MPU6050 IMU, a LiPo power system, front lighting, and an optional isolated camera payload.

The project provides readable PlatformIO firmware, a BLE browser controller, PID stabilisation, calibration instructions, wiring diagrams, motor layout, test procedure, and safety documentation. The code is intentionally commented around each decision point so it can be studied and extended.

**Developed by Amine Saoud ibn al-Bashir.**

> **Safety status:** experimental bench test only. Keep every propeller removed until the wiring, sensor calibration, motor order, motor direction, mixer signs, arming logic, and emergency stop have passed the documented tests. This is not a production flight stack and it is not an autonomous-drone project.

## What This Project Teaches

- A Quad-X mixer for four brushless motors and ESCs.
- MPU6050 I2C sensing, level calibration, and a complementary attitude filter.
- Roll and pitch angle PID plus yaw-rate PID control.
- ESP32 BLE GATT commands and telemetry for a web-style controller.
- ESC-safe boot behaviour, explicit arming, BLE disconnect failsafe, command timeout, and IMU fault disarm.
- Practical LiPo, BEC, grounding, motor direction, and PID tuning workflow.

## Project Layout

| Path | Purpose |
|---|---|
| `firmware/` | PlatformIO Arduino ESP32 flight-controller reference firmware. |
| `web_controller/` | Web Bluetooth controller with directional, yaw, throttle, arm, calibration, and stop controls. |
| `camera_node/` | Integration boundary and bring-up notes for the separate Arducam Mega Wi-Fi camera node. |
| `docs/` | Build, wiring, calibration, PID, test, safety, protocol, BOM, and open-source reference material. |
| `images/` | Mermaid sources for the system, wiring, motor layout, and control-loop diagrams. |
| `assets/` | Place for licensed build photographs, rendered diagrams, and report figures. |

## Hardware Required

| Item | Qty. | Recommended role |
|---|---:|---|
| NanoKit Integrated ESP32 | 1 | Flight-controller board. |
| MPU6050 IMU | 1 | Accelerometer and gyroscope over I2C. |
| 2204-2306 class brushless motors | 4 | Matched motors suitable for the selected frame and battery. |
| 20-35 A BLHeli_S / BLHeli_32 ESCs | 4 | One ESC per motor, with a current rating above measured demand. |
| 4S or 3S LiPo battery | 1 | Choose with the motor, propeller, ESC, and frame as one power system. |
| 5 V BEC, 3 A minimum | 1 | Clean power for NanoKit and low-power accessories. |
| Quad-X frame, 180-250 mm class | 1 | Rigid frame with a centred electronics stack. |
| Matched CW/CCW propeller pairs | 2 pairs | Install only after all prop-off tests pass. |
| XT30/XT60 lead, capacitor, and wiring | 1 set | Select connector and wire gauge for the measured current. |
| Optional camera or gimbal | 1 | Power separately and isolate it from the flight controller. |

## Forward Camera Architecture

The central camera viewport in the Flight Deck is intended for an **Arducam Mega 5 MP SPI camera connected to a separate ESP32 Wi-Fi camera node**. The NanoKit flight controller already builds at 88.5% flash usage with BLE and safety logic, so streaming video inside that same controller would reduce timing and memory margin. The camera node publishes an MJPEG/image endpoint over Wi-Fi; the Flight Deck displays that endpoint without giving the camera any motor-control authority.

See [Camera Integration](docs/Camera_Integration.md) and [Camera Wiring](images/camera-wiring.md).

## Motor Order And Rotation

View the aircraft from above with the nose and camera facing forward:

| Motor | Position | Required rotation | ESC GPIO | NanoKit pin |
|---|---|---|---:|---:|
| M1 | Front left | CCW | GPIO25 | 3 |
| M2 | Front right | CW | GPIO26 | 37 |
| M3 | Rear right | CCW | GPIO27 | 19 |
| M4 | Rear left | CW | GPIO32 | 7 |

The motor numbers, rotation, and mixer are a matched set. Verify all three with propellers removed before fitting the correct propeller type.

## Build And Flash

```powershell
cd D:\GitHub\NanoKit-ESP32\open_source_projects\nanokit_drone_4x_quadcopter\firmware
pio run
pio run --target upload
pio device monitor -b 115200
```

## Web Controller

Serve `web_controller/` from a local HTTPS-eligible or localhost server, then use Chrome or Edge on desktop/Android. The controller uses the ESP32's built-in BLE GATT service named `NanoKit-Drone-4X`; standard browser Web Bluetooth does not connect to an HC-05 Classic SPP module.

## Start With These Documents

- [Architecture](docs/Architecture.md)
- [Wiring](docs/Wiring.md)
- [Motor Layout](images/motor-layout.md)
- [Calibration](docs/Calibration.md)
- [PID Tuning](docs/PID_Tuning.md)
- [Testing](docs/Testing.md)
- [Safety](docs/Safety.md)
- [System Diagram](images/system-diagram.md)
- [Connection Diagram](images/connection-diagram.md)
- [Control Loop Diagram](images/control-loop.md)
- [Open-Source References](docs/Open_Source_References.md)
- [Camera Integration](docs/Camera_Integration.md)
- [Camera Wiring Diagram](images/camera-wiring.md)

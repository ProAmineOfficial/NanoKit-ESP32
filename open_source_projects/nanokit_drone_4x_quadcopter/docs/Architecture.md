
# Architecture - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

## Design Intent

This project is a small, manually controlled Quad-X reference platform. Its physical form follows a compact four-arm drone: a centered flight controller and IMU, four brushless power units, an optional isolated front camera, and BLE control from a browser application. The first milestone is a safe restrained bench system, not autonomous flight.

## Control Chain

```mermaid
flowchart LR
  UI(["Browser BLE controller"]) -->|"target packet at 12.5 Hz"| BLE(["ESP32 BLE GATT"])
  BLE --> CMD[/"Validated control command"/]
  IMU("MPU6050") -->|"I2C accel + gyro"| EST[["Complementary attitude filter"]]
  CMD --> PID[["Roll / pitch / yaw-rate PID"]]
  EST --> PID
  PID --> MIX[["Quad-X mixer"]]
  MIX --> M1("M1 FL CCW ESC + motor")
  MIX --> M2("M2 FR CW ESC + motor")
  MIX --> M3("M3 RR CCW ESC + motor")
  MIX --> M4("M4 RL CW ESC + motor")
  EST --> TEL[/"Telemetry"/]
  TEL --> BLE
  BLE --> UI

  classDef sensor fill:#282039,stroke:#b99bff,stroke-width:1.8px,color:#faf6ff
  classDef communication fill:#123237,stroke:#42d4c5,stroke-width:1.8px,color:#f2fffd
  classDef io fill:#15313c,stroke:#43d5ca,stroke-width:1.8px,color:#f3fffd
  classDef module fill:#202945,stroke:#91a8ff,stroke-width:1.8px,color:#f6f7ff
  classDef actuator fill:#37251a,stroke:#f0a560,stroke-width:1.8px,color:#fff8ef
  class UI,BLE communication
  class IMU sensor
  class CMD,TEL io
  class EST,PID,MIX module
  class M1,M2,M3,M4 actuator
  linkStyle default stroke:#7894a5,stroke-width:1.4px
```

## Firmware Responsibilities

| Stage | Responsibility | Safety behaviour |
|---|---|---|
| PWM boot | Configure four ESC outputs at 1000 us. | Motors receive minimum throttle before sensor or BLE startup. |
| IMU | Wake MPU6050, read accel/gyro, and calculate level offsets. | A missing read or invalid calibration disarms immediately. |
| Estimator | Combine gyro integration with accelerometer roll/pitch. | Yaw stays relative because MPU6050 has no magnetometer. |
| BLE | Receive compact commands and publish telemetry. | BLE disconnect disarms and advertising restarts. |
| PID | Convert target minus measured attitude into corrections. | Integrals are clamped and reset on every disarm. |
| Mixer | Apply corrections to the four ESC pulses. | Each pulse is constrained to the defined idle and maximum limits. |
| Failsafe | Watch the command age. | A stale command after 700 ms stops all motors. |

## Required Separation

- The LiPo powers the ESCs directly through a correctly rated distribution path.
- A regulated 5 V BEC powers NanoKit and low-power equipment; do not use NanoKit 3.3 V for motors, ESCs, or camera equipment.
- The MPU6050 is a 3.3 V logic device connected by I2C.
- All signal systems share a common ground with the ESC/BEC power system.
- The optional camera must remain electrically isolated from the flight control loop. It is not a source of arm, throttle, or stabilisation commands.

## Camera Node Boundary

The Arducam Mega 5 MP camera is treated as a separate ESP32 Wi-Fi node. Its SPI bus follows the documented ESP32 mapping, while its image endpoint is rendered in the Flight Deck's camera viewport. This avoids adding camera transport, Wi-Fi streaming, and large buffers to the NanoKit flight controller, which is already close to its usable flash capacity with BLE safety firmware.

```mermaid
flowchart LR
  Camera("Arducam Mega 5 MP") -->|"SPI"| CameraEsp(["ESP32 camera node"])
  CameraEsp -->|"Wi-Fi MJPEG/image endpoint"| Deck(["NanoKit Flight Deck"])
  Deck -->|"BLE commands only"| Flight("NanoKit flight controller")
  CameraEsp -. "no motor authority" .-> Flight

  classDef controller fill:#102936,stroke:#4dd4ff,stroke-width:2px,color:#f4fbff
  classDef sensor fill:#282039,stroke:#b99bff,stroke-width:1.8px,color:#faf6ff
  classDef external fill:#28263a,stroke:#9ea9ff,stroke-width:1.8px,color:#f7f6ff
  classDef communication fill:#123237,stroke:#42d4c5,stroke-width:1.8px,color:#f2fffd
  class Camera sensor
  class CameraEsp external
  class Deck communication
  class Flight controller
  linkStyle default stroke:#7894a5,stroke-width:1.4px
```

## Open-Source Boundary

The implementation is original educational reference code. It uses published engineering patterns from established open-source flight stacks, without copying their source. See [Open_Source_References](Open_Source_References.md) for the specific projects and the concepts studied.

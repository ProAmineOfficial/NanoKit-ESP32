# Bluetooth Protocol - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

## Why ESP32 BLE

The browser controller uses ESP32 BLE GATT because Web Bluetooth supports BLE GATT devices in Chromium-based browsers. An HC-05 uses Bluetooth Classic SPP and is therefore not accessible from the normal Web Bluetooth API. A native Android application may use Classic Bluetooth or a different BLE client later, but the browser reference targets BLE directly.

## BLE Identity

| Item | Value |
|---|---|
| Advertised name | `NanoKit-Drone-4X` |
| Service UUID | `e1c91000-7a2a-4fd9-9d62-8ad3525b6f10` |
| Control UUID | `e1c91001-7a2a-4fd9-9d62-8ad3525b6f10` |
| Telemetry UUID | `e1c91002-7a2a-4fd9-9d62-8ad3525b6f10` |

## Control Packet

The controller sends a complete acknowledged semicolon-separated packet every 100 ms:

```text
T=250;R=-40;P=30;Y=0;A=1;SEQ=42;
```

| Field | Range | Meaning |
|---|---:|---|
| `T` | 0-1000 | Throttle command. Arming is permitted only near zero. |
| `R` | -100 to 100 | Roll stick value, converted to a conservative angle target. |
| `P` | -100 to 100 | Pitch stick value, converted to a conservative angle target. |
| `Y` | -100 to 100 | Desired yaw rate, not absolute heading. |
| `A` | 0 or 1 | Arm request. `0` immediately disarms. |
| `SEQ` | Increasing integer | Command sequence echoed by telemetry as `ACK` after firmware parsing. |
| `CAL` | 1 optional | Request a disarmed, motionless IMU calibration. |

The firmware clamps every numeric value. It disarms on BLE disconnect or if a current packet is not received for 700 ms.

## Camera Transport Is Not BLE

BLE remains reserved for the low-bandwidth flight command and telemetry link. The Arducam Mega camera uses a separate ESP32 Wi-Fi node that exposes an MJPEG/image URL. The Flight Deck loads that URL only inside its camera viewport; no camera request can arm, throttle, or modify the PID controller.

## Telemetry Packet

```text
PROTO=2;ROLL=-1.2;PITCH=0.5;YAW=3.8;THR=250;ARM=1;LINK=1;IMU=1;CAL=1;ACK=42
```

| Field | Meaning |
|---|---|
| `ROLL`, `PITCH` | Filtered estimated attitude in degrees. |
| `YAW` | Relative integrated yaw angle in degrees; it will drift without a magnetometer. |
| `THR` | Last accepted throttle command. |
| `ARM` | Firmware arm state. |
| `LINK` | Whether the command stream is within the failsafe window. |
| `IMU` | Whether an MPU6050 responded on I2C address `0x68` or `0x69`. |
| `CAL` | Whether IMU calibration is currently valid. |
| `ACK` | Latest parsed `SEQ`; confirms the firmware received the control packet. |
| `PROTO` | Firmware protocol version. The Flight Deck requires version `2`. |

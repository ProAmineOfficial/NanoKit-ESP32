# Control Loop - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

```mermaid
flowchart TD
  START["Start 250 Hz loop"] --> IMU["Read MPU6050"]
  IMU --> OK{"IMU read and calibration valid?"}
  OK -->|"No"| DISARM["Set all ESCs to 1000 us and disarm"]
  OK -->|"Yes"| EST["Update roll, pitch, yaw-rate estimate"]
  EST --> LINK{"BLE command fresh?"}
  LINK -->|"No"| DISARM
  LINK -->|"Yes"| ARMED{"Armed and throttle above minimum?"}
  ARMED -->|"No"| SAFE["Hold all ESCs at 1000 us"]
  ARMED -->|"Yes"| PID["Run roll, pitch, yaw-rate PID"]
  PID --> MIX["Apply Quad-X mix to M1-M4"]
  MIX --> PWM["Clamp and write ESC pulses"]
  SAFE --> TEL["Publish telemetry when due"]
  DISARM --> TEL
  PWM --> TEL
  TEL --> START
```

The loop is intentionally simple. The failsafe branches occur before the mixer so no stale command can continue driving a motor output.

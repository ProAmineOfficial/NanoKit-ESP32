# Control Loop - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

```mermaid
flowchart TD
  START(["Start 250 Hz loop"]) --> IMU[/"Read MPU6050"/]
  IMU --> OK{"IMU read and calibration valid?"}
  OK -->|"No"| DISARM("Set all ESCs to 1000 us and disarm")
  OK -->|"Yes"| EST("Update roll, pitch, yaw-rate estimate")
  EST --> LINK{"BLE command fresh?"}
  LINK -->|"No"| DISARM
  LINK -->|"Yes"| ARMED{"Armed and throttle above minimum?"}
  ARMED -->|"No"| SAFE("Hold all ESCs at 1000 us")
  ARMED -->|"Yes"| PID[["Run roll, pitch, yaw-rate PID"]]
  PID --> MIX[["Apply Quad-X mix to M1-M4"]]
  MIX --> PWM[/"Clamp and write ESC pulses"/]
  SAFE --> TEL[/"Publish telemetry when due"/]
  DISARM --> TEL
  PWM --> TEL
  TEL --> START

  classDef start fill:#17303b,stroke:#61cef3,stroke-width:1.8px,color:#f3fbff
  classDef sensor fill:#282039,stroke:#b99bff,stroke-width:1.8px,color:#faf6ff
  classDef decision fill:#362f18,stroke:#e0c45b,stroke-width:1.8px,color:#fff9de
  classDef process fill:#1a2631,stroke:#7b9bb0,stroke-width:1.6px,color:#f4f8fb
  classDef module fill:#202945,stroke:#91a8ff,stroke-width:1.8px,color:#f6f7ff
  classDef io fill:#15313c,stroke:#43d5ca,stroke-width:1.8px,color:#f3fffd
  classDef safety fill:#3a2025,stroke:#ef747d,stroke-width:1.8px,color:#fff3f4
  class START start
  class IMU sensor
  class OK,LINK,ARMED decision
  class EST process
  class PID,MIX module
  class PWM,TEL io
  class DISARM,SAFE safety
  linkStyle default stroke:#7894a5,stroke-width:1.4px
```

The loop is intentionally simple. The failsafe branches occur before the mixer so no stale command can continue driving a motor output.

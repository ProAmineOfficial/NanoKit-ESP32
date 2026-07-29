# System Diagram - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

```mermaid
flowchart LR
  Browser(["NanoKit Flight Deck browser"]) -->|"BLE GATT control + telemetry"| NanoKit("NanoKit flight controller")
  NanoKit -->|"I2C: GPIO21 SDA / GPIO22 SCL"| MPU("MPU6050 IMU")
  NanoKit -->|"GPIO25 PWM"| ESC1("ESC M1")
  NanoKit -->|"GPIO26 PWM"| ESC2("ESC M2")
  NanoKit -->|"GPIO27 PWM"| ESC3("ESC M3")
  NanoKit -->|"GPIO32 PWM"| ESC4("ESC M4")
  ESC1 --> M1("M1 front-left CCW motor")
  ESC2 --> M2("M2 front-right CW motor")
  ESC3 --> M3("M3 rear-right CCW motor")
  ESC4 --> M4("M4 rear-left CW motor")
  LIPO{{"3S/4S LiPo"}} --> ESC1
  LIPO --> ESC2
  LIPO --> ESC3
  LIPO --> ESC4
  LIPO --> BEC{{"5 V BEC"}}
  BEC --> NanoKit
  Camera{{"Arducam Mega 5 MP"}} -->|"SPI"| CameraNode(["Separate ESP32 Wi-Fi camera node"])
  CameraNode -->|"Wi-Fi MJPEG/image endpoint"| Browser
  CameraNode -. "no motor authority" .-> NanoKit

  classDef controller fill:#102936,stroke:#4dd4ff,stroke-width:2px,color:#f4fbff
  classDef sensor fill:#282039,stroke:#b99bff,stroke-width:1.8px,color:#faf6ff
  classDef communication fill:#123237,stroke:#42d4c5,stroke-width:1.8px,color:#f2fffd
  classDef power fill:#183326,stroke:#7ccd91,stroke-width:1.8px,color:#f4fff6
  classDef actuator fill:#37251a,stroke:#f0a560,stroke-width:1.8px,color:#fff8ef
  classDef external fill:#28263a,stroke:#9ea9ff,stroke-width:1.8px,color:#f7f6ff
  class NanoKit controller
  class MPU sensor
  class Browser,CameraNode communication
  class LIPO,BEC power
  class ESC1,ESC2,ESC3,ESC4,M1,M2,M3,M4 actuator
  class Camera external
  linkStyle default stroke:#7894a5,stroke-width:1.4px
```

The optional front camera is a separate Wi-Fi observation node. It cannot bypass NanoKit, control motors, or share the flight-control timing path.

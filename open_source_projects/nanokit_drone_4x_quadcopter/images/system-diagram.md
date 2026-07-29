# System Diagram - NanoKit Drone 4X (Quadcopter)

```mermaid
flowchart LR
  Browser["NanoKit Flight Deck browser"] -->|"BLE GATT control + telemetry"| NanoKit["NanoKit flight controller"]
  NanoKit -->|"I2C: GPIO21 SDA / GPIO22 SCL"| MPU["MPU6050 IMU"]
  NanoKit -->|"GPIO25 PWM"| ESC1["ESC M1"]
  NanoKit -->|"GPIO26 PWM"| ESC2["ESC M2"]
  NanoKit -->|"GPIO27 PWM"| ESC3["ESC M3"]
  NanoKit -->|"GPIO32 PWM"| ESC4["ESC M4"]
  ESC1 --> M1["M1 front-left CCW motor"]
  ESC2 --> M2["M2 front-right CW motor"]
  ESC3 --> M3["M3 rear-right CCW motor"]
  ESC4 --> M4["M4 rear-left CW motor"]
  LIPO["3S/4S LiPo"] --> ESC1
  LIPO --> ESC2
  LIPO --> ESC3
  LIPO --> ESC4
  LIPO --> BEC["5 V BEC"]
  BEC --> NanoKit
  Camera["Arducam Mega 5 MP"] -->|"SPI"| CameraNode["Separate ESP32 Wi-Fi camera node"]
  CameraNode -->|"Wi-Fi MJPEG/image endpoint"| Browser
  CameraNode -. "no motor authority" .-> NanoKit
```

The optional front camera is a separate Wi-Fi observation node. It cannot bypass NanoKit, control motors, or share the flight-control timing path.

# Camera Wiring - Arducam Mega 5 MP Camera Node

**Developed by Amine Saoud ibn al-Bashir.**

```mermaid
flowchart LR
  Camera("Arducam Mega 5 MP SPI camera")
  Node(["Dedicated ESP32 camera node"])
  Deck(["NanoKit Flight Deck browser"])

  Camera -->|"SCK -> GPIO18 / NanoKit pin 38"| Node
  Camera -->|"MISO -> GPIO19 / NanoKit pin 16"| Node
  Camera -->|"MOSI -> GPIO23 / NanoKit pin 24"| Node
  Camera -->|"CS -> GPIO17 / NanoKit pin 39"| Node
  Camera -->|"3.3 V + GND"| Node
  Node -->|"private Wi-Fi MJPEG/image endpoint"| Deck

  classDef sensor fill:#282039,stroke:#b99bff,stroke-width:1.8px,color:#faf6ff
  classDef external fill:#28263a,stroke:#9ea9ff,stroke-width:1.8px,color:#f7f6ff
  classDef communication fill:#123237,stroke:#42d4c5,stroke-width:1.8px,color:#f2fffd
  class Camera sensor
  class Node external
  class Deck communication
  linkStyle default stroke:#7894a5,stroke-width:1.4px
```

This is a separate camera node. It does not connect to NanoKit motor pins, MPU6050 I2C, PID calculations, or BLE flight-command handling.

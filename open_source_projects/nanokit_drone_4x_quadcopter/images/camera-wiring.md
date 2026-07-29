# Camera Wiring - Arducam Mega 5 MP Camera Node

**Developed by Amine Saoud ibn al-Bashir.**

```mermaid
flowchart LR
  Camera["Arducam Mega 5 MP SPI camera"]
  Node["Dedicated ESP32 camera node"]
  Deck["NanoKit Flight Deck browser"]

  Camera -->|"SCK -> GPIO18 / NanoKit pin 38"| Node
  Camera -->|"MISO -> GPIO19 / NanoKit pin 16"| Node
  Camera -->|"MOSI -> GPIO23 / NanoKit pin 24"| Node
  Camera -->|"CS -> GPIO17 / NanoKit pin 39"| Node
  Camera -->|"3.3 V + GND"| Node
  Node -->|"private Wi-Fi MJPEG/image endpoint"| Deck
```

This is a separate camera node. It does not connect to NanoKit motor pins, MPU6050 I2C, PID calculations, or BLE flight-command handling.

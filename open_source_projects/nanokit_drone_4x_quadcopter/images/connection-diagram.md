# Connection Diagram - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

```mermaid
flowchart TD
  NK("NanoKit ESP32")
  MPU("MPU6050")
  E1("ESC M1 front-left")
  E2("ESC M2 front-right")
  E3("ESC M3 rear-right")
  E4("ESC M4 rear-left")
  BEC{{"Regulated 5 V BEC"}}
  GND{{"Common ground"}}

  NK -->|"Pin 23 / GPIO21 SDA"| MPU
  NK -->|"Pin 18 / GPIO22 SCL"| MPU
  NK -->|"Pin 3 / GPIO25 signal"| E1
  NK -->|"Pin 37 / GPIO26 signal"| E2
  NK -->|"Pin 19 / GPIO27 signal"| E3
  NK -->|"Pin 7 / GPIO32 signal"| E4
  BEC -->|"regulated 5 V"| NK
  NK -->|"3.3 V only"| MPU
  GND --- NK
  GND --- MPU
  GND --- E1
  GND --- E2
  GND --- E3
  GND --- E4
  GND --- BEC

  classDef controller fill:#102936,stroke:#4dd4ff,stroke-width:2px,color:#f4fbff
  classDef sensor fill:#282039,stroke:#b99bff,stroke-width:1.8px,color:#faf6ff
  classDef actuator fill:#37251a,stroke:#f0a560,stroke-width:1.8px,color:#fff8ef
  classDef power fill:#183326,stroke:#7ccd91,stroke-width:1.8px,color:#f4fff6
  class NK controller
  class MPU sensor
  class E1,E2,E3,E4 actuator
  class BEC,GND power
  linkStyle default stroke:#7894a5,stroke-width:1.4px
```

ESC power leads go directly to the LiPo power-distribution path, not through NanoKit. Connect at least one ESC/BEC ground to NanoKit ground so PWM signals have a shared electrical reference.

The Arducam Mega camera wiring is intentionally documented separately in [camera-wiring.md](camera-wiring.md), because it belongs to the independent camera node rather than the flight controller.

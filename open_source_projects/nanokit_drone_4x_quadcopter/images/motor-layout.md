# Motor Layout - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

View from above. The nose, camera, and forward flight direction are at the top of the diagram.

```mermaid
flowchart TB
  NOSE["Nose / optional camera"]
  M1["M1 front-left\nCCW\nGPIO25 / NanoKit pin 3"]
  M2["M2 front-right\nCW\nGPIO26 / NanoKit pin 37"]
  FC["NanoKit ESP32 + MPU6050\ncentred and level"]
  M4["M4 rear-left\nCW\nGPIO32 / NanoKit pin 7"]
  M3["M3 rear-right\nCCW\nGPIO27 / NanoKit pin 19"]
  NOSE --> M1
  NOSE --> M2
  M1 --- FC
  M2 --- FC
  FC --- M4
  FC --- M3
```

| Motor | Position | Rotation | Propeller type |
|---|---|---|---|
| M1 | Front left | CCW | CCW propeller |
| M2 | Front right | CW | CW propeller |
| M3 | Rear right | CCW | CCW propeller |
| M4 | Rear left | CW | CW propeller |

The firmware mixer and this table are coupled. If the physical frame changes motor numbering, update the documentation and code together, then repeat all prop-off validation.

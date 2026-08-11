# Bill of Materials - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

This list separates required bench hardware from planned, unverified expansion hardware. It is not a flight-ready purchasing specification.

## Bench Development

| Item | Quantity | Requirement |
|---|---:|---|
| NanoKit Integrated ESP32 | 1 | Flight-controller development board |
| USB data cable | 1 | Reliable power and serial programming |
| Logic analyzer or oscilloscope | 1 | Verify PWM and timing before ESC connection |
| Current-limited supply | 1 | Controlled bench power |
| I2C sensor wiring | As required | Short 3.3 V-compatible wiring on GPIO21/22 |

## Propulsion - Selection Pending

| Item | Quantity | Gate |
|---|---:|---|
| Quad-X frame | 1 | Mechanical analysis and measured mass |
| Brushless motors | 4 | Selected with propeller, voltage, and thrust target |
| 4-in-1 ESC | 1 | Must explicitly accept analogue 1000-2000 us PWM |
| Propellers | 2 CW + 2 CCW | Removed during all current tests |
| LiPo battery | 1 | Selected from measured current and required C rating |
| Regulated 5 V BEC | 1 | Rated for controller and verified payload load |
| Power distribution, capacitor, wiring, connector | 1 set | Sized from measured peak current |

## Planned Expansion - Disabled

ICM-20948, DPS310, BME280, GNSS, PMW3901, TCA9548A, six VL53L1CX, HC-SR04 with ECHO level shifting, INA226, AT24C256, status LED, buzzer, and the camera/audio/storage/servo payload remain TODO until exact parts and wiring are confirmed.

NanoKit #2 requires 8 MB PSRAM for the proposed combined payload. Verify the physical module and runtime report before buying or wiring the remaining camera subsystem.

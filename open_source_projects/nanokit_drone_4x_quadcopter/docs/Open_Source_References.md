# Open-Source References - NanoKit Drone 4X (Quadcopter)

This project is an original, compact educational implementation. The following mature projects are studied as engineering references, not copied into this repository.

| Project | Useful ideas for NanoKit Drone 4X | Licence / scope note |
|---|---|---|
| [Betaflight](https://github.com/betaflight/betaflight) | Quad-X conventions, mixer reasoning, flight-control safety culture, and tuning vocabulary. | GPL-3.0; do not copy code into a differently licensed project without satisfying GPL obligations. |
| [ArduPilot](https://github.com/ArduPilot/ardupilot) | Sensor calibration discipline, test methodology, flight-mode design, logs, and mature failsafe thinking. | GPL-3.0; a substantially broader autopilot system. |
| [ESP-Drone](https://github.com/espressif/esp-drone) | ESP32-oriented embedded flight work, wireless command concepts, and hardware bring-up lessons. | Check the upstream repository licence and component licences before reuse. |
| [PX4](https://github.com/PX4/PX4-Autopilot) | Estimator architecture, parameter management, simulation, and vehicle integration ideas. | BSD-3-Clause components vary; read the upstream licensing and documentation. |
| [Arducam Mega](https://github.com/ArduCAM/Arducam_Mega) | SPI camera bring-up, supported ESP32 platform examples, and camera-node separation. | MIT; retain its licence and notices for any copied upstream code. |

## How To Reuse Open Source Correctly

1. Link to the upstream source and keep its licence, notices, and attribution with any copied material.
2. Review compatibility before mixing code, documentation, assets, and firmware from different projects.
3. Prefer learning a design pattern and implementing a small, documented version rather than importing a large flight stack blindly.
4. Track upstream version, source file, licence, and modification history for every reused artifact.
5. Test changes on a bench before integrating them into a vehicle.

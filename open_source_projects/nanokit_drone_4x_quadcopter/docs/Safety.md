# Safety - NanoKit Drone 4X (Quadcopter)

## Non-Negotiable Rules

1. Remove propellers for wiring, flashing, calibration, serial tests, BLE tests, motor order, and motor direction checks.
2. Treat every LiPo as a high-current source. Use an appropriate charger, balance lead, storage voltage, fire-resistant charging area, and damaged-pack policy.
3. Keep a physical battery disconnect accessible. Software disarm is useful but is not a substitute for disconnecting power.
4. Never power motors from NanoKit 3.3 V, USB, or an undersized BEC.
5. Never test around people, animals, vehicles, roads, public spaces, or flammable material.
6. Stop at the first sign of heat, swelling, smoke, brownout, unusual motor sound, sensor drift, or inverted response.
7. Follow local aviation, radio, privacy, insurance, registration, and remote-identification requirements before any outdoor operation.

## Firmware Protections

| Condition | Firmware response |
|---|---|
| Boot | Four ESCs receive 1000 us minimum throttle. |
| Arm request with throttle above zero | Arm request is rejected. |
| Missing IMU calibration | Arm request is rejected. |
| BLE disconnect | Immediate disarm; advertising restarts. |
| BLE command age above 700 ms | Immediate disarm. |
| MPU6050 read failure | Immediate disarm. |
| Explicit stop/disarm command | Immediate disarm and PID reset. |

## Limits Of This Reference

This code has no redundant IMU, independent kill switch, barometer, GPS, geofence, return-to-home, authenticated control channel, battery voltage monitoring, blackbox logger, or certified fail-safe architecture. It is an educational starting point for controlled engineering work, not a ready-to-operate aircraft controller.

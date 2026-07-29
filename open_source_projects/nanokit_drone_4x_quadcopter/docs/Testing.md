# Testing - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

## Test Gate

Do not install propellers or attempt tethered flight until every applicable check below passes. Each test must be documented with the firmware version, battery type, motor/ESC set, and result.

| # | Test | Pass condition |
|---:|---|---|
| 1 | Visual inspection | No damaged wire, loose connector, exposed LiPo lead, or conductive debris. |
| 2 | Power-off continuity | No short between battery positive and ground. |
| 3 | First LiPo power | Smoke stopper remains normal; no hot component or unexpected motor motion. |
| 4 | NanoKit serial boot | PWM begins at 1000 us and serial reports a working MPU6050. |
| 5 | IMU alignment | Hand tilts produce the expected roll/pitch telemetry signs. |
| 6 | BLE connection | Browser shows connected state and receives live telemetry. |
| 7 | Arm interlock | A non-zero throttle cannot arm; calibration is required before arm. |
| 8 | Emergency stop | Stop control sets all outputs to 1000 us immediately. |
| 9 | Link loss | Disabling BLE or stopping packets disarms within 700 ms. |
| 10 | Motor order | M1/M2/M3/M4 signal locations exactly match the motor-layout document. |
| 11 | Motor rotation | Each motor spins CW or CCW as specified, with props removed. |
| 12 | Mixer signs | Small frame disturbance produces motor corrections that oppose it. |

## Tethered Test

Only after the test gate is complete, use a clear controlled area, a rigid tether appropriate for the frame, eye protection, a fire-safe LiPo location, and a second person with access to the battery disconnect. Start at the minimum practical throttle and stop immediately on oscillation, reversed response, brownout, excessive heat, or communication loss.

## Flight Readiness Is Not Automatic

Passing the bench checklist does not make the system flight-ready. Flight legality, airspace restrictions, remote-identification requirements, liability, insurance, battery condition, structural integrity, weather, and local rules remain the operator's responsibility.

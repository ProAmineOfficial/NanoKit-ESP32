# Safety - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

## Non-Negotiable Rules

1. Keep propellers removed during wiring, flashing, calibration, network, sensor, mixer, and ESC signal tests.
2. Use a current-limited bench supply where practical and keep a physical battery disconnect accessible.
3. Never enable a Feature Flag to make the interface look complete.
4. Never mark telemetry valid unless a real driver completed a current successful read.
5. Never set `NANOKIT_ESC_ANALOG_PWM_CONFIRMED` to `1` until the exact ESC protocol is verified on the bench.
6. Do not attempt free flight from this reference revision.

## Firmware Protections

| Condition | Result |
|---|---|
| Boot or disarmed state | Four outputs held at 1000 us |
| Emergency stop | Failsafe latched; outputs forced to minimum |
| Armed link disconnect | Immediate failsafe |
| Command older than 600 ms | Immediate failsafe |
| IMU unhealthy or attitude invalid | Arming blocked or immediate fault |
| IMU not calibrated | Arming blocked |
| ESC analogue PWM unconfirmed | Arming blocked and outputs remain minimum |
| Throttle above zero during arm | Arm rejected |
| Advanced mode sensors unavailable | Mode remains unavailable |

Reconnection never restores the previous armed state. ARM must first be released with throttle at zero before another attempt.

## Physical Risks Not Solved By Software

The firmware cannot detect a reversed propeller, incorrect motor order, loose motor, undersized ESC, wrong battery chemistry, damaged LiPo, weak frame, radio interference, unsuitable BEC, or unsafe test area. These require physical inspection, measurement, and experienced supervision.

## Payload Isolation

The camera/audio node exposes only payload services. It cannot write commands into the flight queue, control ESC outputs, or authorize arming. Payload failure must remain independent from flight-control timing.

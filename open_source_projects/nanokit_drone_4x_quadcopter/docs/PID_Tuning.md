# PID Tuning - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

## Current Gate

PID classes and the Quad-X mixer are included for architecture and bench verification, but flight tuning is not authorized. The IMU driver is disabled and the ESC analogue PWM protocol is unconfirmed. Gains cannot be validated without real calibrated rate data and a verified propulsion interface.

## Required Order

1. Verify IMU wiring, orientation, rate, scale, calibration, and disconnect behaviour.
2. Verify the exact ESC accepts analogue 1000-2000 us commands.
3. Verify motor numbering and rotation with propellers removed.
4. Verify mixer signs using controlled low-energy tests.
5. Measure loop timing and confirm no missed 4 ms deadlines.
6. Begin rate-loop tuning on a purpose-built restrained rig.
7. Add attitude outer-loop tuning only after the rate loop is stable.

## Controller Limits

- Use output saturation and integral anti-windup.
- Reset integrators whenever the system disarms or enters failsafe.
- Reject non-finite sensor and controller values.
- Do not copy gains from a different frame, propeller, motor, battery, ESC, or payload.
- Record firmware revision, mass, center of gravity, and hardware for every tuning result.

Navigation and obstacle-avoidance modes require separately verified sensors and are not unlocked by PID tuning.

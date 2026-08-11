# Calibration - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

## Current Status

The ICM-20948 driver and pin-level integration are intentionally disabled. The Calibration button may send a protocol command, but firmware must not claim success until a real healthy sensor produces stable samples. This revision therefore remains calibration-locked.

## Future IMU Procedure

1. Remove propellers and disconnect ESC power.
2. Mount the verified IMU rigidly and document its forward/right/up axes.
3. Place the frame level and motionless on a stable surface.
4. Confirm continuous real accelerometer and gyroscope data.
5. Request calibration while disarmed.
6. Reject calibration if samples move, saturate, time out, or fail plausibility limits.
7. Store offsets only after repeatable validation.
8. Rotate the frame by known angles and compare the reported axes before any mixer test.

Magnetometer calibration requires a separate hard/soft-iron procedure after the final electrical and mechanical assembly. It must not be performed near high-current wires, motors, steel tools, or magnets.

## ESC Calibration

Endpoint calibration is manufacturer-specific and can start motors unexpectedly. First prove that the exact 4-in-1 ESC supports analogue 1000-2000 us PWM. Follow its manufacturer procedure with propellers removed and an immediate power disconnect. Do not add an automatic high-throttle calibration routine to the normal flight firmware.

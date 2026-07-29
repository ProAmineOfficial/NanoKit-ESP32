# PID Tuning - NanoKit Drone 4X (Quadcopter)

**Developed by Amine Saoud ibn al-Bashir.**

## Controller Structure

The reference firmware uses three controllers:

| Controller | Target | Measurement | Output |
|---|---|---|---|
| Roll PID | Desired roll angle | Filtered roll angle | Differential left/right motor correction. |
| Pitch PID | Desired pitch angle | Filtered pitch angle | Differential front/rear motor correction. |
| Yaw-rate PID | Desired yaw rate | Gyroscope Z rate | CW/CCW motor-pair correction. |

The starting gains in `firmware/src/main.cpp` are only conservative placeholders. They are not transferable to another frame, propeller, battery, motor, ESC, or payload combination.

## Safe Tuning Sequence

1. Complete IMU alignment, motor order, motor direction, mixer sign, BLE timeout, and emergency-stop tests with propellers removed.
2. Start with `Ki = 0` and a low `Kp`; use a restrained or tethered test area and keep a physical battery disconnect reachable.
3. Increase `Kp` until the axis responds firmly, then reduce it if fast oscillation appears.
4. Increase `Kd` in small increments to reduce overshoot. Too much derivative amplifies IMU noise and makes motors sound rough.
5. Add only enough `Ki` to correct slow bias or persistent lean. Integral is deliberately small because it can build up during saturation.
6. Tune roll and pitch independently before tuning yaw rate.
7. Change one gain, one axis, and one test condition at a time. Record the exact change and outcome.

## Symptoms

| Symptom | Likely adjustment | First check |
|---|---|---|
| Slow lean and weak correction | Increase roll/pitch `Kp` slightly. | IMU level calibration and frame stiffness. |
| Fast shake or buzzing motors | Reduce `Kp` or `Kd`. | Propeller damage, loose parts, and sensor vibration. |
| Overshoot after a tilt command | Increase `Kd` slightly or reduce `Kp`. | Correct motor direction and mixer sign. |
| Slowly increasing offset | Add very small `Ki`. | Centre of gravity and accelerometer calibration. |
| Yaw response is reversed | Correct motor rotation/mixer sign. | M1/M2/M3/M4 direction table. |

## Important Limits

- The MPU6050 cannot hold an absolute compass heading. Its yaw reading is relative and drifts.
- This first reference has no barometer, GPS, optical flow, altitude hold, return-to-home, or autonomous navigation.
- Do not tune PID gains by flying near people, public areas, roads, or buildings.

# Calibration - NanoKit Drone 4X (Quadcopter)

## 1. Frame And IMU Alignment

Place the Quad-X frame on a known flat surface with the nose pointing forward. The MPU6050 must be rigid and have the same forward/right/up axes as the aircraft. If it is rotated, update the sensor-axis mapping before enabling motors; PID tuning cannot compensate for a wrongly mapped IMU.

## 2. IMU Level Calibration

1. Remove propellers and keep the LiPo disconnected until PWM checks are complete.
2. Flash the firmware, open the serial monitor at 115200 baud, and place the frame level and motionless.
3. At boot, allow the initial calibration to finish. The console must report `Calibration complete`.
4. To recalibrate later, send `CALIBRATE_IMU` in the serial monitor or press **Calibrate IMU** in the BLE controller while disarmed.
5. Confirm roll and pitch telemetry is close to 0 degrees on the level surface, then tilt the frame by hand and verify the changing axis is correct.

The firmware averages 700 samples to estimate gyro drift and level offsets. It does not save these values to flash yet; recalibrate after changing the IMU mount or frame geometry.

## 3. ESC Endpoint Calibration

ESC endpoint calibration is manufacturer specific and can spin motors unexpectedly if attempted incorrectly. Use the ESC manual and perform this operation only with propellers removed, an accessible battery disconnect, and an experienced spotter. The normal flight firmware must remain in its safe 1000 us boot state during all ordinary development.

## 4. Motor Direction

With propellers removed, briefly test each motor at the minimum practical throttle. Verify the exact M1/M2/M3/M4 location and rotation in [Motor Layout](../images/motor-layout.md). Reverse any brushless motor direction by swapping any two of its three motor wires, then retest.

## 5. Mixer Sign Check

Hold the frame securely with props removed and command a small roll, pitch, or yaw correction. The pair of motors that would oppose the measured disturbance must increase relative to the opposite pair. If not, stop and correct the IMU axis orientation or the documented mixer signs before any tethered test.

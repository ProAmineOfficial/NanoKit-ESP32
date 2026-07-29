# Bill Of Materials - NanoKit Drone 4X (Quadcopter)

## Reference 3.5-inch 4S Build Class

This is a selection framework, not an endorsement of one vendor. Confirm electrical compatibility against the motor/propeller manufacturer's thrust data and measured current before purchase.

| Part | Qty. | Selection guidance | Example class |
|---|---:|---|---|
| Flight controller | 1 | NanoKit Integrated ESP32 | NanoKit ESP32 |
| IMU | 1 | 3.3 V MPU6050 breakout with short, secure I2C wiring | MPU6050/GY-521 class |
| Brushless motors | 4 | Same model, 1404 around 3800 KV for a 3.5-inch 4S reference build | T-Motor F1404 3800 KV class |
| ESCs | 4 | 20 A or above with measured-current margin; compatible with selected LiPo | BLHeli_S/BLHeli_32 20-35 A class |
| Propellers | 2 pairs | Matched 3.5-inch CW/CCW props compatible with motor shaft | 3.5x3 to 3.5x3.5 class |
| Frame | 1 | Rigid 3.5-inch Quad-X carbon frame with central electronics bay | 180-200 mm wheelbase class |
| LiPo | 1+ | 4S, capacity and C rating proven for actual current | 850-1300 mAh 4S class |
| BEC | 1 | Regulated 5 V, 3 A minimum, low-noise output | 5 V 3 A switching BEC |
| Power wiring | 1 set | Correct wire gauge, XT30/XT60, capacitor at ESC power input | Match actual current and connector |
| Camera | optional | Separate Wi-Fi camera node; never required for stabilisation | Arducam Mega 5 MP SPI camera |
| Camera MCU | optional | Dedicated ESP32/ESP32-S3 for the Arducam SPI bus and Wi-Fi endpoint | ESP32 DevKit or second NanoKit class |
| Hardware | 1 set | Standoffs, vibration isolation, straps, heat-shrink, smoke stopper | Frame-compatible set |

## Size Match Matters

Do not combine an arbitrary large propeller, high-KV motor, small ESC, and undersized battery. The propeller establishes load, the motor establishes speed/torque, the ESC and wire must handle current, the battery must provide it, and the frame must withstand the resulting thrust and vibration.

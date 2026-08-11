# Sensor Integration - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

## Current Rule

Only the shared I2C bus on GPIO21/GPIO22 is confirmed. A confirmed bus does not confirm a sensor's address, voltage, orientation, interrupt pin, driver, or flight suitability. Every sensor feature therefore defaults to `0` in `firmware/include/config/feature_flags.h`.

## Planned Sensors

| Sensor | Intended role | Current gate |
|---|---|---|
| ICM-20948 | Accelerometer, gyroscope, magnetometer | Wiring, address, orientation, and driver TODO |
| DPS310 | Barometric altitude | Address and driver TODO |
| BME280 | Temperature, humidity, pressure | Address and driver TODO |
| u-blox M9N class GNSS | Position and ground speed | UART pins and voltage TODO |
| PMW3901 | Optical flow | SPI pins, orientation, and driver TODO |
| Six VL53L1CX | Obstacle distances | TCA9548A channels and mounting TODO |
| HC-SR04 | Bench distance input | Trigger/ECHO pins and 5 V ECHO protection TODO |
| INA226 | Battery voltage/current/power | Shunt design, address, and calibration TODO |
| AT24C256 | Persistent configuration | Address, write protection, and driver TODO |

## I2C Address Planning

BME280 and DPS310 address selections can conflict depending on module straps. Scan the bus and record each actual address. If unique addresses cannot be guaranteed, place them on separate TCA9548A channels.

All six VL53L1CX devices start with the same default I2C address. Each sensor must use a separate TCA9548A channel, or be individually isolated and re-addressed during every boot. The reference design uses separate multiplexer channels.

## HC-SR04 Electrical Safety

Many HC-SR04 modules drive ECHO near 5 V. ESP32 GPIO is not 5 V tolerant. Use a calculated resistor divider or a proper logic-level shifter before the ECHO signal reaches NanoKit. Do not enable this feature based only on a software pin definition.

## Driver Acceptance Test

Before changing a feature flag to `1`:

1. Document the exact module, supply voltage, address, and pins.
2. Test initialization and continuous reads with motors disconnected.
3. Verify unplug/replug and bus-fault behaviour.
4. Verify reported units and axis orientation against a known reference.
5. Add validity gating and ensure failed reads cannot leave stale values marked valid.
6. Run the complete safety test matrix before connecting ESC power.

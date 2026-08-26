#pragma once // Prevent this header from being included more than once.

#define NANOKIT_FEATURE_ICM20948 0 // Developed by Amine Saoud ibn al-Bashir. Unverified hardware stays disabled. Enable one flag only after wiring, voltage, address, and driver behaviour have been validated on a propeller-free bench. Each sensor flag protects code whose pins or electrical details are still unverified.
#define NANOKIT_FEATURE_DPS310 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_BME280 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_GNSS_M9N 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_PMW3901 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_TCA9548A 0 // The I2C multiplexer must be enabled before devices with duplicate addresses are used.
#define NANOKIT_FEATURE_VL53L1CX_ARRAY 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_HCSR04 0 // HC-SR04 remains disabled until its 5 V ECHO output has safe level shifting.
#define NANOKIT_FEATURE_INA226 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_AT24C256 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_STATUS_LED 0 // User-interface outputs also stay disabled until their exact pins are confirmed.
#define NANOKIT_FEATURE_BUZZER 0 // Define a compile-time configuration constant.

#define NANOKIT_ESC_ANALOG_PWM_CONFIRMED 0 // Keep this at 0 until the exact 4-in-1 ESC is verified to accept analogue 1000-2000 us commands. At 0, all four outputs stay at the safe minimum.

#define NANOKIT_FEATURE_ALTITUDE_HOLD 0 // Higher-level modes require validated sensors, so all remain compile-time disabled.
#define NANOKIT_FEATURE_POSITION_HOLD 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_NAVIGATION 0 // Define a compile-time configuration constant.
#define NANOKIT_FEATURE_OBSTACLE_AVOIDANCE 0 // Define a compile-time configuration constant.

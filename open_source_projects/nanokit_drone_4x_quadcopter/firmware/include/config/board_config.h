#pragma once // Prevent this header from being included more than once.

#include <Arduino.h> // Arduino provides the fixed-width types and FreeRTOS task types used below.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir. Keep all board-specific constants in one namespace so other modules do not duplicate pin numbers, timing limits, or communication settings.
namespace config { // Open the namespace that owns these project symbols.

constexpr uint8_t I2C_SDA_PIN = 21; // Confirmed NanoKit interfaces only. GPIO21 and GPIO22 are the only currently verified I2C pins for this project.
constexpr uint8_t I2C_SCL_PIN = 22; // Assign this value for the current control or telemetry operation.
constexpr uint32_t I2C_CLOCK_HZ = 400000; // Fast-mode I2C reduces sensor read time while remaining a standard bus speed.

constexpr uint8_t MOTOR_1_PIN = 25; // These four GPIO pins are the confirmed command outputs for motors M1-M4.
constexpr uint8_t MOTOR_2_PIN = 26; // Assign this value for the current control or telemetry operation.
constexpr uint8_t MOTOR_3_PIN = 27; // Assign this value for the current control or telemetry operation.
constexpr uint8_t MOTOR_4_PIN = 32; // Assign this value for the current control or telemetry operation.

constexpr uint8_t MOTOR_1_CHANNEL = 0; // Each motor uses a separate ESP32 LEDC channel so its pulse can be controlled independently.
constexpr uint8_t MOTOR_2_CHANNEL = 1; // Assign this value for the current control or telemetry operation.
constexpr uint8_t MOTOR_3_CHANNEL = 2; // Assign this value for the current control or telemetry operation.
constexpr uint8_t MOTOR_4_CHANNEL = 3; // Assign this value for the current control or telemetry operation.

constexpr uint32_t ESC_PWM_FREQUENCY_HZ = 50; // A 50 Hz, 16-bit signal represents conventional 1000-2000 microsecond ESC commands.
constexpr uint8_t ESC_PWM_RESOLUTION_BITS = 16; // Assign this value for the current control or telemetry operation.
constexpr uint16_t ESC_MINIMUM_US = 1000; // The minimum pulse stops a motor, the idle pulse is the armed floor, and the maximum pulse limits the command sent to a verified ESC.
constexpr uint16_t ESC_IDLE_US = 1060; // Assign this value for the current control or telemetry operation.
constexpr uint16_t ESC_MAXIMUM_US = 2000; // Assign this value for the current control or telemetry operation.

constexpr uint32_t FLIGHT_LOOP_PERIOD_US = 4000; // Four milliseconds gives the flight task its required 250 Hz update rate.
constexpr uint32_t TELEMETRY_PERIOD_MS = 100; // Telemetry is slower than the control loop to avoid unnecessary network load.
constexpr uint32_t COMMAND_TIMEOUT_MS = 600; // A command older than this limit is unsafe and causes the failsafe path.
constexpr uint32_t NETWORK_TASK_DELAY_MS = 2; // The network task yields briefly so it cannot starve the real-time flight task.
constexpr uint8_t FLIGHT_TASK_CORE = 1; // Flight control runs on core 1 while networking runs on core 0 for isolation.
constexpr uint8_t NETWORK_TASK_CORE = 0; // Assign this value for the current control or telemetry operation.
constexpr UBaseType_t FLIGHT_TASK_PRIORITY = 4; // Flight control has the higher priority because motor safety is time-critical.
constexpr UBaseType_t NETWORK_TASK_PRIORITY = 1; // Assign this value for the current control or telemetry operation.

constexpr char ACCESS_POINT_SSID[] = "NanoKit-Drone-4X"; // The controller creates this local Wi-Fi access point for the browser Flight Deck.
constexpr char ACCESS_POINT_PASSWORD[] = "NanoKit4X"; // WPA2 requires at least eight characters; this bench password meets that rule.
constexpr uint8_t ACCESS_POINT_CHANNEL = 6; // A fixed channel makes bench testing repeatable and easier to diagnose.
constexpr uint16_t HTTP_PORT = 80; // HTTP serves the interface and WebSocket carries low-latency commands and telemetry.
constexpr uint16_t WEBSOCKET_PORT = 81; // Assign this value for the current control or telemetry operation.
constexpr uint8_t PROTOCOL_VERSION = 3; // The protocol version lets the firmware and browser reject incompatible packet formats.

constexpr float MAX_ATTITUDE_TARGET_DEG = 25.0f; // These limits constrain browser commands before they enter the PID controllers.
constexpr float MAX_YAW_RATE_DPS = 120.0f; // Assign this value for the current control or telemetry operation.
constexpr uint16_t ARM_THROTTLE_MAX = 0; // Arming is accepted only at zero throttle to prevent an unexpected motor start.

}  // namespace config
}  // namespace nanokit

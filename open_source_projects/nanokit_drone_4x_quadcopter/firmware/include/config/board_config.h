// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <Arduino.h> // Arduino provides the fixed-width types and FreeRTOS task types used below.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir. Keep all board-specific constants in one namespace so other modules do not duplicate pin numbers, timing limits, or communication settings.
namespace config {

constexpr uint8_t I2C_SDA_PIN = 21; // NanoKit Pin 23, GPIO21, VR 0x30. Confirmed I2C0 SDA; GPIO21/GPIO22 are the only verified I2C pins for this project.
constexpr uint8_t I2C_SCL_PIN = 22; // NanoKit Pin 18, GPIO22, VR 0x31. Confirmed I2C0 SCL, 3.3 V logic with module pull-ups.
constexpr uint32_t I2C_CLOCK_HZ = 400000; // Fast-mode I2C reduces sensor read time while remaining a standard bus speed.

constexpr uint8_t MOTOR_1_PIN = 25; // NanoKit Pin 3, GPIO25. M1 front-left CCW signal; the four motor pins are confirmed safe outputs, the ESC protocol is not.
constexpr uint8_t MOTOR_2_PIN = 26; // NanoKit Pin 37, GPIO26. M2 front-right CW signal.
constexpr uint8_t MOTOR_3_PIN = 27; // NanoKit Pin 19, GPIO27. M3 rear-right CCW signal.
constexpr uint8_t MOTOR_4_PIN = 32; // NanoKit Pin 7, GPIO32. M4 rear-left CW signal; ADC1-capable pin used here as a plain output.

constexpr uint8_t MOTOR_1_CHANNEL = 0; // Each motor uses a separate ESP32 LEDC channel so its pulse can be controlled independently.
constexpr uint8_t MOTOR_2_CHANNEL = 1; // LEDC channels 0-3 share one timer so all four pulses stay phase-aligned.
constexpr uint8_t MOTOR_3_CHANNEL = 2;
constexpr uint8_t MOTOR_4_CHANNEL = 3;

constexpr uint32_t ESC_PWM_FREQUENCY_HZ = 50; // A 50 Hz, 16-bit signal represents conventional 1000-2000 microsecond ESC commands.
constexpr uint8_t ESC_PWM_RESOLUTION_BITS = 16; // 16 bits at 50 Hz gives about 0.3 us per step, finer than any ESC can resolve.
constexpr uint16_t ESC_MINIMUM_US = 1000; // The minimum pulse stops a motor, the idle pulse is the armed floor, and the maximum pulse limits the command sent to a verified ESC.
constexpr uint16_t ESC_IDLE_US = 1060; // Armed floor: just above the stop pulse so motors stay synchronised without producing lift.
constexpr uint16_t ESC_MAXIMUM_US = 2000; // Upper clamp applied after mixing so no channel can exceed the ESC range.

constexpr uint32_t FLIGHT_LOOP_PERIOD_US = 4000; // Four milliseconds gives the flight task its required 250 Hz update rate.
constexpr uint32_t TELEMETRY_PERIOD_MS = 100; // Telemetry is slower than the control loop to avoid unnecessary network load.
constexpr uint32_t COMMAND_TIMEOUT_MS = 600; // A command older than this limit is unsafe and causes the failsafe path.
constexpr uint32_t NETWORK_TASK_DELAY_MS = 2; // The network task yields briefly so it cannot starve the real-time flight task.
constexpr uint8_t FLIGHT_TASK_CORE = 1; // Flight control runs on core 1 while networking runs on core 0 for isolation.
constexpr uint8_t NETWORK_TASK_CORE = 0; // Wi-Fi and the WebSocket server already run on core 0, so the network task joins them.
constexpr UBaseType_t FLIGHT_TASK_PRIORITY = 4; // Flight control has the higher priority because motor safety is time-critical.
constexpr UBaseType_t NETWORK_TASK_PRIORITY = 1; // Low priority: a slow browser must never delay the flight loop.

constexpr char ACCESS_POINT_SSID[] = "NanoKit-Drone-4X"; // The controller creates this local Wi-Fi access point for the browser Flight Deck.
constexpr char ACCESS_POINT_PASSWORD[] = "NanoKit4X"; // WPA2 requires at least eight characters; this bench password meets that rule.
constexpr uint8_t ACCESS_POINT_CHANNEL = 6; // A fixed channel makes bench testing repeatable and easier to diagnose.
constexpr uint16_t HTTP_PORT = 80; // HTTP serves the interface and WebSocket carries low-latency commands and telemetry.
constexpr uint16_t WEBSOCKET_PORT = 81; // Separate port keeps the HTTP file server and the real-time command channel independent.
constexpr uint8_t PROTOCOL_VERSION = 3; // The protocol version lets the firmware and browser reject incompatible packet formats.

constexpr float MAX_ATTITUDE_TARGET_DEG = 25.0f; // These limits constrain browser commands before they enter the PID controllers.
constexpr float MAX_YAW_RATE_DPS = 120.0f; // Yaw is rate-controlled, so the limit is in degrees per second rather than degrees.
constexpr uint16_t ARM_THROTTLE_MAX = 0; // Arming is accepted only at zero throttle to prevent an unexpected motor start.

}  // namespace config
}  // namespace nanokit

#pragma once

#include <Arduino.h>

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {
namespace config {

// Confirmed NanoKit interfaces only.
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 400000;

constexpr uint8_t MOTOR_1_PIN = 25;
constexpr uint8_t MOTOR_2_PIN = 26;
constexpr uint8_t MOTOR_3_PIN = 27;
constexpr uint8_t MOTOR_4_PIN = 32;

constexpr uint8_t MOTOR_1_CHANNEL = 0;
constexpr uint8_t MOTOR_2_CHANNEL = 1;
constexpr uint8_t MOTOR_3_CHANNEL = 2;
constexpr uint8_t MOTOR_4_CHANNEL = 3;

constexpr uint32_t ESC_PWM_FREQUENCY_HZ = 50;
constexpr uint8_t ESC_PWM_RESOLUTION_BITS = 16;
constexpr uint16_t ESC_MINIMUM_US = 1000;
constexpr uint16_t ESC_IDLE_US = 1060;
constexpr uint16_t ESC_MAXIMUM_US = 2000;

constexpr uint32_t FLIGHT_LOOP_PERIOD_US = 4000;
constexpr uint32_t TELEMETRY_PERIOD_MS = 100;
constexpr uint32_t COMMAND_TIMEOUT_MS = 600;
constexpr uint32_t NETWORK_TASK_DELAY_MS = 2;
constexpr uint8_t FLIGHT_TASK_CORE = 1;
constexpr uint8_t NETWORK_TASK_CORE = 0;
constexpr UBaseType_t FLIGHT_TASK_PRIORITY = 4;
constexpr UBaseType_t NETWORK_TASK_PRIORITY = 1;

constexpr char ACCESS_POINT_SSID[] = "NanoKit-Drone-4X";
constexpr char ACCESS_POINT_PASSWORD[] = "NanoKit4X";
constexpr uint8_t ACCESS_POINT_CHANNEL = 6;
constexpr uint16_t HTTP_PORT = 80;
constexpr uint16_t WEBSOCKET_PORT = 81;
constexpr uint8_t PROTOCOL_VERSION = 3;

constexpr float MAX_ATTITUDE_TARGET_DEG = 25.0f;
constexpr float MAX_YAW_RATE_DPS = 120.0f;
constexpr uint16_t ARM_THROTTLE_MAX = 0;

}  // namespace config
}  // namespace nanokit

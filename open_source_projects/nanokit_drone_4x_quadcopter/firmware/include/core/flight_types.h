// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <Arduino.h> // Arduino supplies fixed-width integer types and the UINT32_MAX constant.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

enum class FlightMode : uint8_t { Manual = 0, AltitudeHold, PositionHold, Mission }; // FlightMode identifies which command strategy the pilot requested.
enum class SafetyState : uint8_t { // SafetyState is the authoritative lifecycle used to decide whether motors may run.
  Boot = 0,
  Initializing, // Continue the current project declaration or implementation.
  CalibrationRequired, // Continue the current project declaration or implementation.
  Disarmed, // Continue the current project declaration or implementation.
  Armed, // Continue the current project declaration or implementation.
  Failsafe, // Continue the current project declaration or implementation.
  Fault, // Continue the current project declaration or implementation.
};

struct PilotCommand { // PilotCommand is the complete command packet accepted from the Wi-Fi Flight Deck.
  uint32_t sequence = 0; // sequence and receivedAtMs support acknowledgements and stale-command detection.
  uint32_t receivedAtMs = 0;
  uint16_t throttle = 0; // throttle is a normalized pilot demand; axis targets use engineering units.
  float rollTargetDeg = 0.0f;
  float pitchTargetDeg = 0.0f;
  float yawRateTargetDps = 0.0f;
  FlightMode mode = FlightMode::Manual; // mode and action flags describe the requested operating state.
  bool armRequested = false;
  bool calibrateRequested = false;
  bool emergencyStop = false;
};

struct AttitudeState { // AttitudeState holds orientation and angular rates produced by a validated IMU driver.
  float rollDeg = 0.0f; // Euler angles are reported in degrees for telemetry and attitude control.
  float pitchDeg = 0.0f;
  float yawDeg = 0.0f;
  float rollRateDps = 0.0f; // Angular rates are measured in degrees per second for rate control.
  float pitchRateDps = 0.0f;
  float yawRateDps = 0.0f;
  bool valid = false; // valid prevents zero-filled or stale values from being treated as real measurements.
};

struct SensorState { // SensorState is the shared snapshot consumed by safety, control, and telemetry.
  AttitudeState attitude; // Execute this statement as part of the current subsystem operation.
  float barometricAltitudeM = 0.0f; // Optional environment and power fields remain zero until their feature is verified.
  float temperatureC = 0.0f;
  float humidityPercent = 0.0f;
  float batteryVoltageV = 0.0f;
  float batteryCurrentA = 0.0f;
  float batteryPowerW = 0.0f;
  float latitudeDeg = 0.0f; // Optional navigation fields remain invalid until their hardware is enabled.
  float longitudeDeg = 0.0f;
  float groundSpeedMps = 0.0f;
  uint8_t satelliteCount = 0;
  bool gnssFix = false;
  bool opticalFlowValid = false;
  bool obstacleArrayValid = false;
  bool barometerValid = false;
  bool environmentValid = false;
  bool powerMonitorValid = false;
  bool navigationReady = false; // navigationReady summarizes whether all sensors needed by navigation are usable.
  bool imuHealthy = false; // IMU health and calibration are separate safety gates before arming.
  bool imuCalibrated = false;
};

struct MotorOutput { // MotorOutput stores the four ESC pulse widths in microseconds.
  uint16_t m1Us = 1000; // Every default is the safe 1000 microsecond stop command.
  uint16_t m2Us = 1000;
  uint16_t m3Us = 1000;
  uint16_t m4Us = 1000;
};

struct TelemetryFrame { // TelemetryFrame is the firmware snapshot serialized for the browser interface.
  uint32_t sequence = 0; // Sequence and timing fields help the interface detect delay and acknowledgements.
  uint32_t acknowledgedCommand = 0;
  uint32_t uptimeMs = 0;
  uint32_t commandAgeMs = UINT32_MAX;
  SensorState sensors; // Include the latest sensors, motor calculations, safety state, and selected mode.
  MotorOutput motors; // Execute this statement as part of the current subsystem operation.
  SafetyState safetyState = SafetyState::Boot;
  FlightMode mode = FlightMode::Manual;
  uint16_t throttle = 0;
  uint8_t connectedClients = 0; // Connection and safety flags allow the Flight Deck to present truthful controls.
  bool armed = false;
  bool failsafe = false;
  bool escProtocolConfirmed = false;
  char status[96] = "Boot"; // A short human-readable status explains the current lock or fault.
};

const char *safetyStateName(SafetyState state); // These helpers convert compact enum values into readable telemetry labels.
const char *flightModeName(FlightMode mode);

}  // namespace nanokit

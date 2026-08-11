#pragma once

// Arduino supplies fixed-width integer types and the UINT32_MAX constant.
#include <Arduino.h>

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// FlightMode identifies which command strategy the pilot requested.
enum class FlightMode : uint8_t { Manual = 0, AltitudeHold, PositionHold, Mission };
// SafetyState is the authoritative lifecycle used to decide whether motors may run.
enum class SafetyState : uint8_t {
  Boot = 0,
  Initializing,
  CalibrationRequired,
  Disarmed,
  Armed,
  Failsafe,
  Fault,
};

// PilotCommand is the complete command packet accepted from the Wi-Fi Flight Deck.
struct PilotCommand {
  // sequence and receivedAtMs support acknowledgements and stale-command detection.
  uint32_t sequence = 0;
  uint32_t receivedAtMs = 0;
  // throttle is a normalized pilot demand; axis targets use engineering units.
  uint16_t throttle = 0;
  float rollTargetDeg = 0.0f;
  float pitchTargetDeg = 0.0f;
  float yawRateTargetDps = 0.0f;
  // mode and action flags describe the requested operating state.
  FlightMode mode = FlightMode::Manual;
  bool armRequested = false;
  bool calibrateRequested = false;
  bool emergencyStop = false;
};

// AttitudeState holds orientation and angular rates produced by a validated IMU driver.
struct AttitudeState {
  // Euler angles are reported in degrees for telemetry and attitude control.
  float rollDeg = 0.0f;
  float pitchDeg = 0.0f;
  float yawDeg = 0.0f;
  // Angular rates are measured in degrees per second for rate control.
  float rollRateDps = 0.0f;
  float pitchRateDps = 0.0f;
  float yawRateDps = 0.0f;
  // valid prevents zero-filled or stale values from being treated as real measurements.
  bool valid = false;
};

// SensorState is the shared snapshot consumed by safety, control, and telemetry.
struct SensorState {
  AttitudeState attitude;
  // Optional environment and power fields remain zero until their feature is verified.
  float barometricAltitudeM = 0.0f;
  float temperatureC = 0.0f;
  float humidityPercent = 0.0f;
  float batteryVoltageV = 0.0f;
  float batteryCurrentA = 0.0f;
  float batteryPowerW = 0.0f;
  // Optional navigation fields remain invalid until their hardware is enabled.
  float latitudeDeg = 0.0f;
  float longitudeDeg = 0.0f;
  float groundSpeedMps = 0.0f;
  uint8_t satelliteCount = 0;
  bool gnssFix = false;
  bool opticalFlowValid = false;
  bool obstacleArrayValid = false;
  bool barometerValid = false;
  bool environmentValid = false;
  bool powerMonitorValid = false;
  // navigationReady summarizes whether all sensors needed by navigation are usable.
  bool navigationReady = false;
  // IMU health and calibration are separate safety gates before arming.
  bool imuHealthy = false;
  bool imuCalibrated = false;
};

// MotorOutput stores the four ESC pulse widths in microseconds.
struct MotorOutput {
  // Every default is the safe 1000 microsecond stop command.
  uint16_t m1Us = 1000;
  uint16_t m2Us = 1000;
  uint16_t m3Us = 1000;
  uint16_t m4Us = 1000;
};

// TelemetryFrame is the firmware snapshot serialized for the browser interface.
struct TelemetryFrame {
  // Sequence and timing fields help the interface detect delay and acknowledgements.
  uint32_t sequence = 0;
  uint32_t acknowledgedCommand = 0;
  uint32_t uptimeMs = 0;
  uint32_t commandAgeMs = UINT32_MAX;
  // Include the latest sensors, motor calculations, safety state, and selected mode.
  SensorState sensors;
  MotorOutput motors;
  SafetyState safetyState = SafetyState::Boot;
  FlightMode mode = FlightMode::Manual;
  uint16_t throttle = 0;
  // Connection and safety flags allow the Flight Deck to present truthful controls.
  uint8_t connectedClients = 0;
  bool armed = false;
  bool failsafe = false;
  bool escProtocolConfirmed = false;
  // A short human-readable status explains the current lock or fault.
  char status[96] = "Boot";
};

// These helpers convert compact enum values into readable telemetry labels.
const char *safetyStateName(SafetyState state);
const char *flightModeName(FlightMode mode);

}  // namespace nanokit

#pragma once

#include <Arduino.h>

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

enum class FlightMode : uint8_t { Manual = 0, AltitudeHold, PositionHold, Mission };
enum class SafetyState : uint8_t {
  Boot = 0,
  Initializing,
  CalibrationRequired,
  Disarmed,
  Armed,
  Failsafe,
  Fault,
};

struct PilotCommand {
  uint32_t sequence = 0;
  uint32_t receivedAtMs = 0;
  uint16_t throttle = 0;
  float rollTargetDeg = 0.0f;
  float pitchTargetDeg = 0.0f;
  float yawRateTargetDps = 0.0f;
  FlightMode mode = FlightMode::Manual;
  bool armRequested = false;
  bool calibrateRequested = false;
  bool emergencyStop = false;
};

struct AttitudeState {
  float rollDeg = 0.0f;
  float pitchDeg = 0.0f;
  float yawDeg = 0.0f;
  float rollRateDps = 0.0f;
  float pitchRateDps = 0.0f;
  float yawRateDps = 0.0f;
  bool valid = false;
};

struct SensorState {
  AttitudeState attitude;
  float barometricAltitudeM = 0.0f;
  float temperatureC = 0.0f;
  float humidityPercent = 0.0f;
  float batteryVoltageV = 0.0f;
  float batteryCurrentA = 0.0f;
  float batteryPowerW = 0.0f;
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
  bool navigationReady = false;
  bool imuHealthy = false;
  bool imuCalibrated = false;
};

struct MotorOutput {
  uint16_t m1Us = 1000;
  uint16_t m2Us = 1000;
  uint16_t m3Us = 1000;
  uint16_t m4Us = 1000;
};

struct TelemetryFrame {
  uint32_t sequence = 0;
  uint32_t acknowledgedCommand = 0;
  uint32_t uptimeMs = 0;
  uint32_t commandAgeMs = UINT32_MAX;
  SensorState sensors;
  MotorOutput motors;
  SafetyState safetyState = SafetyState::Boot;
  FlightMode mode = FlightMode::Manual;
  uint16_t throttle = 0;
  uint8_t connectedClients = 0;
  bool armed = false;
  bool failsafe = false;
  bool escProtocolConfirmed = false;
  char status[96] = "Boot";
};

const char *safetyStateName(SafetyState state);
const char *flightModeName(FlightMode mode);

}  // namespace nanokit

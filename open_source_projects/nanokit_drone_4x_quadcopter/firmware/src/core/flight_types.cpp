// Import the enum declarations that are converted to readable text here.
#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// Convert every SafetyState value into a stable protocol label.
const char *safetyStateName(SafetyState state) {
  // Explicit cases keep telemetry independent of compiler-specific enum formatting.
  switch (state) {
    case SafetyState::Boot: return "BOOT";
    case SafetyState::Initializing: return "INITIALIZING";
    case SafetyState::CalibrationRequired: return "CALIBRATION_REQUIRED";
    case SafetyState::Disarmed: return "DISARMED";
    case SafetyState::Armed: return "ARMED";
    case SafetyState::Failsafe: return "FAILSAFE";
    case SafetyState::Fault: return "FAULT";
  }
  // UNKNOWN protects diagnostics if a future state is not added to this switch.
  return "UNKNOWN";
}

// Convert every FlightMode value into a stable protocol label.
const char *flightModeName(FlightMode mode) {
  switch (mode) {
    case FlightMode::Manual: return "MANUAL";
    case FlightMode::AltitudeHold: return "ALTITUDE_HOLD";
    case FlightMode::PositionHold: return "POSITION_HOLD";
    case FlightMode::Mission: return "MISSION";
  }
  // Manual is the conservative fallback for an unrecognized mode.
  return "MANUAL";
}

}  // namespace nanokit

#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

const char *safetyStateName(SafetyState state) {
  switch (state) {
    case SafetyState::Boot: return "BOOT";
    case SafetyState::Initializing: return "INITIALIZING";
    case SafetyState::CalibrationRequired: return "CALIBRATION_REQUIRED";
    case SafetyState::Disarmed: return "DISARMED";
    case SafetyState::Armed: return "ARMED";
    case SafetyState::Failsafe: return "FAILSAFE";
    case SafetyState::Fault: return "FAULT";
  }
  return "UNKNOWN";
}

const char *flightModeName(FlightMode mode) {
  switch (mode) {
    case FlightMode::Manual: return "MANUAL";
    case FlightMode::AltitudeHold: return "ALTITUDE_HOLD";
    case FlightMode::PositionHold: return "POSITION_HOLD";
    case FlightMode::Mission: return "MISSION";
  }
  return "MANUAL";
}

}  // namespace nanokit

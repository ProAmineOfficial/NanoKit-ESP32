#include "core/flight_types.h" // Import the enum declarations that are converted to readable text here.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

const char *safetyStateName(SafetyState state) { // Convert every SafetyState value into a stable protocol label.
  switch (state) { // Explicit cases keep telemetry independent of compiler-specific enum formatting.
    case SafetyState::Boot: return "BOOT"; // Handle this specific switch value.
    case SafetyState::Initializing: return "INITIALIZING"; // Handle this specific switch value.
    case SafetyState::CalibrationRequired: return "CALIBRATION_REQUIRED"; // Handle this specific switch value.
    case SafetyState::Disarmed: return "DISARMED"; // Handle this specific switch value.
    case SafetyState::Armed: return "ARMED"; // Handle this specific switch value.
    case SafetyState::Failsafe: return "FAILSAFE"; // Handle this specific switch value.
    case SafetyState::Fault: return "FAULT"; // Handle this specific switch value.
  } // Close the current scope or type definition.
  return "UNKNOWN"; // UNKNOWN protects diagnostics if a future state is not added to this switch.
} // Close the current scope or type definition.

const char *flightModeName(FlightMode mode) { // Convert every FlightMode value into a stable protocol label.
  switch (mode) { // Select the handler that matches the current value.
    case FlightMode::Manual: return "MANUAL"; // Handle this specific switch value.
    case FlightMode::AltitudeHold: return "ALTITUDE_HOLD"; // Handle this specific switch value.
    case FlightMode::PositionHold: return "POSITION_HOLD"; // Handle this specific switch value.
    case FlightMode::Mission: return "MISSION"; // Handle this specific switch value.
  } // Close the current scope or type definition.
  return "MANUAL"; // Manual is the conservative fallback for an unrecognized mode.
} // Close the current scope or type definition.

}  // namespace nanokit

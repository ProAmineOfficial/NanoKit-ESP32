#pragma once // Prevent this header from being included more than once.

#include "core/flight_types.h" // Shared flight types define the safety state and command-related data.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

struct SafetyInputs { // SafetyInputs contains only the facts needed to make an arming decision.
  bool networkClientConnected = false; // A live client and a fresh command are both required for remote control.
  bool commandFresh = false; // Assign this value for the current control or telemetry operation.
  bool imuHealthy = false; // The IMU must be healthy, calibrated, and producing a valid attitude.
  bool imuCalibrated = false; // Assign this value for the current control or telemetry operation.
  bool attitudeValid = false; // Assign this value for the current control or telemetry operation.
  bool escProtocolConfirmed = false; // The exact ESC protocol must be confirmed before motor authority is possible.
  bool armRequested = false; // Pilot action flags and throttle complete the arming and stop gates.
  bool emergencyStop = false; // Assign this value for the current control or telemetry operation.
  uint16_t throttle = 0; // Assign this value for the current control or telemetry operation.
}; // Close the current scope or type definition.

class SafetyManager { // SafetyManager is the single authority that grants or removes the armed state.
 public: // Start this access-control section of the type.
  SafetyState update(const SafetyInputs &inputs); // Evaluate all current inputs and return the resulting safety state.
  SafetyState state() const { return state_; } // Read-only accessors expose decisions without allowing other modules to edit them.
  bool armed() const { return state_ == SafetyState::Armed; } // Continue this function declaration or call across this line.
  bool failsafe() const { return state_ == SafetyState::Failsafe; } // Continue this function declaration or call across this line.
  const char *status() const { return status_; } // Continue this function declaration or call across this line.

 private: // Start this access-control section of the type.
  void setState(SafetyState state, const char *status); // Update the enum and its matching readable explanation together.

  SafetyState state_ = SafetyState::Boot; // Boot is the safest initial state until every required check has run.
  bool armInhibit_ = true; // armInhibit_ requires the pilot to release Arm after a stop or failed attempt.
  char status_[96] = "Boot lock active"; // The fixed buffer avoids dynamic allocation in the real-time flight task.
}; // Close the current scope or type definition.

}  // namespace nanokit

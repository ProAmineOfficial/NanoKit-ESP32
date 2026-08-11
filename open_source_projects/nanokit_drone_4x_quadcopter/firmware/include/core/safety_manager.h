#pragma once

// Shared flight types define the safety state and command-related data.
#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// SafetyInputs contains only the facts needed to make an arming decision.
struct SafetyInputs {
  // A live client and a fresh command are both required for remote control.
  bool networkClientConnected = false;
  bool commandFresh = false;
  // The IMU must be healthy, calibrated, and producing a valid attitude.
  bool imuHealthy = false;
  bool imuCalibrated = false;
  bool attitudeValid = false;
  // The exact ESC protocol must be confirmed before motor authority is possible.
  bool escProtocolConfirmed = false;
  // Pilot action flags and throttle complete the arming and stop gates.
  bool armRequested = false;
  bool emergencyStop = false;
  uint16_t throttle = 0;
};

// SafetyManager is the single authority that grants or removes the armed state.
class SafetyManager {
 public:
  // Evaluate all current inputs and return the resulting safety state.
  SafetyState update(const SafetyInputs &inputs);
  // Read-only accessors expose decisions without allowing other modules to edit them.
  SafetyState state() const { return state_; }
  bool armed() const { return state_ == SafetyState::Armed; }
  bool failsafe() const { return state_ == SafetyState::Failsafe; }
  const char *status() const { return status_; }

 private:
  // Update the enum and its matching readable explanation together.
  void setState(SafetyState state, const char *status);

  // Boot is the safest initial state until every required check has run.
  SafetyState state_ = SafetyState::Boot;
  // armInhibit_ requires the pilot to release Arm after a stop or failed attempt.
  bool armInhibit_ = true;
  // The fixed buffer avoids dynamic allocation in the real-time flight task.
  char status_[96] = "Boot lock active";
};

}  // namespace nanokit

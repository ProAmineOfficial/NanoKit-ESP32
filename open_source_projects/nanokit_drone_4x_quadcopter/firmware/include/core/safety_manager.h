// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include "core/flight_types.h" // Shared flight types define the safety state and command-related data.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

struct SafetyInputs { // SafetyInputs contains only the facts needed to make an arming decision.
  bool networkClientConnected = false; // A live client and a fresh command are both required for remote control.
  bool commandFresh = false;
  bool imuHealthy = false; // The IMU must be healthy, calibrated, and producing a valid attitude.
  bool imuCalibrated = false;
  bool attitudeValid = false;
  bool escProtocolConfirmed = false; // The exact ESC protocol must be confirmed before motor authority is possible.
  bool armRequested = false; // Pilot action flags and throttle complete the arming and stop gates.
  bool emergencyStop = false;
  uint16_t throttle = 0;
};

class SafetyManager { // SafetyManager is the single authority that grants or removes the armed state.
 public:
  SafetyState update(const SafetyInputs &inputs); // Evaluate all current inputs and return the resulting safety state.
  SafetyState state() const { return state_; } // Read-only accessors expose decisions without allowing other modules to edit them.
  bool armed() const { return state_ == SafetyState::Armed; }
  bool failsafe() const { return state_ == SafetyState::Failsafe; }
  const char *status() const { return status_; }

 private:
  void setState(SafetyState state, const char *status); // Update the enum and its matching readable explanation together.

  SafetyState state_ = SafetyState::Boot; // Boot is the safest initial state until every required check has run.
  bool armInhibit_ = true; // armInhibit_ requires the pilot to release Arm after a stop or failed attempt.
  char status_[96] = "Boot lock active"; // The fixed buffer avoids dynamic allocation in the real-time flight task.
};

}  // namespace nanokit

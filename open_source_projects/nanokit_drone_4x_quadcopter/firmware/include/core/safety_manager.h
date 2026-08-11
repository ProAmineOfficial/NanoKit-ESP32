#pragma once

#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

struct SafetyInputs {
  bool networkClientConnected = false;
  bool commandFresh = false;
  bool imuHealthy = false;
  bool imuCalibrated = false;
  bool attitudeValid = false;
  bool escProtocolConfirmed = false;
  bool armRequested = false;
  bool emergencyStop = false;
  uint16_t throttle = 0;
};

class SafetyManager {
 public:
  SafetyState update(const SafetyInputs &inputs);
  SafetyState state() const { return state_; }
  bool armed() const { return state_ == SafetyState::Armed; }
  bool failsafe() const { return state_ == SafetyState::Failsafe; }
  const char *status() const { return status_; }

 private:
  void setState(SafetyState state, const char *status);

  SafetyState state_ = SafetyState::Boot;
  bool armInhibit_ = true;
  char status_[96] = "Boot lock active";
};

}  // namespace nanokit

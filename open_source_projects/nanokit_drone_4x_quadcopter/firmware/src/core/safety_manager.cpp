#include "core/safety_manager.h"

#include <cstring>

#include "config/board_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

SafetyState SafetyManager::update(const SafetyInputs &inputs) {
  if (inputs.emergencyStop) {
    armInhibit_ = true;
    setState(SafetyState::Failsafe, "Emergency stop latched; release ARM and keep throttle zero");
    return state_;
  }

  if (state_ == SafetyState::Armed &&
      (!inputs.networkClientConnected || !inputs.commandFresh)) {
    armInhibit_ = true;
    setState(SafetyState::Failsafe, "Command link lost; motors forced to minimum");
    return state_;
  }

  if (!inputs.imuHealthy || !inputs.attitudeValid) {
    armInhibit_ = true;
    setState(SafetyState::Fault, "ICM-20948 unavailable; flight control locked");
    return state_;
  }

  if (!inputs.imuCalibrated) {
    armInhibit_ = true;
    setState(SafetyState::CalibrationRequired, "IMU calibration required");
    return state_;
  }

  if (!inputs.escProtocolConfirmed) {
    armInhibit_ = true;
    setState(SafetyState::Fault, "ESC analogue PWM acceptance is not confirmed");
    return state_;
  }

  if (!inputs.networkClientConnected || !inputs.commandFresh) {
    setState(SafetyState::Disarmed, "Waiting for a fresh Flight Deck command link");
    return state_;
  }

  if (!inputs.armRequested && inputs.throttle <= config::ARM_THROTTLE_MAX) {
    armInhibit_ = false;
  }

  if (!inputs.armRequested) {
    setState(SafetyState::Disarmed, "Ready; hold ARM with throttle at zero");
    return state_;
  }

  if (inputs.throttle > config::ARM_THROTTLE_MAX) {
    armInhibit_ = true;
    setState(SafetyState::Disarmed, "Arm rejected: throttle is not zero");
    return state_;
  }

  if (armInhibit_) {
    setState(SafetyState::Disarmed, "Arm inhibited: release ARM once before retrying");
    return state_;
  }

  setState(SafetyState::Armed, "Armed: live command link and sensor gates valid");
  return state_;
}

void SafetyManager::setState(SafetyState state, const char *status) {
  state_ = state;
  std::strncpy(status_, status, sizeof(status_) - 1);
  status_[sizeof(status_) - 1] = '\0';
}

}  // namespace nanokit

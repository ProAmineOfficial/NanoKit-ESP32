// Import the SafetyManager contract and SafetyInputs structure.
#include "core/safety_manager.h"

// strncpy copies status text into the fixed diagnostic buffer.
#include <cstring>

// The zero-throttle arming limit comes from the shared board configuration.
#include "config/board_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// Evaluate safety checks in priority order so the most serious condition wins.
SafetyState SafetyManager::update(const SafetyInputs &inputs) {
  // Emergency stop has the highest priority and immediately latches the arm inhibit.
  if (inputs.emergencyStop) {
    armInhibit_ = true;
    setState(SafetyState::Failsafe, "Emergency stop latched; release ARM and keep throttle zero");
    return state_;
  }

  // Losing either the client or fresh commands while armed triggers a failsafe.
  if (state_ == SafetyState::Armed &&
      (!inputs.networkClientConnected || !inputs.commandFresh)) {
    armInhibit_ = true;
    setState(SafetyState::Failsafe, "Command link lost; motors forced to minimum");
    return state_;
  }

  // Flight control cannot operate without a healthy IMU and valid attitude.
  if (!inputs.imuHealthy || !inputs.attitudeValid) {
    armInhibit_ = true;
    setState(SafetyState::Fault, "ICM-20948 unavailable; flight control locked");
    return state_;
  }

  // Calibration is a separate gate because a responding IMU may still have biased data.
  if (!inputs.imuCalibrated) {
    armInhibit_ = true;
    setState(SafetyState::CalibrationRequired, "IMU calibration required");
    return state_;
  }

  // Never arm until the exact ESC has been bench-verified for analogue PWM.
  if (!inputs.escProtocolConfirmed) {
    armInhibit_ = true;
    setState(SafetyState::Fault, "ESC analogue PWM acceptance is not confirmed");
    return state_;
  }

  // A disconnected or stale link remains disarmed even when hardware is healthy.
  if (!inputs.networkClientConnected || !inputs.commandFresh) {
    setState(SafetyState::Disarmed, "Waiting for a fresh Flight Deck command link");
    return state_;
  }

  // Releasing Arm at zero throttle clears the latch before a new arming attempt.
  if (!inputs.armRequested && inputs.throttle <= config::ARM_THROTTLE_MAX) {
    armInhibit_ = false;
  }

  // A normal released Arm request means the system remains ready but disarmed.
  if (!inputs.armRequested) {
    setState(SafetyState::Disarmed, "Ready; hold ARM with throttle at zero");
    return state_;
  }

  // Reject any arming request made above the configured safe throttle threshold.
  if (inputs.throttle > config::ARM_THROTTLE_MAX) {
    armInhibit_ = true;
    setState(SafetyState::Disarmed, "Arm rejected: throttle is not zero");
    return state_;
  }

  // The inhibit forces a release-and-retry sequence after any stop or invalid attempt.
  if (armInhibit_) {
    setState(SafetyState::Disarmed, "Arm inhibited: release ARM once before retrying");
    return state_;
  }

  // Reaching this point means every hardware, link, command, and pilot gate passed.
  setState(SafetyState::Armed, "Armed: live command link and sensor gates valid");
  return state_;
}

// Store a state and safely copy its matching explanation into the fixed buffer.
void SafetyManager::setState(SafetyState state, const char *status) {
  state_ = state;
  // Reserve the final byte so the status string is always null-terminated.
  std::strncpy(status_, status, sizeof(status_) - 1);
  status_[sizeof(status_) - 1] = '\0';
}

}  // namespace nanokit

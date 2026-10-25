#include "core/safety_manager.h" // Import the SafetyManager contract and SafetyInputs structure.

#include <cstring> // strncpy copies status text into the fixed diagnostic buffer.

#include "config/board_config.h" // The zero-throttle arming limit comes from the shared board configuration.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

SafetyState SafetyManager::update(const SafetyInputs &inputs) { // Evaluate safety checks in priority order so the most serious condition wins.
  if (inputs.emergencyStop) { // Emergency stop has the highest priority and immediately latches the arm inhibit.
    armInhibit_ = true;
    setState(SafetyState::Failsafe, "Emergency stop latched; release ARM and keep throttle zero");
    return state_; // Return this result to the caller.
  }

  if (state_ == SafetyState::Armed && // Losing either the client or fresh commands while armed triggers a failsafe.
      (!inputs.networkClientConnected || !inputs.commandFresh)) { // Open this implementation block.
    armInhibit_ = true;
    setState(SafetyState::Failsafe, "Command link lost; motors forced to minimum");
    return state_; // Return this result to the caller.
  }

  if (!inputs.imuHealthy || !inputs.attitudeValid) { // Flight control cannot operate without a healthy IMU and valid attitude.
    armInhibit_ = true;
    setState(SafetyState::Fault, "ICM-20948 unavailable; flight control locked");
    return state_; // Return this result to the caller.
  }

  if (!inputs.imuCalibrated) { // Calibration is a separate gate because a responding IMU may still have biased data.
    armInhibit_ = true;
    setState(SafetyState::CalibrationRequired, "IMU calibration required");
    return state_; // Return this result to the caller.
  }

  if (!inputs.escProtocolConfirmed) { // Never arm until the exact ESC has been bench-verified for analogue PWM.
    armInhibit_ = true;
    setState(SafetyState::Fault, "ESC analogue PWM acceptance is not confirmed");
    return state_; // Return this result to the caller.
  }

  if (!inputs.networkClientConnected || !inputs.commandFresh) { // A disconnected or stale link remains disarmed even when hardware is healthy.
    setState(SafetyState::Disarmed, "Waiting for a fresh Flight Deck command link");
    return state_; // Return this result to the caller.
  }

  if (!inputs.armRequested && inputs.throttle <= config::ARM_THROTTLE_MAX) { // Releasing Arm at zero throttle clears the latch before a new arming attempt.
    armInhibit_ = false;
  }

  if (!inputs.armRequested) { // A normal released Arm request means the system remains ready but disarmed.
    setState(SafetyState::Disarmed, "Ready; hold ARM with throttle at zero");
    return state_; // Return this result to the caller.
  }

  if (inputs.throttle > config::ARM_THROTTLE_MAX) { // Reject any arming request made above the configured safe throttle threshold.
    armInhibit_ = true;
    setState(SafetyState::Disarmed, "Arm rejected: throttle is not zero");
    return state_; // Return this result to the caller.
  }

  if (armInhibit_) { // The inhibit forces a release-and-retry sequence after any stop or invalid attempt.
    setState(SafetyState::Disarmed, "Arm inhibited: release ARM once before retrying");
    return state_; // Return this result to the caller.
  }

  setState(SafetyState::Armed, "Armed: live command link and sensor gates valid"); // Reaching this point means every hardware, link, command, and pilot gate passed.
  return state_; // Return this result to the caller.
}

void SafetyManager::setState(SafetyState state, const char *status) { // Store a state and safely copy its matching explanation into the fixed buffer.
  state_ = state;
  std::strncpy(status_, status, sizeof(status_) - 1); // Reserve the final byte so the status string is always null-terminated.
  status_[sizeof(status_) - 1] = '\0';
}

}  // namespace nanokit

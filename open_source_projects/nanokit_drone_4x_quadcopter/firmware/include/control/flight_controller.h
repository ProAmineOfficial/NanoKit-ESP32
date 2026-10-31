// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include "control/pid_controller.h" // The flight controller combines reusable PID logic with shared flight data types.
#include "core/flight_types.h" // Import the dependency required by this module.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

struct ControlCorrection { // This structure carries the three axis corrections produced during one control cycle.
  float roll = 0.0f; // Roll correction changes the left-to-right motor balance.
  float pitch = 0.0f; // Pitch correction changes the front-to-rear motor balance.
  float yaw = 0.0f; // Yaw correction changes the clockwise-to-counter-clockwise motor balance.
};

class FlightController { // FlightController converts pilot targets and measured attitude into mixer corrections.
 public:
  ControlCorrection update(const PilotCommand &command, const SensorState &sensors, // Run one PID update; armed is supplied so control output can be blocked safely.
                           float dtSeconds, bool armed); // Execute this statement as part of the current subsystem operation.
  void reset(); // Clear stored PID history whenever control is disarmed or invalid.

 private:
  PidController rollPid_{4.2f, 0.04f, 0.45f, 180.0f}; // Roll and pitch use equal tuning because the Quad-X frame is designed symmetrically.
  PidController pitchPid_{4.2f, 0.04f, 0.45f, 180.0f}; // Execute this statement as part of the current subsystem operation.
  PidController yawRatePid_{1.8f, 0.02f, 0.04f, 120.0f}; // Yaw controls angular rate and therefore uses separate, lower gains.
};

}  // namespace nanokit

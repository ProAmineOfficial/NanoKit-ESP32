#pragma once // Prevent this header from being included more than once.

#include "control/pid_controller.h" // The flight controller combines reusable PID logic with shared flight data types.
#include "core/flight_types.h" // Import the dependency required by this module.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

struct ControlCorrection { // This structure carries the three axis corrections produced during one control cycle.
  float roll = 0.0f; // Roll correction changes the left-to-right motor balance.
  float pitch = 0.0f; // Pitch correction changes the front-to-rear motor balance.
  float yaw = 0.0f; // Yaw correction changes the clockwise-to-counter-clockwise motor balance.
}; // Close the current scope or type definition.

class FlightController { // FlightController converts pilot targets and measured attitude into mixer corrections.
 public: // Start this access-control section of the type.
  ControlCorrection update(const PilotCommand &command, const SensorState &sensors, // Run one PID update; armed is supplied so control output can be blocked safely.
                           float dtSeconds, bool armed); // Execute this statement as part of the current subsystem operation.
  void reset(); // Clear stored PID history whenever control is disarmed or invalid.

 private: // Start this access-control section of the type.
  PidController rollPid_{4.2f, 0.04f, 0.45f, 180.0f}; // Roll and pitch use equal tuning because the Quad-X frame is designed symmetrically.
  PidController pitchPid_{4.2f, 0.04f, 0.45f, 180.0f}; // Execute this statement as part of the current subsystem operation.
  PidController yawRatePid_{1.8f, 0.02f, 0.04f, 120.0f}; // Yaw controls angular rate and therefore uses separate, lower gains.
}; // Close the current scope or type definition.

}  // namespace nanokit

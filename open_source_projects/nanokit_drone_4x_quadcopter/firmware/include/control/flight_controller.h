#pragma once

// The flight controller combines reusable PID logic with shared flight data types.
#include "control/pid_controller.h"
#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// This structure carries the three axis corrections produced during one control cycle.
struct ControlCorrection {
  // Roll correction changes the left-to-right motor balance.
  float roll = 0.0f;
  // Pitch correction changes the front-to-rear motor balance.
  float pitch = 0.0f;
  // Yaw correction changes the clockwise-to-counter-clockwise motor balance.
  float yaw = 0.0f;
};

// FlightController converts pilot targets and measured attitude into mixer corrections.
class FlightController {
 public:
  // Run one PID update; armed is supplied so control output can be blocked safely.
  ControlCorrection update(const PilotCommand &command, const SensorState &sensors,
                           float dtSeconds, bool armed);
  // Clear stored PID history whenever control is disarmed or invalid.
  void reset();

 private:
  // Roll and pitch use equal tuning because the Quad-X frame is designed symmetrically.
  PidController rollPid_{4.2f, 0.04f, 0.45f, 180.0f};
  PidController pitchPid_{4.2f, 0.04f, 0.45f, 180.0f};
  // Yaw controls angular rate and therefore uses separate, lower gains.
  PidController yawRatePid_{1.8f, 0.02f, 0.04f, 120.0f};
};

}  // namespace nanokit

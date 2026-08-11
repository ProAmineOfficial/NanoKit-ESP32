#pragma once

#include "control/pid_controller.h"
#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

struct ControlCorrection {
  float roll = 0.0f;
  float pitch = 0.0f;
  float yaw = 0.0f;
};

class FlightController {
 public:
  ControlCorrection update(const PilotCommand &command, const SensorState &sensors,
                           float dtSeconds, bool armed);
  void reset();

 private:
  PidController rollPid_{4.2f, 0.04f, 0.45f, 180.0f};
  PidController pitchPid_{4.2f, 0.04f, 0.45f, 180.0f};
  PidController yawRatePid_{1.8f, 0.02f, 0.04f, 120.0f};
};

}  // namespace nanokit

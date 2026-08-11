#include "control/flight_controller.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

ControlCorrection FlightController::update(const PilotCommand &command,
                                           const SensorState &sensors,
                                           float dtSeconds, bool armed) {
  if (!armed || !sensors.imuHealthy || !sensors.imuCalibrated ||
      !sensors.attitude.valid) {
    reset();
    return ControlCorrection{};
  }

  ControlCorrection correction;
  correction.roll = rollPid_.update(command.rollTargetDeg,
                                    sensors.attitude.rollDeg, dtSeconds);
  correction.pitch = pitchPid_.update(command.pitchTargetDeg,
                                      sensors.attitude.pitchDeg, dtSeconds);
  correction.yaw = yawRatePid_.update(command.yawRateTargetDps,
                                      sensors.attitude.yawRateDps, dtSeconds);
  return correction;
}

void FlightController::reset() {
  rollPid_.reset();
  pitchPid_.reset();
  yawRatePid_.reset();
}

}  // namespace nanokit

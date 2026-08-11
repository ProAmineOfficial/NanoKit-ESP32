// Import the FlightController contract and its embedded PID members.
#include "control/flight_controller.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// Produce corrections only from a healthy, calibrated, and valid attitude source.
ControlCorrection FlightController::update(const PilotCommand &command,
                                           const SensorState &sensors,
                                           float dtSeconds, bool armed) {
  // Reset PID memory whenever safety removes control authority or the IMU is invalid.
  if (!armed || !sensors.imuHealthy || !sensors.imuCalibrated ||
      !sensors.attitude.valid) {
    reset();
    return ControlCorrection{};
  }

  // Begin with zero corrections, then update each independent control axis.
  ControlCorrection correction;
  // Roll and pitch compare target angles with measured angles.
  correction.roll = rollPid_.update(command.rollTargetDeg,
                                    sensors.attitude.rollDeg, dtSeconds);
  correction.pitch = pitchPid_.update(command.pitchTargetDeg,
                                      sensors.attitude.pitchDeg, dtSeconds);
  // Yaw compares a requested angular rate with the measured yaw rate.
  correction.yaw = yawRatePid_.update(command.yawRateTargetDps,
                                      sensors.attitude.yawRateDps, dtSeconds);
  // Return the three corrections to the Quad-X motor mixer.
  return correction;
}

// Clear every axis together so stale integral or derivative state cannot survive disarming.
void FlightController::reset() {
  rollPid_.reset();
  pitchPid_.reset();
  yawRatePid_.reset();
}

}  // namespace nanokit

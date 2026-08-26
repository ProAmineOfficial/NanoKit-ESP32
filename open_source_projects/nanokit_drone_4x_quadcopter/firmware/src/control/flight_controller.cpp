#include "control/flight_controller.h" // Import the FlightController contract and its embedded PID members.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

ControlCorrection FlightController::update(const PilotCommand &command, // Produce corrections only from a healthy, calibrated, and valid attitude source.
                                           const SensorState &sensors, // Continue the current project declaration or implementation.
                                           float dtSeconds, bool armed) { // Open this implementation block.
  if (!armed || !sensors.imuHealthy || !sensors.imuCalibrated || // Reset PID memory whenever safety removes control authority or the IMU is invalid.
      !sensors.attitude.valid) { // Open this implementation block.
    reset(); // Continue this function declaration or call across this line.
    return ControlCorrection{}; // Return this result to the caller.
  } // Close the current scope or type definition.

  ControlCorrection correction; // Begin with zero corrections, then update each independent control axis.
  correction.roll = rollPid_.update(command.rollTargetDeg, // Roll and pitch compare target angles with measured angles.
                                    sensors.attitude.rollDeg, dtSeconds); // Execute this statement as part of the current subsystem operation.
  correction.pitch = pitchPid_.update(command.pitchTargetDeg, // Continue this function declaration or call across this line.
                                      sensors.attitude.pitchDeg, dtSeconds); // Execute this statement as part of the current subsystem operation.
  correction.yaw = yawRatePid_.update(command.yawRateTargetDps, // Yaw compares a requested angular rate with the measured yaw rate.
                                      sensors.attitude.yawRateDps, dtSeconds); // Execute this statement as part of the current subsystem operation.
  return correction; // Return the three corrections to the Quad-X motor mixer.
} // Close the current scope or type definition.

void FlightController::reset() { // Clear every axis together so stale integral or derivative state cannot survive disarming.
  rollPid_.reset(); // Continue this function declaration or call across this line.
  pitchPid_.reset(); // Continue this function declaration or call across this line.
  yawRatePid_.reset(); // Continue this function declaration or call across this line.
} // Close the current scope or type definition.

}  // namespace nanokit

#pragma once // Prevent this header from being included more than once.

#include "core/flight_types.h" // SensorState is the truthful data contract shared with safety and telemetry.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

class SensorHub { // SensorHub owns sensor initialization and publishes one coherent sensor snapshot.
 public: // Start this access-control section of the type.
  void begin(); // Start the confirmed I2C bus and any feature-gated sensor drivers.
  void update(float dtSeconds); // Refresh enabled sensors using the flight-loop time step.
  void requestImuCalibration(); // Ask the verified IMU driver to perform calibration when one is available.
  const SensorState &state() const { return state_; } // Return const references so callers cannot invent or overwrite sensor data.
  const char *status() const { return status_; } // Continue this function declaration or call across this line.

 private: // Start this access-control section of the type.
  SensorState state_; // state_ remains invalid until real hardware produces a successful reading.
  char status_[96] = "Sensor hub not initialized"; // A fixed diagnostic buffer explains unavailable or failed hardware.
}; // Close the current scope or type definition.

}  // namespace nanokit

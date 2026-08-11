#pragma once

// SensorState is the truthful data contract shared with safety and telemetry.
#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// SensorHub owns sensor initialization and publishes one coherent sensor snapshot.
class SensorHub {
 public:
  // Start the confirmed I2C bus and any feature-gated sensor drivers.
  void begin();
  // Refresh enabled sensors using the flight-loop time step.
  void update(float dtSeconds);
  // Ask the verified IMU driver to perform calibration when one is available.
  void requestImuCalibration();
  // Return const references so callers cannot invent or overwrite sensor data.
  const SensorState &state() const { return state_; }
  const char *status() const { return status_; }

 private:
  // state_ remains invalid until real hardware produces a successful reading.
  SensorState state_;
  // A fixed diagnostic buffer explains unavailable or failed hardware.
  char status_[96] = "Sensor hub not initialized";
};

}  // namespace nanokit

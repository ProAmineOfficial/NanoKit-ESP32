#pragma once

#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

class SensorHub {
 public:
  void begin();
  void update(float dtSeconds);
  void requestImuCalibration();
  const SensorState &state() const { return state_; }
  const char *status() const { return status_; }

 private:
  SensorState state_;
  char status_[96] = "Sensor hub not initialized";
};

}  // namespace nanokit

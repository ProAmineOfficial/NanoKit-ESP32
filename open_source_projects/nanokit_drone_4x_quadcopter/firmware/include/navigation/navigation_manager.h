#pragma once

#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

class NavigationManager {
 public:
  FlightMode validateRequestedMode(FlightMode requested, const SensorState &sensors) const;
  const char *status() const { return status_; }

 private:
  mutable const char *status_ = "Manual mode";
};

}  // namespace nanokit

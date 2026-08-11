#pragma once

// FlightMode and SensorState are required to validate requested navigation modes.
#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// NavigationManager rejects modes whose sensors or compile-time features are unavailable.
class NavigationManager {
 public:
  // Return either the requested safe mode or Manual as the conservative fallback.
  FlightMode validateRequestedMode(FlightMode requested, const SensorState &sensors) const;
  // Report why a requested mode was accepted or rejected.
  const char *status() const { return status_; }

 private:
  // mutable permits a const validation function to update its diagnostic message.
  mutable const char *status_ = "Manual mode";
};

}  // namespace nanokit

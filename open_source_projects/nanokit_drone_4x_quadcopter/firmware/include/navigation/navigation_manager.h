#pragma once

#include "core/flight_types.h" // FlightMode and SensorState are required to validate requested navigation modes.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

class NavigationManager { // NavigationManager rejects modes whose sensors or compile-time features are unavailable.
 public:
  FlightMode validateRequestedMode(FlightMode requested, const SensorState &sensors) const; // Return either the requested safe mode or Manual as the conservative fallback.
  const char *status() const { return status_; } // Report why a requested mode was accepted or rejected.

 private:
  mutable const char *status_ = "Manual mode"; // mutable permits a const validation function to update its diagnostic message.
};

}  // namespace nanokit

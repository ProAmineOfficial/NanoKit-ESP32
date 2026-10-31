// SPDX-License-Identifier: AGPL-3.0-only
#include "navigation/navigation_manager.h" // Import the mode validation interface.

#include "config/feature_flags.h" // Feature flags define which advanced flight modes are compiled as available.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

FlightMode NavigationManager::validateRequestedMode(FlightMode requested, // Validate a requested mode against both compile-time features and live sensor validity.
                                                    const SensorState &sensors) const { // Open this implementation block.
  switch (requested) { // Each advanced mode has its own minimum sensor requirement.
    case FlightMode::AltitudeHold: // Handle this specific switch value.
#if NANOKIT_FEATURE_ALTITUDE_HOLD // Remove altitude-hold code entirely until its feature has been validated.
      if (sensors.imuHealthy) { // Evaluate this condition before continuing.
        status_ = "Altitude hold available";
        return requested; // Return this result to the caller.
      }
#endif // Close the compile-time feature selection.
      status_ = "Altitude hold unavailable: required sensors are disabled";
      break; // Leave the current switch case or loop.
    case FlightMode::PositionHold: // Handle this specific switch value.
#if NANOKIT_FEATURE_POSITION_HOLD // Position hold depends on an explicitly valid optical-flow measurement.
      if (sensors.opticalFlowValid) { // Evaluate this condition before continuing.
        status_ = "Position hold available";
        return requested; // Return this result to the caller.
      }
#endif // Close the compile-time feature selection.
      status_ = "Position hold unavailable: optical flow is disabled";
      break; // Leave the current switch case or loop.
    case FlightMode::Mission: // Handle this specific switch value.
#if NANOKIT_FEATURE_NAVIGATION // Mission mode requires validated navigation support and a current GNSS fix.
      if (sensors.gnssFix) { // Evaluate this condition before continuing.
        status_ = "Mission mode available";
        return requested; // Return this result to the caller.
      }
#endif // Close the compile-time feature selection.
      status_ = "Mission mode unavailable: GNSS navigation is disabled";
      break; // Leave the current switch case or loop.
    case FlightMode::Manual: // Handle this specific switch value.
    default: // Handle any switch value not matched above.
      status_ = "Manual mode"; // Manual is always available and is the fallback for unknown values.
      return FlightMode::Manual; // Return this result to the caller.
  }
  return FlightMode::Manual; // Any rejected advanced mode returns Manual instead of partially enabling it.
}

}  // namespace nanokit

// Import the mode validation interface.
#include "navigation/navigation_manager.h"

// Feature flags define which advanced flight modes are compiled as available.
#include "config/feature_flags.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// Validate a requested mode against both compile-time features and live sensor validity.
FlightMode NavigationManager::validateRequestedMode(FlightMode requested,
                                                    const SensorState &sensors) const {
  // Each advanced mode has its own minimum sensor requirement.
  switch (requested) {
    case FlightMode::AltitudeHold:
// Remove altitude-hold code entirely until its feature has been validated.
#if NANOKIT_FEATURE_ALTITUDE_HOLD
      if (sensors.imuHealthy) {
        status_ = "Altitude hold available";
        return requested;
      }
#endif
      status_ = "Altitude hold unavailable: required sensors are disabled";
      break;
    case FlightMode::PositionHold:
// Position hold depends on an explicitly valid optical-flow measurement.
#if NANOKIT_FEATURE_POSITION_HOLD
      if (sensors.opticalFlowValid) {
        status_ = "Position hold available";
        return requested;
      }
#endif
      status_ = "Position hold unavailable: optical flow is disabled";
      break;
    case FlightMode::Mission:
// Mission mode requires validated navigation support and a current GNSS fix.
#if NANOKIT_FEATURE_NAVIGATION
      if (sensors.gnssFix) {
        status_ = "Mission mode available";
        return requested;
      }
#endif
      status_ = "Mission mode unavailable: GNSS navigation is disabled";
      break;
    case FlightMode::Manual:
    default:
      // Manual is always available and is the fallback for unknown values.
      status_ = "Manual mode";
      return FlightMode::Manual;
  }
  // Any rejected advanced mode returns Manual instead of partially enabling it.
  return FlightMode::Manual;
}

}  // namespace nanokit

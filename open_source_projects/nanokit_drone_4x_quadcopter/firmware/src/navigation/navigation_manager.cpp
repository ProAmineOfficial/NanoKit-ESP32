#include "navigation/navigation_manager.h"

#include "config/feature_flags.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

FlightMode NavigationManager::validateRequestedMode(FlightMode requested,
                                                    const SensorState &sensors) const {
  switch (requested) {
    case FlightMode::AltitudeHold:
#if NANOKIT_FEATURE_ALTITUDE_HOLD
      if (sensors.imuHealthy) {
        status_ = "Altitude hold available";
        return requested;
      }
#endif
      status_ = "Altitude hold unavailable: required sensors are disabled";
      break;
    case FlightMode::PositionHold:
#if NANOKIT_FEATURE_POSITION_HOLD
      if (sensors.opticalFlowValid) {
        status_ = "Position hold available";
        return requested;
      }
#endif
      status_ = "Position hold unavailable: optical flow is disabled";
      break;
    case FlightMode::Mission:
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
      status_ = "Manual mode";
      return FlightMode::Manual;
  }
  return FlightMode::Manual;
}

}  // namespace nanokit

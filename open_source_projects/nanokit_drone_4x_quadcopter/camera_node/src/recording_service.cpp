#include "recording_service.h" // Import the RecordingService public contract.

#include "camera_node_config.h" // The microSD feature flag blocks unverified SPI wiring.

namespace nanokit_camera { // Developed by Amine Saoud ibn al-Bashir.

void RecordingService::begin() { // Publish storage availability without touching unknown SPI pins.
#if NANOKIT_CAMERA_SD_ENABLED // Only an explicitly enabled build enters the future microSD integration path.
  status_ = "microSD enabled but verified SPI pin map is still required"; // Assign this value for the current control or telemetry operation.
#else // Select the alternative compile-time feature branch.
  status_ = "microSD disabled pending verified SPI pins"; // Explain why recording endpoints remain unavailable in the current safe build.
#endif // Close the compile-time feature selection.
  ready_ = false; // Never report ready until a real card and filesystem initialization succeeds.
} // Close the current scope or type definition.

}  // namespace nanokit_camera

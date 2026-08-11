// Import the RecordingService public contract.
#include "recording_service.h"

// The microSD feature flag blocks unverified SPI wiring.
#include "camera_node_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

// Publish storage availability without touching unknown SPI pins.
void RecordingService::begin() {
// Only an explicitly enabled build enters the future microSD integration path.
#if NANOKIT_CAMERA_SD_ENABLED
  status_ = "microSD enabled but verified SPI pin map is still required";
#else
  // Explain why recording endpoints remain unavailable in the current safe build.
  status_ = "microSD disabled pending verified SPI pins";
#endif
  // Never report ready until a real card and filesystem initialization succeeds.
  ready_ = false;
}

}  // namespace nanokit_camera

#include "recording_service.h"

#include "camera_node_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

void RecordingService::begin() {
#if NANOKIT_CAMERA_SD_ENABLED
  status_ = "microSD enabled but verified SPI pin map is still required";
#else
  status_ = "microSD disabled pending verified SPI pins";
#endif
  ready_ = false;
}

}  // namespace nanokit_camera

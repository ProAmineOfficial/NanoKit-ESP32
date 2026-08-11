#include "camera_service.h"

#include "camera_node_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

void CameraService::begin() {
  psramBytes_ = ESP.getPsramSize();
  psramValid_ = psramFound() && psramBytes_ >= config::REQUIRED_PSRAM_BYTES;
#if NANOKIT_CAMERA_OV2640_ENABLED
  if (!psramValid_) {
    ready_ = false;
    status_ = "OV2640 blocked: 8 MB PSRAM was not detected";
    return;
  }
  // TODO: add the verified OV2640 pin map and esp_camera configuration.
  ready_ = false;
  status_ = "OV2640 enabled but verified pin map is still required";
#else
  ready_ = false;
  status_ = psramValid_
      ? "OV2640 disabled pending verified camera pin map"
      : "OV2640 disabled; 8 MB PSRAM is also not confirmed";
#endif
}

}  // namespace nanokit_camera

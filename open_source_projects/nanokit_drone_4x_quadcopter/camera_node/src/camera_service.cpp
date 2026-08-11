// Import the CameraService public contract.
#include "camera_service.h"

// Camera feature flags and the required PSRAM capacity come from one configuration.
#include "camera_node_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

// Validate memory first, then apply the compile-time OV2640 integration gate.
void CameraService::begin() {
  // Read the ESP32's actual external RAM capacity instead of assuming the module type.
  psramBytes_ = ESP.getPsramSize();
  // Both a detected PSRAM device and at least 8 MB are required for this design.
  psramValid_ = psramFound() && psramBytes_ >= config::REQUIRED_PSRAM_BYTES;
// Camera driver code is excluded until the feature is intentionally enabled.
#if NANOKIT_CAMERA_OV2640_ENABLED
  // Refuse camera startup if the physical NanoKit does not meet the memory requirement.
  if (!psramValid_) {
    ready_ = false;
    status_ = "OV2640 blocked: 8 MB PSRAM was not detected";
    return;
  }
  // TODO: add the verified OV2640 pin map and esp_camera configuration.
  // Even an enabled build stays unavailable until the exact camera pins are added.
  ready_ = false;
  status_ = "OV2640 enabled but verified pin map is still required";
#else
  // Keep the status precise by reporting both feature and PSRAM validation state.
  ready_ = false;
  status_ = psramValid_
      ? "OV2640 disabled pending verified camera pin map"
      : "OV2640 disabled; 8 MB PSRAM is also not confirmed";
#endif
}

}  // namespace nanokit_camera

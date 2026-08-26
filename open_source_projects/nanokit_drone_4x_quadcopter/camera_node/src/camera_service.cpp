#include "camera_service.h" // Import the CameraService public contract.

#include "camera_node_config.h" // Camera feature flags and the required PSRAM capacity come from one configuration.

namespace nanokit_camera { // Developed by Amine Saoud ibn al-Bashir.

void CameraService::begin() { // Validate memory first, then apply the compile-time OV2640 integration gate.
  psramBytes_ = ESP.getPsramSize(); // Read the ESP32's actual external RAM capacity instead of assuming the module type.
  psramValid_ = psramFound() && psramBytes_ >= config::REQUIRED_PSRAM_BYTES; // Both a detected PSRAM device and at least 8 MB are required for this design.
#if NANOKIT_CAMERA_OV2640_ENABLED // Camera driver code is excluded until the feature is intentionally enabled.
  if (!psramValid_) { // Refuse camera startup if the physical NanoKit does not meet the memory requirement.
    ready_ = false; // Assign this value for the current control or telemetry operation.
    status_ = "OV2640 blocked: 8 MB PSRAM was not detected"; // Assign this value for the current control or telemetry operation.
    return; // Return this result to the caller.
  } // Close the current scope or type definition.
  ready_ = false; // TODO: add the verified OV2640 pin map and esp_camera configuration. Even an enabled build stays unavailable until the exact camera pins are added.
  status_ = "OV2640 enabled but verified pin map is still required"; // Assign this value for the current control or telemetry operation.
#else // Select the alternative compile-time feature branch.
  ready_ = false; // Keep the status precise by reporting both feature and PSRAM validation state.
  status_ = psramValid_ // Assign this value for the current control or telemetry operation.
      ? "OV2640 disabled pending verified camera pin map" // Continue the current project declaration or implementation.
      : "OV2640 disabled; 8 MB PSRAM is also not confirmed"; // Execute this statement as part of the current subsystem operation.
#endif // Close the compile-time feature selection.
} // Close the current scope or type definition.

}  // namespace nanokit_camera

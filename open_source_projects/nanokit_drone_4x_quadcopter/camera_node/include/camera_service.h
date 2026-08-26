#pragma once // Prevent this header from being included more than once.

#include <Arduino.h> // Arduino supplies ESP32 runtime types and PSRAM helper APIs used by the implementation.

namespace nanokit_camera { // Developed by Amine Saoud ibn al-Bashir.

class CameraService { // CameraService owns camera readiness and the mandatory PSRAM validation gate.
 public: // Start this access-control section of the type.
  void begin(); // Inspect PSRAM and initialize OV2640 only when its feature and pins are verified.
  bool ready() const { return ready_; } // Read-only accessors feed truthful camera-node status responses.
  bool psramValid() const { return psramValid_; } // Continue this function declaration or call across this line.
  size_t psramBytes() const { return psramBytes_; } // Continue this function declaration or call across this line.
  const char *status() const { return status_; } // Continue this function declaration or call across this line.

 private: // Start this access-control section of the type.
  bool ready_ = false; // Camera output is unavailable until every runtime and compile-time gate passes.
  bool psramValid_ = false; // Assign this value for the current control or telemetry operation.
  size_t psramBytes_ = 0; // Record the measured capacity so diagnostics show the actual module result.
  const char *status_ = "Camera not initialized"; // Assign this value for the current control or telemetry operation.
}; // Close the current scope or type definition.

}  // namespace nanokit_camera

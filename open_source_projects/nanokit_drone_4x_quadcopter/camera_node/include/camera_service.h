#pragma once

// Arduino supplies ESP32 runtime types and PSRAM helper APIs used by the implementation.
#include <Arduino.h>

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

// CameraService owns camera readiness and the mandatory PSRAM validation gate.
class CameraService {
 public:
  // Inspect PSRAM and initialize OV2640 only when its feature and pins are verified.
  void begin();
  // Read-only accessors feed truthful camera-node status responses.
  bool ready() const { return ready_; }
  bool psramValid() const { return psramValid_; }
  size_t psramBytes() const { return psramBytes_; }
  const char *status() const { return status_; }

 private:
  // Camera output is unavailable until every runtime and compile-time gate passes.
  bool ready_ = false;
  bool psramValid_ = false;
  // Record the measured capacity so diagnostics show the actual module result.
  size_t psramBytes_ = 0;
  const char *status_ = "Camera not initialized";
};

}  // namespace nanokit_camera

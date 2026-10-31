// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <Arduino.h> // Arduino supplies ESP32 runtime types and PSRAM helper APIs used by the implementation.

namespace nanokit_camera { // Developed by Amine Saoud ibn al-Bashir.

class CameraService { // CameraService owns camera readiness and the mandatory PSRAM validation gate.
 public:
  void begin(); // Inspect PSRAM and initialize OV2640 only when its feature and pins are verified.
  bool ready() const { return ready_; } // Read-only accessors feed truthful camera-node status responses.
  bool psramValid() const { return psramValid_; }
  size_t psramBytes() const { return psramBytes_; }
  const char *status() const { return status_; }

 private:
  bool ready_ = false; // Camera output is unavailable until every runtime and compile-time gate passes.
  bool psramValid_ = false;
  size_t psramBytes_ = 0; // Record the measured capacity so diagnostics show the actual module result.
  const char *status_ = "Camera not initialized";
};

}  // namespace nanokit_camera

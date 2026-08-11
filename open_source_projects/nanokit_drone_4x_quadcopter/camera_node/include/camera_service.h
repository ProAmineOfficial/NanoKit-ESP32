#pragma once

#include <Arduino.h>

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

class CameraService {
 public:
  void begin();
  bool ready() const { return ready_; }
  bool psramValid() const { return psramValid_; }
  size_t psramBytes() const { return psramBytes_; }
  const char *status() const { return status_; }

 private:
  bool ready_ = false;
  bool psramValid_ = false;
  size_t psramBytes_ = 0;
  const char *status_ = "Camera not initialized";
};

}  // namespace nanokit_camera

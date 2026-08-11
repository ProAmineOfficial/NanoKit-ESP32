#pragma once

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

class RecordingService {
 public:
  void begin();
  bool ready() const { return ready_; }
  bool recording() const { return false; }
  const char *status() const { return status_; }

 private:
  bool ready_ = false;
  const char *status_ = "microSD disabled pending verified SPI pins";
};

}  // namespace nanokit_camera

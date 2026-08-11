#pragma once

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

class AudioService {
 public:
  void begin();
  bool captureReady() const { return captureReady_; }
  bool playbackReady() const { return playbackReady_; }
  const char *status() const { return status_; }

 private:
  bool captureReady_ = false;
  bool playbackReady_ = false;
  const char *status_ = "Audio disabled pending verified I2S pins";
};

}  // namespace nanokit_camera

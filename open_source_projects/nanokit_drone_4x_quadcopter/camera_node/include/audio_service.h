#pragma once

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

// AudioService reports the availability of separate input and output audio paths.
class AudioService {
 public:
  // Initialize only feature-gated I2S hardware and publish truthful readiness flags.
  void begin();
  // These accessors keep the HTTP status API independent of driver details.
  bool captureReady() const { return captureReady_; }
  bool playbackReady() const { return playbackReady_; }
  const char *status() const { return status_; }

 private:
  // Both paths start unavailable because their exact I2S pins are not confirmed.
  bool captureReady_ = false;
  bool playbackReady_ = false;
  // The status text explains the integration gate to the user interface.
  const char *status_ = "Audio disabled pending verified I2S pins";
};

}  // namespace nanokit_camera

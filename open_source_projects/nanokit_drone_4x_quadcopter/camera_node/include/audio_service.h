// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

namespace nanokit_camera { // Developed by Amine Saoud ibn al-Bashir.

class AudioService { // AudioService reports the availability of separate input and output audio paths.
 public:
  void begin(); // Initialize only feature-gated I2S hardware and publish truthful readiness flags.
  bool captureReady() const { return captureReady_; } // These accessors keep the HTTP status API independent of driver details.
  bool playbackReady() const { return playbackReady_; }
  const char *status() const { return status_; }

 private:
  bool captureReady_ = false; // Both paths start unavailable because their exact I2S pins are not confirmed.
  bool playbackReady_ = false;
  const char *status_ = "Audio disabled pending verified I2S pins"; // The status text explains the integration gate to the user interface.
};

}  // namespace nanokit_camera

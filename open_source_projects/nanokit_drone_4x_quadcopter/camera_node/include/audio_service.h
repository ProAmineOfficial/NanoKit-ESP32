#pragma once // Prevent this header from being included more than once.

namespace nanokit_camera { // Developed by Amine Saoud ibn al-Bashir.

class AudioService { // AudioService reports the availability of separate input and output audio paths.
 public: // Start this access-control section of the type.
  void begin(); // Initialize only feature-gated I2S hardware and publish truthful readiness flags.
  bool captureReady() const { return captureReady_; } // These accessors keep the HTTP status API independent of driver details.
  bool playbackReady() const { return playbackReady_; } // Continue this function declaration or call across this line.
  const char *status() const { return status_; } // Continue this function declaration or call across this line.

 private: // Start this access-control section of the type.
  bool captureReady_ = false; // Both paths start unavailable because their exact I2S pins are not confirmed.
  bool playbackReady_ = false; // Assign this value for the current control or telemetry operation.
  const char *status_ = "Audio disabled pending verified I2S pins"; // The status text explains the integration gate to the user interface.
}; // Close the current scope or type definition.

}  // namespace nanokit_camera

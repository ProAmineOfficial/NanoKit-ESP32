#include "audio_service.h" // Import the AudioService public contract.

#include "camera_node_config.h" // Feature flags decide whether verified I2S integration may be compiled.

namespace nanokit_camera { // Developed by Amine Saoud ibn al-Bashir.

void AudioService::begin() { // Publish audio availability without starting unverified pins or drivers.
#if NANOKIT_AUDIO_INMP441_ENABLED || NANOKIT_AUDIO_MAX98357A_ENABLED // This branch remains guarded until at least one audio path is explicitly enabled.
  status_ = "Audio feature enabled but verified I2S pin map is still required";
#else // Select the alternative compile-time feature branch.
  status_ = "Audio disabled pending verified INMP441/MAX98357A I2S pins"; // Explain that both microphone and amplifier pin maps remain unverified.
#endif // Close the compile-time feature selection.
  captureReady_ = false; // Never claim readiness before successful real hardware initialization.
  playbackReady_ = false;
}

}  // namespace nanokit_camera

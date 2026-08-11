// Import the AudioService public contract.
#include "audio_service.h"

// Feature flags decide whether verified I2S integration may be compiled.
#include "camera_node_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

// Publish audio availability without starting unverified pins or drivers.
void AudioService::begin() {
// This branch remains guarded until at least one audio path is explicitly enabled.
#if NANOKIT_AUDIO_INMP441_ENABLED || NANOKIT_AUDIO_MAX98357A_ENABLED
  status_ = "Audio feature enabled but verified I2S pin map is still required";
#else
  // Explain that both microphone and amplifier pin maps remain unverified.
  status_ = "Audio disabled pending verified INMP441/MAX98357A I2S pins";
#endif
  // Never claim readiness before successful real hardware initialization.
  captureReady_ = false;
  playbackReady_ = false;
}

}  // namespace nanokit_camera

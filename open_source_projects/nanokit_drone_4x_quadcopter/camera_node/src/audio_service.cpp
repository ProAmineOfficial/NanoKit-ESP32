#include "audio_service.h"

#include "camera_node_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

void AudioService::begin() {
#if NANOKIT_AUDIO_INMP441_ENABLED || NANOKIT_AUDIO_MAX98357A_ENABLED
  status_ = "Audio feature enabled but verified I2S pin map is still required";
#else
  status_ = "Audio disabled pending verified INMP441/MAX98357A I2S pins";
#endif
  captureReady_ = false;
  playbackReady_ = false;
}

}  // namespace nanokit_camera

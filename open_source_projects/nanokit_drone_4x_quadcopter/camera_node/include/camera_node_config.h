#pragma once

#include <Arduino.h>

// Developed by Amine Saoud ibn al-Bashir.
// No GPIO is assigned here until the dedicated camera/audio pin map is verified.

#define NANOKIT_CAMERA_OV2640_ENABLED 0
#define NANOKIT_CAMERA_SERVO_ENABLED 0
#define NANOKIT_CAMERA_SD_ENABLED 0
#define NANOKIT_AUDIO_INMP441_ENABLED 0
#define NANOKIT_AUDIO_MAX98357A_ENABLED 0
#define NANOKIT_CAMERA_BH1750_ENABLED 0

namespace nanokit_camera {
namespace config {

constexpr char WIFI_SSID[] = "NanoKit-Drone-4X";
constexpr char WIFI_PASSWORD[] = "NanoKit4X";
constexpr char MDNS_NAME[] = "nanokit-camera";
constexpr uint16_t HTTP_PORT = 80;
constexpr uint32_t WIFI_RETRY_MS = 5000;
constexpr size_t REQUIRED_PSRAM_BYTES = 8UL * 1024UL * 1024UL;

}  // namespace config
}  // namespace nanokit_camera

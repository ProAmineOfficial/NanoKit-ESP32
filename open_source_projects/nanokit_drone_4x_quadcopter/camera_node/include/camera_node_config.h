#pragma once

// Arduino supplies size_t and fixed-width integer types for this configuration.
#include <Arduino.h>

// Developed by Amine Saoud ibn al-Bashir.
// No GPIO is assigned here until the dedicated camera/audio pin map is verified.

// Every optional peripheral remains disabled until its wiring and voltage are confirmed.
#define NANOKIT_CAMERA_OV2640_ENABLED 0
#define NANOKIT_CAMERA_SERVO_ENABLED 0
#define NANOKIT_CAMERA_SD_ENABLED 0
// Audio input and output require separately verified I2S pins.
#define NANOKIT_AUDIO_INMP441_ENABLED 0
#define NANOKIT_AUDIO_MAX98357A_ENABLED 0
#define NANOKIT_CAMERA_BH1750_ENABLED 0

// Network and hardware limits are grouped under a dedicated camera-node namespace.
namespace nanokit_camera {
namespace config {

// The camera node joins the flight controller's existing local access point.
constexpr char WIFI_SSID[] = "NanoKit-Drone-4X";
constexpr char WIFI_PASSWORD[] = "NanoKit4X";
// mDNS provides the optional nanokit-camera.local status endpoint on supported clients.
constexpr char MDNS_NAME[] = "nanokit-camera";
// The camera status and future stream endpoints use the standard HTTP port.
constexpr uint16_t HTTP_PORT = 80;
// Retry slowly enough to avoid a tight reconnect loop when the flight controller is offline.
constexpr uint32_t WIFI_RETRY_MS = 5000;
// OV2640 plus audio and storage requires a physically verified 8 MB PSRAM device.
constexpr size_t REQUIRED_PSRAM_BYTES = 8UL * 1024UL * 1024UL;

}  // namespace config
}  // namespace nanokit_camera

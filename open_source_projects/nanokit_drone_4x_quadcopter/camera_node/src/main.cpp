// NanoKit Drone 4X camera and audio node.
// Developed by Amine Saoud ibn al-Bashir.

// Arduino supplies setup(), loop(), serial output, timing, and String.
#include <Arduino.h>
// mDNS publishes an optional human-readable local hostname.
#include <ESPmDNS.h>
// WebServer exposes status and future payload-control endpoints.
#include <WebServer.h>
// WiFi joins the flight controller's isolated NanoKit SoftAP as a station.
#include <WiFi.h>

// Each payload service owns one optional hardware responsibility.
#include "audio_service.h"
// Shared node configuration holds feature gates, credentials, and PSRAM limits.
#include "camera_node_config.h"
#include "camera_service.h"
#include "recording_service.h"

// Keep implementation globals private to this firmware file.
namespace {

// Use camera-node types without repeating their namespace in local declarations.
using namespace nanokit_camera;

// The camera node exposes one HTTP server and one service object per payload type.
WebServer server(config::HTTP_PORT);
CameraService cameraService;
AudioService audioService;
RecordingService recordingService;
// These values prevent duplicate server startup and pace Wi-Fi reconnection attempts.
bool serverStarted = false;
uint32_t lastWifiAttemptMs = 0;

// Send one JSON response with browser-safe CORS and no-cache headers.
void sendJson(int status, const String &body) {
  // CORS allows the separate Flight Deck host to read this payload node.
  server.sendHeader("Access-Control-Allow-Origin", "*");
  // Status and controls must never use a stale cached response.
  server.sendHeader("Cache-Control", "no-store");
  server.send(status, "application/json", body);
}

// Build a compact status document from real service readiness values.
String statusDocument() {
  // String concatenation is acceptable here because status requests are low frequency.
  String body = "{";
  // Identify this endpoint as the separate camera node and report its assigned IP.
  body += "\"device\":\"NanoKit-Drone-4X-Camera\",";
  body += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  body += "\"psramBytes\":" + String(cameraService.psramBytes()) + ",";
  // Boolean fields describe actual memory, camera, audio, and storage gates.
  body += "\"psram8MB\":" + String(cameraService.psramValid() ? "true" : "false") + ",";
  body += "\"cameraReady\":" + String(cameraService.ready() ? "true" : "false") + ",";
  body += "\"audioCaptureReady\":" + String(audioService.captureReady() ? "true" : "false") + ",";
  body += "\"audioPlaybackReady\":" + String(audioService.playbackReady() ? "true" : "false") + ",";
  body += "\"storageReady\":" + String(recordingService.ready() ? "true" : "false") + ",";
  // The payload node explicitly declares that it can never control motors.
  body += "\"motorAuthority\":false";
  body += "}";
  return body;
}

// Return a structured service-unavailable response for a disabled payload feature.
void unavailable(const char *feature, const char *reason) {
  String body = "{\"ok\":false,\"feature\":\"";
  body += feature;
  body += "\",\"reason\":\"";
  body += reason;
  body += "\"}";
  // HTTP 503 accurately means the endpoint exists but its hardware is unavailable.
  sendJson(503, body);
}

// Register status and feature endpoints before the server begins listening.
void configureRoutes() {
  // Root and /api/status provide the same diagnostic JSON for simple testing.
  server.on("/", HTTP_GET, []() {
    sendJson(200, statusDocument());
  });
  server.on("/api/status", HTTP_GET, []() {
    sendJson(200, statusDocument());
  });
  // Camera endpoints stay present but report their real feature-gate status.
  server.on("/stream", HTTP_GET, []() {
    unavailable("camera", cameraService.status());
  });
  server.on("/api/camera", HTTP_POST, []() {
    unavailable("camera", cameraService.status());
  });
  // Audio and recording controls are isolated from all flight-control functions.
  server.on("/api/audio", HTTP_POST, []() {
    unavailable("audio", audioService.status());
  });
  server.on("/api/recording", HTTP_POST, []() {
    unavailable("recording", recordingService.status());
  });
  // Servo is rejected explicitly because its pin and power path are unverified.
  server.on("/api/servo", HTTP_POST, []() {
    unavailable("servo", "Servo pin and power interface are not verified");
  });
  // Unknown resources return JSON instead of an ambiguous empty response.
  server.onNotFound([]() {
    sendJson(404, "{\"ok\":false,\"reason\":\"Not found\"}");
  });
}

// Start mDNS and HTTP once, only after the station has a valid Wi-Fi connection.
void startServerWhenConnected() {
  // Return early if startup already happened or the station is still offline.
  if (serverStarted || WiFi.status() != WL_CONNECTED) return;
  // mDNS is optional; HTTP still starts if hostname publication fails.
  if (MDNS.begin(config::MDNS_NAME)) {
    MDNS.addService("http", "tcp", config::HTTP_PORT);
  }
  // Mark startup after begin() so loop() can safely service HTTP requests.
  server.begin();
  serverStarted = true;
  Serial.print("[CAM] Status API: http://");
  Serial.println(WiFi.localIP());
}

// Maintain the station connection without blocking the camera node's main loop.
void maintainWifi() {
  // A connected station only needs the one-time server-start check.
  if (WiFi.status() == WL_CONNECTED) {
    startServerWhenConnected();
    return;
  }
  // Pace reconnect attempts to avoid continuous radio and serial activity.
  const uint32_t now = millis();
  if (now - lastWifiAttemptMs < config::WIFI_RETRY_MS) return;
  lastWifiAttemptMs = now;
  // Clear stale station state before joining the flight controller SoftAP again.
  WiFi.disconnect();
  WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
  Serial.println("[CAM] Joining NanoKit-Drone-4X SoftAP...");
}

}  // namespace

// Arduino calls setup() once to initialize diagnostics, services, routes, and Wi-Fi.
void setup() {
  // Match PlatformIO monitor speed and allow the USB-UART bridge to settle.
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("NanoKit Drone 4X - Camera + Audio Node");
  Serial.println("Developed by Amine Saoud ibn al-Bashir.");
  Serial.println("[SAFE] This node has no motor-control authority.");

  // Initialize each service; disabled hardware returns a diagnostic instead of fake data.
  cameraService.begin();
  audioService.begin();
  recordingService.begin();
  // Print every gate result for direct bench diagnosis over serial.
  Serial.print("[CAM] ");
  Serial.println(cameraService.status());
  Serial.print("[AUDIO] ");
  Serial.println(audioService.status());
  Serial.print("[STORE] ");
  Serial.println(recordingService.status());

  // Configure HTTP before enabling station mode and beginning connection attempts.
  configureRoutes();
  WiFi.mode(WIFI_STA);
  maintainWifi();
}

// Arduino calls loop() continuously to maintain Wi-Fi and serve HTTP cooperatively.
void loop() {
  maintainWifi();
  // Do not service the server object until begin() has completed successfully.
  if (serverStarted) server.handleClient();
  // A short delay yields CPU time to the ESP32 Wi-Fi stack.
  delay(2);
}

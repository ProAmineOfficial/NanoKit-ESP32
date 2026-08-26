#include <Arduino.h> // NanoKit Drone 4X camera and audio node. Developed by Amine Saoud ibn al-Bashir. Arduino supplies setup(), loop(), serial output, timing, and String.
#include <ESPmDNS.h> // mDNS publishes an optional human-readable local hostname.
#include <WebServer.h> // WebServer exposes status and future payload-control endpoints.
#include <WiFi.h> // WiFi joins the flight controller's isolated NanoKit SoftAP as a station.

#include "audio_service.h" // Each payload service owns one optional hardware responsibility.
#include "camera_node_config.h" // Shared node configuration holds feature gates, credentials, and PSRAM limits.
#include "camera_service.h" // Import the dependency required by this module.
#include "recording_service.h" // Import the dependency required by this module.

namespace { // Keep implementation globals private to this firmware file.

using namespace nanokit_camera; // Use camera-node types without repeating their namespace in local declarations.

WebServer server(config::HTTP_PORT); // The camera node exposes one HTTP server and one service object per payload type.
CameraService cameraService; // Execute this statement as part of the current subsystem operation.
AudioService audioService; // Execute this statement as part of the current subsystem operation.
RecordingService recordingService; // Execute this statement as part of the current subsystem operation.
bool serverStarted = false; // These values prevent duplicate server startup and pace Wi-Fi reconnection attempts.
uint32_t lastWifiAttemptMs = 0; // Assign this value for the current control or telemetry operation.

void sendJson(int status, const String &body) { // Send one JSON response with browser-safe CORS and no-cache headers.
  server.sendHeader("Access-Control-Allow-Origin", "*"); // CORS allows the separate Flight Deck host to read this payload node.
  server.sendHeader("Cache-Control", "no-store"); // Status and controls must never use a stale cached response.
  server.send(status, "application/json", body); // Continue this function declaration or call across this line.
} // Close the current scope or type definition.

String statusDocument() { // Build a compact status document from real service readiness values.
  String body = "{"; // String concatenation is acceptable here because status requests are low frequency.
  body += "\"device\":\"NanoKit-Drone-4X-Camera\","; // Identify this endpoint as the separate camera node and report its assigned IP.
  body += "\"ip\":\"" + WiFi.localIP().toString() + "\","; // Continue this function declaration or call across this line.
  body += "\"psramBytes\":" + String(cameraService.psramBytes()) + ","; // Continue this function declaration or call across this line.
  body += "\"psram8MB\":" + String(cameraService.psramValid() ? "true" : "false") + ","; // Boolean fields describe actual memory, camera, audio, and storage gates.
  body += "\"cameraReady\":" + String(cameraService.ready() ? "true" : "false") + ","; // Continue this function declaration or call across this line.
  body += "\"audioCaptureReady\":" + String(audioService.captureReady() ? "true" : "false") + ","; // Continue this function declaration or call across this line.
  body += "\"audioPlaybackReady\":" + String(audioService.playbackReady() ? "true" : "false") + ","; // Continue this function declaration or call across this line.
  body += "\"storageReady\":" + String(recordingService.ready() ? "true" : "false") + ","; // Continue this function declaration or call across this line.
  body += "\"motorAuthority\":false"; // The payload node explicitly declares that it can never control motors.
  body += "}"; // Assign this value for the current control or telemetry operation.
  return body; // Return this result to the caller.
} // Close the current scope or type definition.

void unavailable(const char *feature, const char *reason) { // Return a structured service-unavailable response for a disabled payload feature.
  String body = "{\"ok\":false,\"feature\":\""; // Assign this value for the current control or telemetry operation.
  body += feature; // Assign this value for the current control or telemetry operation.
  body += "\",\"reason\":\""; // Assign this value for the current control or telemetry operation.
  body += reason; // Assign this value for the current control or telemetry operation.
  body += "\"}"; // Assign this value for the current control or telemetry operation.
  sendJson(503, body); // HTTP 503 accurately means the endpoint exists but its hardware is unavailable.
} // Close the current scope or type definition.

void configureRoutes() { // Register status and feature endpoints before the server begins listening.
  server.on("/", HTTP_GET, []() { // Root and /api/status provide the same diagnostic JSON for simple testing.
    sendJson(200, statusDocument()); // Continue this function declaration or call across this line.
  }); // Close the current scope or type definition.
  server.on("/api/status", HTTP_GET, []() { // Open this implementation block.
    sendJson(200, statusDocument()); // Continue this function declaration or call across this line.
  }); // Close the current scope or type definition.
  server.on("/stream", HTTP_GET, []() { // Camera endpoints stay present but report their real feature-gate status.
    unavailable("camera", cameraService.status()); // Continue this function declaration or call across this line.
  }); // Close the current scope or type definition.
  server.on("/api/camera", HTTP_POST, []() { // Open this implementation block.
    unavailable("camera", cameraService.status()); // Continue this function declaration or call across this line.
  }); // Close the current scope or type definition.
  server.on("/api/audio", HTTP_POST, []() { // Audio and recording controls are isolated from all flight-control functions.
    unavailable("audio", audioService.status()); // Continue this function declaration or call across this line.
  }); // Close the current scope or type definition.
  server.on("/api/recording", HTTP_POST, []() { // Open this implementation block.
    unavailable("recording", recordingService.status()); // Continue this function declaration or call across this line.
  }); // Close the current scope or type definition.
  server.on("/api/servo", HTTP_POST, []() { // Servo is rejected explicitly because its pin and power path are unverified.
    unavailable("servo", "Servo pin and power interface are not verified"); // Continue this function declaration or call across this line.
  }); // Close the current scope or type definition.
  server.onNotFound([]() { // Unknown resources return JSON instead of an ambiguous empty response.
    sendJson(404, "{\"ok\":false,\"reason\":\"Not found\"}"); // Continue this function declaration or call across this line.
  }); // Close the current scope or type definition.
} // Close the current scope or type definition.

void startServerWhenConnected() { // Start mDNS and HTTP once, only after the station has a valid Wi-Fi connection.
  if (serverStarted || WiFi.status() != WL_CONNECTED) return; // Return early if startup already happened or the station is still offline.
  if (MDNS.begin(config::MDNS_NAME)) { // mDNS is optional; HTTP still starts if hostname publication fails.
    MDNS.addService("http", "tcp", config::HTTP_PORT); // Continue this function declaration or call across this line.
  } // Close the current scope or type definition.
  server.begin(); // Mark startup after begin() so loop() can safely service HTTP requests.
  serverStarted = true; // Assign this value for the current control or telemetry operation.
  Serial.print("[CAM] Status API: http://"); // Print the HTTP scheme before the camera node IP address.
  Serial.println(WiFi.localIP()); // Continue this function declaration or call across this line.
} // Close the current scope or type definition.

void maintainWifi() { // Maintain the station connection without blocking the camera node's main loop.
  if (WiFi.status() == WL_CONNECTED) { // A connected station only needs the one-time server-start check.
    startServerWhenConnected(); // Continue this function declaration or call across this line.
    return; // Return this result to the caller.
  } // Close the current scope or type definition.
  const uint32_t now = millis(); // Pace reconnect attempts to avoid continuous radio and serial activity.
  if (now - lastWifiAttemptMs < config::WIFI_RETRY_MS) return; // Evaluate this condition before continuing.
  lastWifiAttemptMs = now; // Assign this value for the current control or telemetry operation.
  WiFi.disconnect(); // Clear stale station state before joining the flight controller SoftAP again.
  WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD); // Continue this function declaration or call across this line.
  Serial.println("[CAM] Joining NanoKit-Drone-4X SoftAP..."); // Continue this function declaration or call across this line.
} // Close the current scope or type definition.

}  // namespace

void setup() { // Arduino calls setup() once to initialize diagnostics, services, routes, and Wi-Fi.
  Serial.begin(115200); // Match PlatformIO monitor speed and allow the USB-UART bridge to settle.
  delay(200); // Continue this function declaration or call across this line.
  Serial.println(); // Continue this function declaration or call across this line.
  Serial.println("NanoKit Drone 4X - Camera + Audio Node"); // Continue this function declaration or call across this line.
  Serial.println("Developed by Amine Saoud ibn al-Bashir."); // Continue this function declaration or call across this line.
  Serial.println("[SAFE] This node has no motor-control authority."); // Continue this function declaration or call across this line.

  cameraService.begin(); // Initialize each service; disabled hardware returns a diagnostic instead of fake data.
  audioService.begin(); // Continue this function declaration or call across this line.
  recordingService.begin(); // Continue this function declaration or call across this line.
  Serial.print("[CAM] "); // Print every gate result for direct bench diagnosis over serial.
  Serial.println(cameraService.status()); // Continue this function declaration or call across this line.
  Serial.print("[AUDIO] "); // Continue this function declaration or call across this line.
  Serial.println(audioService.status()); // Continue this function declaration or call across this line.
  Serial.print("[STORE] "); // Continue this function declaration or call across this line.
  Serial.println(recordingService.status()); // Continue this function declaration or call across this line.

  configureRoutes(); // Configure HTTP before enabling station mode and beginning connection attempts.
  WiFi.mode(WIFI_STA); // Continue this function declaration or call across this line.
  maintainWifi(); // Continue this function declaration or call across this line.
} // Close the current scope or type definition.

void loop() { // Arduino calls loop() continuously to maintain Wi-Fi and serve HTTP cooperatively.
  maintainWifi(); // Continue this function declaration or call across this line.
  if (serverStarted) server.handleClient(); // Do not service the server object until begin() has completed successfully.
  delay(2); // A short delay yields CPU time to the ESP32 Wi-Fi stack.
} // Close the current scope or type definition.

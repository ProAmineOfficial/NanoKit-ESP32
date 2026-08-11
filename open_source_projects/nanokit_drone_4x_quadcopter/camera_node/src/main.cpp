// NanoKit Drone 4X camera and audio node.
// Developed by Amine Saoud ibn al-Bashir.

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>

#include "audio_service.h"
#include "camera_node_config.h"
#include "camera_service.h"
#include "recording_service.h"

namespace {

using namespace nanokit_camera;

WebServer server(config::HTTP_PORT);
CameraService cameraService;
AudioService audioService;
RecordingService recordingService;
bool serverStarted = false;
uint32_t lastWifiAttemptMs = 0;

void sendJson(int status, const String &body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");
  server.send(status, "application/json", body);
}

String statusDocument() {
  String body = "{";
  body += "\"device\":\"NanoKit-Drone-4X-Camera\",";
  body += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  body += "\"psramBytes\":" + String(cameraService.psramBytes()) + ",";
  body += "\"psram8MB\":" + String(cameraService.psramValid() ? "true" : "false") + ",";
  body += "\"cameraReady\":" + String(cameraService.ready() ? "true" : "false") + ",";
  body += "\"audioCaptureReady\":" + String(audioService.captureReady() ? "true" : "false") + ",";
  body += "\"audioPlaybackReady\":" + String(audioService.playbackReady() ? "true" : "false") + ",";
  body += "\"storageReady\":" + String(recordingService.ready() ? "true" : "false") + ",";
  body += "\"motorAuthority\":false";
  body += "}";
  return body;
}

void unavailable(const char *feature, const char *reason) {
  String body = "{\"ok\":false,\"feature\":\"";
  body += feature;
  body += "\",\"reason\":\"";
  body += reason;
  body += "\"}";
  sendJson(503, body);
}

void configureRoutes() {
  server.on("/", HTTP_GET, []() {
    sendJson(200, statusDocument());
  });
  server.on("/api/status", HTTP_GET, []() {
    sendJson(200, statusDocument());
  });
  server.on("/stream", HTTP_GET, []() {
    unavailable("camera", cameraService.status());
  });
  server.on("/api/camera", HTTP_POST, []() {
    unavailable("camera", cameraService.status());
  });
  server.on("/api/audio", HTTP_POST, []() {
    unavailable("audio", audioService.status());
  });
  server.on("/api/recording", HTTP_POST, []() {
    unavailable("recording", recordingService.status());
  });
  server.on("/api/servo", HTTP_POST, []() {
    unavailable("servo", "Servo pin and power interface are not verified");
  });
  server.onNotFound([]() {
    sendJson(404, "{\"ok\":false,\"reason\":\"Not found\"}");
  });
}

void startServerWhenConnected() {
  if (serverStarted || WiFi.status() != WL_CONNECTED) return;
  if (MDNS.begin(config::MDNS_NAME)) {
    MDNS.addService("http", "tcp", config::HTTP_PORT);
  }
  server.begin();
  serverStarted = true;
  Serial.print("[CAM] Status API: http://");
  Serial.println(WiFi.localIP());
}

void maintainWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    startServerWhenConnected();
    return;
  }
  const uint32_t now = millis();
  if (now - lastWifiAttemptMs < config::WIFI_RETRY_MS) return;
  lastWifiAttemptMs = now;
  WiFi.disconnect();
  WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
  Serial.println("[CAM] Joining NanoKit-Drone-4X SoftAP...");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("NanoKit Drone 4X - Camera + Audio Node");
  Serial.println("Developed by Amine Saoud ibn al-Bashir.");
  Serial.println("[SAFE] This node has no motor-control authority.");

  cameraService.begin();
  audioService.begin();
  recordingService.begin();
  Serial.print("[CAM] ");
  Serial.println(cameraService.status());
  Serial.print("[AUDIO] ");
  Serial.println(audioService.status());
  Serial.print("[STORE] ");
  Serial.println(recordingService.status());

  configureRoutes();
  WiFi.mode(WIFI_STA);
  maintainWifi();
}

void loop() {
  maintainWifi();
  if (serverStarted) server.handleClient();
  delay(2);
}

// Import the FlightLink contract and its HTTP/WebSocket server members.
#include "network/flight_link.h"

// LittleFS serves the packaged Flight Deck files from ESP32 flash.
#include <LittleFS.h>
// WiFi creates the local NanoKit SoftAP used by the browser controller.
#include <WiFi.h>
// Standard utilities provide bounds, number conversion, formatting, and safe copying.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Shared network ports, protocol limits, and command bounds come from board configuration.
#include "config/board_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

namespace {

// Clamp a floating command so remote input cannot exceed configured flight limits.
float clampFloat(float value, float minimum, float maximum) {
  return std::max(minimum, std::min(maximum, value));
}

// Normalize throttle to the protocol's safe 0-1000 range.
uint16_t clampThrottle(long value) {
  return static_cast<uint16_t>(std::max(0L, std::min(1000L, value)));
}

// Convert the text protocol mode into the internal enum, defaulting to Manual.
FlightMode parseMode(const char *value) {
  if (std::strcmp(value, "ALTITUDE_HOLD") == 0) return FlightMode::AltitudeHold;
  if (std::strcmp(value, "POSITION_HOLD") == 0) return FlightMode::PositionHold;
  if (std::strcmp(value, "MISSION") == 0) return FlightMode::Mission;
  return FlightMode::Manual;
}

}  // namespace

// Construct both servers on their fixed configuration ports.
FlightLink::FlightLink()
    : httpServer_(config::HTTP_PORT), webSocket_(config::WEBSOCKET_PORT) {}

// Start the filesystem, isolated Wi-Fi access point, HTTP routes, and WebSocket service.
void FlightLink::begin(QueueHandle_t commandQueue, QueueHandle_t telemetryQueue) {
  // Save queue handles only after setup has successfully allocated them.
  commandQueue_ = commandQueue;
  telemetryQueue_ = telemetryQueue;

  // Mount LittleFS and format only if its existing filesystem cannot be mounted.
  filesystemReady_ = LittleFS.begin(true);
  // AP mode makes the NanoKit the local network host instead of requiring a router.
  WiFi.mode(WIFI_AP);
  // A fixed 192.168.4.1/24 address gives the Flight Deck a predictable endpoint.
  const IPAddress localIp(192, 168, 4, 1);
  const IPAddress gateway(192, 168, 4, 1);
  const IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(localIp, gateway, subnet);
  // Start a visible WPA2 access point with at most four connected stations.
  WiFi.softAP(config::ACCESS_POINT_SSID, config::ACCESS_POINT_PASSWORD,
              config::ACCESS_POINT_CHANNEL, false, 4);

  // Configure routes before accepting HTTP requests.
  configureHttpRoutes();
  httpServer_.begin();

  // Register one callback for WebSocket connection, disconnection, and command events.
  webSocket_.begin();
  webSocket_.onEvent([this](uint8_t client, WStype_t type, uint8_t *payload, size_t length) {
    onWebSocketEvent(client, type, payload, length);
  });

  // Serial output provides the exact network endpoint for bench diagnostics.
  Serial.println("[NET] NanoKit Flight Deck SoftAP ready.");
  Serial.print("[NET] SSID: ");
  Serial.println(config::ACCESS_POINT_SSID);
  Serial.print("[NET] HTTP: http://");
  Serial.println(WiFi.softAPIP());
  Serial.print("[NET] WebSocket port: ");
  Serial.println(config::WEBSOCKET_PORT);
  // A missing filesystem does not affect motor safety, but the web interface cannot load.
  if (!filesystemReady_) {
    Serial.println("[NET] LittleFS unavailable; upload the web_controller filesystem image.");
  }
}

// Service both servers and transmit the newest queued telemetry without blocking.
void FlightLink::loop() {
  httpServer_.handleClient();
  webSocket_.loop();

  // Send a frame only when the flight task has published a newer snapshot.
  TelemetryFrame frame;
  if (telemetryQueue_ != nullptr && xQueueReceive(telemetryQueue_, &frame, 0) == pdTRUE) {
    broadcastTelemetry(frame);
  }
}

// Handle one WebSocket event from one browser client.
void FlightLink::onWebSocketEvent(uint8_t client, WStype_t type,
                                  uint8_t *payload, size_t length) {
  // Connection events update safety state; text events carry commands.
  switch (type) {
    case WStype_CONNECTED:
      // Saturate at 255 because the telemetry field is one byte.
      if (connectedClients_ < 255) ++connectedClients_;
      // The HELLO packet declares identity, transport, protocol, and locked startup state.
      webSocket_.sendTXT(client,
          "PROTO=3;TYPE=HELLO;DEVICE=NanoKit-Drone-4X;TRANSPORT=WIFI;SAFETY=LOCKED;");
      break;
    case WStype_DISCONNECTED:
      // Avoid unsigned underflow if a duplicate disconnect event arrives.
      if (connectedClients_ > 0) --connectedClients_;
      break;
    case WStype_TEXT: {
      // Parse into a local command so invalid input never changes the shared queue.
      PilotCommand command;
      if (parseCommand(payload, length, command) && commandQueue_ != nullptr) {
        // Keep only the newest valid command for the flight task.
        xQueueOverwrite(commandQueue_, &command);
        // Acknowledge the exact sequence so the interface can measure command progress.
        char acknowledgement[96];
        std::snprintf(acknowledgement, sizeof(acknowledgement),
                      "PROTO=3;TYPE=ACK;SEQ=%lu;", static_cast<unsigned long>(command.sequence));
        webSocket_.sendTXT(client, acknowledgement);
      } else {
        // Return a structured protocol error without attempting partial execution.
        webSocket_.sendTXT(client,
            "PROTO=3;TYPE=ERROR;CODE=BAD_COMMAND;MESSAGE=Command rejected;");
      }
      break;
    }
    default:
      break;
  }
}

// Parse one semicolon-separated command packet and enforce protocol bounds.
bool FlightLink::parseCommand(const uint8_t *payload, size_t length,
                              PilotCommand &command) const {
  // Reject null, empty, or oversized payloads before copying into a fixed stack buffer.
  if (payload == nullptr || length == 0 || length >= 384) return false;

  // Copy WebSocket bytes and add the terminator required by C string tokenization.
  char buffer[384];
  std::memcpy(buffer, payload, length);
  buffer[length] = '\0';

  // Both the protocol version and CMD packet type are mandatory.
  bool protocolValid = false;
  bool commandType = false;
  char *context = nullptr;
  // Split the packet into KEY=VALUE fields without using heap allocation.
  for (char *token = strtok_r(buffer, ";", &context); token != nullptr;
       token = strtok_r(nullptr, ";", &context)) {
    // Ignore malformed fields that do not contain an equals separator.
    char *separator = std::strchr(token, '=');
    if (separator == nullptr) continue;
    *separator = '\0';
    const char *key = token;
    const char *value = separator + 1;

    // Parse known fields only; every numeric flight command is clamped before storage.
    if (std::strcmp(key, "PROTO") == 0) {
      protocolValid = std::strtoul(value, nullptr, 10) == config::PROTOCOL_VERSION;
    } else if (std::strcmp(key, "TYPE") == 0) {
      commandType = std::strcmp(value, "CMD") == 0;
    } else if (std::strcmp(key, "SEQ") == 0) {
      command.sequence = std::strtoul(value, nullptr, 10);
    } else if (std::strcmp(key, "T") == 0) {
      command.throttle = clampThrottle(std::strtol(value, nullptr, 10));
    } else if (std::strcmp(key, "R") == 0) {
      command.rollTargetDeg = clampFloat(std::strtof(value, nullptr),
          -config::MAX_ATTITUDE_TARGET_DEG, config::MAX_ATTITUDE_TARGET_DEG);
    } else if (std::strcmp(key, "P") == 0) {
      command.pitchTargetDeg = clampFloat(std::strtof(value, nullptr),
          -config::MAX_ATTITUDE_TARGET_DEG, config::MAX_ATTITUDE_TARGET_DEG);
    } else if (std::strcmp(key, "Y") == 0) {
      command.yawRateTargetDps = clampFloat(std::strtof(value, nullptr),
          -config::MAX_YAW_RATE_DPS, config::MAX_YAW_RATE_DPS);
    } else if (std::strcmp(key, "A") == 0) {
      command.armRequested = std::strcmp(value, "1") == 0;
    } else if (std::strcmp(key, "CAL") == 0) {
      command.calibrateRequested = std::strcmp(value, "1") == 0;
    } else if (std::strcmp(key, "STOP") == 0) {
      command.emergencyStop = std::strcmp(value, "1") == 0;
    } else if (std::strcmp(key, "MODE") == 0) {
      command.mode = parseMode(value);
    }
  }

  // Timestamp accepted parsing attempts for the command-age failsafe calculation.
  command.receivedAtMs = millis();
  // A command is accepted only with the current protocol, correct type, and nonzero sequence.
  return protocolValid && commandType && command.sequence > 0;
}

// Serialize one telemetry snapshot into the documented text protocol.
void FlightLink::broadcastTelemetry(const TelemetryFrame &frame) {
  // Avoid formatting work when no Flight Deck client is connected.
  if (connectedClients_ == 0) return;

  // The fixed buffer avoids heap fragmentation during continuous telemetry.
  char payload[1200];
  // Every validity bit accompanies its numeric field so zero is never mistaken for real data.
  std::snprintf(
      payload, sizeof(payload),
      "PROTO=3;TYPE=TEL;SEQ=%lu;ACK=%lu;UP=%lu;STATE=%s;STATUS=%s;ARM=%u;"
      "FAILSAFE=%u;MODE=%s;CLIENTS=%u;CMD_AGE=%lu;ESC_OK=%u;IMU=%u;CAL=%u;ATT_VALID=%u;"
      "ROLL=%.2f;PITCH=%.2f;YAW=%.2f;RR=%.2f;PR=%.2f;YR=%.2f;THR=%u;"
      "BARO=%u;ENV=%u;POWER=%u;NAV=%u;"
      "ALT=%.2f;TEMP=%.2f;HUM=%.2f;BAT_V=%.2f;BAT_A=%.2f;BAT_W=%.2f;"
      "GNSS=%u;SAT=%u;LAT=%.7f;LON=%.7f;SPD=%.2f;FLOW=%u;OBST=%u;"
      "M1=%u;M2=%u;M3=%u;M4=%u;",
      static_cast<unsigned long>(frame.sequence),
      static_cast<unsigned long>(frame.acknowledgedCommand),
      static_cast<unsigned long>(frame.uptimeMs), safetyStateName(frame.safetyState),
      frame.status, frame.armed ? 1 : 0, frame.failsafe ? 1 : 0,
      flightModeName(frame.mode), frame.connectedClients,
      static_cast<unsigned long>(frame.commandAgeMs), frame.escProtocolConfirmed ? 1 : 0,
      frame.sensors.imuHealthy ? 1 : 0, frame.sensors.imuCalibrated ? 1 : 0,
      frame.sensors.attitude.valid ? 1 : 0, frame.sensors.attitude.rollDeg,
      frame.sensors.attitude.pitchDeg, frame.sensors.attitude.yawDeg,
      frame.sensors.attitude.rollRateDps, frame.sensors.attitude.pitchRateDps,
      frame.sensors.attitude.yawRateDps, frame.throttle,
      frame.sensors.barometerValid ? 1 : 0,
      frame.sensors.environmentValid ? 1 : 0,
      frame.sensors.powerMonitorValid ? 1 : 0,
      frame.sensors.navigationReady ? 1 : 0,
      frame.sensors.barometricAltitudeM, frame.sensors.temperatureC,
      frame.sensors.humidityPercent, frame.sensors.batteryVoltageV,
      frame.sensors.batteryCurrentA, frame.sensors.batteryPowerW,
      frame.sensors.gnssFix ? 1 : 0, frame.sensors.satelliteCount,
      frame.sensors.latitudeDeg, frame.sensors.longitudeDeg,
      frame.sensors.groundSpeedMps, frame.sensors.opticalFlowValid ? 1 : 0,
      frame.sensors.obstacleArrayValid ? 1 : 0, frame.motors.m1Us,
      frame.motors.m2Us, frame.motors.m3Us, frame.motors.m4Us);
  // Broadcast the same coherent frame to every connected monitoring client.
  webSocket_.broadcastTXT(payload);
}

// Register the status API, the Flight Deck entry point, and static-file fallback.
void FlightLink::configureHttpRoutes() {
  // The status endpoint supports diagnostics without opening a WebSocket.
  httpServer_.on("/api/status", HTTP_GET, [this]() {
    char response[256];
    std::snprintf(response, sizeof(response),
                  "{\"device\":\"NanoKit-Drone-4X\",\"transport\":\"wifi\","
                  "\"protocol\":3,\"clients\":%u,\"filesystem\":%s}",
                  connectedClients_, filesystemReady_ ? "true" : "false");
    httpServer_.send(200, "application/json", response);
  });
  // The root path always resolves to the interface entry document.
  httpServer_.on("/", HTTP_GET, [this]() { serveFile("/index.html"); });
  // Other paths are looked up directly in LittleFS for CSS, JavaScript, and assets.
  httpServer_.onNotFound([this]() { serveFile(httpServer_.uri()); });
}

// Validate and stream one requested LittleFS resource.
void FlightLink::serveFile(const String &path) {
  // Report an actionable service error if the interface filesystem is not available.
  if (!filesystemReady_) {
    httpServer_.send(503, "text/plain",
                     "Flight Deck filesystem missing. Run: pio run -t uploadfs");
    return;
  }
  // Directory requests use index.html, matching normal web-server behavior.
  String normalized = path;
  if (normalized.endsWith("/")) normalized += "index.html";
  // Return an HTTP 404 instead of opening a path that does not exist.
  if (!LittleFS.exists(normalized)) {
    httpServer_.send(404, "text/plain", "Not found");
    return;
  }
  // Stream the file to reduce RAM use, then close its handle immediately.
  File file = LittleFS.open(normalized, "r");
  httpServer_.streamFile(file, contentType(normalized));
  file.close();
}

// Map known filename extensions to browser-compatible MIME types.
const char *FlightLink::contentType(const String &path) const {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".svg")) return "image/svg+xml";
  // Unknown files use a safe generic binary type.
  return "application/octet-stream";
}

}  // namespace nanokit

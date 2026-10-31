// SPDX-License-Identifier: AGPL-3.0-only
#include "network/flight_link.h" // Import the FlightLink contract and its HTTP/WebSocket server members.

#include <LittleFS.h> // LittleFS serves the packaged Flight Deck files from ESP32 flash.
#include <WiFi.h> // WiFi creates the local NanoKit SoftAP used by the browser controller.
#include <algorithm> // Standard utilities provide bounds, number conversion, formatting, and safe copying.
#include <cmath> // Import the dependency required by this module.
#include <cstdio> // Import the dependency required by this module.
#include <cstdlib> // Import the dependency required by this module.
#include <cstring> // Import the dependency required by this module.

#include "config/board_config.h" // Shared network ports, protocol limits, and command bounds come from board configuration.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

namespace {

float clampFloat(float value, float minimum, float maximum) { // Clamp a floating command so remote input cannot exceed configured flight limits.
  return std::max(minimum, std::min(maximum, value)); // Return this result to the caller.
}

uint16_t clampThrottle(long value) { // Normalize throttle to the protocol's safe 0-1000 range.
  return static_cast<uint16_t>(std::max(0L, std::min(1000L, value))); // Return this result to the caller.
}

FlightMode parseMode(const char *value) { // Convert the text protocol mode into the internal enum, defaulting to Manual.
  if (std::strcmp(value, "ALTITUDE_HOLD") == 0) return FlightMode::AltitudeHold; // Evaluate this condition before continuing.
  if (std::strcmp(value, "POSITION_HOLD") == 0) return FlightMode::PositionHold; // Evaluate this condition before continuing.
  if (std::strcmp(value, "MISSION") == 0) return FlightMode::Mission; // Evaluate this condition before continuing.
  return FlightMode::Manual; // Return this result to the caller.
}

}  // namespace

FlightLink::FlightLink() // Construct both servers on their fixed configuration ports.
    : httpServer_(config::HTTP_PORT), webSocket_(config::WEBSOCKET_PORT) {}

void FlightLink::begin(QueueHandle_t commandQueue, QueueHandle_t telemetryQueue) { // Start the filesystem, isolated Wi-Fi access point, HTTP routes, and WebSocket service.
  commandQueue_ = commandQueue; // Save queue handles only after setup has successfully allocated them.
  telemetryQueue_ = telemetryQueue;

  filesystemReady_ = LittleFS.begin(true); // Mount LittleFS and format only if its existing filesystem cannot be mounted.
  WiFi.mode(WIFI_AP); // AP mode makes the NanoKit the local network host instead of requiring a router.
  const IPAddress localIp(192, 168, 4, 1); // A fixed 192.168.4.1/24 address gives the Flight Deck a predictable endpoint.
  const IPAddress gateway(192, 168, 4, 1);
  const IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(localIp, gateway, subnet);
  WiFi.softAP(config::ACCESS_POINT_SSID, config::ACCESS_POINT_PASSWORD, // Start a visible WPA2 access point with at most four connected stations.
              config::ACCESS_POINT_CHANNEL, false, 4); // Execute this statement as part of the current subsystem operation.

  configureHttpRoutes(); // Configure routes before accepting HTTP requests.
  httpServer_.begin();

  webSocket_.begin(); // Register one callback for WebSocket connection, disconnection, and command events.
  webSocket_.onEvent([this](uint8_t client, WStype_t type, uint8_t *payload, size_t length) { // Open this implementation block.
    onWebSocketEvent(client, type, payload, length);
  });

  Serial.println("[NET] NanoKit Flight Deck SoftAP ready."); // Serial output provides the exact network endpoint for bench diagnostics.
  Serial.print("[NET] SSID: ");
  Serial.println(config::ACCESS_POINT_SSID);
  Serial.print("[NET] HTTP: http://"); // Print the HTTP scheme before the flight-controller IP address.
  Serial.println(WiFi.softAPIP());
  Serial.print("[NET] WebSocket port: ");
  Serial.println(config::WEBSOCKET_PORT);
  if (!filesystemReady_) { // A missing filesystem does not affect motor safety, but the web interface cannot load.
    Serial.println("[NET] LittleFS unavailable; upload the firmware/web_control LittleFS image."); // Tell the operator to upload the independent Web Control assets stored inside this firmware project.
  }
}

void FlightLink::loop() { // Service both servers and transmit the newest queued telemetry without blocking.
  httpServer_.handleClient();
  webSocket_.loop();

  TelemetryFrame frame; // Send a frame only when the flight task has published a newer snapshot.
  if (telemetryQueue_ != nullptr && xQueueReceive(telemetryQueue_, &frame, 0) == pdTRUE) { // Evaluate this condition before continuing.
    broadcastTelemetry(frame);
  }
}

void FlightLink::onWebSocketEvent(uint8_t client, WStype_t type, // Handle one WebSocket event from one browser client.
                                  uint8_t *payload, size_t length) { // Open this implementation block.
  switch (type) { // Connection events update safety state; text events carry commands.
    case WStype_CONNECTED: // Handle this specific switch value.
      if (connectedClients_ < 255) ++connectedClients_; // Saturate at 255 because the telemetry field is one byte.
      webSocket_.sendTXT(client, // The HELLO packet declares identity, transport, protocol, and locked startup state.
          "PROTO=3;TYPE=HELLO;DEVICE=NanoKit-Drone-4X;TRANSPORT=WIFI;SAFETY=LOCKED;");
      break; // Leave the current switch case or loop.
    case WStype_DISCONNECTED: // Handle this specific switch value.
      if (connectedClients_ > 0) --connectedClients_; // Avoid unsigned underflow if a duplicate disconnect event arrives.
      break; // Leave the current switch case or loop.
    case WStype_TEXT: { // Handle this specific switch value.
      PilotCommand command; // Parse into a local command so invalid input never changes the shared queue.
      if (parseCommand(payload, length, command) && commandQueue_ != nullptr) { // Evaluate this condition before continuing.
        xQueueOverwrite(commandQueue_, &command); // Keep only the newest valid command for the flight task.
        char acknowledgement[96]; // Acknowledge the exact sequence so the interface can measure command progress.
        std::snprintf(acknowledgement, sizeof(acknowledgement),
                      "PROTO=3;TYPE=ACK;SEQ=%lu;", static_cast<unsigned long>(command.sequence));
        webSocket_.sendTXT(client, acknowledgement);
      } else {
        webSocket_.sendTXT(client, // Return a structured protocol error without attempting partial execution.
            "PROTO=3;TYPE=ERROR;CODE=BAD_COMMAND;MESSAGE=Command rejected;");
      }
      break; // Leave the current switch case or loop.
    }
    default: // Handle any switch value not matched above.
      break; // Leave the current switch case or loop.
  }
}

bool FlightLink::parseCommand(const uint8_t *payload, size_t length, // Parse one semicolon-separated command packet and enforce protocol bounds.
                              PilotCommand &command) const { // Open this implementation block.
  if (payload == nullptr || length == 0 || length >= 384) return false; // Reject null, empty, or oversized payloads before copying into a fixed stack buffer.

  char buffer[384]; // Copy WebSocket bytes and add the terminator required by C string tokenization.
  std::memcpy(buffer, payload, length);
  buffer[length] = '\0';

  bool protocolValid = false; // Both the protocol version and CMD packet type are mandatory.
  bool commandType = false;
  char *context = nullptr;
  for (char *token = strtok_r(buffer, ";", &context); token != nullptr; // Split the packet into KEY=VALUE fields without using heap allocation.
       token = strtok_r(nullptr, ";", &context)) { // Open this implementation block.
    char *separator = std::strchr(token, '='); // Ignore malformed fields that do not contain an equals separator.
    if (separator == nullptr) continue; // Evaluate this condition before continuing.
    *separator = '\0';
    const char *key = token;
    const char *value = separator + 1;

    if (std::strcmp(key, "PROTO") == 0) { // Parse known fields only; every numeric flight command is clamped before storage.
      protocolValid = std::strtoul(value, nullptr, 10) == config::PROTOCOL_VERSION;
    } else if (std::strcmp(key, "TYPE") == 0) {
      commandType = std::strcmp(value, "CMD") == 0;
    } else if (std::strcmp(key, "SEQ") == 0) {
      command.sequence = std::strtoul(value, nullptr, 10);
    } else if (std::strcmp(key, "T") == 0) {
      command.throttle = clampThrottle(std::strtol(value, nullptr, 10));
    } else if (std::strcmp(key, "R") == 0) {
      command.rollTargetDeg = clampFloat(std::strtof(value, nullptr),
          -config::MAX_ATTITUDE_TARGET_DEG, config::MAX_ATTITUDE_TARGET_DEG); // Execute this statement as part of the current subsystem operation.
    } else if (std::strcmp(key, "P") == 0) {
      command.pitchTargetDeg = clampFloat(std::strtof(value, nullptr),
          -config::MAX_ATTITUDE_TARGET_DEG, config::MAX_ATTITUDE_TARGET_DEG); // Execute this statement as part of the current subsystem operation.
    } else if (std::strcmp(key, "Y") == 0) {
      command.yawRateTargetDps = clampFloat(std::strtof(value, nullptr),
          -config::MAX_YAW_RATE_DPS, config::MAX_YAW_RATE_DPS); // Execute this statement as part of the current subsystem operation.
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

  command.receivedAtMs = millis(); // Timestamp accepted parsing attempts for the command-age failsafe calculation.
  return protocolValid && commandType && command.sequence > 0; // A command is accepted only with the current protocol, correct type, and nonzero sequence.
}

void FlightLink::broadcastTelemetry(const TelemetryFrame &frame) { // Serialize one telemetry snapshot into the documented text protocol.
  if (connectedClients_ == 0) return; // Avoid formatting work when no Flight Deck client is connected.

  char payload[1200]; // The fixed buffer avoids heap fragmentation during continuous telemetry.
  std::snprintf( // Every validity bit accompanies its numeric field so zero is never mistaken for real data.
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
      frame.status, frame.armed ? 1 : 0, frame.failsafe ? 1 : 0, // Continue the current project declaration or implementation.
      flightModeName(frame.mode), frame.connectedClients,
      static_cast<unsigned long>(frame.commandAgeMs), frame.escProtocolConfirmed ? 1 : 0,
      frame.sensors.imuHealthy ? 1 : 0, frame.sensors.imuCalibrated ? 1 : 0, // Continue the current project declaration or implementation.
      frame.sensors.attitude.valid ? 1 : 0, frame.sensors.attitude.rollDeg, // Continue the current project declaration or implementation.
      frame.sensors.attitude.pitchDeg, frame.sensors.attitude.yawDeg, // Continue the current project declaration or implementation.
      frame.sensors.attitude.rollRateDps, frame.sensors.attitude.pitchRateDps, // Continue the current project declaration or implementation.
      frame.sensors.attitude.yawRateDps, frame.throttle, // Continue the current project declaration or implementation.
      frame.sensors.barometerValid ? 1 : 0, // Continue the current project declaration or implementation.
      frame.sensors.environmentValid ? 1 : 0, // Continue the current project declaration or implementation.
      frame.sensors.powerMonitorValid ? 1 : 0, // Continue the current project declaration or implementation.
      frame.sensors.navigationReady ? 1 : 0, // Continue the current project declaration or implementation.
      frame.sensors.barometricAltitudeM, frame.sensors.temperatureC, // Continue the current project declaration or implementation.
      frame.sensors.humidityPercent, frame.sensors.batteryVoltageV, // Continue the current project declaration or implementation.
      frame.sensors.batteryCurrentA, frame.sensors.batteryPowerW, // Continue the current project declaration or implementation.
      frame.sensors.gnssFix ? 1 : 0, frame.sensors.satelliteCount, // Continue the current project declaration or implementation.
      frame.sensors.latitudeDeg, frame.sensors.longitudeDeg, // Continue the current project declaration or implementation.
      frame.sensors.groundSpeedMps, frame.sensors.opticalFlowValid ? 1 : 0, // Continue the current project declaration or implementation.
      frame.sensors.obstacleArrayValid ? 1 : 0, frame.motors.m1Us, // Continue the current project declaration or implementation.
      frame.motors.m2Us, frame.motors.m3Us, frame.motors.m4Us); // Execute this statement as part of the current subsystem operation.
  webSocket_.broadcastTXT(payload); // Broadcast the same coherent frame to every connected monitoring client.
}

void FlightLink::configureHttpRoutes() { // Register the status API, the Flight Deck entry point, and static-file fallback.
  httpServer_.on("/api/status", HTTP_GET, [this]() { // The status endpoint supports diagnostics without opening a WebSocket.
    char response[256]; // Execute this statement as part of the current subsystem operation.
    std::snprintf(response, sizeof(response),
                  "{\"device\":\"NanoKit-Drone-4X\",\"transport\":\"wifi\"," // Continue the current project declaration or implementation.
                  "\"protocol\":3,\"clients\":%u,\"filesystem\":%s}", // Continue the current project declaration or implementation.
                  connectedClients_, filesystemReady_ ? "true" : "false"); // Execute this statement as part of the current subsystem operation.
    httpServer_.send(200, "application/json", response);
  });
  httpServer_.on("/", HTTP_GET, [this]() { serveFile("/index.html"); }); // The root path always resolves to the interface entry document.
  httpServer_.onNotFound([this]() { serveFile(httpServer_.uri()); }); // Other paths are looked up directly in LittleFS for CSS, JavaScript, and assets.
}

void FlightLink::serveFile(const String &path) { // Validate and stream one requested LittleFS resource.
  if (!filesystemReady_) { // Report an actionable service error if the interface filesystem is not available.
    httpServer_.send(503, "text/plain",
                     "Flight Deck filesystem missing. Run: pio run -t uploadfs"); // Execute this statement as part of the current subsystem operation.
    return; // Return this result to the caller.
  }
  String normalized = path; // Directory requests use index.html, matching normal web-server behavior.
  if (normalized.endsWith("/")) normalized += "index.html"; // Evaluate this condition before continuing.
  if (!LittleFS.exists(normalized)) { // Return an HTTP 404 instead of opening a path that does not exist.
    httpServer_.send(404, "text/plain", "Not found");
    return; // Return this result to the caller.
  }
  File file = LittleFS.open(normalized, "r"); // Stream the file to reduce RAM use, then close its handle immediately.
  httpServer_.streamFile(file, contentType(normalized));
  file.close();
}

const char *FlightLink::contentType(const String &path) const { // Map known filename extensions to browser-compatible MIME types.
  if (path.endsWith(".html")) return "text/html"; // Evaluate this condition before continuing.
  if (path.endsWith(".css")) return "text/css"; // Evaluate this condition before continuing.
  if (path.endsWith(".js")) return "application/javascript"; // Evaluate this condition before continuing.
  if (path.endsWith(".json")) return "application/json"; // Evaluate this condition before continuing.
  if (path.endsWith(".png")) return "image/png"; // Evaluate this condition before continuing.
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg"; // Evaluate this condition before continuing.
  if (path.endsWith(".svg")) return "image/svg+xml"; // Evaluate this condition before continuing.
  return "application/octet-stream"; // Unknown files use a safe generic binary type.
}

}  // namespace nanokit

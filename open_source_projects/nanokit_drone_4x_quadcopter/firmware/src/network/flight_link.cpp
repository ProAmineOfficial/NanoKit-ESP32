#include "network/flight_link.h"

#include <LittleFS.h>
#include <WiFi.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "config/board_config.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

namespace {

float clampFloat(float value, float minimum, float maximum) {
  return std::max(minimum, std::min(maximum, value));
}

uint16_t clampThrottle(long value) {
  return static_cast<uint16_t>(std::max(0L, std::min(1000L, value)));
}

FlightMode parseMode(const char *value) {
  if (std::strcmp(value, "ALTITUDE_HOLD") == 0) return FlightMode::AltitudeHold;
  if (std::strcmp(value, "POSITION_HOLD") == 0) return FlightMode::PositionHold;
  if (std::strcmp(value, "MISSION") == 0) return FlightMode::Mission;
  return FlightMode::Manual;
}

}  // namespace

FlightLink::FlightLink()
    : httpServer_(config::HTTP_PORT), webSocket_(config::WEBSOCKET_PORT) {}

void FlightLink::begin(QueueHandle_t commandQueue, QueueHandle_t telemetryQueue) {
  commandQueue_ = commandQueue;
  telemetryQueue_ = telemetryQueue;

  filesystemReady_ = LittleFS.begin(true);
  WiFi.mode(WIFI_AP);
  const IPAddress localIp(192, 168, 4, 1);
  const IPAddress gateway(192, 168, 4, 1);
  const IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(localIp, gateway, subnet);
  WiFi.softAP(config::ACCESS_POINT_SSID, config::ACCESS_POINT_PASSWORD,
              config::ACCESS_POINT_CHANNEL, false, 4);

  configureHttpRoutes();
  httpServer_.begin();

  webSocket_.begin();
  webSocket_.onEvent([this](uint8_t client, WStype_t type, uint8_t *payload, size_t length) {
    onWebSocketEvent(client, type, payload, length);
  });

  Serial.println("[NET] NanoKit Flight Deck SoftAP ready.");
  Serial.print("[NET] SSID: ");
  Serial.println(config::ACCESS_POINT_SSID);
  Serial.print("[NET] HTTP: http://");
  Serial.println(WiFi.softAPIP());
  Serial.print("[NET] WebSocket port: ");
  Serial.println(config::WEBSOCKET_PORT);
  if (!filesystemReady_) {
    Serial.println("[NET] LittleFS unavailable; upload the web_controller filesystem image.");
  }
}

void FlightLink::loop() {
  httpServer_.handleClient();
  webSocket_.loop();

  TelemetryFrame frame;
  if (telemetryQueue_ != nullptr && xQueueReceive(telemetryQueue_, &frame, 0) == pdTRUE) {
    broadcastTelemetry(frame);
  }
}

void FlightLink::onWebSocketEvent(uint8_t client, WStype_t type,
                                  uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      if (connectedClients_ < 255) ++connectedClients_;
      webSocket_.sendTXT(client,
          "PROTO=3;TYPE=HELLO;DEVICE=NanoKit-Drone-4X;TRANSPORT=WIFI;SAFETY=LOCKED;");
      break;
    case WStype_DISCONNECTED:
      if (connectedClients_ > 0) --connectedClients_;
      break;
    case WStype_TEXT: {
      PilotCommand command;
      if (parseCommand(payload, length, command) && commandQueue_ != nullptr) {
        xQueueOverwrite(commandQueue_, &command);
        char acknowledgement[96];
        std::snprintf(acknowledgement, sizeof(acknowledgement),
                      "PROTO=3;TYPE=ACK;SEQ=%lu;", static_cast<unsigned long>(command.sequence));
        webSocket_.sendTXT(client, acknowledgement);
      } else {
        webSocket_.sendTXT(client,
            "PROTO=3;TYPE=ERROR;CODE=BAD_COMMAND;MESSAGE=Command rejected;");
      }
      break;
    }
    default:
      break;
  }
}

bool FlightLink::parseCommand(const uint8_t *payload, size_t length,
                              PilotCommand &command) const {
  if (payload == nullptr || length == 0 || length >= 384) return false;

  char buffer[384];
  std::memcpy(buffer, payload, length);
  buffer[length] = '\0';

  bool protocolValid = false;
  bool commandType = false;
  char *context = nullptr;
  for (char *token = strtok_r(buffer, ";", &context); token != nullptr;
       token = strtok_r(nullptr, ";", &context)) {
    char *separator = std::strchr(token, '=');
    if (separator == nullptr) continue;
    *separator = '\0';
    const char *key = token;
    const char *value = separator + 1;

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

  command.receivedAtMs = millis();
  return protocolValid && commandType && command.sequence > 0;
}

void FlightLink::broadcastTelemetry(const TelemetryFrame &frame) {
  if (connectedClients_ == 0) return;

  char payload[1200];
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
  webSocket_.broadcastTXT(payload);
}

void FlightLink::configureHttpRoutes() {
  httpServer_.on("/api/status", HTTP_GET, [this]() {
    char response[256];
    std::snprintf(response, sizeof(response),
                  "{\"device\":\"NanoKit-Drone-4X\",\"transport\":\"wifi\","
                  "\"protocol\":3,\"clients\":%u,\"filesystem\":%s}",
                  connectedClients_, filesystemReady_ ? "true" : "false");
    httpServer_.send(200, "application/json", response);
  });
  httpServer_.on("/", HTTP_GET, [this]() { serveFile("/index.html"); });
  httpServer_.onNotFound([this]() { serveFile(httpServer_.uri()); });
}

void FlightLink::serveFile(const String &path) {
  if (!filesystemReady_) {
    httpServer_.send(503, "text/plain",
                     "Flight Deck filesystem missing. Run: pio run -t uploadfs");
    return;
  }
  String normalized = path;
  if (normalized.endsWith("/")) normalized += "index.html";
  if (!LittleFS.exists(normalized)) {
    httpServer_.send(404, "text/plain", "Not found");
    return;
  }
  File file = LittleFS.open(normalized, "r");
  httpServer_.streamFile(file, contentType(normalized));
  file.close();
}

const char *FlightLink::contentType(const String &path) const {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".svg")) return "image/svg+xml";
  return "application/octet-stream";
}

}  // namespace nanokit

#pragma once

#include <WebServer.h>
#include <WebSocketsServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

class FlightLink {
 public:
  FlightLink();
  void begin(QueueHandle_t commandQueue, QueueHandle_t telemetryQueue);
  void loop();
  uint8_t clientCount() const { return connectedClients_; }

 private:
  void onWebSocketEvent(uint8_t client, WStype_t type, uint8_t *payload, size_t length);
  bool parseCommand(const uint8_t *payload, size_t length, PilotCommand &command) const;
  void broadcastTelemetry(const TelemetryFrame &frame);
  void configureHttpRoutes();
  void serveFile(const String &path);
  const char *contentType(const String &path) const;

  WebServer httpServer_;
  WebSocketsServer webSocket_;
  QueueHandle_t commandQueue_ = nullptr;
  QueueHandle_t telemetryQueue_ = nullptr;
  volatile uint8_t connectedClients_ = 0;
  bool filesystemReady_ = false;
};

}  // namespace nanokit

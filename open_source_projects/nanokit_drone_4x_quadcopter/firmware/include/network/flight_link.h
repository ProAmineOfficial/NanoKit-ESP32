#pragma once

// WebServer serves the Flight Deck while WebSocketsServer carries live packets.
#include <WebServer.h>
#include <WebSocketsServer.h>
// FreeRTOS queues safely move commands and telemetry between CPU cores.
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Shared packet structures define the data crossing the network boundary.
#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// FlightLink owns the Wi-Fi SoftAP, HTTP server, WebSocket server, and packet parser.
class FlightLink {
 public:
  // Construct servers on the fixed ports declared in board_config.h.
  FlightLink();
  // Start storage and networking, then connect the cross-core queues.
  void begin(QueueHandle_t commandQueue, QueueHandle_t telemetryQueue);
  // Service HTTP, WebSocket events, and queued telemetry from the network task.
  void loop();
  // Expose the current client count to the safety manager.
  uint8_t clientCount() const { return connectedClients_; }

 private:
  // Convert WebSocket events into connection state or validated PilotCommand packets.
  void onWebSocketEvent(uint8_t client, WStype_t type, uint8_t *payload, size_t length);
  // Parse the compact protocol without modifying shared state until validation succeeds.
  bool parseCommand(const uint8_t *payload, size_t length, PilotCommand &command) const;
  // Serialize one telemetry frame and send it to all connected browser clients.
  void broadcastTelemetry(const TelemetryFrame &frame);
  // Register fixed HTTP routes and static-file handling.
  void configureHttpRoutes();
  // Serve one LittleFS resource with the correct MIME type.
  void serveFile(const String &path);
  const char *contentType(const String &path) const;

  // These servers run only inside the lower-priority network task.
  WebServer httpServer_;
  WebSocketsServer webSocket_;
  // Queue handles are assigned during begin() after FreeRTOS creates them.
  QueueHandle_t commandQueue_ = nullptr;
  QueueHandle_t telemetryQueue_ = nullptr;
  // volatile is required because callbacks and the flight task observe this count.
  volatile uint8_t connectedClients_ = 0;
  // Static routes remain unavailable if LittleFS could not mount safely.
  bool filesystemReady_ = false;
};

}  // namespace nanokit

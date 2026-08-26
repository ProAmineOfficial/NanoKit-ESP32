#pragma once // Prevent this header from being included more than once.

#include <WebServer.h> // WebServer serves the Flight Deck while WebSocketsServer carries live packets.
#include <WebSocketsServer.h> // Import the dependency required by this module.
#include <freertos/FreeRTOS.h> // FreeRTOS queues safely move commands and telemetry between CPU cores.
#include <freertos/queue.h> // Import the dependency required by this module.

#include "core/flight_types.h" // Shared packet structures define the data crossing the network boundary.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

class FlightLink { // FlightLink owns the Wi-Fi SoftAP, HTTP server, WebSocket server, and packet parser.
 public: // Start this access-control section of the type.
  FlightLink(); // Construct servers on the fixed ports declared in board_config.h.
  void begin(QueueHandle_t commandQueue, QueueHandle_t telemetryQueue); // Start storage and networking, then connect the cross-core queues.
  void loop(); // Service HTTP, WebSocket events, and queued telemetry from the network task.
  uint8_t clientCount() const { return connectedClients_; } // Expose the current client count to the safety manager.

 private: // Start this access-control section of the type.
  void onWebSocketEvent(uint8_t client, WStype_t type, uint8_t *payload, size_t length); // Convert WebSocket events into connection state or validated PilotCommand packets.
  bool parseCommand(const uint8_t *payload, size_t length, PilotCommand &command) const; // Parse the compact protocol without modifying shared state until validation succeeds.
  void broadcastTelemetry(const TelemetryFrame &frame); // Serialize one telemetry frame and send it to all connected browser clients.
  void configureHttpRoutes(); // Register fixed HTTP routes and static-file handling.
  void serveFile(const String &path); // Serve one LittleFS resource with the correct MIME type.
  const char *contentType(const String &path) const; // Continue this function declaration or call across this line.

  WebServer httpServer_; // These servers run only inside the lower-priority network task.
  WebSocketsServer webSocket_; // Execute this statement as part of the current subsystem operation.
  QueueHandle_t commandQueue_ = nullptr; // Queue handles are assigned during begin() after FreeRTOS creates them.
  QueueHandle_t telemetryQueue_ = nullptr; // Assign this value for the current control or telemetry operation.
  volatile uint8_t connectedClients_ = 0; // volatile is required because callbacks and the flight task observe this count.
  bool filesystemReady_ = false; // Static routes remain unavailable if LittleFS could not mount safely.
}; // Close the current scope or type definition.

}  // namespace nanokit

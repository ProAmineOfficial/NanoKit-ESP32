// NanoKit Drone 4X Wi-Fi flight-controller reference firmware.
// Developed by Amine Saoud ibn al-Bashir.
// Keep propellers removed until every documented bench test has passed.

// Arduino supplies setup(), loop(), serial diagnostics, and ESP32 timing functions.
#include <Arduino.h>
// FreeRTOS queues and pinned tasks isolate real-time control from networking.
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

// Standard algorithms clamp the measured loop interval to a safe numeric range.
#include <algorithm>
// C string functions copy fixed telemetry status messages without heap allocation.
#include <cstring>

// Board and feature configuration are the single source of hardware truth.
#include "config/board_config.h"
#include "config/feature_flags.h"
// Control modules calculate corrections and map them to four motor outputs.
#include "control/flight_controller.h"
#include "control/motor_mixer.h"
// Shared types and safety logic connect every flight subsystem.
#include "core/flight_types.h"
#include "core/safety_manager.h"
// Networking, navigation, sensors, and optional storage remain separate modules.
#include "network/flight_link.h"
#include "navigation/navigation_manager.h"
#include "sensors/sensor_hub.h"
#include "storage/config_store.h"

// File-local objects are hidden in an anonymous namespace to avoid global name collisions.
namespace {

// Use shared NanoKit types without repeating the namespace inside this implementation.
using namespace nanokit;

// Single-element queues always retain only the newest command and telemetry snapshot.
QueueHandle_t commandQueue = nullptr;
QueueHandle_t telemetryQueue = nullptr;

// Each subsystem has one long-lived instance owned by the firmware process.
MotorMixer motorMixer;
FlightController flightController;
SafetyManager safetyManager;
SensorHub sensorHub;
FlightLink flightLink;
NavigationManager navigationManager;
ConfigStore configStore;

// Build a complete, truthful telemetry snapshot from the current flight state.
void populateTelemetry(TelemetryFrame &frame, const PilotCommand &command,
                       const SensorState &sensors, const MotorOutput &motors,
                       uint32_t commandAgeMs) {
  // A local static counter persists between calls without becoming externally writable.
  static uint32_t telemetrySequence = 0;
  // Reset every field first so no value from an older frame can leak into this one.
  frame = TelemetryFrame{};
  // Sequence and acknowledgement fields let the browser measure packet progress.
  frame.sequence = ++telemetrySequence;
  frame.acknowledgedCommand = command.sequence;
  frame.uptimeMs = millis();
  frame.commandAgeMs = commandAgeMs;
  // Copy the coherent snapshots produced by sensors and the motor mixer.
  frame.sensors = sensors;
  frame.motors = motors;
  frame.safetyState = safetyManager.state();
  frame.mode = command.mode;
  frame.throttle = safetyManager.armed() ? command.throttle : 0;
  // Publish live connection and safety decisions for the interface controls.
  frame.connectedClients = flightLink.clientCount();
  frame.armed = safetyManager.armed();
  frame.failsafe = safetyManager.failsafe();
// Report ESC approval from the same compile-time lock used by MotorMixer.
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED
  frame.escProtocolConfirmed = true;
#else
  frame.escProtocolConfirmed = false;
#endif
  // Copy the readable safety explanation and always terminate the fixed buffer.
  std::strncpy(frame.status, safetyManager.status(), sizeof(frame.status) - 1);
  frame.status[sizeof(frame.status) - 1] = '\0';
}

// Run the deterministic 250 Hz safety and flight-control pipeline on the flight core.
void flightTask(void *parameter) {
  // FreeRTOS requires a generic task parameter even though this task needs no argument.
  (void)parameter;
  // These local snapshots are retained across loop iterations by the task stack.
  PilotCommand command;
  MotorOutput motors;
  // Sequence tracking prevents one calibration button press from being processed repeatedly.
  uint32_t lastCalibrationSequence = 0;
  // Independent timestamps control telemetry pacing and accurate PID timing.
  uint32_t lastTelemetryMs = 0;
  uint32_t lastLoopUs = micros();
  TickType_t lastWake = xTaskGetTickCount();

  // A flight task is intentionally permanent and is scheduled at a fixed period.
  for (;;) {
    // Measure real elapsed time so PID math remains stable when scheduling varies slightly.
    const uint32_t nowUs = micros();
    float dtSeconds = static_cast<float>(nowUs - lastLoopUs) / 1000000.0f;
    lastLoopUs = nowUs;
    // Reject extreme timing values that could destabilize integral or derivative calculations.
    dtSeconds = std::max(0.001f, std::min(0.02f, dtSeconds));

    // Read the newest command without blocking the real-time loop.
    PilotCommand incoming;
    if (xQueueReceive(commandQueue, &incoming, 0) == pdTRUE) {
      command = incoming;
      // Process calibration once for each unique command sequence.
      if (command.calibrateRequested && command.sequence != lastCalibrationSequence) {
        lastCalibrationSequence = command.sequence;
        sensorHub.requestImuCalibration();
      }
    }

    // Refresh real sensors before validating the requested flight mode.
    sensorHub.update(dtSeconds);
    const SensorState &sensors = sensorHub.state();
    command.mode = navigationManager.validateRequestedMode(command.mode, sensors);
    // UINT32_MAX clearly marks a command link that has never received a packet.
    const uint32_t nowMs = millis();
    const uint32_t commandAgeMs = command.sequence == 0
        ? UINT32_MAX
        : nowMs - command.receivedAtMs;

    // Collect all independent safety facts before asking for one authoritative decision.
    SafetyInputs safetyInputs;
    safetyInputs.networkClientConnected = flightLink.clientCount() > 0;
    safetyInputs.commandFresh = command.sequence > 0 &&
                                commandAgeMs <= config::COMMAND_TIMEOUT_MS;
    safetyInputs.imuHealthy = sensors.imuHealthy;
    safetyInputs.imuCalibrated = sensors.imuCalibrated;
    safetyInputs.attitudeValid = sensors.attitude.valid;
// Use the same compile-time ESC proof gate throughout the firmware.
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED
    safetyInputs.escProtocolConfirmed = true;
#else
    safetyInputs.escProtocolConfirmed = false;
#endif
    // Pilot intent is evaluated only after hardware and link facts are populated.
    safetyInputs.armRequested = command.armRequested;
    safetyInputs.emergencyStop = command.emergencyStop;
    safetyInputs.throttle = command.throttle;
    safetyManager.update(safetyInputs);

    // PID output is permitted only when SafetyManager reaches Armed.
    const bool armed = safetyManager.armed();
    const ControlCorrection correction =
        flightController.update(command, sensors, dtSeconds, armed);
    // Calculate Quad-X pulses only while armed; otherwise retain safe defaults.
    if (armed) {
      motors = motorMixer.mix(command.throttle, correction.roll,
                              correction.pitch, correction.yaw);
    } else {
      motors = MotorOutput{};
    }
    // MotorMixer applies its own compile-time and runtime safety gates before hardware writes.
    motorMixer.write(motors, armed);

    // Publish telemetry at its lower configured rate to reduce network and CPU load.
    if (nowMs - lastTelemetryMs >= config::TELEMETRY_PERIOD_MS) {
      lastTelemetryMs = nowMs;
      TelemetryFrame frame;
      populateTelemetry(frame, command, sensors, motors, commandAgeMs);
      // Overwrite keeps the newest frame if the network task is temporarily busy.
      xQueueOverwrite(telemetryQueue, &frame);
    }

    // Delay relative to the previous wake time to maintain a stable 250 Hz schedule.
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(config::FLIGHT_LOOP_PERIOD_US / 1000));
  }
}

// Run HTTP and WebSocket processing on the separate network core.
void networkTask(void *parameter) {
  (void)parameter;
  // Networking starts inside its owning task after both cross-core queues exist.
  flightLink.begin(commandQueue, telemetryQueue);
  // Service network events continuously while yielding at the configured interval.
  for (;;) {
    flightLink.loop();
    vTaskDelay(pdMS_TO_TICKS(config::NETWORK_TASK_DELAY_MS));
  }
}

}  // namespace

// Arduino calls setup() once after boot to initialize safe hardware and both tasks.
void setup() {
  // Start serial diagnostics at the PlatformIO monitor speed and allow USB-UART to settle.
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("NanoKit Drone 4X - Wi-Fi Flight Controller");
  Serial.println("Developed by Amine Saoud ibn al-Bashir.");
  Serial.println("[SAFE] Propellers must remain removed during integration.");

  // Motor outputs are configured first and forced to minimum before any other subsystem starts.
  motorMixer.begin();
  motorMixer.safe();
  sensorHub.begin();
  // Optional storage reports unavailable when its hardware feature remains disabled.
  configStore.begin();
  Serial.print("[STORE] ");
  Serial.println(configStore.status());

  // Length-one queues implement latest-value exchange between the two pinned tasks.
  commandQueue = xQueueCreate(1, sizeof(nanokit::PilotCommand));
  telemetryQueue = xQueueCreate(1, sizeof(nanokit::TelemetryFrame));
  // Abort task creation if memory allocation failed; motors already remain at minimum.
  if (commandQueue == nullptr || telemetryQueue == nullptr) {
    Serial.println("[FAULT] Queue allocation failed. Motors remain at minimum.");
    return;
  }

  // Pin flight control to core 1 with higher priority and enough stack for control modules.
  const BaseType_t flightCreated = xTaskCreatePinnedToCore(
      flightTask, "nanokit-flight", 6144, nullptr,
      nanokit::config::FLIGHT_TASK_PRIORITY, nullptr,
      nanokit::config::FLIGHT_TASK_CORE);
  // Pin network processing to core 0 with lower priority and a larger web-server stack.
  const BaseType_t networkCreated = xTaskCreatePinnedToCore(
      networkTask, "nanokit-network", 8192, nullptr,
      nanokit::config::NETWORK_TASK_PRIORITY, nullptr,
      nanokit::config::NETWORK_TASK_CORE);

  // Any task-creation failure returns motor outputs to the safe minimum.
  if (flightCreated != pdPASS || networkCreated != pdPASS) {
    motorMixer.safe();
    Serial.println("[FAULT] Task creation failed. Motors remain at minimum.");
  }
}

// Arduino still requires loop(), but all meaningful work is owned by FreeRTOS tasks.
void loop() {
  // Work is isolated in pinned FreeRTOS tasks. Arduino loop stays non-critical.
  delay(1000);
}

// SPDX-License-Identifier: AGPL-3.0-only
/*
 * Project: NanoKit Drone 4X
 * Component: Wi-Fi Flight Controller Firmware
 *
 * This file is part of the NanoKit Drone 4X open-source project.
 *
 * Description:
 * This firmware controls the main flight operations of the NanoKit Drone 4X
 * using the NanoKit Integrated ESP32 development board.
 *
 * The flight controller communicates with the web-based Flight Deck through
 * Wi-Fi and WebSocket connections. It receives pilot commands, processes
 * sensor measurements, runs the PID flight-control system, manages the motors,
 * applies flight-safety rules, and publishes real-time telemetry.
 *
 * Main Features:
 * - Wi-Fi SoftAP and network communication
 * - WebSocket command and telemetry link
 * - IMU and flight-sensor processing
 * - PID-based flight stabilization
 * - ESC and motor control
 * - Flight modes and navigation support
 * - Arming, failsafe, and safety management
 * - Real-time telemetry for the Flight Deck interface
 *
 * Hardware:
 * - NanoKit Integrated ESP32 development board
 * - Electronic Speed Controllers (ESCs)
 * - Brushless motors
 * - IMU and navigation sensors
 * - Optional Camera Node and additional sensors
 *
 * Development Environment:
 * - PlatformIO
 * - Arduino Framework for ESP32
 *
 * Developed by Amine Saoud ibn al-Bashir
 * Pro_Amine LLC
 * Official website: https://proamine.tech
 *
 * Initial development date: September 9, 2025
 * Project: NanoKit Drone 4X
 * Repository: NanoKit-ESP32
 * License: GNU AGPL v3.0 only, with the additional terms in NOTICE
 */

#include <Arduino.h> // Provides the Arduino framework, including setup(), loop(), serial I/O, and ESP32 timing functions.
#include <freertos/FreeRTOS.h> // FreeRTOS queues and pinned tasks isolate real-time control from networking.
#include <freertos/queue.h> // Import the dependency required by this module.
#include <freertos/task.h> // Import the dependency required by this module.

#include <algorithm> // Standard algorithms clamp the measured loop interval to a safe numeric range.
#include <cstring> // C string functions copy fixed telemetry status messages without heap allocation.

#include "config/board_config.h" // Board and feature configuration are the single source of hardware truth.
#include "config/feature_flags.h" // Import the dependency required by this module.
#include "control/flight_controller.h" // Control modules calculate corrections and map them to four motor outputs.
#include "control/motor_mixer.h" // Import the dependency required by this module.
#include "core/flight_types.h" // Shared types and safety logic connect every flight subsystem.
#include "core/safety_manager.h" // Import the dependency required by this module.
#include "network/flight_link.h" // Networking, navigation, sensors, and optional storage remain separate modules.
#include "navigation/navigation_manager.h" // Import the dependency required by this module.
#include "sensors/sensor_hub.h" // Import the dependency required by this module.
#include "storage/config_store.h" // Import the dependency required by this module.

namespace { // File-local objects are hidden in an anonymous namespace to avoid global name collisions.

using namespace nanokit; // Use shared NanoKit types without repeating the namespace inside this implementation.

QueueHandle_t commandQueue = nullptr; // Single-element queues always retain only the newest command and telemetry snapshot.
QueueHandle_t telemetryQueue = nullptr;

MotorMixer motorMixer; // Each subsystem has one long-lived instance owned by the firmware process.
FlightController flightController; // Execute this statement as part of the current subsystem operation.
SafetyManager safetyManager; // Execute this statement as part of the current subsystem operation.
SensorHub sensorHub; // Execute this statement as part of the current subsystem operation.
FlightLink flightLink; // Execute this statement as part of the current subsystem operation.
NavigationManager navigationManager; // Execute this statement as part of the current subsystem operation.
ConfigStore configStore; // Execute this statement as part of the current subsystem operation.

void populateTelemetry(TelemetryFrame &frame, const PilotCommand &command, // Build a complete, truthful telemetry snapshot from the current flight state.
                       const SensorState &sensors, const MotorOutput &motors, // Continue the current project declaration or implementation.
                       uint32_t commandAgeMs) { // Open this implementation block.
  static uint32_t telemetrySequence = 0; // A local static counter persists between calls without becoming externally writable.
  frame = TelemetryFrame{}; // Reset every field first so no value from an older frame can leak into this one.
  frame.sequence = ++telemetrySequence; // Sequence and acknowledgement fields let the browser measure packet progress.
  frame.acknowledgedCommand = command.sequence;
  frame.uptimeMs = millis();
  frame.commandAgeMs = commandAgeMs;
  frame.sensors = sensors; // Copy the coherent snapshots produced by sensors and the motor mixer.
  frame.motors = motors;
  frame.safetyState = safetyManager.state();
  frame.mode = command.mode;
  frame.throttle = safetyManager.armed() ? command.throttle : 0;
  frame.connectedClients = flightLink.clientCount(); // Publish live connection and safety decisions for the interface controls.
  frame.armed = safetyManager.armed();
  frame.failsafe = safetyManager.failsafe();
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED // Report ESC approval from the same compile-time lock used by MotorMixer.
  frame.escProtocolConfirmed = true;
#else // Select the alternative compile-time feature branch.
  frame.escProtocolConfirmed = false;
#endif // Close the compile-time feature selection.
  std::strncpy(frame.status, safetyManager.status(), sizeof(frame.status) - 1); // Copy the readable safety explanation and always terminate the fixed buffer.
  frame.status[sizeof(frame.status) - 1] = '\0';
}

void flightTask(void *parameter) { // Run the deterministic 250 Hz safety and flight-control pipeline on the flight core.
  (void)parameter; // FreeRTOS requires a generic task parameter even though this task needs no argument.
  PilotCommand command; // These local snapshots are retained across loop iterations by the task stack.
  MotorOutput motors; // Execute this statement as part of the current subsystem operation.
  uint32_t lastCalibrationSequence = 0; // Sequence tracking prevents one calibration button press from being processed repeatedly.
  uint32_t lastTelemetryMs = 0; // Independent timestamps control telemetry pacing and accurate PID timing.
  uint32_t lastLoopUs = micros();
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) { // A flight task is intentionally permanent and is scheduled at a fixed period.
    const uint32_t nowUs = micros(); // Measure real elapsed time so PID math remains stable when scheduling varies slightly.
    float dtSeconds = static_cast<float>(nowUs - lastLoopUs) / 1000000.0f;
    lastLoopUs = nowUs;
    dtSeconds = std::max(0.001f, std::min(0.02f, dtSeconds)); // Reject extreme timing values that could destabilize integral or derivative calculations.

    PilotCommand incoming; // Read the newest command without blocking the real-time loop.
    if (xQueueReceive(commandQueue, &incoming, 0) == pdTRUE) { // Evaluate this condition before continuing.
      command = incoming;
      if (command.calibrateRequested && command.sequence != lastCalibrationSequence) { // Process calibration once for each unique command sequence.
        lastCalibrationSequence = command.sequence;
        sensorHub.requestImuCalibration();
      }
    }

    sensorHub.update(dtSeconds); // Refresh real sensors before validating the requested flight mode.
    const SensorState &sensors = sensorHub.state();
    command.mode = navigationManager.validateRequestedMode(command.mode, sensors);
    const uint32_t nowMs = millis(); // UINT32_MAX clearly marks a command link that has never received a packet.
    const uint32_t commandAgeMs = command.sequence == 0
        ? UINT32_MAX // Continue the current project declaration or implementation.
        : nowMs - command.receivedAtMs; // Execute this statement as part of the current subsystem operation.

    SafetyInputs safetyInputs; // Collect all independent safety facts before asking for one authoritative decision.
    safetyInputs.networkClientConnected = flightLink.clientCount() > 0;
    safetyInputs.commandFresh = command.sequence > 0 &&
                                commandAgeMs <= config::COMMAND_TIMEOUT_MS;
    safetyInputs.imuHealthy = sensors.imuHealthy;
    safetyInputs.imuCalibrated = sensors.imuCalibrated;
    safetyInputs.attitudeValid = sensors.attitude.valid;
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED // Use the same compile-time ESC proof gate throughout the firmware.
    safetyInputs.escProtocolConfirmed = true;
#else // Select the alternative compile-time feature branch.
    safetyInputs.escProtocolConfirmed = false;
#endif // Close the compile-time feature selection.
    safetyInputs.armRequested = command.armRequested; // Pilot intent is evaluated only after hardware and link facts are populated.
    safetyInputs.emergencyStop = command.emergencyStop;
    safetyInputs.throttle = command.throttle;
    safetyManager.update(safetyInputs);

    const bool armed = safetyManager.armed(); // PID output is permitted only when SafetyManager reaches Armed.
    const ControlCorrection correction =
        flightController.update(command, sensors, dtSeconds, armed);
    if (armed) { // Calculate Quad-X pulses only while armed; otherwise retain safe defaults.
      motors = motorMixer.mix(command.throttle, correction.roll,
                              correction.pitch, correction.yaw); // Execute this statement as part of the current subsystem operation.
    } else {
      motors = MotorOutput{};
    }
    motorMixer.write(motors, armed); // MotorMixer applies its own compile-time and runtime safety gates before hardware writes.

    if (nowMs - lastTelemetryMs >= config::TELEMETRY_PERIOD_MS) { // Publish telemetry at its lower configured rate to reduce network and CPU load.
      lastTelemetryMs = nowMs;
      TelemetryFrame frame; // Execute this statement as part of the current subsystem operation.
      populateTelemetry(frame, command, sensors, motors, commandAgeMs);
      xQueueOverwrite(telemetryQueue, &frame); // Overwrite keeps the newest frame if the network task is temporarily busy.
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(config::FLIGHT_LOOP_PERIOD_US / 1000)); // Delay relative to the previous wake time to maintain a stable 250 Hz schedule.
  }
}

void networkTask(void *parameter) { // Run HTTP and WebSocket processing on the separate network core.
  (void)parameter; // Explicitly mark this required parameter as unused.
  flightLink.begin(commandQueue, telemetryQueue); // Networking starts inside its owning task after both cross-core queues exist.
  for (;;) { // Service network events continuously while yielding at the configured interval.
    flightLink.loop();
    vTaskDelay(pdMS_TO_TICKS(config::NETWORK_TASK_DELAY_MS));
  }
}

}  // namespace

void setup() { // Arduino calls setup() once after boot to initialize safe hardware and both tasks.
  Serial.begin(115200); // Start serial diagnostics at the PlatformIO monitor speed and allow USB-UART to settle.
  delay(200);
  Serial.println();
  Serial.println("NanoKit Drone 4X - Wi-Fi Flight Controller");
  Serial.println("Developed by Amine Saoud ibn al-Bashir.");
  Serial.println("[SAFE] Propellers must remain removed during integration.");

  motorMixer.begin(); // Motor outputs are configured first and forced to minimum before any other subsystem starts.
  motorMixer.safe();
  sensorHub.begin();
  configStore.begin(); // Optional storage reports unavailable when its hardware feature remains disabled.
  Serial.print("[STORE] ");
  Serial.println(configStore.status());

  commandQueue = xQueueCreate(1, sizeof(nanokit::PilotCommand)); // Length-one queues implement latest-value exchange between the two pinned tasks.
  telemetryQueue = xQueueCreate(1, sizeof(nanokit::TelemetryFrame));
  if (commandQueue == nullptr || telemetryQueue == nullptr) { // Abort task creation if memory allocation failed; motors already remain at minimum.
    Serial.println("[FAULT] Queue allocation failed. Motors remain at minimum.");
    return; // Return this result to the caller.
  }

  const BaseType_t flightCreated = xTaskCreatePinnedToCore( // Pin flight control to core 1 with higher priority and enough stack for control modules.
      flightTask, "nanokit-flight", 6144, nullptr, // Continue the current project declaration or implementation.
      nanokit::config::FLIGHT_TASK_PRIORITY, nullptr, // Continue the current project declaration or implementation.
      nanokit::config::FLIGHT_TASK_CORE); // Execute this statement as part of the current subsystem operation.
  const BaseType_t networkCreated = xTaskCreatePinnedToCore( // Pin network processing to core 0 with lower priority and a larger web-server stack.
      networkTask, "nanokit-network", 8192, nullptr, // Continue the current project declaration or implementation.
      nanokit::config::NETWORK_TASK_PRIORITY, nullptr, // Continue the current project declaration or implementation.
      nanokit::config::NETWORK_TASK_CORE); // Execute this statement as part of the current subsystem operation.

  if (flightCreated != pdPASS || networkCreated != pdPASS) { // Any task-creation failure returns motor outputs to the safe minimum.
    motorMixer.safe();
    Serial.println("[FAULT] Task creation failed. Motors remain at minimum.");
  }
}

void loop() { // Arduino still requires loop(), but all meaningful work is owned by FreeRTOS tasks.
  delay(1000); // Work is isolated in pinned FreeRTOS tasks. Arduino loop stays non-critical.
}

// NanoKit Drone 4X Wi-Fi flight-controller reference firmware.
// Developed by Amine Saoud ibn al-Bashir.
// Keep propellers removed until every documented bench test has passed.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <algorithm>
#include <cstring>

#include "config/board_config.h"
#include "config/feature_flags.h"
#include "control/flight_controller.h"
#include "control/motor_mixer.h"
#include "core/flight_types.h"
#include "core/safety_manager.h"
#include "network/flight_link.h"
#include "navigation/navigation_manager.h"
#include "sensors/sensor_hub.h"
#include "storage/config_store.h"

namespace {

using namespace nanokit;

QueueHandle_t commandQueue = nullptr;
QueueHandle_t telemetryQueue = nullptr;

MotorMixer motorMixer;
FlightController flightController;
SafetyManager safetyManager;
SensorHub sensorHub;
FlightLink flightLink;
NavigationManager navigationManager;
ConfigStore configStore;

void populateTelemetry(TelemetryFrame &frame, const PilotCommand &command,
                       const SensorState &sensors, const MotorOutput &motors,
                       uint32_t commandAgeMs) {
  static uint32_t telemetrySequence = 0;
  frame = TelemetryFrame{};
  frame.sequence = ++telemetrySequence;
  frame.acknowledgedCommand = command.sequence;
  frame.uptimeMs = millis();
  frame.commandAgeMs = commandAgeMs;
  frame.sensors = sensors;
  frame.motors = motors;
  frame.safetyState = safetyManager.state();
  frame.mode = command.mode;
  frame.throttle = safetyManager.armed() ? command.throttle : 0;
  frame.connectedClients = flightLink.clientCount();
  frame.armed = safetyManager.armed();
  frame.failsafe = safetyManager.failsafe();
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED
  frame.escProtocolConfirmed = true;
#else
  frame.escProtocolConfirmed = false;
#endif
  std::strncpy(frame.status, safetyManager.status(), sizeof(frame.status) - 1);
  frame.status[sizeof(frame.status) - 1] = '\0';
}

void flightTask(void *parameter) {
  (void)parameter;
  PilotCommand command;
  MotorOutput motors;
  uint32_t lastCalibrationSequence = 0;
  uint32_t lastTelemetryMs = 0;
  uint32_t lastLoopUs = micros();
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    const uint32_t nowUs = micros();
    float dtSeconds = static_cast<float>(nowUs - lastLoopUs) / 1000000.0f;
    lastLoopUs = nowUs;
    dtSeconds = std::max(0.001f, std::min(0.02f, dtSeconds));

    PilotCommand incoming;
    if (xQueueReceive(commandQueue, &incoming, 0) == pdTRUE) {
      command = incoming;
      if (command.calibrateRequested && command.sequence != lastCalibrationSequence) {
        lastCalibrationSequence = command.sequence;
        sensorHub.requestImuCalibration();
      }
    }

    sensorHub.update(dtSeconds);
    const SensorState &sensors = sensorHub.state();
    command.mode = navigationManager.validateRequestedMode(command.mode, sensors);
    const uint32_t nowMs = millis();
    const uint32_t commandAgeMs = command.sequence == 0
        ? UINT32_MAX
        : nowMs - command.receivedAtMs;

    SafetyInputs safetyInputs;
    safetyInputs.networkClientConnected = flightLink.clientCount() > 0;
    safetyInputs.commandFresh = command.sequence > 0 &&
                                commandAgeMs <= config::COMMAND_TIMEOUT_MS;
    safetyInputs.imuHealthy = sensors.imuHealthy;
    safetyInputs.imuCalibrated = sensors.imuCalibrated;
    safetyInputs.attitudeValid = sensors.attitude.valid;
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED
    safetyInputs.escProtocolConfirmed = true;
#else
    safetyInputs.escProtocolConfirmed = false;
#endif
    safetyInputs.armRequested = command.armRequested;
    safetyInputs.emergencyStop = command.emergencyStop;
    safetyInputs.throttle = command.throttle;
    safetyManager.update(safetyInputs);

    const bool armed = safetyManager.armed();
    const ControlCorrection correction =
        flightController.update(command, sensors, dtSeconds, armed);
    if (armed) {
      motors = motorMixer.mix(command.throttle, correction.roll,
                              correction.pitch, correction.yaw);
    } else {
      motors = MotorOutput{};
    }
    motorMixer.write(motors, armed);

    if (nowMs - lastTelemetryMs >= config::TELEMETRY_PERIOD_MS) {
      lastTelemetryMs = nowMs;
      TelemetryFrame frame;
      populateTelemetry(frame, command, sensors, motors, commandAgeMs);
      xQueueOverwrite(telemetryQueue, &frame);
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(config::FLIGHT_LOOP_PERIOD_US / 1000));
  }
}

void networkTask(void *parameter) {
  (void)parameter;
  flightLink.begin(commandQueue, telemetryQueue);
  for (;;) {
    flightLink.loop();
    vTaskDelay(pdMS_TO_TICKS(config::NETWORK_TASK_DELAY_MS));
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("NanoKit Drone 4X - Wi-Fi Flight Controller");
  Serial.println("Developed by Amine Saoud ibn al-Bashir.");
  Serial.println("[SAFE] Propellers must remain removed during integration.");

  motorMixer.begin();
  motorMixer.safe();
  sensorHub.begin();
  configStore.begin();
  Serial.print("[STORE] ");
  Serial.println(configStore.status());

  commandQueue = xQueueCreate(1, sizeof(nanokit::PilotCommand));
  telemetryQueue = xQueueCreate(1, sizeof(nanokit::TelemetryFrame));
  if (commandQueue == nullptr || telemetryQueue == nullptr) {
    Serial.println("[FAULT] Queue allocation failed. Motors remain at minimum.");
    return;
  }

  const BaseType_t flightCreated = xTaskCreatePinnedToCore(
      flightTask, "nanokit-flight", 6144, nullptr,
      nanokit::config::FLIGHT_TASK_PRIORITY, nullptr,
      nanokit::config::FLIGHT_TASK_CORE);
  const BaseType_t networkCreated = xTaskCreatePinnedToCore(
      networkTask, "nanokit-network", 8192, nullptr,
      nanokit::config::NETWORK_TASK_PRIORITY, nullptr,
      nanokit::config::NETWORK_TASK_CORE);

  if (flightCreated != pdPASS || networkCreated != pdPASS) {
    motorMixer.safe();
    Serial.println("[FAULT] Task creation failed. Motors remain at minimum.");
  }
}

void loop() {
  // Work is isolated in pinned FreeRTOS tasks. Arduino loop stays non-critical.
  delay(1000);
}

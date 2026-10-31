// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>

namespace {
constexpr uint8_t LED_CORE0_PIN = 26;         // NanoKit Pin 37, GPIO26. Safe general-purpose output; drives the external LED that belongs to the core 0 task.
constexpr uint8_t LED_CORE1_PIN = 2;          // NanoKit Pin 17, GPIO2. The onboard LED, driven by the core 1 task, so the example shows something with zero wiring too.
constexpr uint32_t SERIAL_BAUD = 115200;      // Standard NanoKit Serial Monitor speed.
constexpr uint32_t CORE0_BLINK_MS = 1000;     // Slow blink on core 0: easy to tell apart from the fast one by eye.
constexpr uint32_t CORE1_BLINK_MS = 300;      // Fast blink on core 1. The two rhythms never drift into each other because each task keeps its own timing.
constexpr uint32_t REPORT_INTERVAL_MS = 5000; // loop() prints memory figures every 5 s without disturbing the tasks.
constexpr uint32_t TASK_STACK_BYTES = 2048;   // On ESP32 the stack size is given in bytes. Enough for a few locals and Serial prints; the report shows how much is really used.
constexpr UBaseType_t TASK_PRIORITY = 1;      // Same priority as the Arduino loop task and above the idle task (0), so no task starves another.
constexpr BaseType_t CORE_0 = 0;              // PRO_CPU: also home of the Wi-Fi/Bluetooth stacks and most system services.
constexpr BaseType_t CORE_1 = 1;              // APP_CPU: where setup() and loop() run by default, and the usual place for time-critical user work.

TaskHandle_t core0TaskHandle = nullptr;       // Handles let loop() ask the scheduler about each task later (stack high-water mark).
TaskHandle_t core1TaskHandle = nullptr;
uint32_t lastReportMs = 0;                    // Timestamp of the last memory report.

void announceTask(const char *name) {
  Serial.print(name);                         // Serial is shared by both cores; keep each print short so lines rarely interleave.
  Serial.print(" started on core ");
  Serial.println(xPortGetCoreID());           // Asks the hardware which core is executing this very line: proves the pinning worked.
}

void blinkCore0Task(void *) {                 // Task functions take one void* argument (unused here) and must never return.
  announceTask("blinkCore0Task");
  bool on = false;
  for (;;) {
    on = !on;
    digitalWrite(LED_CORE0_PIN, on ? HIGH : LOW);
    vTaskDelay(pdMS_TO_TICKS(CORE0_BLINK_MS)); // Puts only this task to sleep; core 0 is free for Wi-Fi, the idle task, and the watchdog feed.
  }
}

void blinkCore1Task(void *) {
  announceTask("blinkCore1Task");
  bool on = false;
  for (;;) {
    on = !on;
    digitalWrite(LED_CORE1_PIN, on ? HIGH : LOW);
    vTaskDelay(pdMS_TO_TICKS(CORE1_BLINK_MS)); // 300 ms converted to scheduler ticks (1 ms each on the Arduino core).
  }
}

void printMemoryReport() {
  Serial.print("Report | free heap: ");
  Serial.print(ESP.getFreeHeap());            // Bytes still available for new tasks, buffers, or String objects. A steady value means no leak.
  Serial.print(" B | stack never used: core0 task ");
  Serial.print(uxTaskGetStackHighWaterMark(core0TaskHandle)); // Smallest free stack ever seen for that task, in bytes. Near zero = about to overflow.
  Serial.print(" B, core1 task ");
  Serial.print(uxTaskGetStackHighWaterMark(core1TaskHandle));
  Serial.println(" B");
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(LED_CORE0_PIN, OUTPUT);             // Configure both pins before any task touches them; the tasks start as soon as they are created.
  pinMode(LED_CORE1_PIN, OUTPUT);
  Serial.println("NanoKit dual core tasks example ready.");
  Serial.print("setup() and loop() run on core ");
  Serial.println(xPortGetCoreID());           // Prints 1: the Arduino core pins the loop task to core 1 by default.
  Serial.println("LEDs: core 0 task on NanoKit Pin 37 / GPIO26 (1000 ms), core 1 task on NanoKit Pin 17 / GPIO2 (300 ms).");

  xTaskCreatePinnedToCore(blinkCore0Task,     // Function the task runs forever.
                          "blinkCore0",       // Name shown by debugging tools; at most 16 characters on ESP32.
                          TASK_STACK_BYTES,   // Stack reserved for this task, in bytes.
                          nullptr,            // Argument passed to the task function (unused).
                          TASK_PRIORITY,
                          &core0TaskHandle,   // Receives the handle used later by uxTaskGetStackHighWaterMark.
                          CORE_0);            // Pin to core 0: the task will never run on core 1.
  xTaskCreatePinnedToCore(blinkCore1Task, "blinkCore1", TASK_STACK_BYTES, nullptr, TASK_PRIORITY, &core1TaskHandle, CORE_1);
}

void loop() {
  const uint32_t nowMs = millis();            // loop() is itself a FreeRTOS task on core 1 and shares that core with blinkCore1Task.
  if (nowMs - lastReportMs >= REPORT_INTERVAL_MS) {
    lastReportMs = nowMs;
    printMemoryReport();
  }
  vTaskDelay(pdMS_TO_TICKS(10));              // Yield the core between checks so the blink task and the idle task get their turn.
}

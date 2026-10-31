// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>

namespace {
constexpr uint8_t TOUCH_PIN = 4;          // NanoKit Pin 10, GPIO4. Touch channel T0: one of the ten GPIOs wired to the ESP32 capacitive touch sensor block.
constexpr uint8_t LED_PIN = 26;           // NanoKit Pin 37, GPIO26. Safe general-purpose output that drives the external LED through a 220 ohm resistor.
constexpr uint32_t SERIAL_BAUD = 115200;  // Standard NanoKit Serial Monitor speed.
constexpr uint8_t BASELINE_SAMPLES = 32;  // Enough samples to average out noise while the pad is untouched.
constexpr uint32_t BASELINE_GAP_MS = 30;  // 32 samples x 30 ms is about one second of hands-off calibration at start-up.
constexpr float TOUCH_RATIO = 0.7f;       // A finger must pull the reading down to 70 % of the baseline (a 30 % drop) to count as a touch.
constexpr uint16_t MIN_DROP_COUNTS = 10;  // A touch must pull the reading at least 10 counts below the baseline, so noise on a low baseline cannot trigger it.
constexpr uint32_t POLL_INTERVAL_MS = 50; // 20 readings per second is plenty for a finger and keeps the Serial output readable.
constexpr uint32_t REPORT_INTERVAL_MS = 250; // Print four lines per second: enough to watch the value move, not enough to flood the monitor.

uint16_t baseline = 0;                    // Untouched reading measured at start-up; different for every board and every wire.
uint16_t threshold = 0;                   // Readings below this value mean "touched".
uint16_t lastValue = 0;                   // Most recent polled reading, shown in the status line.
bool touched = false;                     // Debounced-by-polling state that drives the LED.
volatile bool touchIsrFlag = false;       // Set by the interrupt, cleared by loop(); volatile because the ISR changes it behind the compiler's back.
uint32_t isrEventCount = 0;               // How many times loop() has seen the flag raised.
uint32_t lastPollMs = 0;                  // Timestamp of the last touchRead() in loop().
uint32_t lastReportMs = 0;                // Timestamp of the last status line.

void IRAM_ATTR onTouchIsr() {             // IRAM_ATTR keeps the handler in RAM so it can run even while flash is busy.
  touchIsrFlag = true;                    // Do the minimum inside an ISR: raise a flag, let loop() do the printing.
}

uint16_t measureBaseline() {
  (void)touchRead(TOUCH_PIN);             // First call configures the touch peripheral; its value is discarded on purpose.
  uint32_t sum = 0;
  for (uint8_t i = 0; i < BASELINE_SAMPLES; ++i) {
    sum += touchRead(TOUCH_PIN);
    delay(BASELINE_GAP_MS);               // delay() is acceptable here: this runs once in setup(), before the non-blocking loop starts.
  }
  return static_cast<uint16_t>(sum / BASELINE_SAMPLES);
}

uint16_t computeThreshold(uint16_t base) {
  const uint16_t ratioThreshold = static_cast<uint16_t>(base * TOUCH_RATIO);              // 70 % of the baseline.
  const uint16_t dropThreshold = base > MIN_DROP_COUNTS ? base - MIN_DROP_COUNTS : 0;      // At least 10 counts below the baseline.
  return ratioThreshold < dropThreshold ? ratioThreshold : dropThreshold;                 // Take the lower of the two, i.e. the stricter one, so noise never counts as a touch.
}

void printStatus() {
  Serial.print("value: ");
  Serial.print(lastValue);
  Serial.print(" | baseline: ");
  Serial.print(baseline);
  Serial.print(" | threshold: ");
  Serial.print(threshold);
  Serial.print(" | state: ");
  Serial.print(touched ? "TOUCHED" : "released");
  Serial.print(" | ISR events: ");
  Serial.println(isrEventCount);
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(LED_PIN, OUTPUT);                          // GPIO26 must be an output before digitalWrite can drive the LED.
  digitalWrite(LED_PIN, LOW);                        // Start with the LED off.

  Serial.println("NanoKit touch sensor example.");
  Serial.println("Touch pad: NanoKit Pin 10 / GPIO4 (T0). LED: NanoKit Pin 37 / GPIO26.");
  Serial.println("Measuring the baseline: do NOT touch the pad during the first second...");
  baseline = measureBaseline();                      // On classic ESP32 the reading DROPS when touched, so the baseline is the highest normal value.
  threshold = computeThreshold(baseline);
  Serial.print("Baseline: ");
  Serial.print(baseline);
  Serial.print(" | threshold: ");
  Serial.println(threshold);

  touchAttachInterrupt(TOUCH_PIN, onTouchIsr, threshold); // The hardware compares every measurement with the threshold and raises an interrupt when the value is below it.
  Serial.println("Touch the pad: the LED lights and the status line changes.");
}

void loop() {
  const uint32_t nowMs = millis();

  if (nowMs - lastPollMs >= POLL_INTERVAL_MS) {      // Polling path: read the pad, decide, drive the LED.
    lastPollMs = nowMs;
    lastValue = touchRead(TOUCH_PIN);                // Returns a count that falls when a finger adds capacitance to the pad.
    touched = lastValue < threshold;                 // Below the threshold means touched; above means released.
    digitalWrite(LED_PIN, touched ? HIGH : LOW);     // The LED mirrors the polled state.
  }

  if (touchIsrFlag) {                                // Interrupt path: the hardware noticed a touch on its own, without touchRead().
    touchIsrFlag = false;                            // Clear the flag so the next interrupt can be counted.
    ++isrEventCount;                                 // The counter keeps climbing while the finger stays, one event per touch measurement cycle.
  }

  if (nowMs - lastReportMs >= REPORT_INTERVAL_MS) {  // Periodic report proves the loop is alive and shows the numbers to tune.
    lastReportMs = nowMs;
    printStatus();
  }
}

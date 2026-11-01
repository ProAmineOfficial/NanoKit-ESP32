// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>

namespace {
constexpr uint8_t BUTTON_PIN = 4;            // NanoKit Pin 10, GPIO4. General GPIO with an internal pull-up; every ESP32 GPIO can raise an interrupt.
constexpr uint8_t LED_PIN = 26;              // NanoKit Pin 37, GPIO26. Safe general-purpose output that drives the external LED through a resistor.
constexpr uint32_t TIMER_FREQUENCY_HZ = 1000000; // 1 MHz tick rate: the core divides the 80 MHz APB clock by 80, so one tick is one microsecond.
constexpr uint32_t DEBOUNCE_US = 50000;      // An edge within 50 ms of the last accepted press is contact bounce, not a new press.
constexpr uint32_t BLINK_PERIODS_MS[] = {1000, 500, 250, 125}; // Each press selects the next period, then wraps back to the first.
constexpr uint8_t PERIOD_COUNT = sizeof(BLINK_PERIODS_MS) / sizeof(BLINK_PERIODS_MS[0]);
constexpr uint32_t REPORT_INTERVAL_MS = 2000; // Periodic status line proves the loop is alive between presses.
constexpr uint32_t SERIAL_BAUD = 115200;     // Standard NanoKit Serial Monitor speed.

hw_timer_t *blinkTimer = nullptr;            // Handle returned by timerBegin; every timer* call needs it. The ESP32 has four 64-bit hardware timers.
portMUX_TYPE sharedMux = portMUX_INITIALIZER_UNLOCKED; // Spinlock that makes access to the shared variables below atomic.

volatile bool timerFired = false;            // Set by the timer ISR, cleared by loop(). volatile stops the compiler from caching it in a register.
volatile bool pressPending = false;          // Set by the button ISR, cleared by loop().
volatile uint32_t pressCount = 0;            // Incremented by the button ISR, read by loop().
uint32_t lastAcceptedPressUs = 0;            // Used only inside the button ISR, so it needs neither volatile nor the lock.

uint8_t periodIndex = 0;                     // Which entry of BLINK_PERIODS_MS is active; loop() owns it.
bool ledOn = false;                          // Current LED state; loop() owns it.
uint32_t toggleCount = 0;                    // How many times the LED has toggled; loop() owns it.
uint32_t lastReportMs = 0;                   // Timestamp of the last status line.

// Runs the moment GPIO4 goes HIGH -> LOW. IRAM_ATTR keeps the code in RAM so it can run even while
// the flash cache is busy. It does the bare minimum: reject bounce, count, raise a flag, return.
void IRAM_ATTR onButtonIsr() {
  const uint32_t nowUs = micros();                    // micros() is safe inside an ISR; millis()/delay()/Serial are not.
  if (nowUs - lastAcceptedPressUs < DEBOUNCE_US) {    // Too soon after the last accepted press: this edge is bounce.
    return;
  }
  lastAcceptedPressUs = nowUs;
  portENTER_CRITICAL_ISR(&sharedMux);                 // Lock so loop() can never see a half-updated pair of values.
  ++pressCount;
  pressPending = true;
  portEXIT_CRITICAL_ISR(&sharedMux);
}

// Runs every time the timer alarm fires. It only raises a flag; loop() does the actual LED work.
void IRAM_ATTR onTimerIsr() {
  portENTER_CRITICAL_ISR(&sharedMux);
  timerFired = true;
  portEXIT_CRITICAL_ISR(&sharedMux);
}

void applyBlinkPeriod() {
  const uint64_t periodUs = static_cast<uint64_t>(BLINK_PERIODS_MS[periodIndex]) * 1000ULL; // Ticks are microseconds, so ms * 1000.
  timerWrite(blinkTimer, 0);                          // Restart the count so the new alarm value is always ahead of the counter.
  timerAlarm(blinkTimer, periodUs, true, 0);          // Alarm every periodUs ticks; true = auto-reload for a steady period; 0 = repeat forever.
}

void printStatus(const char *reason, uint32_t presses) {
  Serial.print(reason);
  Serial.print(" | blink period: ");
  Serial.print(BLINK_PERIODS_MS[periodIndex]);
  Serial.print(" ms | toggles: ");
  Serial.print(toggleCount);
  Serial.print(" | presses: ");
  Serial.println(presses);
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(BUTTON_PIN, INPUT_PULLUP);                  // Pull-up holds GPIO4 HIGH; a press pulls it LOW, which is the FALLING edge we wait for.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonIsr, FALLING); // The GPIO hardware now calls the ISR on each HIGH -> LOW edge.

  blinkTimer = timerBegin(TIMER_FREQUENCY_HZ);        // Claims a free hardware timer counting up at 1 MHz (1 us per tick); nullptr if none is free.
  if (blinkTimer == nullptr) {
    Serial.println("No free hardware timer.");
    return;
  }
  timerAttachInterrupt(blinkTimer, &onTimerIsr);      // The timer interrupt now calls onTimerIsr every time the alarm fires.
  applyBlinkPeriod();                                 // Load and arm the first period (1000 ms); timerAlarm starts the alarm.

  Serial.println("NanoKit interrupts and timers example ready.");
  Serial.println("Button: NanoKit Pin 10 / GPIO4 (to GND, FALLING interrupt). LED: NanoKit Pin 37 / GPIO26.");
  Serial.println("Hardware timer at 1 MHz (80 MHz / 80 = 1 us ticks). Press the button to cycle 1000 -> 500 -> 250 -> 125 ms.");
}

void loop() {
  portENTER_CRITICAL(&sharedMux);                     // Copy and clear the flags atomically; an ISR cannot run between these lines.
  const bool handleTimer = timerFired;
  const bool handlePress = pressPending;
  const uint32_t presses = pressCount;
  timerFired = false;
  pressPending = false;
  portEXIT_CRITICAL(&sharedMux);

  if (handleTimer) {                                  // The alarm fired: all the real work happens here, outside the ISR.
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    ++toggleCount;
  }

  if (handlePress) {                                  // A debounced press was confirmed: move to the next blink period.
    periodIndex = (periodIndex + 1) % PERIOD_COUNT;
    applyBlinkPeriod();
    printStatus("Press detected", presses);           // Serial only ever runs from loop(), never from an ISR.
  }

  const uint32_t nowMs = millis();
  if (nowMs - lastReportMs >= REPORT_INTERVAL_MS) {   // Periodic heartbeat so the reader can watch the toggle counter grow.
    lastReportMs = nowMs;
    printStatus("Status", presses);
  }
}

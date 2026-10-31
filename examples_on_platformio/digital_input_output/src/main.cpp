// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>

namespace {
constexpr uint8_t BUTTON_PIN = 4;        // NanoKit Pin 10, GPIO4. General GPIO with an internal pull-up, so the button needs no external resistor.
constexpr uint8_t LED_PIN = 26;          // NanoKit Pin 37, GPIO26. Safe general-purpose output that drives the external LED through a resistor.
constexpr uint8_t BUILTIN_LED_PIN = 2;   // NanoKit Pin 17, GPIO2. The onboard LED mirrors the external LED so the example works with zero wiring too.
constexpr uint32_t SERIAL_BAUD = 115200; // Standard NanoKit Serial Monitor speed.
constexpr uint32_t DEBOUNCE_MS = 40;     // Mechanical contacts bounce for a few milliseconds; 40 ms hides every bounce without feeling slow.
constexpr uint32_t REPORT_INTERVAL_MS = 2000; // A periodic status line proves the loop is alive even when nobody presses the button.

bool ledOn = false;                      // The state we want the LED to show; toggled by each confirmed button press.
int stableButtonLevel = HIGH;            // Last level that survived debouncing. HIGH means released because of the pull-up.
int lastRawLevel = HIGH;                 // Most recent raw reading, used to detect when the signal starts changing.
uint32_t lastChangeMs = 0;               // Timestamp of the last raw change, which starts the debounce window.
uint32_t lastReportMs = 0;               // Timestamp of the last periodic status report.
uint32_t pressCount = 0;                 // Counts confirmed presses so the Serial Monitor shows that bounces are rejected.

void applyLedState() {
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);         // HIGH sources current from GPIO26 into the LED, so HIGH means "on".
  digitalWrite(BUILTIN_LED_PIN, ledOn ? HIGH : LOW); // The onboard LED follows the same state for boards without wiring.
}

void printState(const char *reason) {
  Serial.print(reason);                              // Explain why this line was printed: a press, or the periodic report.
  Serial.print(" | button: ");
  Serial.print(stableButtonLevel == LOW ? "pressed" : "released"); // With INPUT_PULLUP a pressed button reads LOW, not HIGH.
  Serial.print(" | LED: ");
  Serial.print(ledOn ? "ON" : "OFF");
  Serial.print(" | presses: ");
  Serial.println(pressCount);
}

void updateButton() {
  const int rawLevel = digitalRead(BUTTON_PIN);      // Read the pin every loop; this costs almost nothing on ESP32.
  const uint32_t nowMs = millis();

  if (rawLevel != lastRawLevel) {                    // The signal moved: either a real press/release or a bounce.
    lastRawLevel = rawLevel;                         // Remember the new raw level so the next change restarts the window.
    lastChangeMs = nowMs;                            // Restart the debounce window; the level must now stay stable.
  }

  const bool stableLongEnough = (nowMs - lastChangeMs) >= DEBOUNCE_MS; // Only accept a level that held for the whole window.
  if (stableLongEnough && rawLevel != stableButtonLevel) {             // A new stable level is different from the old one.
    stableButtonLevel = rawLevel;                    // Commit the debounced level.
    if (stableButtonLevel == LOW) {                  // Act on the press edge only, so holding the button does not toggle repeatedly.
      ledOn = !ledOn;                                // Toggle the LED once per press.
      ++pressCount;
      applyLedState();
      printState("Press detected");
    }
  }
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(BUTTON_PIN, INPUT_PULLUP);                 // The internal pull-up holds GPIO4 HIGH until the button connects it to GND.
  pinMode(LED_PIN, OUTPUT);                          // GPIO26 must be an output before digitalWrite can drive the LED.
  pinMode(BUILTIN_LED_PIN, OUTPUT);                  // GPIO2 drives the onboard LED.
  applyLedState();                                   // Start with both LEDs off so the first press turns them on.
  Serial.println("NanoKit digital input/output example ready.");
  Serial.println("Button: NanoKit Pin 10 / GPIO4 (to GND). LED: NanoKit Pin 37 / GPIO26 (+ onboard LED on Pin 17 / GPIO2).");
  Serial.println("Press the button to toggle the LED.");
}

void loop() {
  updateButton();                                    // Non-blocking debounce keeps the loop responsive; no delay() is needed.

  const uint32_t nowMs = millis();
  if (nowMs - lastReportMs >= REPORT_INTERVAL_MS) {  // Periodic heartbeat so beginners can see the loop running.
    lastReportMs = nowMs;
    printState("Status");
  }
}

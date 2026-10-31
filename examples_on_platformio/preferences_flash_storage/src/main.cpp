// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>
#include <Preferences.h>

namespace {
constexpr uint8_t BUTTON_PIN = 4;             // NanoKit Pin 10, GPIO4. General GPIO with an internal pull-up, so the button needs no external resistor.
constexpr uint8_t BUILTIN_LED_PIN = 2;        // NanoKit Pin 17, GPIO2. The onboard LED shows the saved state, so the example needs only the button wired.
constexpr uint32_t SERIAL_BAUD = 115200;      // Standard NanoKit Serial Monitor speed.
constexpr uint32_t DEBOUNCE_MS = 40;          // Mechanical contacts bounce for a few milliseconds; 40 ms hides every bounce without feeling slow.
constexpr size_t COMMAND_MAX_LENGTH = 15;     // Longest Serial command we accept; "clear" and "stats" fit with room to spare.
constexpr const char *NVS_NAMESPACE = "nanokit"; // Namespace: a drawer inside NVS that keeps this example's keys apart from other firmware. Max 15 characters.
constexpr const char *KEY_BOOTS = "boots";    // Key names are limited to 15 characters too.
constexpr const char *KEY_LED = "led";

Preferences prefs;                            // Handle to the open namespace; stays open for the whole program.
bool ledOn = false;                           // The state we want the LED to show; restored from flash at boot, toggled by each press.
uint32_t bootCount = 0;                       // How many times the board has started since the namespace was last cleared.
int stableButtonLevel = HIGH;                 // Last level that survived debouncing. HIGH means released because of the pull-up.
int lastRawLevel = HIGH;                      // Most recent raw reading, used to detect when the signal starts changing.
uint32_t lastChangeMs = 0;                    // Timestamp of the last raw change, which starts the debounce window.
char commandBuffer[COMMAND_MAX_LENGTH + 1];   // Characters typed so far, plus room for the terminating '\0'.
size_t commandLength = 0;

void applyLedState() {
  digitalWrite(BUILTIN_LED_PIN, ledOn ? HIGH : LOW);
}

void printStats() {
  Serial.print("Stats | boots: ");
  Serial.print(bootCount);
  Serial.print(" | LED: ");
  Serial.print(ledOn ? "ON" : "OFF");
  Serial.print(" | free NVS entries: ");
  Serial.println(prefs.freeEntries());        // How many more key/value pairs the NVS partition can hold; it only matters for big projects.
}

void saveLedState() {
  prefs.putBool(KEY_LED, ledOn);              // One flash write per press. Flash wears out after many writes, so this must never run every loop.
  Serial.print("Saved | LED: ");
  Serial.println(ledOn ? "ON" : "OFF");
}

void updateButton() {
  const int rawLevel = digitalRead(BUTTON_PIN);
  const uint32_t nowMs = millis();
  if (rawLevel != lastRawLevel) {             // The signal moved: either a real press/release or a bounce.
    lastRawLevel = rawLevel;
    lastChangeMs = nowMs;                     // Restart the debounce window; the level must now stay stable.
  }
  const bool stableLongEnough = (nowMs - lastChangeMs) >= DEBOUNCE_MS;
  if (stableLongEnough && rawLevel != stableButtonLevel) {
    stableButtonLevel = rawLevel;             // Commit the debounced level.
    if (stableButtonLevel == LOW) {           // Press edge only: holding the button toggles once, not continuously.
      ledOn = !ledOn;
      applyLedState();
      saveLedState();                         // Save right away, so a reset one second later still finds the new state.
    }
  }
}

void handleCommand(const char *line) {
  if (strcmp(line, "clear") == 0) {
    prefs.clear();                            // Erases every key in the "nanokit" namespace; keys of other namespaces are untouched.
    prefs.end();                              // Close the handle cleanly before the reset.
    Serial.println("Namespace cleared. Rebooting; the boot counter restarts at 1.");
    Serial.flush();                           // Let the message leave the UART before the chip restarts.
    ESP.restart();
  } else if (strcmp(line, "stats") == 0) {
    printStats();
  } else {
    Serial.println("Unknown command. Type 'stats' or 'clear'.");
  }
}

void readSerialCommands() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\n' || c == '\r') {             // End of line: the Serial Monitor sends this when you press Enter.
      if (commandLength > 0) {
        commandBuffer[commandLength] = '\0';  // Turn the collected characters into a C string.
        handleCommand(commandBuffer);
        commandLength = 0;                    // Start collecting the next command.
      }
    } else if (commandLength < COMMAND_MAX_LENGTH) {
      commandBuffer[commandLength++] = c;     // Extra characters beyond the limit are dropped, so the buffer can never overflow.
    }
  }
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(BUTTON_PIN, INPUT_PULLUP);          // The internal pull-up holds GPIO4 HIGH until the button connects it to GND.
  pinMode(BUILTIN_LED_PIN, OUTPUT);

  prefs.begin(NVS_NAMESPACE, false);          // false = read-write. The namespace is created the first time it is opened.
  bootCount = prefs.getUInt(KEY_BOOTS, 0) + 1; // Default 0 on a fresh chip, so the very first boot is number 1.
  prefs.putUInt(KEY_BOOTS, bootCount);        // One write per boot: the only write in this program not caused by a button press.
  ledOn = prefs.getBool(KEY_LED, false);      // Restore the LED exactly as it was before the reset or power loss.
  applyLedState();

  Serial.println("NanoKit Preferences flash storage example ready.");
  Serial.println("Button: NanoKit Pin 10 / GPIO4 (to GND). LED: onboard, NanoKit Pin 17 / GPIO2.");
  Serial.println("Press the button to toggle and save the LED. Serial commands: stats, clear.");
  printStats();
}

void loop() {
  updateButton();                             // Non-blocking debounce; writes to flash only on a confirmed press.
  readSerialCommands();                       // Collects typed characters without ever waiting for them.
}

// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>

namespace {
constexpr uint8_t BUILTIN_LED_PIN = 2;        // NanoKit Pin 17, GPIO2. Onboard LED is the magnet indicator, so the example needs no wiring at all.
constexpr uint32_t SERIAL_BAUD = 115200;      // Standard NanoKit Serial Monitor speed.
constexpr uint16_t BASELINE_SAMPLES = 64;     // Many samples at start-up give a stable "no magnet" reference.
constexpr uint8_t SAMPLES_PER_READING = 16;   // Each live reading averages 16 samples; the raw sensor output jumps around by tens of counts.
constexpr uint32_t READ_INTERVAL_MS = 100;    // Ten readings per second: fast enough to follow a hand-held magnet, slow enough to read on screen.
constexpr int DETECT_THRESHOLD = 30;          // A delta larger than this (either sign) counts as a magnet. Starting point: tune it for your magnet and module.

int baseline = 0;                             // Sensor reading with no magnet nearby, measured once at start-up.
uint32_t lastReadMs = 0;                      // Timestamp of the last live reading.

int averageHall(uint16_t samples) {
  long sum = 0;                               // hallRead() can return negative values, so a signed accumulator is required.
  for (uint16_t i = 0; i < samples; ++i) {
    sum += hallRead();                        // Reads the Hall element through ADC1 channels 0 and 3 (NanoKit Pin 5 / GPIO36 and Pin 4 / GPIO39).
  }
  return static_cast<int>(sum / samples);     // Averaging divides the noise by roughly the square root of the sample count.
}

const char *polarityWord(int delta) {
  if (delta > DETECT_THRESHOLD) {
    return "north face (positive delta)";      // Which sign belongs to which pole depends on how the module sits on the board; calibrate once with a known magnet.
  }
  if (delta < -DETECT_THRESHOLD) {
    return "south face (negative delta)";      // The opposite pole pushes the reading the other way.
  }
  return "no magnet";
}

void printReading(int value, int delta) {
  Serial.print("value: ");
  Serial.print(value);
  Serial.print(" | baseline: ");
  Serial.print(baseline);
  Serial.print(" | delta: ");
  Serial.print(delta);
  Serial.print(" | ");
  Serial.println(polarityWord(delta));
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(BUILTIN_LED_PIN, OUTPUT);                  // GPIO2 drives the onboard LED.
  digitalWrite(BUILTIN_LED_PIN, LOW);                // Start with the indicator off.

  Serial.println("NanoKit Hall-effect sensor example.");
  Serial.println("No wiring needed. Keep NanoKit Pin 4 / GPIO39 and Pin 5 / GPIO36 unconnected: hallRead() uses them.");
  Serial.println("Measuring the baseline: keep magnets away from the module...");
  baseline = averageHall(BASELINE_SAMPLES);          // The baseline absorbs the offset of this particular chip, so only the change matters afterwards.
  Serial.print("Baseline: ");
  Serial.println(baseline);
  Serial.println("Bring a magnet near the metal shield of the NanoKit-iM module. LED on NanoKit Pin 17 / GPIO2 lights on detection.");
}

void loop() {
  const uint32_t nowMs = millis();
  if (nowMs - lastReadMs < READ_INTERVAL_MS) {       // Non-blocking pacing: come back later instead of delay().
    return;
  }
  lastReadMs = nowMs;

  const int value = averageHall(SAMPLES_PER_READING); // Fresh averaged reading.
  const int delta = value - baseline;                // Positive or negative shift away from the "no magnet" level.
  const int magnitude = delta < 0 ? -delta : delta;  // Distance from the baseline, whatever the pole.
  const bool detected = magnitude > DETECT_THRESHOLD; // Either pole counts as a magnet; the sign only tells which face is closer.

  digitalWrite(BUILTIN_LED_PIN, detected ? HIGH : LOW); // The onboard LED follows the detection state.
  printReading(value, delta);
}

// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>

namespace {
constexpr uint8_t LED_PIN = 26;              // NanoKit Pin 37, GPIO26. Safe general-purpose output; the GPIO matrix can route any LEDC channel to it.
constexpr uint8_t BUILTIN_LED_PIN = 2;       // NanoKit Pin 17, GPIO2. The onboard LED receives the same duty so the fade is visible with zero wiring.
constexpr uint32_t PWM_FREQUENCY_HZ = 5000;  // 5 kHz is far above the flicker the eye can see and still leaves room for 8-bit resolution.
constexpr uint8_t PWM_RESOLUTION_BITS = 8;   // 8 bits = 256 duty steps. Enough for a smooth fade and comfortable at 5 kHz.
constexpr uint16_t DUTY_MAX = (1u << PWM_RESOLUTION_BITS) - 1; // 255 for 8 bits; derived from the resolution so the two never disagree.
constexpr uint32_t FADE_STEP_MS = 8;         // One brightness step every 8 ms: 255 steps up and 255 down make a cycle of about 4 s.
constexpr uint32_t REPORT_INTERVAL_MS = 500; // Print the duty twice per second so the Serial Monitor shows the ramp without flooding.
constexpr uint32_t SERIAL_BAUD = 115200;     // Standard NanoKit Serial Monitor speed.

int fadeLevel = 0;            // Linear position in the fade, 0..DUTY_MAX. This is what we count, not what we write to the pin.
int fadeDirection = +1;       // +1 while brightening, -1 while dimming.
uint16_t currentDuty = 0;     // The corrected duty actually written to both LED pins.
uint32_t lastStepMs = 0;      // Timestamp of the last fade step.
uint32_t lastReportMs = 0;    // Timestamp of the last Serial report.

// The eye judges brightness on a roughly logarithmic scale: duty 10 to 20 looks like a big jump,
// while 200 to 210 is barely noticeable. A linear duty ramp therefore seems to reach "full"
// early and then sit there. Squaring the normalised level spends most of the ramp at low duty,
// which the eye perceives as an even climb. This is a cheap stand-in for a real gamma curve.
uint16_t perceivedDuty(int level) {
  const float normalised = static_cast<float>(level) / DUTY_MAX;            // 0.0 (off) .. 1.0 (full).
  return static_cast<uint16_t>(normalised * normalised * DUTY_MAX + 0.5f);  // Square, scale back to duty steps, round to nearest.
}

void applyDuty(uint16_t duty) {
  ledcWrite(LED_PIN, duty);             // Arduino-ESP32 3.x addresses LEDC by pin. The hardware keeps this duty until the next write; the CPU is free in between.
  ledcWrite(BUILTIN_LED_PIN, duty);     // Same duty on the onboard LED.
}

void stepFade() {
  fadeLevel += fadeDirection;
  if (fadeLevel >= DUTY_MAX) {          // Reached full brightness: turn around and dim.
    fadeLevel = DUTY_MAX;
    fadeDirection = -1;
  } else if (fadeLevel <= 0) {          // Reached off: turn around and brighten.
    fadeLevel = 0;
    fadeDirection = +1;
  }
  currentDuty = perceivedDuty(fadeLevel);
  applyDuty(currentDuty);
}

void printDuty() {
  Serial.print("Level ");
  Serial.print(fadeLevel);               // The linear counter, so the reader can compare it with the corrected duty.
  Serial.print("/");
  Serial.print(DUTY_MAX);
  Serial.print(" | duty written: ");
  Serial.print(currentDuty);             // What the LEDC peripheral is really outputting right now.
  Serial.print(" | direction: ");
  Serial.println(fadeDirection > 0 ? "up" : "down");
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  // ledcAttach picks a free LEDC channel, programs its timer (5 kHz, 256 duty steps), and routes it to the pin
  // through the GPIO matrix. It returns false if no channel or timer is free. No pinMode is needed for LEDC.
  if (!ledcAttach(LED_PIN, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS) ||
      !ledcAttach(BUILTIN_LED_PIN, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS)) {
    Serial.println("LEDC setup failed: no free channel or timer.");
  }
  applyDuty(0);                                        // Start dark so the first ramp is clearly visible.
  Serial.println("NanoKit PWM LED fade example ready.");
  Serial.println("LED: NanoKit Pin 37 / GPIO26 (+ onboard LED on Pin 17 / GPIO2), each on its own LEDC channel.");
  Serial.println("LEDC: 5000 Hz, 8-bit resolution, duty 0-255. One fade step every 8 ms.");
}

void loop() {
  const uint32_t nowMs = millis();

  if (nowMs - lastStepMs >= FADE_STEP_MS) {           // Time for the next step; no delay() so the loop stays free for other work.
    lastStepMs = nowMs;
    stepFade();
  }

  if (nowMs - lastReportMs >= REPORT_INTERVAL_MS) {   // Periodic report shows the linear level next to the corrected duty.
    lastReportMs = nowMs;
    printDuty();
  }
}

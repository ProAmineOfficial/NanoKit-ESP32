// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>

namespace {
constexpr uint8_t POT_PIN = 34;              // NanoKit Pin 2, GPIO34. ADC1_CH6 and input-only. ADC1 keeps working if Wi-Fi is enabled later; ADC2 does not.
constexpr uint8_t BUILTIN_LED_PIN = 2;       // NanoKit Pin 17, GPIO2. The onboard LED shows the knob position as brightness, so no extra wiring is needed.
constexpr uint32_t PWM_FREQUENCY_HZ = 5000;  // Well above visible flicker; see the pwm_led_fade example for the frequency/resolution trade-off.
constexpr uint8_t PWM_RESOLUTION_BITS = 8;   // 256 brightness steps are plenty for a visual indicator.
constexpr uint8_t ADC_RESOLUTION_BITS = 12;  // The ESP32 ADC is 12-bit: analogRead returns 0..4095.
constexpr uint16_t ADC_MAX = (1u << ADC_RESOLUTION_BITS) - 1; // 4095, derived so the percent maths follows the resolution.
constexpr uint8_t SAMPLE_COUNT = 16;         // Averaging 16 conversions divides random noise by about 4 (square root of 16).
constexpr uint32_t SAMPLE_INTERVAL_MS = 100; // Ten updates per second is fast enough for a knob and slow enough to read.
constexpr uint8_t BAR_WIDTH = 20;            // Characters in the text bar; 20 means one '#' per 5 %.
constexpr uint32_t SERIAL_BAUD = 115200;     // Standard NanoKit Serial Monitor speed.

uint32_t lastSampleMs = 0;                   // Timestamp of the last measurement.

// Reads the pin SAMPLE_COUNT times and returns the averaged raw count (0..4095) and the averaged
// calibrated voltage. analogReadMilliVolts applies the correction values that Espressif burns into
// each chip's eFuse at the factory, so it is closer to a multimeter than raw * 3300 / 4095 would be.
void readPotentiometer(uint16_t &rawAverage, uint32_t &milliVoltsAverage) {
  uint32_t rawSum = 0;
  uint32_t milliVoltSum = 0;
  for (uint8_t i = 0; i < SAMPLE_COUNT; ++i) {
    rawSum += analogRead(POT_PIN);             // One conversion takes a few microseconds; 16 of them are invisible to the user.
    milliVoltSum += analogReadMilliVolts(POT_PIN);
  }
  rawAverage = static_cast<uint16_t>(rawSum / SAMPLE_COUNT);
  milliVoltsAverage = milliVoltSum / SAMPLE_COUNT;
}

void printBar(uint8_t percent) {
  const uint8_t filled = (percent * BAR_WIDTH) / 100;   // Number of '#' characters for this percentage.
  Serial.print('[');
  for (uint8_t i = 0; i < BAR_WIDTH; ++i) {
    Serial.print(i < filled ? '#' : '.');
  }
  Serial.print(']');
}

void reportAndDrive(uint16_t raw, uint32_t milliVolts) {
  const uint8_t percent = static_cast<uint8_t>((raw * 100UL) / ADC_MAX);          // 0..100, computed in 32 bits to avoid overflow.
  const uint8_t duty = static_cast<uint8_t>(raw >> (ADC_RESOLUTION_BITS - PWM_RESOLUTION_BITS)); // Drop 4 bits: 12-bit reading -> 8-bit duty.

  ledcWrite(BUILTIN_LED_PIN, duty);          // Arduino-ESP32 3.x addresses LEDC by pin. The onboard LED brightness now follows the knob.

  Serial.print("raw: ");
  Serial.print(raw);                         // 0..4095 from the ADC after averaging.
  Serial.print(" | ");
  Serial.print(milliVolts);                  // Calibrated millivolts, the number to compare with a multimeter on the wiper.
  Serial.print(" mV | ");
  Serial.print(percent);
  Serial.print(" % | ");
  printBar(percent);
  Serial.print(" | LED duty: ");
  Serial.println(duty);
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  analogSetPinAttenuation(POT_PIN, ADC_11db); // 11 dB attenuation lets the ADC measure the full 0-3.3 V swing of the wiper.
  if (!ledcAttach(BUILTIN_LED_PIN, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS)) { // Picks a free LEDC channel and routes it to GPIO2.
    Serial.println("LEDC setup failed: no free channel or timer.");
  }
  ledcWrite(BUILTIN_LED_PIN, 0);               // Start dark until the first measurement arrives.
  Serial.println("NanoKit analog input example ready.");
  Serial.println("Potentiometer wiper: NanoKit Pin 2 / GPIO34 (ADC1_CH6, input-only). Onboard LED: Pin 17 / GPIO2.");
  Serial.println("ADC: 12-bit, 11 dB attenuation, 16 samples averaged every 100 ms. Turn the knob.");
}

void loop() {
  const uint32_t nowMs = millis();
  if (nowMs - lastSampleMs < SAMPLE_INTERVAL_MS) { // Not yet time for the next measurement; keep the loop free for other work.
    return;
  }
  lastSampleMs = nowMs;

  uint16_t raw = 0;
  uint32_t milliVolts = 0;
  readPotentiometer(raw, milliVolts);
  reportAndDrive(raw, milliVolts);
}

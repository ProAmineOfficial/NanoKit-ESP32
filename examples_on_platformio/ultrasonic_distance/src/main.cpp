#include <Arduino.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr uint8_t TRIGGER_PIN = 5;       // NanoKit Pin 36, GPIO5, safe digital output for the HC-SR04 trigger pulse.
constexpr uint8_t ECHO_PIN = 34;         // NanoKit Pin 2, GPIO34, input-only pin, ideal for reading the echo signal.
constexpr uint32_t SERIAL_BAUD = 115200; // Fast enough for clear measurements in the PlatformIO Serial Monitor.
constexpr uint32_t MEASURE_INTERVAL_MS = 250; // Four readings per second keeps output readable and sensor timing stable.
constexpr uint32_t ECHO_TIMEOUT_US = 25000;   // 25 ms covers about 4.2 m and prevents blocking forever when no echo returns.
constexpr float SOUND_SPEED_CM_PER_US = 0.0343f; // Speed of sound near room temperature in centimeters per microsecond.
constexpr size_t FILTER_WINDOW = 5;      // Median filtering removes one-off spikes caused by reflections.

std::array<float, FILTER_WINDOW> samples{}; // Fixed-size storage avoids heap allocation on a microcontroller.
uint32_t lastMeasurementMs = 0;          // Timestamp used to run the measurement loop without delay-heavy code.

void sendTriggerPulse() {
  digitalWrite(TRIGGER_PIN, LOW);        // Start from LOW so the ultrasonic module sees a clean rising edge.
  delayMicroseconds(2);                  // The short settling time avoids a false trigger on noisy wiring.
  digitalWrite(TRIGGER_PIN, HIGH);       // The sensor begins one ranging cycle when this pulse goes HIGH.
  delayMicroseconds(10);                 // HC-SR04 style modules require a trigger pulse of at least 10 microseconds.
  digitalWrite(TRIGGER_PIN, LOW);        // Return to LOW so the next measurement can create a new pulse.
}

float readDistanceCm() {
  sendTriggerPulse();                    // Ask the ultrasonic module to emit a short acoustic burst.
  const uint32_t echoTimeUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US); // Measure how long the echo pin stays HIGH.

  if (echoTimeUs == 0) {                 // A zero result means no echo arrived before the safety timeout.
    return NAN;                          // NAN lets the caller handle an invalid reading explicitly.
  }

  return (echoTimeUs * SOUND_SPEED_CM_PER_US) / 2.0f; // Divide by two because sound travels to the object and back.
}

float readFilteredDistanceCm() {
  for (size_t index = 0; index < samples.size(); ++index) { // Collect several readings for a stable beginner-friendly result.
    samples[index] = readDistanceCm();     // Store each raw distance measurement in the fixed filter buffer.
    delay(15);                             // Small gap prevents back-to-back ultrasonic bursts from interfering.
  }

  std::sort(samples.begin(), samples.end()); // Sorting makes the middle value the median, rejecting extreme spikes.
  return samples[samples.size() / 2];        // The median is usually more stable than a single raw reading.
}

void printDistance(float distanceCm) {
  if (std::isnan(distanceCm)) {           // Invalid readings are normal when the target is missing or too far away.
    Serial.println("No echo detected. Check wiring, target angle, and sensor power.");
    return;
  }

  Serial.print("Distance: ");           // Keep the output label short so it is easy to read in Serial Monitor.
  Serial.print(distanceCm, 1);            // One decimal place is realistic for a low-cost ultrasonic module.
  Serial.println(" cm");                // Units are printed every time to avoid ambiguity for students.
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);              // Start USB Serial so PlatformIO can display measurements.
  pinMode(TRIGGER_PIN, OUTPUT);           // GPIO5 drives the trigger input, so it must be configured as an output.
  pinMode(ECHO_PIN, INPUT);               // GPIO34 is input-only and safely reads the echo signal from the sensor.
  digitalWrite(TRIGGER_PIN, LOW);         // Keep the trigger idle LOW to avoid accidental measurements during boot.
  Serial.println("NanoKit ultrasonic distance example ready.");
  Serial.println("Trigger: NanoKit Pin 36 / GPIO5, Echo: NanoKit Pin 2 / GPIO34.");
}

void loop() {
  const uint32_t nowMs = millis();         // Capture time once so interval math stays consistent.

  if (nowMs - lastMeasurementMs >= MEASURE_INTERVAL_MS) { // Run the sensor at a controlled sampling interval.
    lastMeasurementMs = nowMs;             // Update the timestamp before measuring so the schedule remains stable.
    const float distanceCm = readFilteredDistanceCm(); // Read and filter the sensor output.
    printDistance(distanceCm);             // Send the educational result to the Serial Monitor.
  }
}

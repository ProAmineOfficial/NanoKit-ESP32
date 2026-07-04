#include <Arduino.h>
#include <cstdio>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;       // Serial Monitor speed used to receive simple target coordinates.
constexpr uint8_t PAN_SERVO_PIN = 2;           // NanoKit Pin 17, GPIO2, PWM-capable output for pan servo signal.
constexpr uint8_t TILT_SERVO_PIN = 26;         // NanoKit Pin 37, GPIO26, PWM-capable output for tilt servo signal.
constexpr uint8_t PAN_PWM_CHANNEL = 0;         // ESP32 LEDC channel dedicated to the pan servo.
constexpr uint8_t TILT_PWM_CHANNEL = 1;        // ESP32 LEDC channel dedicated to the tilt servo.
constexpr uint16_t SERVO_PWM_HZ = 50;          // Hobby servos expect a 50 Hz control signal.
constexpr uint8_t SERVO_PWM_BITS = 16;         // High PWM resolution gives smooth pulse-width control.
constexpr uint16_t SERVO_MIN_US = 500;         // Conservative minimum pulse width for many 180-degree servos.
constexpr uint16_t SERVO_MAX_US = 2500;        // Conservative maximum pulse width for many 180-degree servos.
constexpr int FRAME_CENTER_X = 160;            // Example assumes a 320-pixel-wide tracking frame.
constexpr int FRAME_CENTER_Y = 120;            // Example assumes a 240-pixel-high tracking frame.
constexpr float TRACKING_GAIN = 0.035f;        // Small gain prevents jerky motion when the target is off center.

float panAngle = 90.0f;                        // Start centered so the camera points forward at boot.
float tiltAngle = 90.0f;                       // Start centered so the tilt mechanism is mechanically safe.
String serialLine;                             // Accumulates one coordinate command from Serial.
uint32_t lastDemoUpdateMs = 0;                 // Drives the built-in simulated target when no Serial command is sent.

uint32_t servoDutyFromAngle(float angleDegrees) {
  const float safeAngle = constrain(angleDegrees, 0.0f, 180.0f); // Clamp to servo-safe mechanical range.
  const float pulseUs = SERVO_MIN_US + (safeAngle / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US); // Convert angle to pulse width.
  return static_cast<uint32_t>((pulseUs * ((1UL << SERVO_PWM_BITS) - 1UL)) / 20000.0f); // Convert 20 ms frame pulse to LEDC duty.
}

void writeServo(uint8_t channel, float angleDegrees) {
  ledcWrite(channel, servoDutyFromAngle(angleDegrees)); // Write the PWM duty that represents the requested servo angle.
}

void applyServoAngles() {
  writeServo(PAN_PWM_CHANNEL, panAngle);       // Update the pan servo with the latest computed horizontal angle.
  writeServo(TILT_PWM_CHANNEL, tiltAngle);     // Update the tilt servo with the latest computed vertical angle.
}

void updateTrackingFromTarget(int targetX, int targetY) {
  const int errorX = targetX - FRAME_CENTER_X; // Positive error means the target is to the right of center.
  const int errorY = targetY - FRAME_CENTER_Y; // Positive error means the target is below the vertical center.

  panAngle = constrain(panAngle + errorX * TRACKING_GAIN, 20.0f, 160.0f);  // Move pan gradually and avoid hard end stops.
  tiltAngle = constrain(tiltAngle - errorY * TRACKING_GAIN, 30.0f, 150.0f); // Invert Y because screen down usually means servo tilt down.
  applyServoAngles();                         // Send the corrected angles to the PWM outputs.

  Serial.print("Target X=");                 // Report the received target for learning and debugging.
  Serial.print(targetX);
  Serial.print(" Y=");
  Serial.print(targetY);
  Serial.print(" -> pan=");
  Serial.print(panAngle, 1);
  Serial.print(" tilt=");
  Serial.println(tiltAngle, 1);
}

bool parseTargetLine(const String& line, int& targetX, int& targetY) {
  return std::sscanf(line.c_str(), "X=%d Y=%d", &targetX, &targetY) == 2; // Accept commands like X=170 Y=110.
}

void readSerialTarget() {
  while (Serial.available() > 0) {            // Read all incoming bytes so the parser stays responsive.
    const char incoming = static_cast<char>(Serial.read()); // Pull one byte from the USB Serial buffer.

    if (incoming == '\n' || incoming == '\r') { // Newline means the coordinate command is complete.
      if (serialLine.length() > 0) {          // Skip empty lines produced by different Serial Monitor settings.
        int targetX = FRAME_CENTER_X;
        int targetY = FRAME_CENTER_Y;
        if (parseTargetLine(serialLine, targetX, targetY)) { // Validate the coordinate format before moving hardware.
          updateTrackingFromTarget(targetX, targetY);
        } else {
          Serial.println("Use format: X=160 Y=120"); // Teach the correct command instead of failing silently.
        }
        serialLine = "";                     // Clear the buffer for the next command.
      }
    } else {
      serialLine += incoming;                 // Store regular characters until a newline arrives.
    }
  }
}

void runDemoTargetWhenIdle() {
  const uint32_t nowMs = millis();             // Use a timestamp so demo motion is non-blocking.
  if (nowMs - lastDemoUpdateMs < 600) {        // Slow demo movement makes servo behavior easy to observe.
    return;
  }

  lastDemoUpdateMs = nowMs;                    // Save the time for the next demo step.
  const int phase = (nowMs / 600) % 6;          // Repeat a small target path across the frame.
  const int demoX = 80 + phase * 32;            // Move horizontally across the simulated image.
  const int demoY = (phase % 2 == 0) ? 90 : 150; // Alternate vertical position to show tilt correction.
  updateTrackingFromTarget(demoX, demoY);       // Feed the simulated target into the same tracking algorithm.
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);                    // Start Serial for coordinate input and tracking logs.
  ledcSetup(PAN_PWM_CHANNEL, SERVO_PWM_HZ, SERVO_PWM_BITS);  // Configure PWM timing for the pan servo.
  ledcSetup(TILT_PWM_CHANNEL, SERVO_PWM_HZ, SERVO_PWM_BITS); // Configure PWM timing for the tilt servo.
  ledcAttachPin(PAN_SERVO_PIN, PAN_PWM_CHANNEL);             // Connect GPIO2 to the pan LEDC channel.
  ledcAttachPin(TILT_SERVO_PIN, TILT_PWM_CHANNEL);           // Connect GPIO26 to the tilt LEDC channel.
  applyServoAngles();                          // Move both servos to the safe center position.
  Serial.println("NanoKit camera tracking example ready.");
  Serial.println("Send coordinates as: X=160 Y=120");
}

void loop() {
  readSerialTarget();                           // Prefer real coordinates from Serial when a vision program sends them.
  if (serialLine.length() == 0) {               // If no partial command is being typed, run a safe built-in demo path.
    runDemoTargetWhenIdle();                    // Demo mode lets the project move immediately after upload.
  }
}

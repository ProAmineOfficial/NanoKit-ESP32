#include <Arduino.h>
#include <cstdio>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;      // USB Serial link used by the companion vision process.
constexpr uint8_t PAN_SERVO_PIN = 26;         // NanoKit Pin 37, GPIO26, PWM output for horizontal camera rotation.
constexpr uint8_t TILT_SERVO_PIN = 25;        // NanoKit Pin 3, GPIO25, PWM output for vertical camera rotation.
constexpr uint8_t STATUS_LED_PIN = 2;         // NanoKit Pin 17, GPIO2, onboard LED used as tracking status feedback.
constexpr uint8_t PAN_CHANNEL = 0;            // Dedicated LEDC PWM channel for pan control.
constexpr uint8_t TILT_CHANNEL = 1;           // Dedicated LEDC PWM channel for tilt control.
constexpr uint16_t SERVO_PWM_HZ = 50;         // Standard hobby servo control frequency.
constexpr uint8_t SERVO_PWM_BITS = 16;        // 16-bit duty resolution gives smooth servo positioning.
constexpr uint16_t SERVO_MIN_US = 500;        // Conservative minimum pulse width for common servos.
constexpr uint16_t SERVO_MAX_US = 2500;       // Conservative maximum pulse width for common servos.
constexpr int FRAME_CENTER_X = 160;           // Vision side should send coordinates from a 320x240 frame.
constexpr int FRAME_CENTER_Y = 120;           // Center Y coordinate used by the proportional controller.
constexpr float MIN_CONFIDENCE = 0.55f;       // Ignore weak detections so the camera does not chase noise.
constexpr float PAN_GAIN = 0.030f;            // Horizontal proportional gain tuned for smooth demonstration motion.
constexpr float TILT_GAIN = 0.025f;           // Vertical proportional gain is slightly smaller to reduce overshoot.
constexpr uint32_t LINK_TIMEOUT_MS = 2000;    // If no face message arrives for two seconds, mark tracking as idle.

struct FaceObservation {
  int x = FRAME_CENTER_X;                     // Last detected face X coordinate.
  int y = FRAME_CENTER_Y;                     // Last detected face Y coordinate.
  float confidence = 0.0f;                    // Detection confidence from the vision side.
  bool detected = false;                      // True when the message contains a usable face.
};

float panAngle = 90.0f;                       // Start centered to avoid sudden motion at boot.
float tiltAngle = 90.0f;                      // Start centered to protect the camera mechanism.
uint32_t lastVisionMessageMs = 0;             // Tracks communication freshness from the vision process.
String lineBuffer;                            // Stores one newline-terminated Serial message.

uint32_t servoDutyFromAngle(float angleDegrees) {
  const float safeAngle = constrain(angleDegrees, 10.0f, 170.0f); // Avoid mechanical hard stops on low-cost brackets.
  const float pulseUs = SERVO_MIN_US + (safeAngle / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US); // Convert angle to pulse width.
  return static_cast<uint32_t>((pulseUs * ((1UL << SERVO_PWM_BITS) - 1UL)) / 20000.0f); // Convert pulse width into LEDC duty.
}

void writeServo(uint8_t channel, float angleDegrees) {
  ledcWrite(channel, servoDutyFromAngle(angleDegrees)); // Send the requested angle to the selected PWM channel.
}

void applyServoState() {
  writeServo(PAN_CHANNEL, panAngle);          // Update horizontal rotation output.
  writeServo(TILT_CHANNEL, tiltAngle);        // Update vertical rotation output.
}

void setTrackingLed(bool active) {
  digitalWrite(STATUS_LED_PIN, active ? HIGH : LOW); // LED ON means valid face tracking is currently active.
}

bool parseFaceMessage(const String& line, FaceObservation& observation) {
  if (line == "NOFACE") {                    // Vision side sends NOFACE when no target is visible.
    observation.detected = false;
    observation.confidence = 0.0f;
    return true;
  }

  int parsedX = FRAME_CENTER_X;
  int parsedY = FRAME_CENTER_Y;
  float parsedConfidence = 0.0f;
  const int matched = std::sscanf(line.c_str(), "FACE %d %d %f", &parsedX, &parsedY, &parsedConfidence); // Expected format: FACE 160 120 0.91.

  if (matched != 3) {                         // Reject malformed commands before they can move the camera.
    return false;
  }

  observation.x = parsedX;                    // Store horizontal face location.
  observation.y = parsedY;                    // Store vertical face location.
  observation.confidence = parsedConfidence;  // Store detection confidence for filtering.
  observation.detected = parsedConfidence >= MIN_CONFIDENCE; // Treat weak detections as no face.
  return true;
}

void updateCameraAim(const FaceObservation& observation) {
  if (!observation.detected) {                // No reliable face means the safest behavior is to hold position.
    setTrackingLed(false);
    Serial.println("Tracking idle: no reliable face.");
    return;
  }

  const int errorX = observation.x - FRAME_CENTER_X; // Positive X error means the face is to the right.
  const int errorY = observation.y - FRAME_CENTER_Y; // Positive Y error means the face is below center.
  panAngle = constrain(panAngle + errorX * PAN_GAIN, 20.0f, 160.0f);   // Smoothly rotate toward the face horizontally.
  tiltAngle = constrain(tiltAngle - errorY * TILT_GAIN, 30.0f, 150.0f); // Smoothly rotate toward the face vertically.
  applyServoState();                          // Apply both corrected servo angles at the same time.
  setTrackingLed(true);                       // Show that the system is actively tracking a valid target.

  Serial.print("FACE ok x=");               // Log compact state for debugging the vision-to-firmware link.
  Serial.print(observation.x);
  Serial.print(" y=");
  Serial.print(observation.y);
  Serial.print(" conf=");
  Serial.print(observation.confidence, 2);
  Serial.print(" pan=");
  Serial.print(panAngle, 1);
  Serial.print(" tilt=");
  Serial.println(tiltAngle, 1);
}

void handleVisionLine(String line) {
  line.trim();                                // Remove CR/LF characters from the Serial command.
  FaceObservation observation;                // Create a fresh parsed observation for this message.

  if (!parseFaceMessage(line, observation)) { // Validate command format before changing actuator state.
    Serial.println("Invalid message. Use: FACE x y confidence  or  NOFACE");
    return;
  }

  lastVisionMessageMs = millis();             // Mark the vision link as alive after any valid message.
  updateCameraAim(observation);               // Move or hold the camera based on the parsed observation.
}

void readVisionSerial() {
  while (Serial.available() > 0) {            // Process all pending bytes from the companion vision program.
    const char incoming = static_cast<char>(Serial.read()); // Read one character from USB Serial.

    if (incoming == '\n' || incoming == '\r') { // Newline terminates one vision message.
      if (lineBuffer.length() > 0) {          // Ignore blank lines from different serial terminal settings.
        handleVisionLine(lineBuffer);
        lineBuffer = "";                     // Reset buffer for the next message.
      }
    } else {
      lineBuffer += incoming;                 // Accumulate message text until newline arrives.
    }
  }
}

void handleLinkTimeout() {
  if (millis() - lastVisionMessageMs > LINK_TIMEOUT_MS) { // Detect stale vision data.
    setTrackingLed(false);                    // Turn off tracking LED when the vision process stops sending data.
  }
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);                  // Start the serial link used by PlatformIO Monitor and the vision process.
  pinMode(STATUS_LED_PIN, OUTPUT);            // GPIO2 drives the onboard status LED.
  digitalWrite(STATUS_LED_PIN, LOW);          // Start with LED off until a valid face is received.
  ledcSetup(PAN_CHANNEL, SERVO_PWM_HZ, SERVO_PWM_BITS);   // Configure ESP32 PWM for pan servo control.
  ledcSetup(TILT_CHANNEL, SERVO_PWM_HZ, SERVO_PWM_BITS);  // Configure ESP32 PWM for tilt servo control.
  ledcAttachPin(PAN_SERVO_PIN, PAN_CHANNEL);              // Route pan PWM to NanoKit Pin 37 / GPIO26.
  ledcAttachPin(TILT_SERVO_PIN, TILT_CHANNEL);            // Route tilt PWM to NanoKit Pin 3 / GPIO25.
  applyServoState();                          // Center the camera before any face commands arrive.
  lastVisionMessageMs = millis();             // Initialize timeout timer from boot time.
  Serial.println("NanoKit face rotation tracking firmware ready.");
  Serial.println("Expected messages: FACE 160 120 0.90 or NOFACE");
}

void loop() {
  readVisionSerial();                          // Receive face coordinates from the computer vision side.
  handleLinkTimeout();                         // Mark tracking idle if the vision stream stops.
}

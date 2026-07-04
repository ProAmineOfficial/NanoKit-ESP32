#include <Arduino.h>
#include <array>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;     // Standard NanoKit Serial Monitor speed for interactive commands.
constexpr uint16_t DEFAULT_STEP_DELAY_MS = 3; // Safe starting speed for a 28BYJ-48 style stepper through ULN2003.
constexpr long DEFAULT_MOVE_STEPS = 2048;    // One common full output-shaft revolution for a geared 28BYJ-48 motor.

constexpr std::array<uint8_t, 4> MOTOR_PINS = {
  25, // NanoKit Pin 3, GPIO25, IN1 on the driver board.
  26, // NanoKit Pin 37, GPIO26, IN2 on the driver board.
  27, // NanoKit Pin 19, GPIO27, IN3 on the driver board.
  32  // NanoKit Pin 7, GPIO32, IN4 on the driver board.
};

constexpr uint8_t HALF_STEP_SEQUENCE[8][4] = {
  {1, 0, 0, 0}, // Coil A only: gentle first half-step state.
  {1, 1, 0, 0}, // Coils A+B: adds torque between full-step positions.
  {0, 1, 0, 0}, // Coil B only: next stable magnetic position.
  {0, 1, 1, 0}, // Coils B+C: smooth transition to the next phase.
  {0, 0, 1, 0}, // Coil C only: third stable magnetic position.
  {0, 0, 1, 1}, // Coils C+D: keeps torque high while moving.
  {0, 0, 0, 1}, // Coil D only: fourth stable magnetic position.
  {1, 0, 0, 1}  // Coils D+A: wraps the sequence back to the beginning.
};

int currentSequenceIndex = 0;              // Tracks which half-step pattern is currently active.
long remainingSteps = DEFAULT_MOVE_STEPS;  // Counts down the requested movement without blocking Serial input forever.
bool clockwise = true;                     // Direction flag; true follows the sequence forward.
uint16_t stepDelayMs = DEFAULT_STEP_DELAY_MS; // Delay between half-steps; lower values rotate faster.
uint32_t lastStepMs = 0;                   // Timestamp for non-blocking stepping.
String commandBuffer;                      // Stores one Serial command line at a time.

void applyStepPattern(int sequenceIndex) {
  for (size_t pinIndex = 0; pinIndex < MOTOR_PINS.size(); ++pinIndex) { // Update all four driver inputs every step.
    digitalWrite(MOTOR_PINS[pinIndex], HALF_STEP_SEQUENCE[sequenceIndex][pinIndex]); // Energize each coil according to the table.
  }
}

void releaseMotor() {
  for (const uint8_t pin : MOTOR_PINS) {   // Turn off all driver inputs when the motor is idle.
    digitalWrite(pin, LOW);               // LOW removes holding current and keeps the driver cooler.
  }
}

void scheduleMove(long steps, bool directionClockwise) {
  remainingSteps = abs(steps);             // Store only the number of steps; direction is held separately.
  clockwise = directionClockwise;          // Update the direction for the next movement.
  Serial.print("Scheduled steps: ");      // Echo the command so beginners know it was accepted.
  Serial.println(remainingSteps);
}

void performOneStep() {
  currentSequenceIndex += clockwise ? 1 : -1; // Move forward or backward through the half-step sequence.

  if (currentSequenceIndex > 7) {          // Wrap after the last pattern so rotation continues smoothly.
    currentSequenceIndex = 0;
  }

  if (currentSequenceIndex < 0) {          // Wrap before the first pattern when moving counter-clockwise.
    currentSequenceIndex = 7;
  }

  applyStepPattern(currentSequenceIndex);  // Output the selected magnetic coil pattern to the driver board.
  --remainingSteps;                        // Mark one half-step as completed.

  if (remainingSteps == 0) {               // Once the move is done, stop driving the motor coils.
    releaseMotor();                        // Releasing the motor saves power and heat in beginner experiments.
    Serial.println("Move complete. Motor released.");
  }
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  CW 2048   - rotate clockwise for 2048 half-steps");
  Serial.println("  CCW 2048  - rotate counter-clockwise for 2048 half-steps");
  Serial.println("  SPEED 3   - set delay between steps in milliseconds");
  Serial.println("  STOP      - stop and release the motor");
}

void handleCommand(String command) {
  command.trim();                          // Remove newline and spaces from the Serial Monitor input.
  command.toUpperCase();                   // Make commands beginner-friendly by accepting lowercase input.

  if (command == "HELP") {                // Print usage without changing motor state.
    printHelp();
    return;
  }

  if (command == "STOP") {                // Emergency stop command for safe lab testing.
    remainingSteps = 0;
    releaseMotor();
    Serial.println("Motor stopped and released.");
    return;
  }

  if (command.startsWith("SPEED ")) {     // SPEED changes timing but keeps the same wiring and sequence.
    const int requestedDelay = command.substring(6).toInt(); // Convert the numeric text after SPEED into an integer.
    stepDelayMs = constrain(requestedDelay, 2, 20); // Clamp speed to a safe range for a small beginner motor.
    Serial.print("Step delay set to ");
    Serial.print(stepDelayMs);
    Serial.println(" ms.");
    return;
  }

  if (command.startsWith("CW ")) {        // Clockwise move command.
    scheduleMove(command.substring(3).toInt(), true);
    return;
  }

  if (command.startsWith("CCW ")) {       // Counter-clockwise move command.
    scheduleMove(command.substring(4).toInt(), false);
    return;
  }

  Serial.println("Unknown command. Type HELP for options."); // Keep invalid input educational instead of silent.
}

void readSerialCommands() {
  while (Serial.available() > 0) {         // Process every received character so commands are responsive.
    const char incoming = static_cast<char>(Serial.read()); // Read one byte from USB Serial.

    if (incoming == '\n' || incoming == '\r') { // A newline marks the end of one command.
      if (commandBuffer.length() > 0) {    // Ignore empty lines so repeated Enter presses do nothing.
        handleCommand(commandBuffer);
        commandBuffer = "";               // Clear the buffer for the next command.
      }
    } else {
      commandBuffer += incoming;           // Append normal command characters until a newline arrives.
    }
  }
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);               // Start USB Serial for command input and learning feedback.

  for (const uint8_t pin : MOTOR_PINS) {   // Configure every NanoKit output connected to the stepper driver.
    pinMode(pin, OUTPUT);                  // Driver inputs must be outputs because the ESP32 controls coil states.
    digitalWrite(pin, LOW);                // Start LOW so the motor does not twitch during boot.
  }

  Serial.println("NanoKit stepper motor control example ready.");
  printHelp();                             // Show commands immediately so the project is self-documenting.
  scheduleMove(DEFAULT_MOVE_STEPS, true);  // Run one slow default revolution after upload for quick verification.
}

void loop() {
  readSerialCommands();                    // Keep listening for user commands even while a move is active.

  const uint32_t nowMs = millis();          // Read the current time once for non-blocking motor timing.
  if (remainingSteps > 0 && nowMs - lastStepMs >= stepDelayMs) { // Step only when the selected delay has elapsed.
    lastStepMs = nowMs;                     // Save the timestamp for the next step interval.
    performOneStep();                       // Advance the motor by one half-step.
  }
}

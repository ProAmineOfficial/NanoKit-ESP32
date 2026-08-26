#include "control/motor_mixer.h" // Import the MotorMixer public contract.

#include <algorithm> // The standard limits keep motor pulse calculations inside a safe range.

#include "config/board_config.h" // Board configuration provides confirmed pins and pulse limits.
#include "config/feature_flags.h" // Feature flags enforce the compile-time ESC safety lock.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

namespace { // Open the namespace that owns these project symbols.
uint16_t clampPulse(float pulse) { // Limit one floating-point mixer result before converting it to a pulse width.
  const float limited = std::max(static_cast<float>(config::ESC_IDLE_US), // Armed outputs may not fall below idle or rise above the configured maximum.
                                 std::min(static_cast<float>(config::ESC_MAXIMUM_US), pulse)); // Continue this function declaration or call across this line.
  return static_cast<uint16_t>(limited); // Return this result to the caller.
} // Close the current scope or type definition.
}  // namespace

void MotorMixer::begin() { // Configure all four LEDC channels with the same ESC timing.
  ledcSetup(config::MOTOR_1_CHANNEL, config::ESC_PWM_FREQUENCY_HZ, // Create one independent PWM generator for each motor.
            config::ESC_PWM_RESOLUTION_BITS); // Execute this statement as part of the current subsystem operation.
  ledcSetup(config::MOTOR_2_CHANNEL, config::ESC_PWM_FREQUENCY_HZ, // Continue this function declaration or call across this line.
            config::ESC_PWM_RESOLUTION_BITS); // Execute this statement as part of the current subsystem operation.
  ledcSetup(config::MOTOR_3_CHANNEL, config::ESC_PWM_FREQUENCY_HZ, // Continue this function declaration or call across this line.
            config::ESC_PWM_RESOLUTION_BITS); // Execute this statement as part of the current subsystem operation.
  ledcSetup(config::MOTOR_4_CHANNEL, config::ESC_PWM_FREQUENCY_HZ, // Continue this function declaration or call across this line.
            config::ESC_PWM_RESOLUTION_BITS); // Execute this statement as part of the current subsystem operation.
  ledcAttachPin(config::MOTOR_1_PIN, config::MOTOR_1_CHANNEL); // Route each generator to its confirmed NanoKit GPIO pin.
  ledcAttachPin(config::MOTOR_2_PIN, config::MOTOR_2_CHANNEL); // Continue this function declaration or call across this line.
  ledcAttachPin(config::MOTOR_3_PIN, config::MOTOR_3_CHANNEL); // Continue this function declaration or call across this line.
  ledcAttachPin(config::MOTOR_4_PIN, config::MOTOR_4_CHANNEL); // Continue this function declaration or call across this line.
  safe(); // Write the minimum command immediately after configuration.
} // Close the current scope or type definition.

MotorOutput MotorMixer::mix(uint16_t throttle, float rollCorrection, // Apply the standard Quad-X equations to normalized throttle and PID corrections.
                            float pitchCorrection, float yawCorrection) const { // Open this implementation block.
  const float span = static_cast<float>(config::ESC_MAXIMUM_US - config::ESC_IDLE_US); // Calculate the usable pulse range above idle.
  const float base = config::ESC_IDLE_US + // Normalize throttle to 0-1000 before mapping it into the pulse range.
                     (std::min<uint16_t>(throttle, 1000) / 1000.0f) * span; // Continue this function declaration or call across this line.
  MotorOutput output; // Add or subtract axis corrections according to each motor's frame position and rotation.
  output.m1Us = clampPulse(base + pitchCorrection + rollCorrection - yawCorrection); // Continue this function declaration or call across this line.
  output.m2Us = clampPulse(base + pitchCorrection - rollCorrection + yawCorrection); // Continue this function declaration or call across this line.
  output.m3Us = clampPulse(base - pitchCorrection - rollCorrection - yawCorrection); // Continue this function declaration or call across this line.
  output.m4Us = clampPulse(base - pitchCorrection + rollCorrection + yawCorrection); // Continue this function declaration or call across this line.
  return output; // Return this result to the caller.
} // Close the current scope or type definition.

void MotorMixer::write(const MotorOutput &output, bool armed) { // Write motor commands only when both runtime safety and compile-time ESC validation allow it.
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED // This compile-time gate prevents accidental motor authority on an unverified 4-in-1 ESC.
  if (armed) { // Runtime arming is still required even after the ESC protocol is confirmed.
    writePulse(config::MOTOR_1_CHANNEL, output.m1Us); // Continue this function declaration or call across this line.
    writePulse(config::MOTOR_2_CHANNEL, output.m2Us); // Continue this function declaration or call across this line.
    writePulse(config::MOTOR_3_CHANNEL, output.m3Us); // Continue this function declaration or call across this line.
    writePulse(config::MOTOR_4_CHANNEL, output.m4Us); // Continue this function declaration or call across this line.
    return; // Return this result to the caller.
  } // Close the current scope or type definition.
#else // Select the alternative compile-time feature branch.
  (void)output; // Mark unused inputs explicitly so a locked build compiles without warnings.
  (void)armed; // Explicitly mark this required parameter as unused.
#endif // Close the compile-time feature selection.
  safe(); // Every blocked or disarmed path finishes at the minimum pulse.
} // Close the current scope or type definition.

void MotorMixer::safe() { // Send the stop pulse to all four channels.
  writePulse(config::MOTOR_1_CHANNEL, config::ESC_MINIMUM_US); // Continue this function declaration or call across this line.
  writePulse(config::MOTOR_2_CHANNEL, config::ESC_MINIMUM_US); // Continue this function declaration or call across this line.
  writePulse(config::MOTOR_3_CHANNEL, config::ESC_MINIMUM_US); // Continue this function declaration or call across this line.
  writePulse(config::MOTOR_4_CHANNEL, config::ESC_MINIMUM_US); // Continue this function declaration or call across this line.
} // Close the current scope or type definition.

uint32_t MotorMixer::pulseToDuty(uint16_t pulseUs) const { // Translate microseconds into the integer duty cycle used by ESP32 LEDC hardware.
  const uint32_t maximumDuty = (1UL << config::ESC_PWM_RESOLUTION_BITS) - 1UL; // A 16-bit timer has values from zero through 2^16 minus one.
  return (static_cast<uint32_t>(pulseUs) * config::ESC_PWM_FREQUENCY_HZ * maximumDuty) / // frequency times pulse width gives the fraction of one PWM period.
         1000000UL; // Execute this statement as part of the current subsystem operation.
} // Close the current scope or type definition.

void MotorMixer::writePulse(uint8_t channel, uint16_t pulseUs) const { // Convert and write one pulse without exposing LEDC details to callers.
  ledcWrite(channel, pulseToDuty(pulseUs)); // Continue this function declaration or call across this line.
} // Close the current scope or type definition.

}  // namespace nanokit

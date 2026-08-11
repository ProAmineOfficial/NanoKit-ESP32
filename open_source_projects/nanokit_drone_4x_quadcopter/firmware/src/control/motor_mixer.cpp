// Import the MotorMixer public contract.
#include "control/motor_mixer.h"

// The standard limits keep motor pulse calculations inside a safe range.
#include <algorithm>

// Board configuration provides confirmed pins and pulse limits.
#include "config/board_config.h"
// Feature flags enforce the compile-time ESC safety lock.
#include "config/feature_flags.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

namespace {
// Limit one floating-point mixer result before converting it to a pulse width.
uint16_t clampPulse(float pulse) {
  // Armed outputs may not fall below idle or rise above the configured maximum.
  const float limited = std::max(static_cast<float>(config::ESC_IDLE_US),
                                 std::min(static_cast<float>(config::ESC_MAXIMUM_US), pulse));
  return static_cast<uint16_t>(limited);
}
}  // namespace

// Configure all four LEDC channels with the same ESC timing.
void MotorMixer::begin() {
  // Create one independent PWM generator for each motor.
  ledcSetup(config::MOTOR_1_CHANNEL, config::ESC_PWM_FREQUENCY_HZ,
            config::ESC_PWM_RESOLUTION_BITS);
  ledcSetup(config::MOTOR_2_CHANNEL, config::ESC_PWM_FREQUENCY_HZ,
            config::ESC_PWM_RESOLUTION_BITS);
  ledcSetup(config::MOTOR_3_CHANNEL, config::ESC_PWM_FREQUENCY_HZ,
            config::ESC_PWM_RESOLUTION_BITS);
  ledcSetup(config::MOTOR_4_CHANNEL, config::ESC_PWM_FREQUENCY_HZ,
            config::ESC_PWM_RESOLUTION_BITS);
  // Route each generator to its confirmed NanoKit GPIO pin.
  ledcAttachPin(config::MOTOR_1_PIN, config::MOTOR_1_CHANNEL);
  ledcAttachPin(config::MOTOR_2_PIN, config::MOTOR_2_CHANNEL);
  ledcAttachPin(config::MOTOR_3_PIN, config::MOTOR_3_CHANNEL);
  ledcAttachPin(config::MOTOR_4_PIN, config::MOTOR_4_CHANNEL);
  // Write the minimum command immediately after configuration.
  safe();
}

// Apply the standard Quad-X equations to normalized throttle and PID corrections.
MotorOutput MotorMixer::mix(uint16_t throttle, float rollCorrection,
                            float pitchCorrection, float yawCorrection) const {
  // Calculate the usable pulse range above idle.
  const float span = static_cast<float>(config::ESC_MAXIMUM_US - config::ESC_IDLE_US);
  // Normalize throttle to 0-1000 before mapping it into the pulse range.
  const float base = config::ESC_IDLE_US +
                     (std::min<uint16_t>(throttle, 1000) / 1000.0f) * span;
  // Add or subtract axis corrections according to each motor's frame position and rotation.
  MotorOutput output;
  output.m1Us = clampPulse(base + pitchCorrection + rollCorrection - yawCorrection);
  output.m2Us = clampPulse(base + pitchCorrection - rollCorrection + yawCorrection);
  output.m3Us = clampPulse(base - pitchCorrection - rollCorrection - yawCorrection);
  output.m4Us = clampPulse(base - pitchCorrection + rollCorrection + yawCorrection);
  return output;
}

// Write motor commands only when both runtime safety and compile-time ESC validation allow it.
void MotorMixer::write(const MotorOutput &output, bool armed) {
// This compile-time gate prevents accidental motor authority on an unverified 4-in-1 ESC.
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED
  // Runtime arming is still required even after the ESC protocol is confirmed.
  if (armed) {
    writePulse(config::MOTOR_1_CHANNEL, output.m1Us);
    writePulse(config::MOTOR_2_CHANNEL, output.m2Us);
    writePulse(config::MOTOR_3_CHANNEL, output.m3Us);
    writePulse(config::MOTOR_4_CHANNEL, output.m4Us);
    return;
  }
#else
  // Mark unused inputs explicitly so a locked build compiles without warnings.
  (void)output;
  (void)armed;
#endif
  // Every blocked or disarmed path finishes at the minimum pulse.
  safe();
}

// Send the stop pulse to all four channels.
void MotorMixer::safe() {
  writePulse(config::MOTOR_1_CHANNEL, config::ESC_MINIMUM_US);
  writePulse(config::MOTOR_2_CHANNEL, config::ESC_MINIMUM_US);
  writePulse(config::MOTOR_3_CHANNEL, config::ESC_MINIMUM_US);
  writePulse(config::MOTOR_4_CHANNEL, config::ESC_MINIMUM_US);
}

// Translate microseconds into the integer duty cycle used by ESP32 LEDC hardware.
uint32_t MotorMixer::pulseToDuty(uint16_t pulseUs) const {
  // A 16-bit timer has values from zero through 2^16 minus one.
  const uint32_t maximumDuty = (1UL << config::ESC_PWM_RESOLUTION_BITS) - 1UL;
  // frequency times pulse width gives the fraction of one PWM period.
  return (static_cast<uint32_t>(pulseUs) * config::ESC_PWM_FREQUENCY_HZ * maximumDuty) /
         1000000UL;
}

// Convert and write one pulse without exposing LEDC details to callers.
void MotorMixer::writePulse(uint8_t channel, uint16_t pulseUs) const {
  ledcWrite(channel, pulseToDuty(pulseUs));
}

}  // namespace nanokit

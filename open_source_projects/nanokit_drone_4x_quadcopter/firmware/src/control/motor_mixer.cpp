#include "control/motor_mixer.h"

#include <algorithm>

#include "config/board_config.h"
#include "config/feature_flags.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

namespace {
uint16_t clampPulse(float pulse) {
  const float limited = std::max(static_cast<float>(config::ESC_IDLE_US),
                                 std::min(static_cast<float>(config::ESC_MAXIMUM_US), pulse));
  return static_cast<uint16_t>(limited);
}
}  // namespace

void MotorMixer::begin() {
  ledcSetup(config::MOTOR_1_CHANNEL, config::ESC_PWM_FREQUENCY_HZ,
            config::ESC_PWM_RESOLUTION_BITS);
  ledcSetup(config::MOTOR_2_CHANNEL, config::ESC_PWM_FREQUENCY_HZ,
            config::ESC_PWM_RESOLUTION_BITS);
  ledcSetup(config::MOTOR_3_CHANNEL, config::ESC_PWM_FREQUENCY_HZ,
            config::ESC_PWM_RESOLUTION_BITS);
  ledcSetup(config::MOTOR_4_CHANNEL, config::ESC_PWM_FREQUENCY_HZ,
            config::ESC_PWM_RESOLUTION_BITS);
  ledcAttachPin(config::MOTOR_1_PIN, config::MOTOR_1_CHANNEL);
  ledcAttachPin(config::MOTOR_2_PIN, config::MOTOR_2_CHANNEL);
  ledcAttachPin(config::MOTOR_3_PIN, config::MOTOR_3_CHANNEL);
  ledcAttachPin(config::MOTOR_4_PIN, config::MOTOR_4_CHANNEL);
  safe();
}

MotorOutput MotorMixer::mix(uint16_t throttle, float rollCorrection,
                            float pitchCorrection, float yawCorrection) const {
  const float span = static_cast<float>(config::ESC_MAXIMUM_US - config::ESC_IDLE_US);
  const float base = config::ESC_IDLE_US +
                     (std::min<uint16_t>(throttle, 1000) / 1000.0f) * span;
  MotorOutput output;
  output.m1Us = clampPulse(base + pitchCorrection + rollCorrection - yawCorrection);
  output.m2Us = clampPulse(base + pitchCorrection - rollCorrection + yawCorrection);
  output.m3Us = clampPulse(base - pitchCorrection - rollCorrection - yawCorrection);
  output.m4Us = clampPulse(base - pitchCorrection + rollCorrection + yawCorrection);
  return output;
}

void MotorMixer::write(const MotorOutput &output, bool armed) {
#if NANOKIT_ESC_ANALOG_PWM_CONFIRMED
  if (armed) {
    writePulse(config::MOTOR_1_CHANNEL, output.m1Us);
    writePulse(config::MOTOR_2_CHANNEL, output.m2Us);
    writePulse(config::MOTOR_3_CHANNEL, output.m3Us);
    writePulse(config::MOTOR_4_CHANNEL, output.m4Us);
    return;
  }
#else
  (void)output;
  (void)armed;
#endif
  safe();
}

void MotorMixer::safe() {
  writePulse(config::MOTOR_1_CHANNEL, config::ESC_MINIMUM_US);
  writePulse(config::MOTOR_2_CHANNEL, config::ESC_MINIMUM_US);
  writePulse(config::MOTOR_3_CHANNEL, config::ESC_MINIMUM_US);
  writePulse(config::MOTOR_4_CHANNEL, config::ESC_MINIMUM_US);
}

uint32_t MotorMixer::pulseToDuty(uint16_t pulseUs) const {
  const uint32_t maximumDuty = (1UL << config::ESC_PWM_RESOLUTION_BITS) - 1UL;
  return (static_cast<uint32_t>(pulseUs) * config::ESC_PWM_FREQUENCY_HZ * maximumDuty) /
         1000000UL;
}

void MotorMixer::writePulse(uint8_t channel, uint16_t pulseUs) const {
  ledcWrite(channel, pulseToDuty(pulseUs));
}

}  // namespace nanokit

#pragma once

// MotorOutput is shared with safety, telemetry, and control modules.
#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// MotorMixer maps throttle and axis corrections to the four Quad-X motors.
class MotorMixer {
 public:
  // Configure the four ESP32 LEDC outputs and immediately place them in a safe state.
  void begin();
  // Calculate motor pulses without writing hardware, which keeps the math testable.
  MotorOutput mix(uint16_t throttle, float rollCorrection, float pitchCorrection,
                  float yawCorrection) const;
  // Send a calculated output only when all safety gates report an armed state.
  void write(const MotorOutput &output, bool armed);
  // Force every motor channel to the minimum command.
  void safe();

 private:
  // Convert a pulse width in microseconds into the duty value expected by LEDC.
  uint32_t pulseToDuty(uint16_t pulseUs) const;
  // Apply one bounded pulse to one already configured LEDC channel.
  void writePulse(uint8_t channel, uint16_t pulseUs) const;
};

}  // namespace nanokit

#pragma once

#include "core/flight_types.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

class MotorMixer {
 public:
  void begin();
  MotorOutput mix(uint16_t throttle, float rollCorrection, float pitchCorrection,
                  float yawCorrection) const;
  void write(const MotorOutput &output, bool armed);
  void safe();

 private:
  uint32_t pulseToDuty(uint16_t pulseUs) const;
  void writePulse(uint8_t channel, uint16_t pulseUs) const;
};

}  // namespace nanokit

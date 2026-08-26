#pragma once // Prevent this header from being included more than once.

#include "core/flight_types.h" // MotorOutput is shared with safety, telemetry, and control modules.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

class MotorMixer { // MotorMixer maps throttle and axis corrections to the four Quad-X motors.
 public: // Start this access-control section of the type.
  void begin(); // Configure the four ESP32 LEDC outputs and immediately place them in a safe state.
  MotorOutput mix(uint16_t throttle, float rollCorrection, float pitchCorrection, // Calculate motor pulses without writing hardware, which keeps the math testable.
                  float yawCorrection) const; // Execute this statement as part of the current subsystem operation.
  void write(const MotorOutput &output, bool armed); // Send a calculated output only when all safety gates report an armed state.
  void safe(); // Force every motor channel to the minimum command.

 private: // Start this access-control section of the type.
  uint32_t pulseToDuty(uint16_t pulseUs) const; // Convert a pulse width in microseconds into the duty value expected by LEDC.
  void writePulse(uint8_t channel, uint16_t pulseUs) const; // Apply one bounded pulse to one already configured LEDC channel.
}; // Close the current scope or type definition.

}  // namespace nanokit

#pragma once // Prevent this header from being included more than once.

#include <algorithm> // std::min and std::max provide portable limits without custom clamp code.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

class PidController { // PidController is a small reusable proportional-integral-derivative controller.
 public: // Start this access-control section of the type.
  PidController(float kp, float ki, float kd, float outputLimit) // Store fixed gains and one symmetric output limit for this control axis.
      : kp_(kp), ki_(ki), kd_(kd), outputLimit_(outputLimit) {} // Continue this function declaration or call across this line.

  float update(float target, float measurement, float dtSeconds) { // Calculate one bounded PID output from the target, measurement, and elapsed time.
    if (dtSeconds <= 0.0f) return 0.0f; // A non-positive time step would make the derivative invalid, so return no correction.
    const float error = target - measurement; // Error is positive when the requested value is above the measured value.
    integral_ = std::max(-outputLimit_, std::min(outputLimit_, integral_ + error * dtSeconds)); // Clamp the integral term to reduce windup during a sustained large error.
    const float derivative = initialized_ ? (error - previousError_) / dtSeconds : 0.0f; // Skip the derivative on the first sample because no previous error exists yet.
    initialized_ = true; // Save the current sample so the next call can calculate a derivative.
    previousError_ = error; // Assign this value for the current control or telemetry operation.
    const float output = kp_ * error + ki_ * integral_ + kd_ * derivative; // Combine the three PID terms using the configured gains.
    return std::max(-outputLimit_, std::min(outputLimit_, output)); // Limit the final correction so one axis cannot demand an unsafe mixer value.
  } // Close the current scope or type definition.

  void reset() { // Reset all time-dependent state when control authority is removed.
    integral_ = 0.0f; // Assign this value for the current control or telemetry operation.
    previousError_ = 0.0f; // Assign this value for the current control or telemetry operation.
    initialized_ = false; // Assign this value for the current control or telemetry operation.
  } // Close the current scope or type definition.

 private: // Start this access-control section of the type.
  float kp_; // These constants define proportional, integral, derivative, and output strength.
  float ki_; // Execute this statement as part of the current subsystem operation.
  float kd_; // Execute this statement as part of the current subsystem operation.
  float outputLimit_; // Execute this statement as part of the current subsystem operation.
  float integral_ = 0.0f; // These values preserve state between consecutive control-loop updates.
  float previousError_ = 0.0f; // Assign this value for the current control or telemetry operation.
  bool initialized_ = false; // Assign this value for the current control or telemetry operation.
}; // Close the current scope or type definition.

}  // namespace nanokit

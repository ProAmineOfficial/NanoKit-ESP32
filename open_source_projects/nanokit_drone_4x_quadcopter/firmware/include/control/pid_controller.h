#pragma once

// std::min and std::max provide portable limits without custom clamp code.
#include <algorithm>

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// PidController is a small reusable proportional-integral-derivative controller.
class PidController {
 public:
  // Store fixed gains and one symmetric output limit for this control axis.
  PidController(float kp, float ki, float kd, float outputLimit)
      : kp_(kp), ki_(ki), kd_(kd), outputLimit_(outputLimit) {}

  // Calculate one bounded PID output from the target, measurement, and elapsed time.
  float update(float target, float measurement, float dtSeconds) {
    // A non-positive time step would make the derivative invalid, so return no correction.
    if (dtSeconds <= 0.0f) return 0.0f;
    // Error is positive when the requested value is above the measured value.
    const float error = target - measurement;
    // Clamp the integral term to reduce windup during a sustained large error.
    integral_ = std::max(-outputLimit_, std::min(outputLimit_, integral_ + error * dtSeconds));
    // Skip the derivative on the first sample because no previous error exists yet.
    const float derivative = initialized_ ? (error - previousError_) / dtSeconds : 0.0f;
    // Save the current sample so the next call can calculate a derivative.
    initialized_ = true;
    previousError_ = error;
    // Combine the three PID terms using the configured gains.
    const float output = kp_ * error + ki_ * integral_ + kd_ * derivative;
    // Limit the final correction so one axis cannot demand an unsafe mixer value.
    return std::max(-outputLimit_, std::min(outputLimit_, output));
  }

  // Reset all time-dependent state when control authority is removed.
  void reset() {
    integral_ = 0.0f;
    previousError_ = 0.0f;
    initialized_ = false;
  }

 private:
  // These constants define proportional, integral, derivative, and output strength.
  float kp_;
  float ki_;
  float kd_;
  float outputLimit_;
  // These values preserve state between consecutive control-loop updates.
  float integral_ = 0.0f;
  float previousError_ = 0.0f;
  bool initialized_ = false;
};

}  // namespace nanokit

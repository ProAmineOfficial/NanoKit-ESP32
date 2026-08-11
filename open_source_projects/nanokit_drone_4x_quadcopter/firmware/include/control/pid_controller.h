#pragma once

#include <algorithm>

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

class PidController {
 public:
  PidController(float kp, float ki, float kd, float outputLimit)
      : kp_(kp), ki_(ki), kd_(kd), outputLimit_(outputLimit) {}

  float update(float target, float measurement, float dtSeconds) {
    if (dtSeconds <= 0.0f) return 0.0f;
    const float error = target - measurement;
    integral_ = std::max(-outputLimit_, std::min(outputLimit_, integral_ + error * dtSeconds));
    const float derivative = initialized_ ? (error - previousError_) / dtSeconds : 0.0f;
    initialized_ = true;
    previousError_ = error;
    const float output = kp_ * error + ki_ * integral_ + kd_ * derivative;
    return std::max(-outputLimit_, std::min(outputLimit_, output));
  }

  void reset() {
    integral_ = 0.0f;
    previousError_ = 0.0f;
    initialized_ = false;
  }

 private:
  float kp_;
  float ki_;
  float kd_;
  float outputLimit_;
  float integral_ = 0.0f;
  float previousError_ = 0.0f;
  bool initialized_ = false;
};

}  // namespace nanokit

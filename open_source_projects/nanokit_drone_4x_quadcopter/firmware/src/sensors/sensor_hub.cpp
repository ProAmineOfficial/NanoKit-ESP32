// Import the SensorHub contract and shared SensorState.
#include "sensors/sensor_hub.h"

// Wire provides the confirmed ESP32 I2C bus used by future sensor drivers.
#include <Wire.h>
// strncpy writes bounded diagnostic text without dynamic memory.
#include <cstring>

// Board constants hold the verified I2C pins and bus speed.
#include "config/board_config.h"
// Feature flags prevent unverified sensor code from becoming active.
#include "config/feature_flags.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// Start only the confirmed shared bus and publish an explicitly invalid sensor state.
void SensorHub::begin() {
  // Initialize I2C on NanoKit GPIO21/GPIO22 at the configured standard speed.
  Wire.begin(config::I2C_SDA_PIN, config::I2C_SCL_PIN);
  Wire.setClock(config::I2C_CLOCK_HZ);

  // No synthetic attitude is produced. Until the ICM-20948 driver and its
  // physical orientation are verified, the safety manager must see invalid IMU.
  // Value initialization clears every reading and validity flag consistently.
  state_ = SensorState{};
// Select a truthful diagnostic for the current compile-time driver state.
#if NANOKIT_FEATURE_ICM20948
  std::strncpy(status_, "ICM-20948 feature enabled; driver integration still required",
               sizeof(status_) - 1);
#else
  std::strncpy(status_, "ICM-20948 disabled pending verified wiring and driver",
               sizeof(status_) - 1);
#endif
  // Guarantee termination even if a future status message fills the buffer.
  status_[sizeof(status_) - 1] = '\0';
}

// Update feature-gated drivers without inventing values for absent hardware.
void SensorHub::update(float dtSeconds) {
  // The time step will be consumed when the verified fusion driver is integrated.
  (void)dtSeconds;

  // Priority 1 TODO: acquire calibrated ICM-20948 acceleration, angular rate,
  // and magnetic field; then publish a validated fused attitude here.
  // Priority 2/3 sensors remain guarded by feature_flags.h.
  // Keep every IMU gate false until a real read and calibration succeeds.
  state_.imuHealthy = false;
  state_.imuCalibrated = false;
  state_.attitude.valid = false;
}

// Record a calibration request without falsely reporting success.
void SensorHub::requestImuCalibration() {
  // Calibration remains false because the ICM-20948 driver is not integrated yet.
  state_.imuCalibrated = false;
  std::strncpy(status_, "Calibration blocked until ICM-20948 driver is integrated",
               sizeof(status_) - 1);
  status_[sizeof(status_) - 1] = '\0';
}

}  // namespace nanokit

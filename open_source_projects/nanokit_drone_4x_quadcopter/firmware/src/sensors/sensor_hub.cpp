#include "sensors/sensor_hub.h"

#include <Wire.h>
#include <cstring>

#include "config/board_config.h"
#include "config/feature_flags.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

void SensorHub::begin() {
  Wire.begin(config::I2C_SDA_PIN, config::I2C_SCL_PIN);
  Wire.setClock(config::I2C_CLOCK_HZ);

  // No synthetic attitude is produced. Until the ICM-20948 driver and its
  // physical orientation are verified, the safety manager must see invalid IMU.
  state_ = SensorState{};
#if NANOKIT_FEATURE_ICM20948
  std::strncpy(status_, "ICM-20948 feature enabled; driver integration still required",
               sizeof(status_) - 1);
#else
  std::strncpy(status_, "ICM-20948 disabled pending verified wiring and driver",
               sizeof(status_) - 1);
#endif
  status_[sizeof(status_) - 1] = '\0';
}

void SensorHub::update(float dtSeconds) {
  (void)dtSeconds;

  // Priority 1 TODO: acquire calibrated ICM-20948 acceleration, angular rate,
  // and magnetic field; then publish a validated fused attitude here.
  // Priority 2/3 sensors remain guarded by feature_flags.h.
  state_.imuHealthy = false;
  state_.imuCalibrated = false;
  state_.attitude.valid = false;
}

void SensorHub::requestImuCalibration() {
  state_.imuCalibrated = false;
  std::strncpy(status_, "Calibration blocked until ICM-20948 driver is integrated",
               sizeof(status_) - 1);
  status_[sizeof(status_) - 1] = '\0';
}

}  // namespace nanokit

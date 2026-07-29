// NanoKit Drone 4X (Quadcopter) - original educational reference firmware.
// Developed by Amine Saoud ibn al-Bashir.
// Keep propellers removed until every documented bench test has passed.
#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Wire.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

// This file is intentionally self-contained and heavily commented so the complete
// sensor -> estimator -> PID -> Quad-X mixer -> ESC path can be studied in one place.
namespace {

// Serial output is used for setup, calibration, and safe bench-test diagnostics.
constexpr uint32_t SERIAL_BAUD = 115200;

// NanoKit pins for the MPU6050 I2C bus. The IMU must use 3.3 V logic.
constexpr uint8_t I2C_SDA_PIN = 21; // NanoKit pin 23.
constexpr uint8_t I2C_SCL_PIN = 22; // NanoKit pin 18.
constexpr uint8_t MPU6050_ADDRESS_LOW = 0x68;
constexpr uint8_t MPU6050_ADDRESS_HIGH = 0x69;
constexpr uint8_t MPU6050_PWR_MGMT_1 = 0x6B;
constexpr uint8_t MPU6050_ACCEL_XOUT_H = 0x3B;
constexpr uint32_t I2C_CLOCK_HZ = 100000; // Robust on a breadboard and longer IMU leads.
constexpr uint16_t I2C_TIMEOUT_MS = 50;   // Bound a stalled transaction instead of blocking the loop.

// Quad-X motor signals. Motor order is defined in README.md and motor-layout.md.
constexpr uint8_t M1_FRONT_LEFT_PIN = 25;  // NanoKit pin 3, M1 front-left CCW.
constexpr uint8_t M2_FRONT_RIGHT_PIN = 26; // NanoKit pin 37, M2 front-right CW.
constexpr uint8_t M3_REAR_RIGHT_PIN = 27;  // NanoKit pin 19, M3 rear-right CCW.
constexpr uint8_t M4_REAR_LEFT_PIN = 32;   // NanoKit pin 7, M4 rear-left CW.

// Each ESC has its own ESP32 PWM channel.
constexpr uint8_t M1_CHANNEL = 0;
constexpr uint8_t M2_CHANNEL = 1;
constexpr uint8_t M3_CHANNEL = 2;
constexpr uint8_t M4_CHANNEL = 3;

// Standard analogue ESC pulse configuration. Verify the ESC specification first.
constexpr uint32_t PWM_FREQUENCY_HZ = 50;
constexpr uint8_t PWM_RESOLUTION_BITS = 16;
constexpr uint16_t ESC_MIN_US = 1000;
constexpr uint16_t ESC_IDLE_US = 1060;
constexpr uint16_t ESC_MAX_US = 1900;

// The attitude estimator runs faster than BLE telemetry and ESC PWM updates.
constexpr uint32_t CONTROL_LOOP_US = 4000;      // 250 Hz control calculation.
constexpr uint32_t TELEMETRY_INTERVAL_MS = 100; // 10 Hz phone/browser telemetry.
constexpr uint32_t FAILSAFE_TIMEOUT_MS = 700;   // Disarm after a lost command stream.
constexpr uint32_t IMU_RETRY_INTERVAL_MS = 1000;
constexpr uint32_t SERIAL_IMU_INTERVAL_MS = 500;
constexpr uint16_t IMU_CALIBRATION_SAMPLES = 700;
constexpr uint8_t IMU_FAILURES_BEFORE_RECOVERY = 3;
constexpr uint8_t FLIGHT_PROTOCOL_VERSION = 2;

// MPU6050 scale factors for +/-2 g accelerometer and +/-250 degrees/second gyro.
// Arduino defines RAD_TO_DEG as a macro, so this identifier must not reuse that name.
constexpr float RADIANS_TO_DEGREES = 57.2957795f;
constexpr float ACCEL_SCALE = 16384.0f;
constexpr float GYRO_SCALE = 131.0f;
constexpr float FILTER_ALPHA = 0.98f;

// BLE UUIDs are private to this project. The browser filters on the advertised name.
constexpr char BLE_DEVICE_NAME[] = "NanoKit-Drone-4X";
constexpr char BLE_SERVICE_UUID[] = "e1c91000-7a2a-4fd9-9d62-8ad3525b6f10";
constexpr char BLE_CONTROL_UUID[] = "e1c91001-7a2a-4fd9-9d62-8ad3525b6f10";
constexpr char BLE_TELEMETRY_UUID[] = "e1c91002-7a2a-4fd9-9d62-8ad3525b6f10";

// Raw signed 16-bit MPU6050 readings collected from one I2C transaction.
struct ImuRaw {
  int16_t ax = 0;
  int16_t ay = 0;
  int16_t az = 0;
  int16_t gx = 0;
  int16_t gy = 0;
  int16_t gz = 0;
};

// Estimated aircraft attitude. Yaw is relative because MPU6050 has no magnetometer.
struct Attitude {
  float rollDeg = 0.0f;
  float pitchDeg = 0.0f;
  float yawDeg = 0.0f;
  float yawRateDps = 0.0f;
};

// Calibration holds static gyro bias and the measured level-frame offsets.
struct ImuCalibration {
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
  float accelRoll = 0.0f;
  float accelPitch = 0.0f;
  bool valid = false;
};

// Commands arrive from the BLE controller as throttle, attitude targets, and arm state.
struct ControlCommand {
  float throttle = 0.0f;       // 0..1000 command units.
  float rollTargetDeg = 0.0f;  // Desired roll angle.
  float pitchTargetDeg = 0.0f; // Desired pitch angle.
  float yawRateDps = 0.0f;     // Desired yaw rate, not absolute heading.
  bool armRequested = false;
};

// A small PID controller with integral clamping prevents unbounded windup on the bench.
struct PidController {
  float kp = 0.0f;
  float ki = 0.0f;
  float kd = 0.0f;
  float integral = 0.0f;
  float previousError = 0.0f;
  float outputLimit = 100.0f;

  // Arduino ESP32 currently builds this project as C++11, where this type is not
  // aggregate-initialisable because it has default member initialisers and methods.
  PidController(float proportional, float integralGain, float derivativeGain, float limit)
      : kp(proportional), ki(integralGain), kd(derivativeGain), outputLimit(limit) {}

  float update(float setpoint, float measurement, float dt) {
    const float error = setpoint - measurement;
    integral += error * dt;
    integral = std::max(-outputLimit, std::min(outputLimit, integral));
    const float derivative = dt > 0.0f ? (error - previousError) / dt : 0.0f;
    previousError = error;
    const float output = (kp * error) + (ki * integral) + (kd * derivative);
    return std::max(-outputLimit, std::min(outputLimit, output));
  }

  void reset() {
    integral = 0.0f;
    previousError = 0.0f;
  }
};

// Global state stays explicit so the educational control flow is easy to trace.
Attitude attitude;
ImuCalibration calibration;
ControlCommand command;

// Starting gains only. Tune with propellers removed first, then restrained tether tests.
PidController rollPid{4.2f, 0.04f, 0.45f, 180.0f};
PidController pitchPid{4.2f, 0.04f, 0.45f, 180.0f};
PidController yawRatePid{1.8f, 0.02f, 0.04f, 120.0f};

BLECharacteristic *telemetryCharacteristic = nullptr;
BLEServer *flightServer = nullptr;
bool bleConnected = false;
bool bleAdvertising = false;
bool bleRestartPending = false;
bool bleControlPacketSeen = false;
bool armed = false;
bool imuCalibrationRequested = false;
bool imuOnline = false;
uint8_t mpu6050Address = 0;
uint32_t lastCommandMs = 0;
uint32_t lastCommandSequence = 0;
uint32_t lastTelemetryMs = 0;
uint32_t lastLoopUs = 0;
uint32_t bleRestartAtMs = 0;
uint32_t lastImuProbeMs = 0;
uint32_t lastSerialImuMs = 0;
uint8_t consecutiveImuReadFailures = 0;

// Clamp a floating-point value into a safe inclusive range.
float clampFloat(float value, float minimum, float maximum) {
  return std::max(minimum, std::min(maximum, value));
}

// Clamp then convert a mixer result into an ESC pulse width.
uint16_t clampPulse(float value, uint16_t minimum, uint16_t maximum) {
  return static_cast<uint16_t>(clampFloat(value, minimum, maximum));
}

// Convert a microsecond pulse into the duty value expected by ESP32 LEDC.
uint32_t pulseToDuty(uint16_t pulseUs) {
  const uint32_t maxDuty = (1UL << PWM_RESOLUTION_BITS) - 1UL;
  return (static_cast<uint32_t>(pulseUs) * PWM_FREQUENCY_HZ * maxDuty) / 1000000UL;
}

// Write one ESC command through its assigned LEDC channel.
void writePulse(uint8_t channel, uint16_t pulseUs) {
  ledcWrite(channel, pulseToDuty(pulseUs));
}

// Send minimum throttle to every ESC. This is the only output used while disarmed.
void setMotorsSafe() {
  writePulse(M1_CHANNEL, ESC_MIN_US);
  writePulse(M2_CHANNEL, ESC_MIN_US);
  writePulse(M3_CHANNEL, ESC_MIN_US);
  writePulse(M4_CHANNEL, ESC_MIN_US);
}

// Reset all PID memory whenever motors are stopped or a new arm cycle begins.
void resetControllers() {
  rollPid.reset();
  pitchPid.reset();
  yawRatePid.reset();
}

// Centralised disarm keeps every failure path identical and therefore reviewable.
void disarm(const char *reason) {
  if (armed) {
    Serial.print("[SAFE] Disarmed: ");
    Serial.println(reason);
  }
  armed = false;
  command.armRequested = false;
  command.throttle = 0.0f;
  resetControllers();
  setMotorsSafe();
}

// Advertising is started only after the GATT service exists. Keeping this in
// one function makes boot, disconnect recovery, and serial recovery identical.
void startBleAdvertising(const char *reason) {
  if (!flightServer) {
    Serial.println("[BLE] Cannot advertise: BLE server is not ready.");
    return;
  }

  flightServer->startAdvertising();
  bleAdvertising = true;
  Serial.print("[BLE] Advertising started: ");
  Serial.println(reason);
}

// Write one MPU6050 register, used during initial wake-up.
bool writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission(true) == 0;
}

// Recreate the ESP32 I2C driver after repeated failures. The NanoKit MPU6050
// wiring is often tested on a breadboard, where 100 kHz is substantially more
// tolerant of lead capacitance and marginal pull-up resistors than 400 kHz.
void configureI2cBus(bool resetDriver) {
  if (resetDriver) {
    Wire.end();
  }
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);
  Wire.setTimeOut(I2C_TIMEOUT_MS);
}

// Empty an incomplete response so the next transaction starts with a clean buffer.
void discardPendingI2cBytes() {
  while (Wire.available() > 0) {
    Wire.read();
  }
}

// Read one full sensor frame from the selected MPU6050 I2C address.
bool readImuRawFromAddress(uint8_t address, ImuRaw &raw) {
  Wire.beginTransmission(address);
  Wire.write(MPU6050_ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) {
    discardPendingI2cBytes();
    return false;
  }

  // Explicit types choose TwoWire::requestFrom(uint8_t, size_t, bool) on ESP32.
  const size_t received = Wire.requestFrom(
      address,
      static_cast<size_t>(14),
      true);
  if (received != 14) {
    discardPendingI2cBytes();
    return false;
  }

  raw.ax = (Wire.read() << 8) | Wire.read();
  raw.ay = (Wire.read() << 8) | Wire.read();
  raw.az = (Wire.read() << 8) | Wire.read();
  Wire.read(); // Ignore the two MPU6050 temperature bytes in this reference project.
  Wire.read();
  raw.gx = (Wire.read() << 8) | Wire.read();
  raw.gy = (Wire.read() << 8) | Wire.read();
  raw.gz = (Wire.read() << 8) | Wire.read();
  return true;
}

// Read from the detected address only. An unavailable IMU is never treated as
// a valid zero-degree measurement.
bool readImuRaw(ImuRaw &raw) {
  return mpu6050Address != 0 && readImuRawFromAddress(mpu6050Address, raw);
}

// Initialise I2C, wake the MPU6050, and accept either legal MPU6050 address.
bool initializeMpu6050(bool resetI2cDriver = false) {
  configureI2cBus(resetI2cDriver);
  lastImuProbeMs = millis();
  mpu6050Address = 0;
  imuOnline = false;

  const uint8_t candidates[] = {MPU6050_ADDRESS_LOW, MPU6050_ADDRESS_HIGH};
  for (const uint8_t address : candidates) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission(true) != 0 ||
        !writeRegister(address, MPU6050_PWR_MGMT_1, 0x00)) {
      continue;
    }

    delay(100);
    ImuRaw raw;
    if (readImuRawFromAddress(address, raw)) {
      mpu6050Address = address;
      imuOnline = true;
      consecutiveImuReadFailures = 0;
      Serial.printf("[IMU] MPU6050: OK at 0x%02X\n", mpu6050Address);
      return true;
    }
  }

  calibration.valid = false;
  Serial.println("[IMU] MPU6050 not found at 0x68 or 0x69.");
  return false;
}

// Calculate roll from gravity measured by the accelerometer.
float accelRollDeg(const ImuRaw &raw) {
  const float ay = raw.ay / ACCEL_SCALE;
  const float az = raw.az / ACCEL_SCALE;
  return std::atan2(ay, az) * RADIANS_TO_DEGREES;
}

// Calculate pitch from gravity measured by the accelerometer.
float accelPitchDeg(const ImuRaw &raw) {
  const float ax = raw.ax / ACCEL_SCALE;
  const float ay = raw.ay / ACCEL_SCALE;
  const float az = raw.az / ACCEL_SCALE;
  return std::atan2(-ax, std::sqrt((ay * ay) + (az * az))) * RADIANS_TO_DEGREES;
}

// Sample a motionless level frame to estimate gyro drift and the installed IMU's level offset.
void calibrateImu() {
  Serial.println("[IMU] Keep the quadcopter level and still. Calibrating...");
  ImuRaw raw;
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
  float roll = 0.0f;
  float pitch = 0.0f;
  uint16_t validSamples = 0;

  for (uint16_t i = 0; i < IMU_CALIBRATION_SAMPLES; ++i) {
    if (readImuRaw(raw)) {
      gyroX += raw.gx / GYRO_SCALE;
      gyroY += raw.gy / GYRO_SCALE;
      gyroZ += raw.gz / GYRO_SCALE;
      roll += accelRollDeg(raw);
      pitch += accelPitchDeg(raw);
      ++validSamples;
    }
    delay(3);
  }

  if (validSamples == 0) {
    calibration.valid = false;
    Serial.println("[IMU] Calibration failed: no valid samples.");
    return;
  }

  calibration.gyroX = gyroX / validSamples;
  calibration.gyroY = gyroY / validSamples;
  calibration.gyroZ = gyroZ / validSamples;
  calibration.accelRoll = roll / validSamples;
  calibration.accelPitch = pitch / validSamples;
  calibration.valid = true;
  attitude = {};
  Serial.println("[IMU] Calibration complete.");
}

// Fuse fast gyro data with gravity-derived roll and pitch using a complementary filter.
bool updateAttitude(float dt) {
  ImuRaw raw;
  if (!readImuRaw(raw)) {
    ++consecutiveImuReadFailures;

    // A single missed I2C frame must not turn a bench test into a continuous
    // disconnect/reconnect loop. Flight is still disarmed immediately if this
    // happens while armed; a stopped bench test gets a few retry attempts.
    if (armed || consecutiveImuReadFailures >= IMU_FAILURES_BEFORE_RECOVERY) {
      imuOnline = false;
      calibration.valid = false;
      return false;
    }
    return true;
  }

  consecutiveImuReadFailures = 0;
  imuOnline = true;

  const float gyroX = (raw.gx / GYRO_SCALE) - (calibration.valid ? calibration.gyroX : 0.0f);
  const float gyroY = (raw.gy / GYRO_SCALE) - (calibration.valid ? calibration.gyroY : 0.0f);
  const float gyroZ = (raw.gz / GYRO_SCALE) - (calibration.valid ? calibration.gyroZ : 0.0f);
  const float rollFromAccel = accelRollDeg(raw) - (calibration.valid ? calibration.accelRoll : 0.0f);
  const float pitchFromAccel = accelPitchDeg(raw) - (calibration.valid ? calibration.accelPitch : 0.0f);

  // Bench telemetry remains live before calibration so wiring and sensor axes
  // can be tested without motors. Arming still independently requires CAL=1.
  if (!calibration.valid) {
    attitude.rollDeg = rollFromAccel;
    attitude.pitchDeg = pitchFromAccel;
    attitude.yawDeg += gyroZ * dt;
    attitude.yawRateDps = gyroZ;
    return true;
  }

  attitude.rollDeg = (FILTER_ALPHA * (attitude.rollDeg + (gyroX * dt))) + ((1.0f - FILTER_ALPHA) * rollFromAccel);
  attitude.pitchDeg = (FILTER_ALPHA * (attitude.pitchDeg + (gyroY * dt))) + ((1.0f - FILTER_ALPHA) * pitchFromAccel);
  attitude.yawDeg += gyroZ * dt;
  attitude.yawRateDps = gyroZ;
  return true;
}

// Re-probe a physically reconnected IMU without automatically calibrating a
// moving airframe. The operator must still request calibration while level.
void recoverImuIfNeeded() {
  if (imuOnline || millis() - lastImuProbeMs < IMU_RETRY_INTERVAL_MS) {
    return;
  }

  if (initializeMpu6050(true)) {
    Serial.println("[IMU] Sensor restored. Calibrate before arming.");
  }
}

// Mirror useful attitude telemetry to USB Serial so it can be compared with
// the BLE dashboard without flooding the monitor at the 250 Hz control rate.
void printSerialImuTelemetry() {
  if (millis() - lastSerialImuMs < SERIAL_IMU_INTERVAL_MS) {
    return;
  }
  lastSerialImuMs = millis();

  if (!imuOnline) {
    Serial.printf("[IMU] offline; failed reads=%u; waiting to re-probe.\n", consecutiveImuReadFailures);
    return;
  }

  Serial.printf("[IMU] roll=%.1f deg pitch=%.1f deg yaw=%.1f deg calibrated=%d\n",
                attitude.rollDeg,
                attitude.pitchDeg,
                attitude.yawDeg,
                calibration.valid ? 1 : 0);
}

// Parse one KEY=value field from the semicolon-separated BLE packet.
float parseField(const std::string &packet, const char *key, float fallback) {
  const std::string needle = std::string(key) + "=";
  const size_t start = packet.find(needle);
  if (start == std::string::npos) {
    return fallback;
  }
  const size_t valueStart = start + needle.length();
  const size_t valueEnd = packet.find(';', valueStart);
  const std::string value = packet.substr(valueStart, valueEnd == std::string::npos ? std::string::npos : valueEnd - valueStart);
  return static_cast<float>(std::atof(value.c_str()));
}

// Convert a validated packet into conservative flight targets and optional calibration request.
void applyControlPacket(const std::string &packet) {
  const float throttle = clampFloat(parseField(packet, "T", command.throttle), 0.0f, 1000.0f);
  const float rollInput = clampFloat(parseField(packet, "R", 0.0f), -100.0f, 100.0f);
  const float pitchInput = clampFloat(parseField(packet, "P", 0.0f), -100.0f, 100.0f);
  const float yawInput = clampFloat(parseField(packet, "Y", 0.0f), -100.0f, 100.0f);
  const bool requestedArm = parseField(packet, "A", command.armRequested ? 1.0f : 0.0f) > 0.5f;
  const bool requestedCalibration = parseField(packet, "CAL", 0.0f) > 0.5f;
  const float sequence = parseField(packet, "SEQ", -1.0f);

  command.throttle = throttle;
  command.rollTargetDeg = rollInput * 0.15f;
  command.pitchTargetDeg = pitchInput * 0.15f;
  command.yawRateDps = yawInput * 0.9f;
  command.armRequested = requestedArm;
  imuCalibrationRequested = requestedCalibration || imuCalibrationRequested;
  lastCommandMs = millis();
  if (sequence >= 0.0f) {
    lastCommandSequence = static_cast<uint32_t>(sequence);
  }

  // Arming is accepted only with valid calibration, a live BLE link, and near-zero throttle.
  if (requestedArm && !armed && throttle < 20.0f && calibration.valid && bleConnected) {
    armed = true;
    resetControllers();
    Serial.println("[SAFE] Armed. Bench-test with propellers removed.");
  }

  // Any explicit disarm request wins over all other packet fields.
  if (!requestedArm) {
    disarm("remote command");
  }
}

// BLE connection changes are safety events: loss of the controller stops every motor.
class FlightServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *) override {
    bleConnected = true;
    bleAdvertising = false;
    bleControlPacketSeen = false;
    Serial.println("[BLE] Controller connected.");
  }

  void onDisconnect(BLEServer *) override {
    bleConnected = false;
    bleAdvertising = false;
    bleControlPacketSeen = false;
    disarm("BLE disconnected");
    // Let the Bluetooth stack complete the disconnect before restarting the
    // radio, as recommended by the ESP32 Arduino BLE server example.
    bleRestartAtMs = millis() + 500;
    bleRestartPending = true;
    Serial.println("[BLE] Controller disconnected. Advertising restart queued.");
  }
};

// Every incoming BLE write is treated as a complete control packet.
class ControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    const std::string value = characteristic->getValue().c_str();
    if (!value.empty()) {
      if (!bleControlPacketSeen) {
        bleControlPacketSeen = true;
        Serial.println("[BLE] First control packet received.");
      }
      applyControlPacket(value);
    }
  }
};

// Create the BLE service used by the browser controller for commands and telemetry.
void setupBle() {
  BLEDevice::init(BLE_DEVICE_NAME);
  flightServer = BLEDevice::createServer();
  flightServer->setCallbacks(new FlightServerCallbacks());

  BLEService *service = flightServer->createService(BLE_SERVICE_UUID);
  BLECharacteristic *controlCharacteristic = service->createCharacteristic(
      BLE_CONTROL_UUID,
      // Browser control uses acknowledged writes. This is more reliable than
      // write-without-response on Windows and older ESP32 BLE stacks.
      BLECharacteristic::PROPERTY_WRITE);
  controlCharacteristic->setCallbacks(new ControlCallbacks());

  telemetryCharacteristic = service->createCharacteristic(
      BLE_TELEMETRY_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  telemetryCharacteristic->addDescriptor(new BLE2902());

  service->start();
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  startBleAdvertising("boot");
  Serial.print("[BLE] Device address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
  Serial.println("[BLE] Advertising as NanoKit-Drone-4X.");
}

// Configure all four PWM outputs before any motor battery is connected.
void setupPwm() {
  ledcSetup(M1_CHANNEL, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(M2_CHANNEL, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(M3_CHANNEL, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(M4_CHANNEL, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcAttachPin(M1_FRONT_LEFT_PIN, M1_CHANNEL);
  ledcAttachPin(M2_FRONT_RIGHT_PIN, M2_CHANNEL);
  ledcAttachPin(M3_REAR_RIGHT_PIN, M3_CHANNEL);
  ledcAttachPin(M4_REAR_LEFT_PIN, M4_CHANNEL);
  setMotorsSafe();
  delay(2500); // Let ESCs observe a minimum-throttle signal after boot.
}

// Mix throttle and PID corrections into the four Quad-X ESC outputs.
void mixAndWriteOutputs(float dt) {
  const bool linkAlive = bleConnected && (millis() - lastCommandMs <= FAILSAFE_TIMEOUT_MS);
  if (!linkAlive) {
    disarm("BLE command timeout");
    return;
  }

  // Disarmed motors are always held at the ESC's documented minimum pulse.
  if (!armed || command.throttle < 20.0f) {
    setMotorsSafe();
    return;
  }

  const float throttlePulse = ESC_IDLE_US + ((command.throttle / 1000.0f) * (ESC_MAX_US - ESC_IDLE_US));
  const float rollCorrection = rollPid.update(command.rollTargetDeg, attitude.rollDeg, dt);
  const float pitchCorrection = pitchPid.update(command.pitchTargetDeg, attitude.pitchDeg, dt);
  const float yawCorrection = yawRatePid.update(command.yawRateDps, attitude.yawRateDps, dt);

  // Quad-X layout, nose forward: M1 FL CCW, M2 FR CW, M3 RR CCW, M4 RL CW.
  // Verify every sign with propellers removed; frame and IMU mounting can reverse axes.
  const uint16_t m1 = clampPulse(throttlePulse - pitchCorrection + rollCorrection - yawCorrection, ESC_IDLE_US, ESC_MAX_US);
  const uint16_t m2 = clampPulse(throttlePulse - pitchCorrection - rollCorrection + yawCorrection, ESC_IDLE_US, ESC_MAX_US);
  const uint16_t m3 = clampPulse(throttlePulse + pitchCorrection - rollCorrection - yawCorrection, ESC_IDLE_US, ESC_MAX_US);
  const uint16_t m4 = clampPulse(throttlePulse + pitchCorrection + rollCorrection + yawCorrection, ESC_IDLE_US, ESC_MAX_US);

  writePulse(M1_CHANNEL, m1);
  writePulse(M2_CHANNEL, m2);
  writePulse(M3_CHANNEL, m3);
  writePulse(M4_CHANNEL, m4);
}

// Broadcast compact telemetry used by the dashboard; yaw is relative and drifts over time.
void sendTelemetry() {
  if (!telemetryCharacteristic || !bleConnected) {
    return;
  }

  char payload[192];
  std::snprintf(
      payload,
      sizeof(payload),
      "PROTO=%u;ROLL=%.1f;PITCH=%.1f;YAW=%.1f;THR=%.0f;ARM=%d;LINK=%d;IMU=%d;CAL=%d;ACK=%lu",
      FLIGHT_PROTOCOL_VERSION,
      attitude.rollDeg,
      attitude.pitchDeg,
      attitude.yawDeg,
      command.throttle,
      armed ? 1 : 0,
      (millis() - lastCommandMs <= FAILSAFE_TIMEOUT_MS) ? 1 : 0,
      imuOnline ? 1 : 0,
      calibration.valid ? 1 : 0,
      static_cast<unsigned long>(lastCommandSequence));
  telemetryCharacteristic->setValue(payload);
  telemetryCharacteristic->notify();
}

// Provide low-risk serial commands for setup: DISARM, CALIBRATE_IMU, STATUS, and BLE_ADVERTISE.
void handleSerialCommand() {
  static String line;
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\n' || c == '\r') {
      line.trim();
      if (line == "DISARM") {
        disarm("serial command");
      } else if (line == "CALIBRATE_IMU") {
        disarm("IMU calibration");
        calibrateImu();
      } else if (line == "STATUS") {
        Serial.printf("[STATUS] roll=%.1f pitch=%.1f yaw=%.1f throttle=%.0f armed=%d imuOnline=%d calibrated=%d bleConnected=%d advertising=%d ack=%lu\n",
                      attitude.rollDeg,
                      attitude.pitchDeg,
                      attitude.yawDeg,
                      command.throttle,
                      armed ? 1 : 0,
                      imuOnline ? 1 : 0,
                      calibration.valid ? 1 : 0,
                      bleConnected ? 1 : 0,
                      bleAdvertising ? 1 : 0,
                      static_cast<unsigned long>(lastCommandSequence));
      } else if (line == "BLE_ADVERTISE") {
        if (bleConnected) {
          Serial.println("[BLE] A controller is connected; advertising resumes after disconnect.");
        } else {
          startBleAdvertising("serial command");
        }
      }
      line = "";
    } else {
      line += c;
    }
  }
}

} // namespace

// Configure every output safe before starting the IMU, BLE, and control loop.
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);
  Serial.println();
  Serial.println("[BOOT] NanoKit Drone 4X reference firmware");

  setupPwm();
  const bool imuReady = initializeMpu6050();
  if (imuReady) {
    calibrateImu();
  }
  setupBle();
  lastLoopUs = micros();
  Serial.println("[BOOT] Ready. Keep propellers removed for all initial tests.");
}

// Run the fixed-rate estimator, safety checks, mixer, telemetry, and serial diagnostics.
void loop() {
  handleSerialCommand();
  recoverImuIfNeeded();

  // Restart advertisements outside the BLE callback after the stack has had
  // time to complete the previous GATT disconnect.
  if (bleRestartPending && !bleConnected && millis() >= bleRestartAtMs) {
    bleRestartPending = false;
    startBleAdvertising("controller disconnected");
  }

  if (imuCalibrationRequested) {
    imuCalibrationRequested = false;
    disarm("BLE IMU calibration");
    calibrateImu();
  }

  const uint32_t nowUs = micros();
  if (nowUs - lastLoopUs < CONTROL_LOOP_US) {
    return;
  }

  const float dt = (nowUs - lastLoopUs) / 1000000.0f;
  lastLoopUs = nowUs;

  if (!updateAttitude(dt)) {
    disarm("IMU read or calibration fault");
  } else {
    mixAndWriteOutputs(dt);
  }

  if (millis() - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = millis();
    sendTelemetry();
  }

  printSerialImuTelemetry();
}

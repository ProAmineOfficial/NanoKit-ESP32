#pragma once

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// ConfigStore isolates optional AT24C256 persistence from flight-control code.
class ConfigStore {
 public:
  // Initialize storage only when its feature flag and wiring have been verified.
  bool begin();
  // These accessors expose availability and its diagnostic reason.
  bool available() const { return available_; }
  const char *status() const { return status_; }

 private:
  // Storage is unavailable by default so flight safety never depends on missing EEPROM.
  bool available_ = false;
  const char *status_ = "AT24C256 disabled pending verified wiring";
};

}  // namespace nanokit

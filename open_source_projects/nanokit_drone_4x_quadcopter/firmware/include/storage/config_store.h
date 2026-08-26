#pragma once // Prevent this header from being included more than once.

namespace nanokit { // Developed by Amine Saoud ibn al-Bashir.

class ConfigStore { // ConfigStore isolates optional AT24C256 persistence from flight-control code.
 public: // Start this access-control section of the type.
  bool begin(); // Initialize storage only when its feature flag and wiring have been verified.
  bool available() const { return available_; } // These accessors expose availability and its diagnostic reason.
  const char *status() const { return status_; } // Continue this function declaration or call across this line.

 private: // Start this access-control section of the type.
  bool available_ = false; // Storage is unavailable by default so flight safety never depends on missing EEPROM.
  const char *status_ = "AT24C256 disabled pending verified wiring"; // Assign this value for the current control or telemetry operation.
}; // Close the current scope or type definition.

}  // namespace nanokit

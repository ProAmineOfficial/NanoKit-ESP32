#pragma once

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

class ConfigStore {
 public:
  bool begin();
  bool available() const { return available_; }
  const char *status() const { return status_; }

 private:
  bool available_ = false;
  const char *status_ = "AT24C256 disabled pending verified wiring";
};

}  // namespace nanokit

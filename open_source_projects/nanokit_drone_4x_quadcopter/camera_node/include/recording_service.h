#pragma once

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit_camera {

// RecordingService isolates optional microSD recording from the HTTP interface.
class RecordingService {
 public:
  // Initialize storage only after the SPI pins and electrical interface are verified.
  void begin();
  // Recording cannot begin while ready() is false.
  bool ready() const { return ready_; }
  // The current placeholder always reports false to avoid a fake recording state.
  bool recording() const { return false; }
  const char *status() const { return status_; }

 private:
  // Storage remains unavailable by default because its pin map is still a TODO.
  bool ready_ = false;
  const char *status_ = "microSD disabled pending verified SPI pins";
};

}  // namespace nanokit_camera

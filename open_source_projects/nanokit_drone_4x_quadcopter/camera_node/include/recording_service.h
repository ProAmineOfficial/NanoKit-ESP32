#pragma once

namespace nanokit_camera { // Developed by Amine Saoud ibn al-Bashir.

class RecordingService { // RecordingService isolates optional microSD recording from the HTTP interface.
 public:
  void begin(); // Initialize storage only after the SPI pins and electrical interface are verified.
  bool ready() const { return ready_; } // Recording cannot begin while ready() is false.
  bool recording() const { return false; } // The current placeholder always reports false to avoid a fake recording state.
  const char *status() const { return status_; }

 private:
  bool ready_ = false; // Storage remains unavailable by default because its pin map is still a TODO.
  const char *status_ = "microSD disabled pending verified SPI pins";
};

}  // namespace nanokit_camera

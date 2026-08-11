// Import the optional persistent configuration interface.
#include "storage/config_store.h"

// The AT24C256 feature flag prevents access before wiring is confirmed.
#include "config/feature_flags.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

// Initialize persistent storage conservatively and report whether it is usable.
bool ConfigStore::begin() {
// Compile storage integration only after the hardware feature is explicitly enabled.
#if NANOKIT_FEATURE_AT24C256
  // TODO: add verified address, page size, CRC, and atomic configuration slots.
  available_ = false;
  status_ = "AT24C256 enabled but driver integration is incomplete";
#else
  // A disabled device is unavailable by design, not treated as a runtime fault.
  available_ = false;
  status_ = "AT24C256 disabled pending verified wiring";
#endif
  // Callers use this result to avoid reading or writing unavailable storage.
  return available_;
}

}  // namespace nanokit

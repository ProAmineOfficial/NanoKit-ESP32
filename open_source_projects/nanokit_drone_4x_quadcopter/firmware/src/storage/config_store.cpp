#include "storage/config_store.h"

#include "config/feature_flags.h"

// Developed by Amine Saoud ibn al-Bashir.
namespace nanokit {

bool ConfigStore::begin() {
#if NANOKIT_FEATURE_AT24C256
  // TODO: add verified address, page size, CRC, and atomic configuration slots.
  available_ = false;
  status_ = "AT24C256 enabled but driver integration is incomplete";
#else
  available_ = false;
  status_ = "AT24C256 disabled pending verified wiring";
#endif
  return available_;
}

}  // namespace nanokit

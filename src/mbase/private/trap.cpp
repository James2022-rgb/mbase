// public project headers -------------------------------
#include "mbase/public/trap.h"

#include "mbase/public/platform.h"
#include "mbase/public/log.h"

namespace mbase {

void Trap() {
#if MBASE_PLATFORM_WINDOWS
  __debugbreak();
#elif MBASE_PLATFORM_LINUX || MBASE_PLATFORM_ANDROID
  __builtin_trap();
#else
# error Unknown platform.
#endif
}

void TrapIfWithMessage(bool condition, std::string_view message) {
  if (condition) {
    MBASE_LOG_ERROR("Trap: {}", message);
    Trap();
  }
}

}

// public project headers -------------------------------
#include "mbase/public/trap.h"

// platform detection -----------------------------------
#include "mbase/public/platform.h"

// conditional platform headers -------------------------
#if MBASE_PLATFORM_WEB
# include <emscripten/emscripten.h>
#endif

// project headers --------------------------------------
#include "mbase/public/log.h"

namespace mbase {

void Trap() {
#if MBASE_PLATFORM_WINDOWS
  __debugbreak();
#elif MBASE_PLATFORM_LINUX || MBASE_PLATFORM_ANDROID || MBASE_PLATFORM_PSP
  __builtin_trap();
#elif MBASE_PLATFORM_WEB
  emscripten_debugger();
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

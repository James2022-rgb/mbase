// my header --------------------------------------------
#include "mbase/public/memory.h"

// c++ headers ------------------------------------------
#include <cstdlib>

// project headers --------------------------------------
#include "mbase/public/platform.h"
#include "mbase/public/log.h"

namespace mbase {

void* AlignedAlloc(uint64_t size, uint64_t alignment) {
#if MBASE_PLATFORM_WINDOWS
  return _aligned_malloc(size, alignment);
#elif MBASE_PLATFORM_PSP
  return aligned_alloc(alignment, size);
#else
  // Assume POSIX

  if (alignment < sizeof(void*)) {
    alignment = sizeof(void*);
  }

  void* block = nullptr;
  int ret = posix_memalign(&block, alignment, size);
  if (ret != 0) {
    char err_name[32];
    strerror_r(ret, err_name, sizeof(err_name));

    MBASE_LOG_ERROR("Failed to allocate memory; size:{}, alignment:{} err: {}", size, alignment, err_name);
  }
  return block;
#endif
}
void AlignedFree(void* block) {
#if MBASE_PLATFORM_WINDOWS
  _aligned_free(block);
#elif MBASE_PLATFORM_PSP
  free(block);
#else
  // Assume POSIX
  free(block);
#endif
}

} // namespace mbase

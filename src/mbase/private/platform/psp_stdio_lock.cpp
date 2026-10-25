// PSP newlib declares flockfile()/ftrylockfile()/funlockfile() in <stdio.h> but does not implement them.
// fmt (>= 11.1) detects them via the declaration and calls them, so provide them here,
// mirroring newlib's internal _flockfile()/_funlockfile() macros (which are C-only expressions).

#include "mbase/public/platform.h"

#if MBASE_PLATFORM_PSP

#include <stdio.h>
#include <sys/lock.h>

extern "C" {

void flockfile(FILE* fp) {
  // __SSTR: private to the sprintf/sscanf family; its lock is not initialized.
  if (!(fp->_flags & __SSTR)) {
    __lock_acquire_recursive(fp->_lock);
  }
}

int ftrylockfile(FILE* fp) {
  if (fp->_flags & __SSTR) {
    return 0;
  }
  return __lock_try_acquire_recursive(fp->_lock);
}

void funlockfile(FILE* fp) {
  if (!(fp->_flags & __SSTR)) {
    __lock_release_recursive(fp->_lock);
  }
}

}  // extern "C"

#endif  // MBASE_PLATFORM_PSP

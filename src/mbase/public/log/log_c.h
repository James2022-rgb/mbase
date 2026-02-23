#pragma once

#include "mbase/public/call.h"

#if defined(__cplusplus)
extern "C" {
#endif

MBASE_NO_THROW void MBASE_STDCALL MbLoggerInitialize(void);
MBASE_NO_THROW void MBASE_STDCALL MbLoggerShutdown(void);

#if defined(__cplusplus)
} // extern "C"
#endif

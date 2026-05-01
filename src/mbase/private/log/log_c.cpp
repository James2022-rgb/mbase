// public project headers -------------------------------
#include "mbase/public/log/log_c.h"
#include "mbase/public/log/log.h"

extern "C" {

MBASE_NO_THROW void MBASE_STDCALL MbLoggerInitialize(void) { mbase::Logger::Initialize(); }
MBASE_NO_THROW void MBASE_STDCALL MbLoggerShutdown(void)   { mbase::Logger::Shutdown(); }

} // extern "C"

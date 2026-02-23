// public project headers -------------------------------
#include "mbase/public/log/log_c.h"
#include "mbase/public/log/log.h"

extern "C" {

void MbLoggerInitialize(void) { mbase::Logger::Initialize(); }
void MbLoggerShutdown(void)   { mbase::Logger::Shutdown(); }

} // extern "C"

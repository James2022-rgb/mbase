// my header --------------------------------------------
#include "mbase/public/profiling/scoped_timer.h"

// c++ headers ------------------------------------------

// public project headers -------------------------------
#include "mbase/public/log.h"
#include "mbase/public/format.h"

namespace mbase {

ScopedTimer::~ScopedTimer() {
  this->MarkEnd();
  this->Report();
}

void ScopedTimer::MarkStart() {
  start_ = Clock::now();
}

void ScopedTimer::MarkEnd() {
  end_ = Clock::now();
}

void ScopedTimer::Report() const {
  auto elapsed = end_ - start_;
  auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

  MBASE_LOG_TRACE("\"{}\" done in {}us", label_, mbase::Commaize(elapsed_us));
}

} // namespace mbase

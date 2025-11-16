#pragma once

// c++ headers ------------------------------------------
#include <chrono>
#include <string>

// public project headers -------------------------------
#include "mbase/public/access.h"
#include "mbase/public/pp.h"

#define MBASE_SCOPED_TIMER(label) mbase::ScopedTimer MBASE_PP_CONCAT(mbase_scoped_timer_, __COUNTER__)(label);

namespace mbase {

class ScopedTimer final {
public:
  using Clock = std::chrono::high_resolution_clock;

  explicit ScopedTimer(std::string label) :
    label_(std::move(label))
  {
    this->MarkStart();
  }
  ~ScopedTimer();
  MBASE_DISALLOW_COPY_MOVE(ScopedTimer);

private:
  void MarkStart();
  void MarkEnd();
  void Report() const;

  std::string label_;

  Clock::time_point start_;
  Clock::time_point end_;
};

} // namespace mbase

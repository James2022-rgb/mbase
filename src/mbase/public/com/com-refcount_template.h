#pragma once

// c++ headers ------------------------------------------
#include <type_traits>
#include <atomic>
#include <functional>

// public project headers -------------------------------
#include "mbase/public/assert.h"
#include "mbase/public/access.h"
#include "mbase/public/com/com.h"

namespace mbase {

template<class T>
class ReferenceCountedBaseTemplate : public T {
public:
  static_assert(std::is_base_of_v<IMbUnknown, T>, "T MUST derive from IMbUnknown");

  //
  // IMbUnknown implementation
  //

  MBASE_NO_THROW IMbUnknown::Result MBASE_STDCALL QueryInterface(MbUuid const& uuid, void** out_interface) override {
    if (uuid == MB_UUID_IMbUnknown) {
      this->AddRef();
      *out_interface = this;
      return IMbUnknown::kOk;
    }

    *out_interface = nullptr;
    return IMbUnknown::kNoInterface;
  }

  MBASE_NO_THROW IMbUnknown::ReferenceCount MBASE_STDCALL AddRef() override {
    return ++ref_count_;
  }

  MBASE_NO_THROW IMbUnknown::ReferenceCount MBASE_STDCALL Release() override {
    MBASE_ASSERT(ref_count_ > 0);

    IMbUnknown::ReferenceCount const new_ref_count = --ref_count_;

    if (new_ref_count == 0) {
      if (deleter_) {
        deleter_(this);
      }
      else {
        delete this;
      }
    }
    return new_ref_count;
  }


  void SetDeleter(std::function<void(void*)> deleter) {
    deleter_ = deleter;
  }

protected:
  ReferenceCountedBaseTemplate() = default;
  virtual ~ReferenceCountedBaseTemplate() = default;

private:
  std::atomic<IMbUnknown::ReferenceCount> ref_count_ { 1 };

  std::function<void(void*)> deleter_ = nullptr;
};

} // namespace mbase

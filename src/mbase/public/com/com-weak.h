#pragma once

// public project headers -------------------------------
#include "mbase/public/com/com.h"

class IMbWeakReference : public IMbUnknown {
public:
  // {499650F8-1305-4B50-81FB-EE0036B261C8}
  MBASE_COM_INTERFACE(
    0x499650f8,
    0x1305,
    0x4b50,
    { 0x81, 0xfb, 0xee, 0x0, 0x36, 0xb2, 0x61, 0xc8 }
  );

  virtual MBASE_NO_THROW uint32_t MBASE_STDCALL IsExpired() const = 0;

  /// If called on an expired object, the result is still `kOk`, but the `out_interface` is set to `nullptr`, as per the Microsoft `IWeakReference` interface.
  virtual MBASE_NO_THROW Result MBASE_STDCALL UpgradeToStrong(MbUuid const& riid, IMbUnknown** out_interface) = 0;

  //
  // Convenience methods
  //

  template<class Q>
  Result UpgradeToStrong(Q** out_interface) {
    return this->UpgradeToStrong(Q::GetTypeGuid(), reinterpret_cast<IMbUnknown**>(out_interface));
  }

  template<class Q>
  Q* UpgradeToStrongOrNull() {
    Q* result = nullptr;
    this->UpgradeToStrong(&result);
    return result;
  }
};

class IMbWeakReferenceSource : public IMbUnknown {
public:
  // {502F3D33-DA86-46C3-A29A-4FCE73BE5DFA}
  MBASE_COM_INTERFACE(
    0x502f3d33,
    0xda86,
    0x46c3,
    { 0xa2, 0x9a, 0x4f, 0xce, 0x73, 0xbe, 0x5d, 0xfa }
  );

  virtual MBASE_NO_THROW Result MBASE_STDCALL GetWeakReference(IMbWeakReference** out_weak_reference) = 0;

  //
  // Convenience methods
  //

  IMbWeakReference* GetWeakReferenceOrNull() {
    IMbWeakReference* result = nullptr;
    this->GetWeakReference(&result);
    return result;
  }
};

namespace mbase {

template<class T>
class WeakPtr {
public:
  using Type = T;
  using ThisType = WeakPtr;

  //
  // Constructors
  //

  WeakPtr() = default;
  MBASE_FORCE_INLINE WeakPtr(std::nullptr_t) :
      weak_(nullptr)
  {
  }
  MBASE_FORCE_INLINE explicit WeakPtr(IMbWeakReference* weak) :
      weak_(weak)
  {
    if (weak_ != nullptr) {
      weak_->AddRef();
    }
  }
  MBASE_FORCE_INLINE explicit WeakPtr(IMbWeakReferenceSource* source) :
      weak_(source->GetWeakReferenceOrNull())
  {
    if (weak_ != nullptr) {
      weak_->AddRef();
    }
  }

  MBASE_FORCE_INLINE WeakPtr(WeakPtr const& rhs) :
      weak_(rhs.weak_)
  {
    if (weak_ != nullptr) {
      weak_->AddRef();
    }
  }
  MBASE_FORCE_INLINE ThisType& operator=(ThisType& rhs) {
    if (weak_ != nullptr) {
      weak_->RemoveRef();
    }

    weak_ = rhs.weak_;
    rhs.weak_ = nullptr;
    return *this;
  }

  MBASE_FORCE_INLINE WeakPtr(WeakPtr&& rhs) noexcept :
      weak_(rhs.weak_)
  {
    rhs.weak_ = nullptr; // Clear the source weak reference.
  }
  MBASE_FORCE_INLINE ThisType& operator=(ThisType&& rhs) noexcept {
    if (weak_ != nullptr) {
      weak_->RemoveRef();
    }

    weak_ = rhs.weak_;
    rhs.weak_ = nullptr;  // Clear the source weak reference.
    return *this;
  }

  // Destructor
  MBASE_FORCE_INLINE ~WeakPtr() {
    if (weak_ != nullptr) {
      weak_->RemoveRef();
    }
  }

  //
  // Operators
  //

  friend bool operator==(WeakPtr const& lhs, WeakPtr const& rhs) {
    return lhs.weak_ == rhs.weak_;
  }
  friend bool operator!=(WeakPtr const& lhs, WeakPtr const& rhs) {
    return !(lhs == rhs);
  }
  friend bool operator<(WeakPtr const& lhs, WeakPtr const& rhs) {
    return lhs.weak_ < rhs.weak_;
  }
  friend bool operator>(WeakPtr const& lhs, WeakPtr const& rhs) {
    return rhs < lhs;
  }
  friend bool operator<=(WeakPtr const& lhs, WeakPtr const& rhs) {
    return !(rhs < lhs);
  }
  friend bool operator>=(WeakPtr const& lhs, WeakPtr const& rhs) {
    return !(lhs < rhs);
  }
  friend bool operator==(WeakPtr const& lhs, std::nullptr_t) {
    return lhs.weak_ == nullptr;
  }
  friend bool operator!=(WeakPtr const& lhs, std::nullptr_t) {
    return !(lhs == nullptr);
  }

  //
  // Methods
  //

  MBASE_FORCE_INLINE IMbWeakReference* Get() const { return weak_; }

  MBASE_FORCE_INLINE bool IsExpired() const {
    if (weak_ == nullptr) {
      return true; // No weak reference.
    }
    return weak_->IsExpired() != 0;
  }

  MBASE_FORCE_INLINE MbResult UpgradeToStrong(T** out_interface) const {
    return weak_->UpgradeToStrong(T::GetTypeGuid(), reinterpret_cast<IMbUnknown**>(out_interface));
  }

  MBASE_FORCE_INLINE T* UpgradeToStrongOrNull() const {
    T* result = nullptr;
    this->UpgradeToStrong(&result);
    return result;
  }

  MBASE_FORCE_INLINE void SetNull() {
    weak_ = nullptr;
  }

private:
  IMbWeakReference* weak_ = nullptr;
};

} // namespace mbase

template<class T>
using MbWeakPtr = mbase::WeakPtr<T>;

namespace std {

template<class T>
struct hash<MbWeakPtr<T>> {
  size_t operator()(MbWeakPtr<T> const& ptr) const {
    if (ptr.Get() == nullptr) {
      return 0;
    }
    return std::hash<IMbWeakReference*>()(ptr.Get());
  }
};

} // namespace std


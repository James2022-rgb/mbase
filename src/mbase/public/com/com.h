#pragma once

//
// COM-like interface, inspired by slang, inspired by the `IUnknown` interface.
//

#include <cstddef>
#include <cstdint>

#include <unordered_map> // for `std::hash`.

#include "mbase/public/call.h"

#define MBASE_COM_INTERFACE(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7)  \
  public:                                                             \
  MBASE_FORCE_INLINE constexpr static MbUuid GetTypeGuid()            \
  {                                                                   \
      return {a, b, c, d0, d1, d2, d3, d4, d5, d6, d7};               \
  }

struct MbUuid {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8];

    friend bool operator==(MbUuid const& lhs, MbUuid const& rhs) {
      using CmpType = uint32_t; // Largest type that honors the alignment of `MbUuid`.

      union Compare {
        MbUuid uuid;
        CmpType data[sizeof(MbUuid) / sizeof(CmpType)];
      };

      CmpType const* a = reinterpret_cast<Compare const&>(lhs).data;
      CmpType const* b = reinterpret_cast<Compare const&>(rhs).data;

      // Compare the data in 32-bit chunks.
      return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2]) | (a[3] ^ b[3])) == 0;;
    }

    friend bool operator!=(MbUuid const& lhs, MbUuid const& rhs) {
      return !(lhs == rhs);
    }
};

using MbResult = int32_t;
static constexpr MbResult kMbResultOk = 0;
static constexpr MbResult kMbResultFalse = 1;
static constexpr MbResult kMbResultNoInterface = 0x80004002;
static constexpr MbResult kMbResultFail = 0x80004005;
static constexpr MbResult kMbResultInvalidArg = 0x80070057;
static constexpr MbResult kMbResultNotImplemented = 0x80004001;
static constexpr MbResult kMbResultOutOfMemory = 0x8007000E;
static constexpr MbResult kMbResultInvalidPointer = 0x80004003;

#define MBASE_SUCCEEDED(hr) (((MbResult)(hr)) >= 0)
#define MBASE_FAILED(hr) (((MbResult)(hr)) < 0)

/// Compatible with the COM `IUnknown` interface.
struct IMbUnknown {
  MBASE_COM_INTERFACE(
    0x00000000,
    0x0000,
    0x0000,
    {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}
  );

  /// Result type for `QueryInterface`: `HRESULT` compatible.
  using Result = int32_t;
  static constexpr Result kOk = kMbResultOk;
  static constexpr Result kFalse = kMbResultFalse;
  static constexpr Result kNoInterface = kMbResultNoInterface;
  static constexpr Result kFail = kMbResultFail;
  static constexpr Result kInvalidArg = kMbResultInvalidArg;
  static constexpr Result kNotImplemented = kMbResultNotImplemented;
  static constexpr Result kOutOfMemory = kMbResultOutOfMemory;
  static constexpr Result kInvalidPointer = kMbResultInvalidPointer;
  
  using ReferenceCount = uint32_t;

  virtual MBASE_NO_THROW Result MBASE_STDCALL QueryInterface(MbUuid const& riid, void** out_interface) = 0;

  /// Increments the reference count.
  /// Returns the new reference count.
  virtual MBASE_NO_THROW ReferenceCount MBASE_STDCALL AddRef() = 0;

  /// Decrements the reference count.
  /// Deallocates the object if the reference count reaches zero.
  /// Returns the new reference count.
  virtual MBASE_NO_THROW ReferenceCount MBASE_STDCALL Release() = 0;

  //
  // Convenience methods
  //

#if defined(GUID_DEFINED)
  /// Provide the same signature as `IUnknown::QueryInterface`.
  int32_t MBASE_STDCALL QueryInterface(GUID const& riid, void** ppvObject) {
    return QueryInterface(*reinterpret_cast<MbUuid const*>(&riid), ppvObject);
  }
#endif

  /// We prefer this method name.
  uint32_t MBASE_STDCALL RemoveRef() { return this->Release(); }

  template<class Q>
  Result QueryInterface(Q** out_interface) {
    return this->QueryInterface(Q::GetTypeGuid(), reinterpret_cast<void**>(out_interface));
  }

  template<class Q>
  Q* QueryInterfaceOrNull() {
    Q* out_interface = nullptr;
    Result const result = this->QueryInterface(&out_interface);
    if (result == kOk) {
      return out_interface;
    }
    return nullptr;
  }
};

#define MB_UUID_IMbUnknown IMbUnknown::GetTypeGuid()

namespace mbase {

struct InitAttachTag final {};
static constexpr InitAttachTag InitAttach {};

template<class T>
class ComPtr {
public:
  using Type = T;
  using ThisType = ComPtr;
  using Ptr = IMbUnknown*;

  //
  // Constructors
  //

  ComPtr() = default;
  MBASE_FORCE_INLINE ComPtr(std::nullptr_t) :
      ptr_(nullptr)
  {
  }
  MBASE_FORCE_INLINE explicit ComPtr(T* ptr) :
    ptr_(ptr)
  {
    if (ptr_ != nullptr)
      ptr_->AddRef();
  }
  MBASE_FORCE_INLINE ComPtr(ThisType const& rhs) :
    ptr_(rhs.ptr_)
  {
    if (ptr_ != nullptr)
      ptr_->AddRef();
  }

  MBASE_FORCE_INLINE ComPtr(InitAttachTag, T* ptr) :
    ptr_(ptr)
  {
  }
  MBASE_FORCE_INLINE ComPtr(InitAttachTag, ThisType const& rhs) :
    ptr_(rhs.ptr_)
  {
  }

  MBASE_FORCE_INLINE ComPtr(ThisType&& rhs) noexcept :
    ptr_(rhs.ptr_)
  {
    rhs.ptr_ = nullptr;
  }
  MBASE_FORCE_INLINE ComPtr& operator=(ThisType&& rhs) noexcept {
    if (ptr_ != nullptr) {
      ptr_->RemoveRef();
    }

    ptr_ = rhs.ptr_;
    rhs.ptr_ = nullptr;
    return *this;
  }

  // Destructor
  MBASE_FORCE_INLINE ~ComPtr() {
    if (ptr_ != nullptr) {
      ptr_->RemoveRef();
    }
  }

  //
  // Operators
  //

  // TODO: Disallow implicit conversions...
  MBASE_FORCE_INLINE operator T*() const { return ptr_; }

  MBASE_FORCE_INLINE T& operator*() const { return *ptr_; }
  MBASE_FORCE_INLINE T* operator->() const { return ptr_; }

  MBASE_FORCE_INLINE ComPtr& operator=(ThisType const& rhs) {
    if (rhs.ptr_ != nullptr) {
      rhs.ptr_->AddRef();
    }
    if (ptr_ != nullptr) {
      ptr_->RemoveRef();
    }
    ptr_ = rhs.ptr_;
    return *this;
  }

  MBASE_FORCE_INLINE ComPtr& operator=(T* ptr) {
    if (ptr != nullptr) {
      ptr->AddRef();
    }
    if (ptr_ != nullptr) {
      ptr_->RemoveRef();
    }
    ptr_ = ptr;
    return *this;
  }

  //
  // Methods
  //

  MBASE_FORCE_INLINE T* Get() const { return ptr_; }

  /// Sets the pointer to `nullptr`, decrementing the reference count if there is a pointer.
  MBASE_FORCE_INLINE void SetNull() {
    if (ptr_ != nullptr) {
      ptr_->RemoveRef();
    }
    ptr_ = nullptr;
  }

  /// Detaches the pointer, without decrementing its reference count.
  MBASE_FORCE_INLINE T* Detach() {
    T* ptr = ptr_;
    ptr_ = nullptr;
    return ptr;
  }

  /// Attaches a new pointer, without incrementing its reference count.
  MBASE_FORCE_INLINE void Attach(T* ptr) {
    ptr_ = ptr;
  }

  MBASE_FORCE_INLINE T** WriteRef() {
    SetNull();
    return &ptr_;
  }

  MBASE_FORCE_INLINE T* const* ReadRef() const {
    return &ptr_;
  }

  MBASE_FORCE_INLINE void Swap(ThisType& rhs) {
    T* tmp = ptr_;
    ptr_ = rhs.ptr_;
    rhs.ptr_ = tmp;
  }

private:
  T* ptr_ = nullptr;
};

} // namespace mbase

using MbUnknownResult = IMbUnknown::Result;

static constexpr mbase::InitAttachTag MbInitAttach {};

template<class T>
using MbComPtr = mbase::ComPtr<T>;

/// Creates an `MbComPtr`, incrementing the reference count.
template<class T>
MbComPtr<T> MbComPtrOwn(T* ptr) {
  return MbComPtr<T>(ptr);
}

/// Creates an `MbComPtr` but keeps the reference count.
template<class T>
MbComPtr<T> MbComPtrAttach(T* ptr) {
  return MbComPtr<T>(MbInitAttach, ptr);
}

template<class Q, class T>
MbComPtr<Q> MbComPtrQueryInterfaceAndAttach(T* ptr) {
  Q* out_interface = nullptr;
  if (ptr->QueryInterface(Q::GetTypeGuid(), reinterpret_cast<void**>(&out_interface)) == IMbUnknown::kOk) {
    return MbComPtrAttach(out_interface);
  }
  return MbComPtr<Q>();
}

namespace std {

template<class T>
struct hash<MbComPtr<T>> {
  size_t operator()(MbComPtr<T> const& ptr) const {
    if (ptr.Get() == nullptr) {
      return 0;
    }
    return std::hash<T*>()(ptr.Get());
  }
};

} // namespace std

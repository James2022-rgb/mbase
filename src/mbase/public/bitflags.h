#pragma once

#include <functional>

#include "mbase/public/type_util.h"

#define MBASE_DEFINE_ENUM_CLASS_BITFLAGS_OPERATORS(T)                         \
  [[maybe_unused]] constexpr T operator|(const T lhs, const T rhs) {          \
      using U = typename std::underlying_type<T>::type;                       \
      return static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));       \
  }                                                                           \
  [[maybe_unused]] constexpr T operator&(const T lhs, const T rhs) {          \
      using U = typename std::underlying_type<T>::type;                       \
      return static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));       \
  }                                                                           \
  [[maybe_unused]] constexpr T operator^(const T lhs, const T rhs) {          \
      using U = typename std::underlying_type<T>::type;                       \
      return static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));       \
  }                                                                           \
  [[maybe_unused]] constexpr T operator~(const T val) {                       \
      using U = typename std::underlying_type<T>::type;                       \
      return static_cast<T>(~static_cast<U>(val));                            \
  }                                                                           \
  [[maybe_unused]] inline T& operator|=(T& lhs, const T& rhs) {               \
      using U = typename std::underlying_type<T>::type;                       \
      return lhs = static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs)); \
  }                                                                           \
  [[maybe_unused]] inline T& operator&=(T& lhs, const T& rhs) {               \
      using U = typename std::underlying_type<T>::type;                       \
      return lhs = static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs)); \
  }                                                                           \
  [[maybe_unused]] inline T& operator^=(T& lhs, const T& rhs) {               \
      using U = typename std::underlying_type<T>::type;                       \
      return lhs = static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs)); \
  }  


namespace mbase {

template<class TBits>
class BitFlags {
public:
  using StorageType = typename std::underlying_type<TBits>::type;

  constexpr BitFlags() = default;
  ~BitFlags() = default;

  static constexpr BitFlags Identity() { return {}; }

  constexpr BitFlags(TBits rhs) noexcept : value_(static_cast<StorageType>(rhs)) {}
  constexpr BitFlags(BitFlags<TBits> const& rhs) noexcept : value_(rhs.value_) {}
  constexpr BitFlags(BitFlags<TBits>&& rhs) noexcept : value_(rhs.value_) {}
  constexpr explicit BitFlags(StorageType rhs) noexcept : value_(rhs) {}

  BitFlags<TBits>& operator=(BitFlags<TBits> const& rhs) noexcept { value_ = rhs.value_; return *this; }
  BitFlags<TBits>& operator=(BitFlags<TBits>&& rhs) noexcept { value_ = rhs.value_; return *this; }

  BitFlags<TBits>& operator=(StorageType rhs) noexcept { value_ = rhs; return *this; }

  constexpr bool operator<(BitFlags<TBits> const& rhs) const noexcept { return value_ < rhs.value_; }
  constexpr bool operator<=(BitFlags<TBits> const& rhs) const noexcept { return value_ <= rhs.value_; }
  constexpr bool operator>(BitFlags<TBits> const& rhs) const noexcept { return value_ > rhs.value_; }
  constexpr bool operator>=(BitFlags<TBits> const& rhs) const noexcept { return value_ >= rhs.value_; }
  constexpr bool operator==(BitFlags<TBits> const& rhs) const noexcept { return value_ == rhs.value_; }
  constexpr bool operator!=(BitFlags<TBits> const& rhs) const noexcept { return value_ != rhs.value_; }

  constexpr bool operator!() const noexcept { return !value_; }

  constexpr BitFlags<TBits> operator|(BitFlags<TBits> const& rhs) const noexcept { return BitFlags<TBits>(value_ | rhs.value_); }
  constexpr BitFlags<TBits> operator&(BitFlags<TBits> const& rhs) const noexcept { return BitFlags<TBits>(value_ & rhs.value_); }
  constexpr BitFlags<TBits> operator^(BitFlags<TBits> const& rhs) const noexcept { return BitFlags<TBits>(value_ ^ rhs.value_); }
  constexpr BitFlags<TBits> operator~() const noexcept { return BitFlags<TBits>(~value_); }

  BitFlags<TBits>& operator|=(BitFlags<TBits> const& rhs) noexcept { value_ |= rhs.value_; return *this; }
  BitFlags<TBits>& operator&=(BitFlags<TBits> const& rhs) noexcept { value_ &= rhs.value_; return *this; }
  BitFlags<TBits>& operator^=(BitFlags<TBits> const& rhs) noexcept { value_ ^= rhs.value_; return *this; }

  constexpr StorageType value() const noexcept { return value_; }

  explicit constexpr operator bool() const noexcept { return !!value_; }
  explicit constexpr operator StorageType() const noexcept { return value_; }

  [[nodiscard]] constexpr bool HasAnyOf(BitFlags<TBits> const& mask) const noexcept { return (value_ & mask.value_) != 0; }
  [[nodiscard]] constexpr bool HasNoneOf(BitFlags<TBits> const& mask) const noexcept { return (value_ & mask.value_) == 0; }
  [[nodiscard]] constexpr bool HasAllOf(BitFlags<TBits> const& mask) const noexcept { return (value_ & mask.value_) == mask.value_; }

  void Clear() noexcept { value_ = 0; }
  void Clear(BitFlags<TBits> const& mask) noexcept { value_ &= ~mask.value_; }

  bool CheckAnyAndClear(BitFlags<TBits> const& mask) noexcept {
    bool const result = HasAnyOf(mask);
    if (result) {
      Clear(mask);
    }
    return result;
  }
  bool CheckAllAndClear(BitFlags<TBits> const& mask) noexcept {
    bool const result = HasAllOf(mask);
    if (result) {
      Clear(mask);
    }
    return result;
  }

private:
  StorageType value_ = 0;
};

constexpr uint32_t BitMask(uint32_t bit_count) {
  return (1 << bit_count) - 1;
}

template<class T>
constexpr bool CheckBitFlags(T value, T mask) {
  return UnderlyingCast(value & mask) != 0;
}

} // namespace mbase

namespace std {

template<class TBits>
struct hash<mbase::BitFlags<TBits>> {
  size_t operator()(mbase::BitFlags<TBits> const& value) const noexcept {
    return std::hash<typename mbase::BitFlags<TBits>::StorageType>()(value.value());
  }
};

}

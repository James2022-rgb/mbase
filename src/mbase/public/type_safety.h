#pragma once

#include <functional>

namespace mbase {

template<class Tag>
struct TypesafeBool {
  explicit TypesafeBool(bool value) : value(value) {}
  bool operator*() const { return value; }

  bool value;
};

template<class Tag, class TStorage, TStorage InvalidValue>
class TypesafeHandle {
public:
  using StorageType = TStorage;

  static TypesafeHandle Invalid() { return TypesafeHandle(InvalidValue); }

  constexpr TypesafeHandle() = default;
  ~TypesafeHandle() = default;
  constexpr explicit TypesafeHandle(StorageType value) : value_(value) {}
  constexpr TypesafeHandle(TypesafeHandle const&) = default;
  constexpr TypesafeHandle(TypesafeHandle&& rhs) noexcept : value_(rhs.value_) { rhs.value_ = InvalidValue; }

  constexpr explicit operator StorageType() const { return value_; }

  constexpr StorageType Get() const { return value_; }

  constexpr bool IsValid() const { return value_ != InvalidValue; }

  constexpr TypesafeHandle& operator=(TypesafeHandle const&) = default;
  TypesafeHandle& operator=(TypesafeHandle&& rhs) {
    value_ = rhs.value_;
    rhs.value_ = InvalidValue;
    return *this;
  }

  constexpr bool operator==(TypesafeHandle const& rhs) const { return value_ == rhs.value_; }
  constexpr bool operator!=(TypesafeHandle const& rhs) const { return value_ != rhs.value_; }

  constexpr bool operator<(TypesafeHandle const& rhs) const { return value_ < rhs.value_; }
  constexpr bool operator>(TypesafeHandle const& rhs) const { return value_ > rhs.value_; }

  struct Hasher final {
    size_t operator()(TypesafeHandle const& v) const {
      return std::hash<StorageType>{}(v.value_);
    }
  };

private:
  StorageType value_ = InvalidValue;
};

} // namespace mbase

namespace std {

template<class Tag, class TStorage, TStorage InvalidValue>
struct hash<mbase::TypesafeHandle<Tag, TStorage, InvalidValue>> {
  size_t operator()(mbase::TypesafeHandle<Tag, TStorage, InvalidValue> const& v) const {
    return typename mbase::TypesafeHandle<Tag, TStorage, InvalidValue>::Hasher{}(v);
  }
};

} // namespace std

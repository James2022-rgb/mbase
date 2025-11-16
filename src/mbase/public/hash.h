#pragma once

// c++ headers ------------------------------------------
#include <cstdint>
#include <cstddef>

#include <type_traits>
#include <string>

// public project headers -------------------------------
#include "mbase/public/platform.h"
#include "mbase/public/access.h"

namespace mbase {

namespace detail {

template<class T>
struct HasherStateTraits final {
};

template<>
struct HasherStateTraits<uint32_t> final {
  static constexpr uint32_t kStorageSize = 48;
  static constexpr uint32_t kStorageAlign = 4;
};

template<>
struct HasherStateTraits<uint64_t> final {
  static constexpr uint32_t kStorageSize = 88;
  static constexpr uint32_t kStorageAlign = 8;
};

template<bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template<class T>
bool constexpr is_arithmetic_or_enum_v = std::is_arithmetic_v<std::remove_all_extents_t<T>> || std::is_enum_v <std::remove_all_extents_t<T>>;

template<class T, class U = void>
struct integer_traits {
};

template<class T>
struct integer_traits<T, enable_if_t<std::is_unsigned<T>::value && sizeof(T) == 4>> {
  using stdint_type = uint32_t;
};

template<class T>
struct integer_traits<T, enable_if_t<std::is_unsigned<T>::value && sizeof(T) == 8>> {
  using stdint_type = uint64_t;
};

template<class Type>
class TypeHasData {
  using Yes = char;
  using No = long;

  template<class T> static Yes Test(T* p, enable_if_t<std::is_same_v<typename T::value_type*, decltype(p->data())>>* = 0);
  static No Test(...);
public:
  static bool constexpr value = sizeof(Test((std::remove_reference_t<Type>*)(0))) == sizeof(Yes);
};

template<class T>
bool constexpr is_contiguous_simple_container_v = TypeHasData<T>::value;
template<class T>
bool constexpr has_random_access_iterator = std::is_convertible_v<typename std::iterator_traits<typename T::iterator>::iterator_category, std::random_access_iterator_tag>;
template<class T>
bool constexpr is_contiguous_container_v = is_contiguous_simple_container_v<T> || has_random_access_iterator<T>;

} // namespace detail

template<class THash = uint64_t>
class HasherN final {
public:
  using ValueType = THash;

  static ValueType ComputeBytes(void const* s, uint64_t length);

  template<class T, std::enable_if_t<detail::is_arithmetic_or_enum_v<T>, std::nullptr_t> = nullptr>
  static ValueType Compute(T const& s) {
    return ComputePod(s);
  }
  template<class T, std::enable_if_t<detail::TypeHasData<T>::value, std::nullptr_t> = nullptr>
  static ValueType Compute(T const& s) {
    return ComputeArray(s.data(), s.size());
  }

  template<class T>
  static ValueType ComputePod(T const& s) {
    return ComputeBytes(&s, sizeof(T));
  }
  template<class T>
  static ValueType ComputeArray(T const* s, uint64_t count) {
    return ComputeBytes(s, sizeof(T) * count);
  }

  HasherN();
  ~HasherN();
  MBASE_DEFAULT_COPY_MOVE(HasherN);

  ValueType Finish();

  void Reset();

  void DoBytes(void const* s, uint64_t length);

  template<class T, std::enable_if_t<std::is_trivially_copyable_v<T> && !detail::TypeHasData<T>::value, std::nullptr_t> = nullptr>
  void Do(T const& s) {
    DoBytes(&s, sizeof(T));
  }

  template<class TArg, class ... TRestArgs>
  void Do(TArg const& arg, TRestArgs const& ... rest_args) {
    Do(arg);
    Do(rest_args...);
  }

  template<class T, std::enable_if_t<detail::TypeHasData<T>::value && std::is_trivially_copyable_v<typename T::value_type>, std::nullptr_t> = nullptr>
  void Do(T const& contiguous_container) {
    static_assert(detail::is_contiguous_container_v<T>);
    DoContiguousContainer(contiguous_container);
  }

  template<class T>
  void DoArray(T const* s, uint64_t count) {
    DoBytes(s, sizeof(T) * count);
  }

  template<class T>
  void DoContiguousContainer(T const& contiguous_container) {
    static_assert(detail::is_contiguous_container_v<T>);
    Do(uint32_t(contiguous_container.size()));
    DoArray(contiguous_container.data(), contiguous_container.size());
  }

private:
  using Storage = std::byte[detail::HasherStateTraits<ValueType>::kStorageSize];
  static constexpr ValueType kDefaultValue = 0;

  alignas(detail::HasherStateTraits<ValueType>::kStorageAlign) Storage storage_ {};
  ValueType value_ = kDefaultValue;
};

using Hasher32 = HasherN<uint32_t>;
using Hasher64 = HasherN<uint64_t>;
using HasherSizeT = HasherN<detail::integer_traits<size_t>::stdint_type>;

using Hasher = Hasher64;

namespace detail {

inline void hash_combine_impl(size_t& seed, size_t value)
{
#if MBASE_PLATFORM_32_BIT
  static_assert(sizeof(size_t) == sizeof(uint32_t));

  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
#elif MBASE_PLATFORM_64_BIT
  static_assert(sizeof(size_t) == sizeof(uint64_t));

  // https://suzulang.com/cpp-64bit-hash-combine/
  // https://stackoverflow.com/questions/8513911/how-to-create-a-good-hash-combine-with-64-bit-output-inspired-by-boosthash-co
  seed ^= value + 0x9e3779b97f4a7c15 + (seed << 12) + (seed >> 4);
#endif
}

}

template<class T>
inline void hash_combine(size_t& seed, T const& value) {
  detail::hash_combine_impl(seed, std::hash<T>()(value));
}

} // namespace mbase

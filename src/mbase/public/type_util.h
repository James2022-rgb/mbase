#pragma once

#include <cstddef>

#include <type_traits>

// See: https://clang.llvm.org/docs/AttributeReference.html#nullable
#ifndef MBASE_NULLABLE
# ifdef __clang__
#  define MBASE_NULLABLE _Nullable
# else
#  define MBASE_NULLABLE
# endif
#endif

// See: https://clang.llvm.org/docs/AttributeReference.html#nonnull
#ifndef MBASE_NOT_NULL
# ifdef __clang__
#  define MBASE_NOT_NULL _Nonnull
# else
#  define MBASE_NOT_NULL
# endif
#endif

# define MBASE_STATIC_ASSERT_NO_PADDING(type, last_member) \
  static_assert(offsetof(type, last_member) == sizeof(type) - sizeof(type::last_member))

namespace mbase {

template<class T>
constexpr typename std::underlying_type<T>::type UnderlyingCast(T value) {
  return static_cast<typename std::underlying_type<T>::type>(value);
}

} // namespace mbase

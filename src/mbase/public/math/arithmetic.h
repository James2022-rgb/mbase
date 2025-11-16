#pragma once

#include <cstdint>

#include <type_traits>
#include <algorithm>

namespace mbase::math {

template<class T>
[[nodiscard]]
constexpr bool IsPowerOf2(T value) {
  static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
  return (value != 0) && ((value & (value - 1)) == 0);
}

[[nodiscard]]
constexpr uint32_t FloorLog2(uint32_t x) {
  return x == 1 ? 0 : 1 + FloorLog2(x >> 1);
}

[[nodiscard]]
constexpr uint32_t CeilLog2(uint32_t x) {
  return x == 1 ? 0 : FloorLog2(x - 1) + 1;
}

template<class T>
[[nodiscard]]
constexpr T Max(T const& a, T const& b) {
  return std::max(a, b);
}
template<class T, class ... Args>
[[nodiscard]]
constexpr T Max(T const& a, T const& b, Args ... args) {
  return Max(Max(a, b), args...);
}

template<class T>
[[nodiscard]]
constexpr T Min(T const& a, T const& b) {
  return std::min(a, b);
}
template<class T, class ... Args>
[[nodiscard]]
constexpr T Min(T const& a, T const& b, Args ... args) {
  return Min(Min(a, b), args...);
}

template<class TValue, class TDivisor>
[[nodiscard]]
constexpr TValue RoundToNextMultiple(TValue value, TDivisor divisor) {
  if (value == 0 && divisor == 0)
    return 0;
  return ((value + (divisor - 1)) / divisor) * divisor;
}

template<class TValue, class TDivisor>
[[nodiscard]]
constexpr TValue RoundToNextMultiplePot(TValue value, TDivisor pot_divisor) {
  if (value == 0 && pot_divisor == 0)
    return 0;

  // Create alignment mask (pot_divisor - 1) with all bits below the highest set bit of pot_divisor set to 1.
  TDivisor mask = pot_divisor - 1;
    
  // Add alignment offset to cross into the next multiple,
  // and clear the lower bits to round down.
  return (value + mask) & ~mask;
}

template<class TValue, TValue NDivisor>
[[nodiscard]]
constexpr TValue RoundToNextMultipleCompCond(TValue value) {
  if constexpr ((NDivisor & (NDivisor - 1)) == 0) {
    return RoundToNextMultiplePot(value, NDivisor);
  }
  else {
    return RoundToNextMultiple(value, NDivisor);
  }
}

template<class TValue, class TDivisor>
void RoundToNextMultipleInplace(TValue& value, TDivisor divisor) {
  value = RoundToNextMultiple(value, divisor);
}

template<class T>
[[nodiscard]]
uint32_t ComputeDivisibility(T value, T divisor) {
  uint32_t i;
  for (i = 0; (value % divisor) == 0; ++i)
    value /= divisor;
  
  return i;
}

template<class T>
[[nodiscard]]
T Gcd(T a, T b) {
  if (a % b == 0) {
    return b;
  }
  return Gcd(b, a % b);
}

template<class T>
[[nodiscard]]
T Lcm(T a, T b) {
  return a * b / Gcd(a, b);
}


}

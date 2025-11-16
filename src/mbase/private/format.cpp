// my header --------------------------------------------
#include "mbase/public/format.h"

// c++ headers ------------------------------------------
#include <functional>
#include <sstream>
#include <iomanip>

// public project headers -------------------------------
#include "mbase/public/assert.h"

namespace mbase {

template<class T>
constexpr T Power(T base, T exponent) noexcept {
  if constexpr (std::is_signed_v<T>) {
    MBASE_ASSERT(exponent >= 0);
  }

  return exponent <= 0 ? 1 : (exponent == 1 ? base : base * Power(base, exponent - 1));
}


std::string Commaize(uint64_t value) {
  constexpr uint64_t kBase = 10;
  constexpr uint64_t kExponent = 3;
  constexpr uint64_t kBaseRaised = Power(kBase, kExponent);

  std::ostringstream oss;

  std::function<void(uint64_t)> f = [&f, &oss](uint64_t n) {
    if (n < kBaseRaised) {
      oss << n;
      return;
    }

    f(n / kBaseRaised);
    oss << "," << std::setw(kExponent) << std::setfill('0') << (n % kBaseRaised);
  };
  f(value);

  return oss.str();

}

} // namespace mbase


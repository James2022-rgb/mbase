#pragma once

#include <string>
#include <string_view>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4459) // declaration of '...' hides global declaration
#endif

#include <fmt/format.h>

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#include "mbase/public/trap.h"
#include "mbase/public/log.h"

#define MBASE_ASSERT(condition) mbase::detail::AssertTerseImpl(condition, #condition, nostd::source_location::current())
#define MBASE_ASSERT_MSG(condition, fmt, ...) mbase::detail::AssertImpl(condition, #condition, nostd::source_location::current(), fmt, ##__VA_ARGS__)

namespace mbase {

namespace detail {

template<typename... Args>
void AssertTerseImpl(bool condition, std::string_view condition_str, nostd::source_location source_location) {
  if (!condition) {
    MBASE_LOG_ERROR("Assertion failed at [{}:{}][{}]: {}", source_location.file_name(), source_location.line(), source_location.function_name(), condition_str);
    Trap();
  }
}

template<typename... Args>
void AssertImpl(bool condition, std::string_view condition_str, nostd::source_location source_location, fmt::format_string<Args...> fmt, Args &&...args) {
  if (!condition) {
    std::string message = fmt::format(fmt, std::forward<Args>(args)...);
    MBASE_LOG_ERROR("Assertion failed at [{}:{}][{}]: {} : {}", source_location.file_name(), source_location.line(), source_location.function_name(), condition_str, message);
    Trap();
  }
}

}

}

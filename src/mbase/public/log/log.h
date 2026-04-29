#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "source_location/source_location.hpp"

#ifdef _MSC_VER
# pragma warning(push)
// Defensive: silence C4459 if a fmt version we pin starts shadowing names internally.
# pragma warning(disable: 4459) // declaration of '...' hides global declaration
#endif

#include <fmt/format.h>

#ifdef _MSC_VER
# pragma warning(pop)
#endif

namespace mbase {

struct LogTag final {
  nostd::source_location location;
};

#define MBASE_LOG_TAG_ARGUMENT std::optional<LogTag> const& tag

#define __MBASE_LOG_TRACE_ mbase::Logger::Trace
#define __MBASE_LOG_DEBUG_ mbase::Logger::Debug
#define __MBASE_LOG_INFO_ mbase::Logger::Info
#define __MBASE_LOG_WARN_ mbase::Logger::Warn
#define __MBASE_LOG_ERROR_ mbase::Logger::Error
#define __MBASE_LOG_CRITICAL_ mbase::Logger::Critical

#define MBASE_LOG_TRACE(...)    __MBASE_LOG_TRACE_(mbase::LogTag { nostd::source_location::current() }, __VA_ARGS__)
#define MBASE_LOG_DEBUG(...)    __MBASE_LOG_DEBUG_(mbase::LogTag { nostd::source_location::current() }, __VA_ARGS__)
#define MBASE_LOG_INFO(...)     __MBASE_LOG_INFO_(mbase::LogTag { nostd::source_location::current() }, __VA_ARGS__)
#define MBASE_LOG_WARN(...)     __MBASE_LOG_WARN_(mbase::LogTag { nostd::source_location::current() }, __VA_ARGS__)
#define MBASE_LOG_ERROR(...)    __MBASE_LOG_ERROR_(mbase::LogTag { nostd::source_location::current() }, __VA_ARGS__)
#define MBASE_LOG_CRITICAL(...) __MBASE_LOG_CRITICAL_(mbase::LogTag { nostd::source_location::current() }, __VA_ARGS__)

#define MBASE_LOG(level, ...) mbase::Logger::Log(mbase::LogTag { nostd::source_location::current() }, level, __VA_ARGS__)

class Logger final {
public:
  enum class Level {
    kTrace,
    kDebug,
    kInfo,
    kWarn,
    kError,
    kCritical
  };

  using LogCallback = std::function<void(Level, std::chrono::system_clock::time_point, std::string_view)>;

  static void Initialize();
  static void Shutdown();

  static void SetLevel(Level value);

  /// Non-template entry point used by the templates below. Defined in log.cpp;
  /// takes an already-formatted payload.
  static void LogImpl(MBASE_LOG_TAG_ARGUMENT, Level level, std::string_view payload);

  template<typename... Args>
  static void Log(MBASE_LOG_TAG_ARGUMENT, Level level, fmt::format_string<Args...> fmt, Args &&...args) {
    LogImpl(tag, level, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void Trace(MBASE_LOG_TAG_ARGUMENT, fmt::format_string<Args...> fmt, Args &&...args) {
    LogImpl(tag, Level::kTrace, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void Debug(MBASE_LOG_TAG_ARGUMENT, fmt::format_string<Args...> fmt, Args &&...args) {
    LogImpl(tag, Level::kDebug, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void Info(MBASE_LOG_TAG_ARGUMENT, fmt::format_string<Args...> fmt, Args &&...args) {
    LogImpl(tag, Level::kInfo, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void Warn(MBASE_LOG_TAG_ARGUMENT, fmt::format_string<Args...> fmt, Args &&...args) {
    LogImpl(tag, Level::kWarn, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void Error(MBASE_LOG_TAG_ARGUMENT, fmt::format_string<Args...> fmt, Args &&...args) {
    LogImpl(tag, Level::kError, fmt::format(fmt, std::forward<Args>(args)...));
  }

  template<typename... Args>
  static void Critical(MBASE_LOG_TAG_ARGUMENT, fmt::format_string<Args...> fmt, Args &&...args) {
    LogImpl(tag, Level::kCritical, fmt::format(fmt, std::forward<Args>(args)...));
  }

  /// mbase-owned dispatch sink interface. Callers can register a `LogCallback`
  /// to receive log events.
  class IDistSink {
  public:
    virtual ~IDistSink() = default;
    IDistSink(IDistSink const&) = delete;
    IDistSink& operator=(IDistSink const&) = delete;
    IDistSink(IDistSink&&) = delete;
    IDistSink& operator=(IDistSink&&) = delete;

    virtual void AddCallback(std::string const& name, LogCallback const& value) = 0;
    virtual void RemoveCallback(std::string const& name) = 0;

  protected:
    IDistSink() = default;
  };

  /// Returns the dist sink installed by `Initialize()`, or nullptr if not initialized.
  static std::shared_ptr<IDistSink> GetDistSink();
};

} // namespace mbase

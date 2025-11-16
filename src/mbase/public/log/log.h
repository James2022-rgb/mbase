#pragma once

#include <string_view>
#include <functional>
#include <optional>
#include <type_traits>

#include "source_location/source_location.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/fmt/ostr.h"

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

  using LogCallback = std::function<void(Level, spdlog::log_clock::time_point, std::string_view)>;

  static void Initialize();
  static void Shutdown();

  static void SetLevel(Level value);

  template<typename... Args>
  static void Log(MBASE_LOG_TAG_ARGUMENT, Level level, spdlog::format_string_t<Args...> fmt, Args &&...args) {
    if (tag.has_value()) {
      spdlog::source_loc source_loc { tag->location.file_name(), static_cast<int>(tag->location.line()), tag->location.function_name() };
      logger_->log(source_loc, ToSpdLogLevel(level), fmt, std::forward<Args>(args)...);
    }
    else {
      logger_->log(ToSpdLogLevel(level), fmt, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static void Trace(MBASE_LOG_TAG_ARGUMENT, spdlog::format_string_t<Args...> fmt, Args &&...args) {
    Log(tag, Level::kTrace, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Debug(MBASE_LOG_TAG_ARGUMENT, spdlog::format_string_t<Args...> fmt, Args &&...args) {
    Log(tag, Level::kDebug, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Info(MBASE_LOG_TAG_ARGUMENT, spdlog::format_string_t<Args...> fmt, Args &&...args) {
    Log(tag, Level::kInfo, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Warn(MBASE_LOG_TAG_ARGUMENT, spdlog::format_string_t<Args...> fmt, Args &&...args) {
    Log(tag, Level::kWarn, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Error(MBASE_LOG_TAG_ARGUMENT, spdlog::format_string_t<Args...> fmt, Args &&...args) {
    Log(tag, Level::kError, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Critical(MBASE_LOG_TAG_ARGUMENT, spdlog::format_string_t<Args...> fmt, Args &&...args) {
    Log(tag, Level::kCritical, fmt, std::forward<Args>(args)...);
  }

  using Mutex = std::mutex;

  class IDistSink : public spdlog::sinks::base_sink<Mutex> {
  public:
    virtual  ~IDistSink() = default;
    IDistSink(IDistSink const&) = delete;
    IDistSink& operator=(IDistSink const&) = delete;
    IDistSink(IDistSink&&) = delete;
    IDistSink& operator=(IDistSink&&) = delete;

    virtual void AddCallback(std::string const& name, LogCallback const& value) = 0;
    virtual void RemoveCallback(std::string const& name) = 0;

  protected:
    IDistSink() = default;
  };

  static spdlog::level::level_enum ToSpdLogLevel(Level level);
  static Level ToLoggerLevel(spdlog::level::level_enum level);

private:
  static inline std::shared_ptr<spdlog::logger> logger_;
  static inline std::shared_ptr<IDistSink> dist_sink_;
};

} // namespace mabse

// my header --------------------------------------------
#include "mbase/public/log.h"

// c++ headers ------------------------------------------
#include <mutex>
#include <unordered_map>
#include <sstream>
#include <filesystem>

// platform deteection headers --------------------------
#include "mbase/public/platform.h"

// external headers -------------------------------------
#include "spdlog/sinks/basic_file_sink.h"

// conditional platform headers -------------------------
#if MBASE_PLATFORM_ANDROID
# include <android/log.h>
#endif

// conditional external headers -------------------------
#if MBASE_PLATFORM_LINUX || MBASE_PLATFORM_WEB
# include "spdlog/sinks/stdout_color_sinks.h"
#endif

// project headers --------------------------------------
#include "mbase/public/tsa.h"

namespace mbase {

namespace {

class ICustomSink : public spdlog::sinks::base_sink<Lockable<Logger::Mutex>> {
public:
  using BaseType = spdlog::sinks::base_sink<Lockable<Logger::Mutex>>;

  ~ICustomSink() override = default;
  ICustomSink(ICustomSink const&) = delete;
  ICustomSink& operator=(ICustomSink const&) = delete;
  ICustomSink(ICustomSink&&) = delete;
  ICustomSink& operator=(ICustomSink&&) = delete;

protected:
  friend class DistSink;
  ICustomSink() = default;

  static std::string MakeLogString(spdlog::details::log_msg const& msg) {
    std::string log;
    log.reserve(msg.payload.size() + 64);

    if (!msg.source.empty()) {
      log += fmt::format(" {}[{}] ({})", msg.source.filename, msg.source.line, msg.source.funcname);
    }
    log += std::string(std::begin(msg.payload), std::end(msg.payload)) + "\n";
    return log;
  }
};

#if MBASE_PLATFORM_WINDOWS

// Mostly directly taken from spdlog/sinks/wincolor_sink.h
class WincolorStdoutSink final : public ICustomSink {
public:
  using BaseType = ICustomSink;

  const WORD BOLD = FOREGROUND_INTENSITY;
  const WORD RED = FOREGROUND_RED;
  const WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
  const WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  const WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;

  WincolorStdoutSink() {
    out_handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
    colors_[spdlog::level::trace] = CYAN;
    colors_[spdlog::level::debug] = CYAN;
    colors_[spdlog::level::info] = WHITE | BOLD;
    colors_[spdlog::level::warn] = YELLOW | BOLD;
    colors_[spdlog::level::err] = RED | BOLD; // red bold
    colors_[spdlog::level::critical] = BACKGROUND_RED | WHITE | BOLD; // white bold on red background
    colors_[spdlog::level::off] = 0;
  }
  ~WincolorStdoutSink() = default;
  MBASE_DISALLOW_COPY_MOVE(WincolorStdoutSink);

  void sink_it_(const spdlog::details::log_msg& msg) override {
    WORD const& color = colors_[msg.level];
    WORD const& orig_attribs = set_console_attribs(color);
 
    WriteConsoleA(out_handle_, msg.payload.data(), static_cast<DWORD>(msg.payload.size()), nullptr, nullptr);
    WriteConsoleA(out_handle_, "\n", 1, nullptr, nullptr);
    SetConsoleTextAttribute(out_handle_, orig_attribs); //reset to orig colors
  }
  void flush_() override {
  }

private:
  // set color and return the orig console attributes (for resetting later)
  WORD set_console_attribs(WORD attribs)
  {
      CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
      GetConsoleScreenBufferInfo(out_handle_, &orig_buffer_info);
      WORD back_color = orig_buffer_info.wAttributes;
      // retrieve the current background color
      back_color &= static_cast<WORD>(~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY));
      // keep the background color unchanged
      SetConsoleTextAttribute(out_handle_, attribs | back_color);
      return  orig_buffer_info.wAttributes; //return orig attribs
  }

  HANDLE out_handle_ = nullptr;
  std::unordered_map<spdlog::level::level_enum, WORD> colors_;
};

#elif MBASE_PLATFORM_ANDROID

// Mostly directly taken from spdlog/sinks/android_sink.h
class AndroidSink final : public ICustomSink {
public:
  using BaseType = ICustomSink;

  static constexpr int kRetries = 2;

  explicit AndroidSink(const std::string& tag = "spdlog") : tag_(tag) {}

  void sink_it_(const spdlog::details::log_msg& msg) override MBASE_REQUIRES(BaseType::mutex_) {
    const android_LogPriority priority = convert_to_android(msg.level);
    std::string const null_terminated = msg.payload.data();

    // See system/core/liblog/logger_write.c for explanation of return value
    int ret = __android_log_write(priority, tag_.c_str(), null_terminated.c_str());
    int retry_count = 0;
    while ((ret == -11/*EAGAIN*/) && (retry_count < kRetries))
    {
      spdlog::details::os::sleep_for_millis(5);
      ret = __android_log_write(priority, tag_.c_str(), null_terminated.c_str());
      retry_count++;
    }

    if (ret < 0)
    {
      throw spdlog::spdlog_ex("__android_log_write() failed", ret);
    }
  }
  void flush_() override {
  }
private:
  static android_LogPriority convert_to_android(spdlog::level::level_enum level) {
    switch(level)
    {
    case spdlog::level::trace:
        return ANDROID_LOG_VERBOSE;
    case spdlog::level::debug:
        return ANDROID_LOG_DEBUG;
    case spdlog::level::info:
        return ANDROID_LOG_INFO;
    case spdlog::level::warn:
        return ANDROID_LOG_WARN;
    case spdlog::level::err:
        return ANDROID_LOG_ERROR;
    case spdlog::level::critical:
        return ANDROID_LOG_FATAL;
    default:
        return ANDROID_LOG_DEFAULT;
    }
  }

  std::string tag_;
};

#elif MBASE_PLATFORM_LINUX || MBASE_PLATFORM_WEB

class LinuxConsoleSink final : public ICustomSink {
public:
  using BaseType = ICustomSink;

  LinuxConsoleSink() = default;
  ~LinuxConsoleSink() override = default;
  MBASE_DISALLOW_COPY_MOVE(LinuxConsoleSink);

  void sink_it_(const spdlog::details::log_msg& msg) override {
    inner_.log(msg);
  }
  void flush_() override {
    inner_.flush();
  }
  void set_pattern_(const std::string &pattern) override {
    inner_.set_pattern(pattern);
  }
  void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter) override {
    inner_.set_formatter(std::move(sink_formatter));
  }

private:
  spdlog::sinks::stdout_color_sink_mt inner_;
};

#elif MBASE_PLATFORM_PSP

// PSP does not have console support for logging.
// Therefore we only provide an empty sink implementation.
class PspNullSink final : public ICustomSink {
public:
  using BaseType = ICustomSink;

  PspNullSink() = default;
  ~PspNullSink() override = default;
  MBASE_DISALLOW_COPY_MOVE(PspNullSink);

  void sink_it_(const spdlog::details::log_msg&) override {
    // No-op
  }
  void flush_() override {
    // No-op
  }
};

#else
# error "Unsupported platform"
#endif

// Mostly directly taken from spdlog/sinks/file_sinks.h
class SimpleFileSink final : public ICustomSink {
public:
  using BaseType = ICustomSink;

  explicit SimpleFileSink(const spdlog::filename_t &filename, bool truncate = false): force_flush_(false)
  {
    file_helper_.open(filename, truncate);
  }

  void set_force_flush(bool force_flush)
  {
    force_flush_ = force_flush;
  }

  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t memory_buf;
    memory_buf.append(msg.payload);
    memory_buf.push_back('\n');
    file_helper_.write(memory_buf);
    if (force_flush_ || spdlog::level::err <= msg.level) {
      file_helper_.flush();
    }
  }
  void flush_() override {
    file_helper_.flush();
  }

private:
  spdlog::details::file_helper file_helper_;
  bool force_flush_;
};

class DistSink final : public Logger::IDistSink {
public:
  DistSink() = default;
  ~DistSink() override = default;
  MBASE_DISALLOW_COPY_MOVE(DistSink);

  void AddCallback(std::string const& name, Logger::LogCallback const& value) override {
    LockGuard lock(mutex_);
    callbacks_[name] = value;
  }
  void RemoveCallback(std::string const& name) override {
    LockGuard lock(mutex_);
    callbacks_.erase(name);
  }

  //
  // Module local
  //

  void AddSink(std::shared_ptr<ICustomSink> const& value) {
    LockGuard lock(mutex_);
    sinks_.emplace_back(value);
  }
  void ClearSinks() {
    LockGuard lock(mutex_);
    sinks_.clear();
  }

protected:

  //
  // spdlog::sinks::base_sink<TMutex> implementation
  //

  void sink_it_(const spdlog::details::log_msg& msg) override {
    auto owned_msg = OwnedLogMsg {
      .logger_name = std::string(std::begin(msg.logger_name), std::end(msg.logger_name)),
      .level = msg.level,
      .time = msg.time,
      .thread_id = msg.thread_id,
      .color_range_start = msg.color_range_start,
      .color_range_end = msg.color_range_end,
      .source = msg.source,
      .payload = std::string(std::begin(msg.payload), std::end(msg.payload))
    };

    LockGuard lock(mutex_);

    log_msgs_.emplace_back(std::move(owned_msg));
  }

  void flush_() override {
    LockGuard lock(mutex_);

    for (OwnedLogMsg const& owned_msg : log_msgs_) {
      for (std::shared_ptr<ICustomSink> const& sink : sinks_) {
        if (sink->should_log(owned_msg.level)) {
          sink->sink_it_(
            spdlog::details::log_msg(
              owned_msg.time,
              owned_msg.source,
              spdlog::string_view_t(owned_msg.logger_name.data(), owned_msg.logger_name.size()),
              owned_msg.level,
              spdlog::string_view_t(owned_msg.payload.data(), owned_msg.payload.size())
            )
          );
        }
      }

      for (auto const& [name, callback] : callbacks_) {
        if (callback) {
          callback(
            Logger::ToLoggerLevel(owned_msg.level),
            owned_msg.time,
            owned_msg.payload
          );
        }
      }
    }

    log_msgs_.clear();
  }

  void set_pattern_(const std::string &pattern) override {
    LockGuard lock(mutex_);

    for (std::shared_ptr<ICustomSink> const& sink : sinks_) {
      sink->set_pattern_(pattern);
    }
  }

  void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter) override {
    LockGuard lock(mutex_);

    for (std::shared_ptr<ICustomSink> const& sink : sinks_) {
      sink->set_formatter_(std::move(sink_formatter));
    }
  }

private:
  struct OwnedLogMsg final {
    std::string logger_name;
    spdlog::level::level_enum level{ spdlog::level::off };
    spdlog::log_clock::time_point time;
    size_t thread_id{ 0 };

    // wrapping the formatted text with color (updated by pattern_formatter).
    mutable size_t color_range_start{ 0 };
    mutable size_t color_range_end{ 0 };

    spdlog::source_loc source;
    std::string payload;
  };

  Lockable<std::mutex> mutex_;

  std::vector<std::shared_ptr<ICustomSink>> sinks_ MBASE_GUARDED_BY(mutex_);
  std::unordered_map<std::string, Logger::LogCallback> callbacks_ MBASE_GUARDED_BY(mutex_);

  std::vector<OwnedLogMsg> log_msgs_ MBASE_GUARDED_BY(mutex_);
};

} // namespace

void Logger::Initialize() {
  // Create a directory for log files, if there isn't any.
#if MBASE_PLATFORM_WINDOWS
  {
    namespace fs = std::filesystem;

    const fs::path path("log");

    if (!fs::exists(path)) {
      fs::create_directory(path);
    }
  }
#endif // MBASE_PLATFORM_WINDOWS

  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "log/%Y-%m-%d-%H-%M-%S");
  auto name = oss.str() + ".txt";

#if MBASE_PLATFORM_WINDOWS
  auto console_sink = std::make_shared<WincolorStdoutSink>();
  auto file_sink = std::make_shared<SimpleFileSink>(name);
#elif MBASE_PLATFORM_LINUX || MBASE_PLATFORM_WEB
  auto console_sink = std::make_shared<LinuxConsoleSink>();
#elif MBASE_PLATFORM_ANDROID
  auto console_sink = std::make_shared<AndroidSink>("machina");
#elif MBASE_PLATFORM_PSP
  auto console_sink = std::make_shared<PspNullSink>();
#endif

  std::shared_ptr<DistSink> dist_sink = std::make_shared<DistSink>();

  dist_sink->AddSink(console_sink);
#if MBASE_PLATFORM_WINDOWS
  dist_sink->AddSink(file_sink);
#endif

  dist_sink_ = dist_sink;

  logger_ = std::make_shared<spdlog::logger>("machina_logger", dist_sink_);
  logger_->set_pattern(R"([%Y-%m-%d %R][%g:%#][%^%l%$] %v)");
  logger_->flush_on(spdlog::level::info);
  spdlog::register_logger(logger_);

  SetLevel(Level::kTrace);
}

void Logger::Shutdown() {
  spdlog::drop("machina_logger");
  dist_sink_.reset();
  logger_.reset();
}

void Logger::SetLevel(Logger::Level value) {
  logger_->set_level(ToSpdLogLevel(value));
}

spdlog::level::level_enum Logger::ToSpdLogLevel(Logger::Level level) {
  switch (level) {
  case Logger::Level::kTrace:
    return spdlog::level::trace;
  case Logger::Level::kDebug:
    return spdlog::level::debug;
  case Logger::Level::kInfo:
    return spdlog::level::info;
  case Logger::Level::kWarn:
    return spdlog::level::warn;
  case Logger::Level::kError:
    return spdlog::level::err;
  case Logger::Level::kCritical:
    return spdlog::level::critical;
  default:
    return spdlog::level::off;
  }
}

//
// static functions
//

Logger::Level Logger::ToLoggerLevel(spdlog::level::level_enum level) {
  switch (level) {
  case spdlog::level::trace:
    return Logger::Level::kTrace;
  case spdlog::level::debug:
    return Logger::Level::kDebug;
  case spdlog::level::info:
    return Logger::Level::kInfo;
  case spdlog::level::warn:
    return Logger::Level::kWarn;
  case spdlog::level::err:
    return Logger::Level::kError;
  case spdlog::level::critical:
    return Logger::Level::kCritical;
  default:
    return Logger::Level::kError;
  }
}

} // namespace mbase

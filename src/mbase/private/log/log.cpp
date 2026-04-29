// my header --------------------------------------------
#include "mbase/public/log.h"

// c++ headers ------------------------------------------
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// platform deteection headers --------------------------
#include "mbase/public/platform.h"

// conditional platform headers -------------------------
#if MBASE_PLATFORM_WINDOWS
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#if MBASE_PLATFORM_ANDROID
# include <android/log.h>
#endif

// project headers --------------------------------------
#include "mbase/public/tsa.h"

namespace mbase {

namespace {

// ----------------------------------------------------------------------------------------------------
// IPlatformSink
//
// Abstract base for an output target. All sinks receive a fully-formatted line
// plus the metadata needed for level-based color decisions and Android-style
// per-level dispatching.

class IPlatformSink {
public:
  virtual ~IPlatformSink() = default;
  IPlatformSink(IPlatformSink const&) = delete;
  IPlatformSink& operator=(IPlatformSink const&) = delete;
  IPlatformSink(IPlatformSink&&) = delete;
  IPlatformSink& operator=(IPlatformSink&&) = delete;

  virtual void Sink(Logger::Level level, std::string_view formatted_line) = 0;
  virtual void Flush() {}

protected:
  IPlatformSink() = default;
};

// ----------------------------------------------------------------------------------------------------
// FormatLogLine
//
// Builds a line in the same shape as the previous spdlog pattern:
//   [YYYY-MM-DD HH:MM][file:line][LEVEL] payload

char const* LevelLabel(Logger::Level level) {
  switch (level) {
  case Logger::Level::kTrace: return "trace";
  case Logger::Level::kDebug: return "debug";
  case Logger::Level::kInfo: return "info";
  case Logger::Level::kWarn: return "warning";
  case Logger::Level::kError: return "error";
  case Logger::Level::kCritical: return "critical";
  }
  return "?";
}

std::string FormatLogLine(
  Logger::Level level,
  std::chrono::system_clock::time_point time,
  char const* file,
  int line,
  char const* /*func*/,
  std::string_view payload
) {
  // Filename basename only.
  std::string_view file_view(file != nullptr ? file : "");
  size_t const slash = file_view.find_last_of("/\\");
  if (slash != std::string_view::npos) {
    file_view.remove_prefix(slash + 1);
  }

  std::time_t t = std::chrono::system_clock::to_time_t(time);
  std::tm tm{};
#if MBASE_PLATFORM_WINDOWS
  ::localtime_s(&tm, &t);
#else
  ::localtime_r(&t, &tm);
#endif

  std::ostringstream oss;
  oss << '[' << std::put_time(&tm, "%Y-%m-%d %H:%M") << ']'
      << '[' << file_view << ':' << line << ']'
      << '[' << LevelLabel(level) << "] "
      << payload;
  return oss.str();
}

#if MBASE_PLATFORM_WINDOWS

// Console sink with color, modeled after spdlog/sinks/wincolor_sink.h.
class WincolorStdoutSink final : public IPlatformSink {
public:
  WincolorStdoutSink() {
    out_handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
    constexpr WORD BOLD = FOREGROUND_INTENSITY;
    constexpr WORD RED = FOREGROUND_RED;
    constexpr WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
    constexpr WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    constexpr WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
    colors_[static_cast<size_t>(Logger::Level::kTrace)] = CYAN;
    colors_[static_cast<size_t>(Logger::Level::kDebug)] = CYAN;
    colors_[static_cast<size_t>(Logger::Level::kInfo)] = WHITE | BOLD;
    colors_[static_cast<size_t>(Logger::Level::kWarn)] = YELLOW | BOLD;
    colors_[static_cast<size_t>(Logger::Level::kError)] = RED | BOLD;
    colors_[static_cast<size_t>(Logger::Level::kCritical)] = BACKGROUND_RED | WHITE | BOLD;
  }
  ~WincolorStdoutSink() override = default;

  void Sink(Logger::Level level, std::string_view line) override {
    LockGuard lock(mutex_);
    WORD const color = colors_[static_cast<size_t>(level)];
    WORD const orig_attribs = SetConsoleAttribs(color);
    WriteConsoleA(out_handle_, line.data(), static_cast<DWORD>(line.size()), nullptr, nullptr);
    WriteConsoleA(out_handle_, "\n", 1, nullptr, nullptr);
    SetConsoleTextAttribute(out_handle_, orig_attribs);
  }

private:
  // Returns previous attributes for restore.
  WORD SetConsoleAttribs(WORD attribs) {
    CONSOLE_SCREEN_BUFFER_INFO orig{};
    GetConsoleScreenBufferInfo(out_handle_, &orig);
    WORD back_color = orig.wAttributes;
    back_color &= static_cast<WORD>(~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY));
    SetConsoleTextAttribute(out_handle_, attribs | back_color);
    return orig.wAttributes;
  }

  HANDLE out_handle_ = nullptr;
  std::array<WORD, 6> colors_{};
  Lockable<std::mutex> mutex_;
};

#elif MBASE_PLATFORM_ANDROID

class AndroidSink final : public IPlatformSink {
public:
  explicit AndroidSink(std::string tag) : tag_(std::move(tag)) {}
  ~AndroidSink() override = default;

  void Sink(Logger::Level level, std::string_view line) override {
    LockGuard lock(mutex_);
    android_LogPriority const priority = ToAndroidPriority(level);
    std::string null_terminated(line);
    __android_log_write(priority, tag_.c_str(), null_terminated.c_str());
  }

private:
  static android_LogPriority ToAndroidPriority(Logger::Level level) {
    switch (level) {
    case Logger::Level::kTrace: return ANDROID_LOG_VERBOSE;
    case Logger::Level::kDebug: return ANDROID_LOG_DEBUG;
    case Logger::Level::kInfo: return ANDROID_LOG_INFO;
    case Logger::Level::kWarn: return ANDROID_LOG_WARN;
    case Logger::Level::kError: return ANDROID_LOG_ERROR;
    case Logger::Level::kCritical: return ANDROID_LOG_FATAL;
    }
    return ANDROID_LOG_DEFAULT;
  }

  std::string tag_;
  Lockable<std::mutex> mutex_;
};

#elif MBASE_PLATFORM_LINUX || MBASE_PLATFORM_WEB

class LinuxConsoleSink final : public IPlatformSink {
public:
  LinuxConsoleSink() = default;
  ~LinuxConsoleSink() override = default;

  void Sink(Logger::Level level, std::string_view line) override {
    LockGuard lock(mutex_);
    char const* color = AnsiColor(level);
    if (color != nullptr) {
      std::fwrite(color, 1, std::strlen(color), stdout);
    }
    std::fwrite(line.data(), 1, line.size(), stdout);
    if (color != nullptr) {
      char const* reset = "\033[0m";
      std::fwrite(reset, 1, std::strlen(reset), stdout);
    }
    std::fputc('\n', stdout);
    std::fflush(stdout);
  }

private:
  static char const* AnsiColor(Logger::Level level) {
    switch (level) {
    case Logger::Level::kTrace: return "\033[36m";          // cyan
    case Logger::Level::kDebug: return "\033[36m";          // cyan
    case Logger::Level::kInfo: return "\033[1;37m";         // bold white
    case Logger::Level::kWarn: return "\033[1;33m";         // bold yellow
    case Logger::Level::kError: return "\033[1;31m";        // bold red
    case Logger::Level::kCritical: return "\033[1;41;37m";  // bold white on red
    }
    return nullptr;
  }

  Lockable<std::mutex> mutex_;
};

#elif MBASE_PLATFORM_PSP

class PspNullSink final : public IPlatformSink {
public:
  PspNullSink() = default;
  ~PspNullSink() override = default;

  void Sink(Logger::Level /*level*/, std::string_view /*line*/) override {
    // No-op
  }
};

#else
# error "Unsupported platform"
#endif

#if MBASE_PLATFORM_WINDOWS

class SimpleFileSink final : public IPlatformSink {
public:
  explicit SimpleFileSink(std::string const& filename, bool truncate = false) {
    auto mode = std::ios::out | std::ios::binary;
    mode |= truncate ? std::ios::trunc : std::ios::app;
    stream_.open(filename, mode);
  }
  ~SimpleFileSink() override {
    if (stream_.is_open()) {
      stream_.flush();
    }
  }

  void Sink(Logger::Level level, std::string_view line) override {
    LockGuard lock(mutex_);
    if (!stream_.is_open()) {
      return;
    }
    stream_.write(line.data(), static_cast<std::streamsize>(line.size()));
    stream_.put('\n');
    if (level >= Logger::Level::kError) {
      stream_.flush();
    }
  }

  void Flush() override {
    LockGuard lock(mutex_);
    if (stream_.is_open()) {
      stream_.flush();
    }
  }

private:
  std::ofstream stream_;
  Lockable<std::mutex> mutex_;
};

#endif // MBASE_PLATFORM_WINDOWS

// ----------------------------------------------------------------------------------------------------
// DistSink
//
// Holds the registered IPlatformSinks plus the application LogCallback map.

class DistSink final : public Logger::IDistSink {
public:
  DistSink() = default;
  ~DistSink() override = default;

  void AddCallback(std::string const& name, Logger::LogCallback const& value) override {
    LockGuard lock(mutex_);
    callbacks_[name] = value;
  }
  void RemoveCallback(std::string const& name) override {
    LockGuard lock(mutex_);
    callbacks_.erase(name);
  }

  // Module-local API (called by Logger::Initialize).
  void AddSink(std::shared_ptr<IPlatformSink> const& sink) {
    LockGuard lock(mutex_);
    sinks_.push_back(sink);
  }

  // Called by Logger::LogImpl.
  void Dispatch(
    Logger::Level level,
    std::chrono::system_clock::time_point time,
    std::string_view formatted_line,
    std::string_view raw_payload
  ) {
    LockGuard lock(mutex_);
    for (auto const& sink : sinks_) {
      sink->Sink(level, formatted_line);
    }
    for (auto const& [name, callback] : callbacks_) {
      if (callback) {
        callback(level, time, raw_payload);
      }
    }
  }

private:
  Lockable<std::mutex> mutex_;
  std::vector<std::shared_ptr<IPlatformSink>> sinks_ MBASE_GUARDED_BY(mutex_);
  std::unordered_map<std::string, Logger::LogCallback> callbacks_ MBASE_GUARDED_BY(mutex_);
};

// ----------------------------------------------------------------------------------------------------
// File-scope state.

std::shared_ptr<DistSink> g_dist_sink;
std::atomic<Logger::Level> g_min_level{ Logger::Level::kTrace };

} // namespace

// ----------------------------------------------------------------------------------------------------
// Logger
//

void Logger::Initialize() {
  std::shared_ptr<DistSink> dist_sink = std::make_shared<DistSink>();

#if MBASE_PLATFORM_WINDOWS
  // Create a directory for log files if it doesn't exist.
  {
    namespace fs = std::filesystem;
    fs::path const path("log");
    if (!fs::exists(path)) {
      fs::create_directory(path);
    }
  }
#endif

  std::time_t const t = std::time(nullptr);
  std::tm tm{};
#if MBASE_PLATFORM_WINDOWS
  ::localtime_s(&tm, &t);
#else
  ::localtime_r(&t, &tm);
#endif

  std::ostringstream oss;
  oss << std::put_time(&tm, "log/%Y-%m-%d-%H-%M-%S") << ".txt";
  std::string const log_filename = oss.str();

#if MBASE_PLATFORM_WINDOWS
  dist_sink->AddSink(std::make_shared<WincolorStdoutSink>());
  dist_sink->AddSink(std::make_shared<SimpleFileSink>(log_filename));
#elif MBASE_PLATFORM_LINUX || MBASE_PLATFORM_WEB
  dist_sink->AddSink(std::make_shared<LinuxConsoleSink>());
#elif MBASE_PLATFORM_ANDROID
  dist_sink->AddSink(std::make_shared<AndroidSink>("machina"));
#elif MBASE_PLATFORM_PSP
  dist_sink->AddSink(std::make_shared<PspNullSink>());
#endif

  g_dist_sink = dist_sink;
  g_min_level.store(Level::kTrace, std::memory_order_relaxed);
}

void Logger::Shutdown() {
  g_dist_sink.reset();
}

void Logger::SetLevel(Level value) {
  g_min_level.store(value, std::memory_order_relaxed);
}

std::shared_ptr<Logger::IDistSink> Logger::GetDistSink() {
  return g_dist_sink;
}

void Logger::LogImpl(MBASE_LOG_TAG_ARGUMENT, Level level, std::string_view payload) {
  if (level < g_min_level.load(std::memory_order_relaxed)) {
    return;
  }

  std::shared_ptr<DistSink> const dist_sink = g_dist_sink;
  if (!dist_sink) {
    return;
  }

  auto const time = std::chrono::system_clock::now();

  char const* file = "";
  int line = 0;
  char const* func = "";
  if (tag.has_value()) {
    file = tag->location.file_name();
    line = static_cast<int>(tag->location.line());
    func = tag->location.function_name();
  }

  std::string const formatted = FormatLogLine(level, time, file, line, func, payload);
  dist_sink->Dispatch(level, time, formatted, payload);
}

} // namespace mbase

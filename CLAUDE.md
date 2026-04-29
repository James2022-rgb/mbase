# mbase

A C++ base library providing fundamental utilities.

## Language

All code comments MUST be written in English.

## Symbols

It is forbidden to use the full width forms of symbols that have counterparts in ASCII.

e.g. `()`, `:`, `,`, `0-9`

## Build Requirements

- CMake 3.22+
- C++23

## Directory Structure

```
src/mbase/
  public/   - Public headers (API)
  private/  - Implementation files
natvis/     - Visual Studio debugger visualizers
thirdparty/ - External dependencies
```

## Components

- `assert.h` - Assertion utilities (uses fmt)
- `format.h` - String formatting (Commaize, etc.)
- `hash.h` - Hashing (xxHash)
- `log.h` - Logging (custom implementation on top of fmt + per-platform sinks)
- `memory.h` - Memory utilities
- `profiling.h` - Scoped timer and profiling
- `platform.h` - Platform abstractions
- `com/` - COM helper utilities
- `container.h` - Container utilities
- `type_safety.h` - Type safety utilities
- `tsa.h` - Thread safety annotations (Clang)
- `trap.h` - `Trap()` / `TrapIfWithMessage()`

## Logging

`log/log.h` exposes `MBASE_LOG_*` macros and `mbase::Logger`. The format
templates take `fmt::format_string<Args...>` for compile-time format-string
checking. The implementation in `private/log/log.cpp` is custom (no spdlog):
per-platform sinks (Windows colored console, Android log, Linux/web ANSI
console, PSP null) plus a `SimpleFileSink` for log file output, all
aggregated by a `DistSink` that also dispatches to user-provided
`LogCallback`s.

`Logger::IDistSink` is a pure mbase abstract interface; obtain the active
sink with `Logger::GetDistSink()` to register `AddCallback` / `RemoveCallback`.

## Dependencies

- `fmt` - Formatting library, fetched via FetchContent (`fmtlib/fmt` 11.0.2)
  and linked PUBLIC. Consumers can use `fmt::format` and `fmt::format_string`
  directly via `<fmt/format.h>`.
- `xxHash` - Hash library (submodule)
- `cereal` - Serialization library (submodule)
- `source_location` - C++20 source_location polyfill (submodule)

Note: spdlog is no longer used. The submodule was removed; `log.cpp`
implements logging directly on top of fmt and platform APIs.

#pragma once

#if defined(_MSC_VER)
# define MBASE_PLATFORM_WINDOWS 1
# define MBASE_PLATFORM_LINUX   0
# define MBASE_PLATFORM_ANDROID 0
# define MBASE_PLATFORM_WEB     0
# define MBASE_PLATFORM_PSP     0
#elif defined(__ANDROID__)
# define MBASE_PLATFORM_WINDOWS 0
# define MBASE_PLATFORM_LINUX   0
# define MBASE_PLATFORM_ANDROID 1
# define MBASE_PLATFORM_WEB     0
# define MBASE_PLATFORM_PSP     0
// __linux__ is also defined on Android, hence the order of the checks.
#elif __linux__
# define MBASE_PLATFORM_WINDOWS 0
# define MBASE_PLATFORM_LINUX   1
# define MBASE_PLATFORM_ANDROID 0
# define MBASE_PLATFORM_WEB     0
# define MBASE_PLATFORM_PSP     0
#elif defined(__EMSCRIPTEN__)
# define MBASE_PLATFORM_WINDOWS 0
# define MBASE_PLATFORM_LINUX   0
# define MBASE_PLATFORM_ANDROID 0
# define MBASE_PLATFORM_WEB     1
# define MBASE_PLATFORM_PSP     0
#elif defined(__PSP__)
# define MBASE_PLATFORM_WINDOWS 0
# define MBASE_PLATFORM_LINUX   0
# define MBASE_PLATFORM_ANDROID 0
# define MBASE_PLATFORM_WEB     0
# define MBASE_PLATFORM_PSP     1
#endif

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
# define MBASE_PLATFORM_64_BIT 1
# define MBASE_PLATFORM_32_BIT 0
#else
# define MBASE_PLATFORM_64_BIT 0
# define MBASE_PLATFORM_32_BIT 1
#endif

#if defined(__x86_64__) || defined(_M_X64)
# define MBASE_PLATFORM_X86_32  0
# define MBASE_PLATFORM_X86_64  1
# define MBASE_PLATFORM_AARCH32 0
# define MBASE_PLATFORM_AARCH64 0
# define MBASE_PLATFORM_MIPS    0
#elif defined(__i386__) || defined(_M_IX86)
# define MBASE_PLATFORM_X86_32  1
# define MBASE_PLATFORM_X86_64  0
# define MBASE_PLATFORM_AARCH32 0
# define MBASE_PLATFORM_AARCH64 0
# define MBASE_PLATFORM_MIPS    0
#elif defined(__aarch64__) || defined(_M_ARM64)
# define MBASE_PLATFORM_X86_32  0
# define MBASE_PLATFORM_X86_64  0
# define MBASE_PLATFORM_AARCH32 0
# define MBASE_PLATFORM_AARCH64 1
# define MBASE_PLATFORM_MIPS    0
#elif defined(__arm__) || defined(_M_ARM)
# define MBASE_PLATFORM_X86_32  0
# define MBASE_PLATFORM_X86_64  0
# define MBASE_PLATFORM_AARCH32 1
# define MBASE_PLATFORM_AARCH64 0
# define MBASE_PLATFORM_MIPS    0
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__)
# define MBASE_PLATFORM_X86_32  0
# define MBASE_PLATFORM_X86_64  0
# define MBASE_PLATFORM_AARCH32 0
# define MBASE_PLATFORM_AARCH64 0
# define MBASE_PLATFORM_MIPS    1
#endif

#define MBASE_PLATFORM_X86 (MBASE_PLATFORM_X86_32 || MBASE_PLATFORM_X86_64)
#define MBASE_PLATFORM_AARCH (MBASE_PLATFORM_AARCH32 || MBASE_PLATFORM_AARCH64)

// ------------------------------------------------------
// Endianess detection macros
// Taken from: https://stackoverflow.com/a/79281141/4093267 by sleeptightAnsiC
//

#define MBASE_ENDIAN_LITTLE (1234)
#define MBASE_ENDIAN_BIG (4321)
#define MBASE_ENDIAN_PDP (3412)

#if defined(__BYTE_ORDER__)
  // if __BYTE_ORDER__ is defined, we can safely use it
  // and we can asume that __ORDER_LITTLE_ENDIAN__
  // __ORDER_BIG_ENDIAN__ and __ORDER_PDP_ENDIAN__ are also defined
  #define MBASE_ENDIAN_ORDER __BYTE_ORDER__
  // TCC mob-devel seems to be missing this one single define
  #if defined(__TINYC__) && !defined(__ORDER_PDP_ENDIAN__)
      #define __ORDER_PDP_ENDIAN__ ENDIAN_PDP
  #endif
  // check if our macros match with those predefined
  // this will also fail if some of them are missing
  #if (MBASE_ENDIAN_LITTLE != __ORDER_LITTLE_ENDIAN__) || (MBASE_ENDIAN_BIG != __ORDER_BIG_ENDIAN__) || (MBASE_ENDIAN_PDP != __ORDER_PDP_ENDIAN__)
      #error "Mismatch between ENDIAN_* and __ORDER_*_ENDIAN__ macros detected !"
  #endif
#elif defined(_MSC_VER)
  // MSVC does NOT predefine anything about Endianess
  // and this compiler only suports targets with Little Endian
  #define MBASE_ENDIAN_ORDER MBASE_ENDIAN_LITTLE
#else
  #error "Unable to determine MBASE_ENDIAN_ORDER !"
#endif

// May trigger someday for something like Honeywell 316 (aka ENDIAN_BIG_WORD)
// currently nothing I tested really suports this Endianess
#if (MBASE_ENDIAN_ORDER != MBASE_ENDIAN_LITTLE) && (MBASE_ENDIAN_ORDER != MBASE_ENDIAN_BIG) && (MBASE_ENDIAN_ORDER != MBASE_ENDIAN_PDP)
    #error "Unknown ENDIAN_ORDER !"
#endif

#pragma once

#if defined(__cplusplus)
# include <cstdint>
#else
# include <stdint.h>
#endif

/* 32-bit boolean with a fixed size regardless of ABI. Use `MBASE_TRUE` / `MBASE_FALSE`. */
typedef uint32_t MbBool32;

#define MBASE_TRUE  1u
#define MBASE_FALSE 0u

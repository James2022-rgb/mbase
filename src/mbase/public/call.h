#pragma once

#if !defined(MBASE_FORCE_INLINE)
# if defined(_MSC_VER)
#  define MBASE_FORCE_INLINE __forceinline
# else
#  define MBASE_FORCE_INLINE inline
# endif
#endif

#if !defined(MBASE_NO_THROW)
# if defined(_MSC_VER)
#  define MBASE_NO_THROW __declspec(nothrow)
# else
#  define MBASE_NO_THROW
# endif
#endif

#if !defined(MBASE_STDCALL)
# if defined(_WIN32)
#  define MBASE_STDCALL __stdcall
# else
#  define MBASE_STDCALL
# endif
#endif

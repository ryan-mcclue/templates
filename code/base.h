// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

// NOTE(Ryan): On windows, clang tries to emulate cl and so some defines are shared on them both
// ∴, do clang first

// ALSO DO ARCHITECTURE CHECK
#if defined(__GNUC__)
  #define COMPILER_GCC 1
  #if defined(_WIN32)
    #define OS_WINDOWS 1
  #elif defined(__gnu_linux__)
    #define OS_LINUX 1
  #endif
#elif defined(__clang__)
  #define COMPILER_CLANG 1
#else
  #error Compiler not supported
#endif

#if !defined(COMPILER_GCC)
  #define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
  #define COMPILER_CLANG 0
#endif

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

// Zero-out macros here
#if !defined(COMPILER_GCC)
  #define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
  #define COMPILER_CLANG 0
#endif

#if !defined(ENABLE_ASSERT)
  #define ENABLE_ASSERT 0
#endif

#define STATEMENT(s) do { s } while (0);

// TODO(ASSERT_BREAK handling)

#if defined(ENABLE_ASSERT)
  #define ASSERT(c) STATEMENT(if (!(c)) { ASSERT_BREAK(); })
#else
  #define ASSERT(c)
#endif

// IMPORTANT(Ryan): To handle macros not expanding
#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

#define GLUE_(a, b) a##b
#define GLUE(a, b) GLUE_(a, b)

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

// IMPORTANT(Ryan): Most compilers like pointer arithmetic here, however some have their own idioms
#define INT_FROM_PTR(p) ((unsigned long long)((char *)p - (char *)0))
#define PTR_FROM_INT(n) ((void *)((char *)0 + (n)))

// can't read or write from this. only as an abstraction of the member, e.g. can get sizeof member 
#define ABSTRACT_MEMBER(s, member) (((s *)0)->member)

#define OFFSET_OF_MEMBER(s, member) INT_FROM_PTR(&ABSTRACT_MEMBER(s, member))

#define MIN(x, y)
#define MAX(x, y)

// Allows to search for all of them easily
#define GLOBAL static
#define LOCAL static
#define FUNCTION static

// avoid confusing auto-indenter
#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

// use i+=1 in for loop syntax?
// use separate line for each for loop

// IMPORTANT(Ryan): all array macros assume static array

#include <string.h>
#define MEMORY_ZERO(p, n) memset((p), 0, (n))
#define MEMORY_ZERO_STRUCT(p) MEMORY_ZERO((p), sizeof(*(p)))

// IMPORTANT(Ryan): No tests, doesn't work!
// However, important to recognise can just have through-away tests
// i.e. no need for long living regression testing as whenever there is a bug
// in these, they will manifest themselves outwardly

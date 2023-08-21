// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include <stdint.h>
typedef int8_t i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef u8 b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;
typedef float f32;
typedef double f64;

#include <stdbool.h>

#define GLOBAL static
#define LOCAL_PERSIST static
#if !defined(TEST_BUILD)
 #define INTERNAL static
#else
  #define INTERNAL
#endif

#define GLOBAL_CONST PROGMEM

// NOTE(Ryan): For finishing superloop in test build  
#if defined(TEST_BUILD)
  GLOBAL u32 global_forever_counter = 1;
  #define FOREVER (global_forever_counter--) 
#else
  #define FOREVER 1
#endif

#if defined(SIMULATOR_BUILD) || defined(TEST_BUILD)
  #define WANT_MOCKS 1
#endif

#define I8_MIN  ((i8)0x80)
#define I16_MIN ((i16)0x8000)
#define I32_MIN ((i32)0x80000000)
#define I64_MIN ((i64)0x8000000000000000ll)
#define II8_MAX ((i8)0x7f) 
#define I16_MAX ((i16)0x7fff)
#define I32_MAX ((i32)0x7fffffff)
#define I64_MAX ((i64)0x7fffffffffffffffll)
#define U8_MAX  ((u8)0xff)
#define U16_MAX ((u16)0xffff)
#define U32_MAX ((u32)0xffffffff)
#define U64_MAX ((u64)0xffffffffffffffffllu)
// NOTE(Ryan): GCC will have the enum size accomodate the largest member
#define ENUM_U32_SIZE U32_MAX

// NOTE(Ryan): IEEE float 7 decimal places, double 15 decimal places
#define F32_MACHINE_EPSILON 1.1920929e-7f
#define F32_PI 3.1415926f
#define F32_TAU 6.2831853f
#define F32_HALF_PI 1.5707963f
#define F32_E 2.7182818f
#define F32_GOLD_BIG 1.6180339f
#define F32_GOLD_SMALL 0.6180339f
#define F64_MACHINE_EPSILON 2.220446049250313e-16
#define F64_PI 3.141592653589793
#define F64_TAU 6.283185307179586
#define F64_E 2.718281828459045
#define F64_GOLD_BIG 1.618033988749894
#define F64_GOLD_SMALL 0.618033988749894

// NOTE(Ryan): Taken from https://docs.oracle.com/cd/E19205-01/819-5265/bjbeh/index.html
INTERNAL f32
inf_f32(void)
{
  u32 temp = 0x7f800000;
  return *(f32 *)(&temp);
}

INTERNAL f32
neg_inf_f32(void)
{
  u32 temp = 0xff800000;
  return *(f32 *)(&temp);
}

INTERNAL f64
inf_f64(void)
{
  u64 temp = 0x7ff0000000000000;
  return *(f64 *)(&temp);
}

INTERNAL f64
neg_inf_f64(void)
{
  u64 temp = 0xfff0000000000000;
  return *(f64 *)(&temp);
}

INTERNAL f32  
abs_f32(f32 x)
{
  u32 temp = *(u32 *)(&x);
  temp &= 0x7fffffff;
  return *(f32 *)(&temp);
}

INTERNAL f64  
abs_f64(f64 x)
{
  u64 temp = *(u64 *)(&x);
  temp &= 0x7fffffffffffffff;
  return *(f64 *)(&temp);
}

typedef struct SourceLoc SourceLoc;
struct SourceLoc
{
  const char *file_name;
  const char *function_name;
  u64 line_number;
};
#define SOURCE_LOC { __FILE__, __func__, __LINE__ }
#define LITERAL_SOURCE_LOC LITERAL(SourceLoc) SOURCE_LOC 

#include <signal.h>

// NOTE(Ryan): Allow for simple runtime debugger attachment
GLOBAL b32 global_debugger_present;

#if defined(DEBUG_BUILD)
  #define BP() \
  do \
  { \
    if (global_debugger_present) \
    { \
      raise(SIGTRAP); \
    } \
  } while (0)
#else
  #define BP()
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
// NOTE(Ryan): Returns a u32 to account for situations when used in a ternary that requires operands of the same type 

/* NOTE(Ryan): Example:
 * attempt_msg: "Couldn’t parse config file: /etc/sample-config.properties"
 * reason_msg: "Failed to create an initial snapshot of the data; database user 'snapper' is lacking the required permissions 'SELECT', 'REPLICATION'"
 * resolution_msg: "Please see https://example.com/knowledge-base/snapshot-permissions/ for the complete set of necessary permissions"
 */
#define FATAL_ERROR(attempt_msg, reason_msg, resolution_msg) \
  __fatal_error(__FILE__, __func__, __LINE__, attempt_msg, reason_msg, resolution_msg)

NORETURN INTERNAL void
__fatal_error(const char *file_name, const char *func_name, int line_num,
              const char *attempt_msg, const char *reason_msg, const char *resolution_msg)
{ 
#if defined(RELEASE_BUILD)
  /* TODO(Ryan): Add stack trace to message
#include <execinfo.h>
  void *callstack_addr[128] = ZERO_STRUCT;
  int num_backtrace_frames = backtrace(callstack_addr, 128);

  // TODO(Ryan): addr2line could convert addresses to names
  char **backtrace_strs = backtrace_symbols(callstack_addr, num_backtrace_frames);

  u32 max_backtrace_str_len = 255;
  int message_size = sizeof(backtrace_strs) * max_backtrace_str_len;

  for (int i = 0; i < num_backtrace_frames; ++i) {
      printf("%s\n", strs[i]);
  }
  free(strs); 
  */
#endif

  syslog(LOG_EMERG, "(%s:%s():%d)\n %s\n%s\n%s", 
         file_name, func_name, line_num, 
         attempt_msg, reason_msg, resolution_msg);

  BP();

  exit(1); 
}

// TODO(Ryan): Use syslog_r() for threadsafe
/* NOTE(Ryan): Example:
 * what_msg: Initialised logging  
 * why_msg: To provide trace information to understand program flow in the event of a bug
 */
#define DBG(fmt, ...) \
  syslog(LOG_DEBUG, fmt, ##__VA_ARGS__);

#define TRACE(what_msg, why_msg) \
  do \
  { \
    printf("\x1B[91m"); fflush(stdout); \
    syslog(LOG_INFO, "%s():\n%s\n%s", __func__, what_msg, why_msg); \
    printf("\033[0m"); fflush(stdout); \
  } while (0)

#if defined(MAIN_DEBUG)
#define WARN(what_msg, why_msg) \
  do \
  { \
    BP(); \
    syslog(LOG_CRIT, "%s():\n%s\n%s", __func__, what_msg, why_msg); \
  } while (0)
#else
#define WARN(what_msg, why_msg) \
  syslog(LOG_WARNING, "%s():\n%s\n%s", __func__, what_msg, why_msg);
#endif

#if defined(MAIN_DEBUG)
  #define ASSERT(c) do { if (!(c)) { FATAL_ERROR(STRINGIFY(c), "Assertion error", ""); } } while (0)
  #define UNREACHABLE_CODE_PATH ASSERT(!"UNREACHABLE_CODE_PATH")
  #define UNREACHABLE_DEFAULT_CASE default: { UNREACHABLE_CODE_PATH }
#else
  #define ASSERT(c)
  #define UNREACHABLE_CODE_PATH UNREACHABLE() 
  #define UNREACHABLE_DEFAULT_CASE default: { UNREACHABLE() }
#endif

#define STATIC_ASSERT(cond, line) typedef u8 PASTE(line, __LINE__) [(cond)?1:-1]
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

// to get (count, array); use to pass array inline to function
#define ARRAY_EXPAND(type, ...) ARRAY_COUNT(((type[]){ __VA_ARGS__ })), (type[]){ __VA_ARGS__ }

// NOTE(Ryan): Avoid having to worry about pernicous macro expansion
#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

// IMPORTANT(Ryan): Cannot paste token delimiters like '.', '!' etc. so cannot do 'a. ## b'
#define PASTE_(a, b) a##b
#define PASTE(a, b) PASTE_(a, b)

#define PAD(n) char PASTE(pad, __LINE__)[n]

#define UNIQUE_NAME(name) PASTE(name, __LINE__)

#define DEFER_LOOP(begin, end) \
  for (int UNIQUE_NAME(var) = (begin, 0); \
       UNIQUE_NAME(var) == 0; \
       UNIQUE_NAME(var) += 1, end)
#define DEFER_LOOP_CHECKED(begin, end) \
  for (int UNIQUE_NAME(var) = 2 * !(begin); \
       (UNIQUE_NAME(var) == 2 ? ((end), 0) : !UNIQUE_NAME(var)); \
       UNIQUE_NAME(var) += 1, (end))
#define SCOPED(end) \
  for (int UNIQUE_NAME(var) = 0; \
       UNIQUE_NAME(var) == 0; \
       UNIQUE_NAME(var) += 1, end)

// TODO: MEM_SCOPED() which encases a scratch arena

// IMPORTANT(Ryan): Maybe have to do (void)sizeof(name) for C++?
#define IGNORED(name) (void)(name) 

#define SWAP(t, a, b) do { t PASTE(temp__, __LINE__) = a; a = b; b = PASTE(temp__, __LINE__); } while(0)

#define DEG_TO_RAD(v) ((PI_F32 / 180.0f) * (v))
#define RAD_TO_DEG(v) ((180.0f / PI_F32) * (v))

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

#define INT_FROM_PTR(p) ((uintptr_t)((char *)p - (char *)0))
#define PTR_FROM_INT(n) ((void *)((char *)0 + (n)))

#define ABSTRACT_MEMBER(s, member) (((s *)0)->member)
#define OFFSET_OF_MEMBER(s, member) INT_FROM_PTR(&ABSTRACT_MEMBER(s, member))
#define CAST_FROM_MEMBER(S,m,p) (S*)(((u8*)p) - OFFSET_OF_MEMBER(S,m))

#define SET_FLAG(field, flag) ((field) |= (flag))
#define REMOVE_FLAG(field, flag) ((field) &= ~(flag))
#define TOGGLE_FLAG(field, flag) ((field) ^= (flag))
#define HAS_FLAGS_ANY(field, flags) (!!((field) & (flags)))
#define HAS_FLAGS_ALL(field, flags) (((field) & (flags)) == (flags))

#define SIGN_OF(x) ((x > 0) - (x < 0))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(min,x,max) (((x)<(min))?(min):((max)<(x))?(max):(x))
#define CLAMP_TOP(a,b) MIN(a,b)
#define CLAMP_BOTTOM(a,b) MAX(a,b)

#define IS_POW2_ALIGNED(x, p) (((x) & ((p) - 1)) == 0)
#define IS_POW2(x) IS_POW2_ALIGNED(x, x) 
// -x == ~(x - 1)?
#define ALIGN_POW2_DOWN(x, p)       ((x) & -(p))
#define ALIGN_POW2_UP(x, p)       (-(-(x) & -(p)))
#define ALIGN_POW2_INCREASE(x, p)         (-(~(x) & -(p)))

#define THOUSAND(x) ((x)*1000LL)
#define MILLI_TO_SEC(x) ((x)*1000ULL)

#define MILLION(x)  ((x)*1000000LL)
#define MICRO_TO_SEC(x)  ((x)*1000000ULL)

#define BILLION(x)  ((x)*1000000000LL)
#define NANO_TO_SEC(x)  ((x)*1000000000ULL)

#define TRILLION(x) ((x)*1000000000000LL)
#define PICO_TO_SEC(x) ((x)*1000000000000ULL)

#define INC_SATURATE_U8(x) ((x) = ((x) >= (MAX_U8) ? (MAX_U8) : (x + 1)))
#define INC_SATURATE_U16(x) ((x) = ((x) >= (MAX_U16) ? (MAX_U16) : (x + 1)))
#define INC_SATURATE_U32(x) ((x) = ((x) >= (MAX_U32) ? (MAX_U32) : (x + 1)))

#include <stdio.h>
#define PRINT_INT(i) printf("%s = %d\n", STRINGIFY(i), (int)(i))

//#define EACH_TREEMAP_NODE(it, first) TreeMapNode *it = (first); (it != NULL); it = it->next
//#define ENUMERATE_TREEMAP_NODE(it, first) \
//  struct {TreeMapNode *it; u32 i} e = {(first), 0}; (e.it != NULL); e.it = e.it->next, e.i++;

// like a dequeue
// IMPORTANT(Ryan): Better than templates as no complicated type checking or generation of little functions
#define __DLL_PUSH_FRONT(first, last, node, next, prev) \
(\
  ((first) == NULL) ? \
  (\
    ((first) = (last) = (node)), \
    ((node)->next = (node)->prev = NULL) \
  )\
  : \
  (\
    ((node)->prev = NULL), \
    ((node)->next = (first)), \
    ((first)->prev = (node)), \
    ((first) = (node)) \
  )\
)
#define DLL_PUSH_FRONT(first, last, node) \
  __DLL_PUSH_FRONT(first, last, node, next, prev)

// TODO(Ryan): Have macros for DLL_PUSH_FRONT_TREE_CHILD(), DLL_PUSH_FRONT_HASH_ITEM()
// the basic one assumes self contained
  
#define __DLL_PUSH_BACK(first, last, node, next, prev) \
(\
  ((first) == NULL) ? \
  (\
    ((first) = (last) = (node)), \
    ((node)->next = (node)->prev = NULL) \
  )\
  : \
  (\
    ((node)->prev = (last)), \
    ((node)->next = NULL), \
    ((last)->next = (node)), \
    ((last) = (node)) \
  )\
)
#define DLL_PUSH_BACK(first, last, node) \
  __DLL_PUSH_BACK(first, last, node, next, prev)

#define __DLL_REMOVE(first, last, node, next, prev) \
(\
  ((node) == (first)) ? \
  (\
    ((first) == (last)) ? \
    (\
      ((first) = (last) = NULL) \
    )\
    : \
    (\
      ((first) = (first)->next), \
      ((first)->prev = NULL) \
    )\
  )\
  : \
  (\
    ((node) == (last)) ? \
    (\
      ((last) = (last)->prev), \
      ((last)->next = NULL) \
    )\
    : \
    (\
      ((node)->next->prev = (node)->prev), \
      ((node)->prev->next = (node)->next) \
    )\
  )\
)
#define DLL_REMOVE(first, last, node) \
  __DLL_REMOVE(first, last, node, next, prev) 

#define __SLL_QUEUE_PUSH(first, last, node, next) \
(\
  ((first) == NULL) ? \
   (\
    ((first) = (last) = (node)), \
    ((node)->next = NULL) \
   )\
  : \
  (\
    ((last)->next = (node)), \
    ((last) = (node)), \
    ((node)->next = NULL) \
  )\
)
#define SLL_QUEUE_PUSH(first, last, node) \
  __SLL_QUEUE_PUSH(first, last, node, next)

#define __SLL_QUEUE_POP(first, last, next) \
(\
  ((first) == (last)) ? \
    (\
     ((first) = (last) = NULL) \
    ) \
  : \
  (\
    ((first) = (first)->next) \
  )\
)
#define SLL_QUEUE_POP(first, last) \
  __SLL_QUEUE_POP(first, last, next)

#define __SLL_STACK_PUSH(first, node, next) \
(\
  ((node)->next = (first)), \
  ((first) = (node)) \
)
#define SLL_STACK_PUSH(first, node) \
  __SLL_STACK_PUSH(first, node, next)

#define __SLL_STACK_POP(first, next) \
(\
  ((first) != NULL) ? \
    (\
     ((first) = (first)->next) \
    )\
  : \
  (\
    (NULL) \
  )\
)
#define SLL_STACK_POP(first) \
  __SLL_STACK_POP(first, next)

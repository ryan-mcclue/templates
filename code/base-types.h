// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_TYPES_H)
#define BASE_TYPES_H

#include <stdint.h>
typedef int8_t i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;
// TODO(Ryan): It seems that on embedded, using say 'uint_fast8_t' can provide information to compiler to possibly
// use a register to hold the value for say array index incrementing
typedef uint8_t u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef float f32;
typedef double f64;

#define GLOBAL static
#define LOCAL static
#define INTERNAL static
#define GLOBAL_CONST PROGMEM

IGNORE_WARNING_USELESS_CAST_PUSH()

// TODO(Ryan): Seems use global variables for constants, macros for functions? we get added type safety and can get pointer to them
// we know that any compile time constants will save RAM. compiler optimisation should save codespace also
// IMPORTANT(Ryan): C99 diverged from C++ C, so as these defined in C99, perhaps not in C++
GLOBAL i8 MIN_S8 = (i8)0x80; 
GLOBAL i16 MIN_S16 = (i16)0x8000; 
GLOBAL i32 MIN_S32 = (i32)0x80000000; 
GLOBAL i64 MIN_S64 = (i64)0x8000000000000000ll; 

GLOBAL i8 MAX_S8 = (i8)0x7f; 
GLOBAL i16 MAX_S16 = (i16)0x7fff; 
GLOBAL i32 MAX_S32 = (i32)0x7fffffff; 
GLOBAL i64 MAX_S64 = (i64)0x7fffffffffffffffll; 

GLOBAL u8 MAX_U8 = (u8)0xff; 
GLOBAL u16 MAX_U16 = (u16)0xffff; 
GLOBAL u32 MAX_U32 = (u32)0xffffffff; 
GLOBAL u64 MAX_U64 = (u64)0xffffffffffffffffllu; 

// IMPORTANT(Ryan): GCC will have the enum size accomodate the largest member
#define ENUM_U32_SIZE 0xffffffff

IGNORE_WARNING_POP()

// NOTE(Ryan): IEEE float 7 decimal places, double 15 decimal places
GLOBAL f32 MACHINE_EPSILON_F32 = 1.1920929e-7f;
GLOBAL f32 PI_F32 = 3.1415926f;
GLOBAL f32 TAU_F32 = 6.2831853f;
GLOBAL f32 E_F32 = 2.7182818f;
GLOBAL f32 GOLD_BIG_F32 = 1.6180339f;
GLOBAL f32 GOLD_SMALL_F32 = 0.6180339f;

GLOBAL f64 MACHINE_EPSILON_F64 = 2.220446049250313e-16;
GLOBAL f64 PI_F64 =  3.141592653589793;
GLOBAL f64 TAU_F64 = 6.283185307179586;
GLOBAL f64 E_F64 =        2.718281828459045;
GLOBAL f64 GOLD_BIG_F64 = 1.618033988749894;
GLOBAL f64 GOLD_SMALL_F64 = 0.618033988749894;

GLOBAL u64 BITMASKS[65] = { 
    0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F,
    0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF,
    0xFFFF, 0x1FFFF, 0x3FFFF, 0x7FFFF, 0xFFFFF, 0x1FFFFF, 0x3FFFFF, 0x7FFFFF,
    0xFFFFFF, 0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF,
    0xFFFFFFFF, 0x1FFFFFFFF, 0x3FFFFFFFF, 0x7FFFFFFFF, 0xFFFFFFFFF, 0x1FFFFFFFFF, 0x3FFFFFFFFF, 0x7FFFFFFFFF,
    0xFFFFFFFFFF, 0x1FFFFFFFFFF, 0x3FFFFFFFFFF, 0x7FFFFFFFFFF, 0xFFFFFFFFFFF, 0x1FFFFFFFFFFF, 0x3FFFFFFFFFFF, 0x7FFFFFFFFFFF,
    0xFFFFFFFFFFFF, 0x1FFFFFFFFFFFF, 0x3FFFFFFFFFFFF, 0x7FFFFFFFFFFFF, 0xFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFF, 0x3FFFFFFFFFFFFF, 0x7FFFFFFFFFFFFF, 
    0xFFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 
    0xFFFFFFFFFFFFFFFF,
};

GLOBAL u32 BIT_1  = 1 << 0;
GLOBAL u32 BIT_2  = 1 << 1;
GLOBAL u32 BIT_3  = 1 << 2;
GLOBAL u32 BIT_4  = 1 << 3;
GLOBAL u32 BIT_5  = 1 << 4;
GLOBAL u32 BIT_6  = 1 << 5;
GLOBAL u32 BIT_7  = 1 << 6;
GLOBAL u32 BIT_8  = 1 << 7;
GLOBAL u32 BIT_9  = 1 << 8;
GLOBAL u32 BIT_10 = 1 << 9;
GLOBAL u32 BIT_11 = 1 << 10;
GLOBAL u32 BIT_12 = 1 << 11;
GLOBAL u32 BIT_13 = 1 << 12;
GLOBAL u32 BIT_14 = 1 << 13;
GLOBAL u32 BIT_15 = 1 << 14;
GLOBAL u32 BIT_16 = 1 << 15;
GLOBAL u32 BIT_17 = 1 << 16;
GLOBAL u32 BIT_18 = 1 << 17;
GLOBAL u32 BIT_19 = 1 << 18;
GLOBAL u32 BIT_20 = 1 << 19;
GLOBAL u32 BIT_21 = 1 << 20;
GLOBAL u32 BIT_22 = 1 << 21;
GLOBAL u32 BIT_23 = 1 << 22;
GLOBAL u32 BIT_24 = 1 << 23;
GLOBAL u32 BIT_25 = 1 << 24;
GLOBAL u32 BIT_26 = 1 << 25;
GLOBAL u32 BIT_27 = 1 << 26;
GLOBAL u32 BIT_28 = 1 << 27;
GLOBAL u32 BIT_29 = 1 << 28;
GLOBAL u32 BIT_30 = 1 << 29;
GLOBAL u32 BIT_31 = 1 << 30;
GLOBAL u32 BIT_32 = (u32)1 << 31;

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
  // just setting sign bit
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

typedef struct SourceLocation SourceLocation;
struct SourceLocation
{
  const char *file_name;
  const char *function_name;
  u64 line_number;
};
#define SOURCE_LOCATION { __FILE__, __func__, __LINE__ }
#define LITERAL_SOURCE_LOCATION LITERAL(SourceLocation) SOURCE_LOCATION 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
// NOTE(Ryan): Returns a u32 to account for situations when used in a ternary that requires operands of the same type 
INTERNAL u32 __fatal_error(const char *file_name, const char *func_name, int line_num, 
                            const char *message)
{ 
  fprintf(stderr, "FATAL ERROR TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, 
          func_name, line_num, message);
  #if !defined(MAIN_DEBUGGER)
    // IMPORTANT(Ryan): If using fork(), will not exit entire program
    exit(1); 
  #endif

  return 0;
}

INTERNAL u32 __fatal_error_errno(const char *file_name, const char *func_name, int line_num, 
                                  const char *message)
{ 
  const char *errno_msg = strerror(errno);
  fprintf(stderr, "FATAL ERROR ERRNO TRIGGERED! (%s:%s:%d)\n%s\n\"%s\"\n", file_name, 
          func_name, line_num, errno_msg, message);
  #if !defined(MAIN_DEBUGGER)
    // IMPORTANT(Ryan): If using fork(), will not exit entire program
    exit(1);
  #endif

  return 0;
}

INTERNAL void errno_log(void)
{
  const char *errno_msg = strerror(errno);
  return;
}

INTERNAL void __bp(void) {}

#define FATAL_ERROR(msg) __fatal_error(__FILE__, __func__, __LINE__, msg)
#define ERRNO_FATAL_ERROR(msg) __fatal_error_errno(__FILE__, __func__, __LINE__, msg)

#if defined(MAIN_DEBUG)
  // IMPORTANT(Ryan): assert() when never want to handle in production
  #define ASSERT(c) do { if (!(c)) { FATAL_ERROR(STRINGIFY(c)); } } while (0)
  #define ERRNO_ASSERT(c) do { if (!(c)) { ERRNO_FATAL_ERROR(STRINGIFY(c)); } } while (0)
  #define BP() __bp()
  #define UNREACHABLE_CODE_PATH ASSERT(!"UNREACHABLE_CODE_PATH")
  #define UNREACHABLE_DEFAULT_CASE default: { UNREACHABLE_CODE_PATH }
#else
  #define ASSERT(c)
  #define ERRNO_ASSERT(c)
  #define BP()
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

#define INT_FROM_PTR(p) ((unsigned long long)((char *)p - (char *)0))
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

#include <stdio.h>
#define PRINT_INT(i) printf("%s = %d\n", STRINGIFY(i), (int)(i))

// IMPORTANT(Ryan): Better than templates as no complicated type checking or generation of little functions
#define DLL_PUSH_FRONT(first, last, node) \
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
  

#define DLL_PUSH_BACK(first, last, node) \
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

#define DLL_REMOVE(first, last, node) \
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

#define SLL_QUEUE_PUSH(first, last, node) \
(\
  ((first) == NULL) ? \
   (\
    ((first) = (last) = (node)), \
    ((node)->next = NULL) \
   )\
  : \
  (\
    ((node)->next = (first)), \
    ((first) = (node)) \
  )\
)

#define SLL_QUEUE_POP(first, last) \
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

#define SLL_STACK_PUSH(first, node) \
(\
  ((node)->next = (first)), \
  ((first) = (node)) \
)

#define SLL_STACK_POP(first) \
(\
  ((first) != NULL) ? \
    (\
     ((first) = (first)->next) \
    )\
)

#endif

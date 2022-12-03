// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

// TODO(Ryan): sse_mathfun.h

// better to use #pragma region, #pragma endregion?
#pragma mark - M_CONTEXT_CRACKING

#if defined(__GNUC__)
  #define COMPILER_GCC 1
  #if defined(__gnu_linux__)
    #define OS_LINUX 1
    // TODO(Ryan): Synchronisation atomics
    // #define AtomicAdd64(ptr, v) _InterlockedExchangeAdd64((ptr), (v))
    // #define MEMORY_BARRIER()
  #else
    #error OS not supported
  #endif
  #if defined(__x86_64__)
    #define ARCH_X86_64 1
  #else
    #error Arch not supported
  #endif

  #if __GNUC__ < 10
    #error GCC 10+ required for builtin static analysis
  #endif

  #if __SANITIZE_ADDRESS__
    #define NO_ASAN __attribute__((__no_sanitize_address__))
  #endif

  #define CASE_FALLTHROUGH __attribute__((fallthrough))

  #define THREAD_LOCAL __thread

  #define NEVER_INLINE   __attribute__((noinline))
  #define USED_FUNC  __attribute__((used,noinline))
  #define ALWAYS_INLINE __attribute__((optimize("inline-functions"),always_inline))
  
  #define UNREACHABLE() __builtin_unreachable()

  #define PUSH_OPTIMISATION_MODE() \
    _Pragma("GCC push_options") \
    _Pragma("GCC optimize (\"O3\")")

  #define POP_OPTIMISATION_MODE() \
    _Pragma("GCC pop_options")

  #define IGNORE_WARNING_USELESS_CAST_PUSH() \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wuseless-cast\"")
  
  #define IGNORE_WARNING_POP() \
    _Pragma("GCC diagnostic pop")

  #define LIKELY(x)   __builtin_expect(!!(x), 1) 
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)

  // TODO(Ryan): Perhaps also do #define MACRO_BEGIN ({
  // for compiler specific expression statements
#else
  #error Compiler not supported
#endif

#define LCOV_EXCL_LINE
#define LCOV_EXCL_START
#define LCOV_EXCL_STOP

#if defined(__cplusplus)
  #if __cplusplus <= 199711L
    #define CPP_VERSION 98
  #elif __cplusplus <= 201103L
    #define CPP_VERSION 11
  #endif

  #define LANG_CPP 1
  // NOTE(Ryan): Avoid confusing auto-indenter
  // TODO: if on windows, require dll specifier
  #define EXPORT_BEGIN extern "C" {
  #define EXPORT_END }
  #define EXPORT extern "C"
  #define ZERO_STRUCT {}
  #define RESTRICT __restrict__
  #define LITERAL(t) t
#else
  #if __STDC_VERSION__ <= 199901L
    #define C_VERSION 99
  #elif __STDC_VERSION__ <= 201112L
    #define C_VERSION 11
  #endif

  #define LANG_C 1
  #define EXPORT_BEGIN
  #define EXPORT_END
  #define EXPORT
  #define RESTRICT restrict
  #define ZERO_STRUCT {0}
  #define LITERAL(t) (t)
#endif

#pragma mark - M_TYPES_AND_CONSTANTS

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

IGNORE_WARNING_USELESS_CAST_PUSH()

// TODO(Ryan): Seems use global variables for constants, macros for functions? we get added type safety
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

typedef struct SourceLocation SourceLocation;
struct SourceLocation
{
  const char *file_name;
  const char *function_name;
  u64 line_number;
};
#define SOURCE_LOCATION { __FILE__, __func__, __LINE__ }
#define LITERAL_SOURCE_LOCATION LITERAL(SourceLocation) SOURCE_LOCATION 

typedef enum AXIS
{
  // getting info out of vectors
  AXIS_X,
  AXIS_Y,
  AXIS_Z,
  AXIS_W,
  AXIS_COUNT,
  AXIS_MAKE_ENUM_32BIT = ENUM_U32_SIZE,
} AXIS;

typedef enum SIDE
{
  // left-right, top-bottom
  SIDE_MIN,
  SIDE_MAX,
} SIDE;

#pragma mark - M_BREAKPOINTS_AND_ASSERTS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
INTERNAL void __fatal_error(const char *file_name, const char *func_name, int line_num, 
                            const char *message)
{ 
  fprintf(stderr, "FATAL ERROR TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, 
          func_name, line_num, message);
  #if !defined(MAIN_DEBUGGER)
    // IMPORTANT(Ryan): If using fork(), will not exit entire program
    exit(1); 
  #endif
}

INTERNAL void __fatal_error_errno(const char *file_name, const char *func_name, int line_num, 
                                  const char *message)
{ 
  const char *errno_msg = strerror(errno);
  fprintf(stderr, "FATAL ERROR ERRNO TRIGGERED! (%s:%s:%d)\n%s\n\"%s\"\n", file_name, 
          func_name, line_num, errno_msg, message);
  #if !defined(MAIN_DEBUGGER)
    // IMPORTANT(Ryan): If using fork(), will not exit entire program
    exit(1);
  #endif
}

INTERNAL void __bp(void) {}

#define FATAL_ERROR(msg) __fatal_error(__FILE__, __func__, __LINE__, msg)
#define ERRNO_FATAL_ERROR(msg) __fatal_error_errno(__FILE__, __func__, __LINE__, msg)

#if defined(MAIN_DEBUG)
  #define ASSERT(c) do { (if !(c)) { FATAL_ERROR(STRINGIFY(PASTE(ASSERTION, c))); } } while (0)
  #define ERRNO_ASSERT(c) do { if (!(c)) { ERRNO_FATAL_ERROR(STRINGIFY(PASTE(ASSERTION, c))); } } while (0)
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

#pragma mark - M_UTILITIES

// to get (count, array); use to pass array inline to function
#define ARRAY_EXPAND(type, ...) ARRAY_COUNT(((type[]){ __VA_ARGS__ })), (type[]){ __VA_ARGS__ }

// NOTE(Ryan): Avoid having to worry about pernicous macro expansion
#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

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
#define CAST_FROM_MEMBER(S,m,p) (S*)(((U8*)p) - OFFSET_OF_MEMBER(S,m))

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

#if 0
#if defined(COMPILER_GCC) && defined(ARCH_X86_64)
  #include <x86intrin.h>

  INTERNAL ALWAYS_INLINE u32 
  count_bits_set_u32(u32 val)
  {
    return __builtin_popcount(val);
  }

  // NOTE(Ryan): This is from most significant bits
  INTERNAL ALWAYS_INLINE u32 
  count_leading_zeroes_u32(u32 val)
  {
    return __builtin_clz(val);
  }

  INTERNAL ALWAYS_INLINE u32 
  count_trailing_zeroes_u32(u32 val)
  {
    return __builtin_ctz(val);
  }

  INTERNAL ALWAYS_INLINE u32 
  get_parity_u32(u32 val)
  {
    return __builtin_parity(val);
  }

  INTERNAL ALWAYS_INLINE u16 
  endianness_swap_u16(u16 val)
  {
    return __builtin_bswap16(val);
  }

  INTERNAL ALWAYS_INLINE u32 
  endianness_swap_u32(u32 val)
  {
    return __builtin_bswap32(val);
  }

  INTERNAL ALWAYS_INLINE u64 
  endianness_swap_u64(u64 val)
  {
    return __builtin_bswap64(val);
  }

  // TODO(Ryan): Enable optimisation flags for particular math routines we have no need to step through
  // math.h functions
  r64 __builtin_powi(r64, u32)
  r32 __builtin_powif(r32, u32)

#endif

#endif


// malloc designed for arbitrary sizes and lifetimes
// this can lead to rats nests of lifetimes and computational issues freeing small nodes
// by enforcing generalities (i.e. very little constraints on interface), 
// allow for many possible solutions, which leads to complexity!
// also, doesn't allow for performance, and creates many wild usage patterns
// (developers stuck in managing program inertia, e.g. bug-fixing)
//
// think about hard to write first, then hard to maintain
//
// IMPORTANT: interface and implementation are coupled to a large extent. 
// can't keep the same interface with major implementation changes
// go against this downfall of king abstractions
//
// however, these are avoidable without a new language!
//
// can't use stack allocator, think have to use extermemly generic heap allocator
// (alloca?)
//
// MEMORY LEAK MISNOMERS
// when call malloc, request OS to map pages into virtual address space and record it in its page table
// whenever a process crashes, i.e. hits a hardware exception like a page fault
// OS continues running and will release the physical pages
// in essence, OS is ultimate garbage collector
//
// RAII is one option that inserts code that mallocs on object creation and frees when object goes out of scope
// i.e. constructor/destructors
// Garbage collection interrupts your code periodically to determine what object lifetimes are dead and to free them
//
// Unfortunately prevailing thought in computing is that problems are too complex, so we need tooling to fix them
// This in-fact compounds complexity over time as creates more dependencies
// Instead, we must change the interface
//
// The stack allocation is simple, and often not what people criticise when talking about memory management
// However, not feasible for certain lifetimes and sizes
//
// An arena allocator is many stacks, but can solve lifetime issues
// In almost every case, a large number of allocations can be bucketed into same arena (performance)
// Also, freed ourselves of having to deallocate.
// Can also track lifetimes easily
// An allocation is bound to an arena handle
//
// functions ask the user where to allocate
// function(Arena *arena)

// TODO(Ryan): malloc() is 16-byte aligned for possible SIMD (use of xmm registers)
// understand this alignment?

// use i+=1 in for loop syntax?
// use separate line for each for loop

#pragma mark - M_MEMORY

// TODO(Ryan): Seems that xxhash is best?

#include <string.h>

// TODO(Ryan): For large copies, perhaps builtin rep instruction better performance?
#define MEMORY_ZERO(p, n) memset((p), 0, (n))
#define MEMORY_ZERO_STRUCT(p) MEMORY_ZERO((p), sizeof(*(p)))
#define MEMORY_ZERO_ARRAY(a) MEMORY_ZERO((a), sizeof(a[0]))

#define MEMORY_COPY(d, s, n) memmove((d), (s), (n))
#define MEMORY_COPY_STRUCT(d, s, n) MEMORY_COPY((d), (s), sizeof(*(s)))
#define MEMORY_COPY_ARRAY(d, s, n) MEMORY_COPY((d), (s), sizeof((s)))

#define MEMORY_MATCH(a, b, n) (memcmp((a), (b), (n)) == 0)

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) (((u64)x) << 30)
#define TB(x) (((u64)x) << 40)

#define THOUSAND(x) ((x)*1000LL)
#define MILLI_TO_SEC(x) ((x)*1000ULL)

#define MILLION(x)  ((x)*1000000LL)
#define MICRO_TO_SEC(x)  ((x)*1000000ULL)

#define BILLION(x)  ((x)*1000000000LL)
#define NANO_TO_SEC(x)  ((x)*1000000000ULL)

#define TRILLION(x) ((x)*1000000000000LL)
#define PICO_TO_SEC(x) ((x)*1000000000000ULL)

// IMPORTANT(Ryan): No tests, doesn't work!
// However, important to recognise can just have through-away tests
// i.e. no need for long living regression testing as whenever there is a bug
// in these, they will manifest themselves outwardly

#include <stdio.h>
#define PRINT_INT(i) printf("%s = %d\n", STRINGIFY(i), (int)(i))

// technically procedure and INTERNAL pointers not same size (some compilers throw warnings)
// so use this when declaring a INTERNAL pointer to ensure has same size
typedef void VoidFunc(void);

// type conversion using unions is undefined behaviour (however seems to work on all major arches; however custom compilers on embedded)


INTERNAL u64
round_to_nearest(u64 val, u64 near)
{
  u64 result = val;

  result += near - 1;
  result -= result % near;
  
  return result;
}

#if 0
// IMPORTANT(Ryan): Due to ASLR, could use pointer address where appropriate 
INTERNAL void
linux_get_entropy(void *buffer, size_t length)
{
  ERRNO_ASSERT(getentropy(buffer, length) != -1);
}

INTERNAL u32 
rand_u32(random_series *r)
{
    uint32_t x = r->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->state = x;
    return r->state;
}

// returns random [0, 1) float
static inline float random_unilateral (random_series_t *r)
{
    // NOTE: Stolen from rnd.h, courtesy of Jonatan Hedborg
    uint32_t exponent = 127;
    uint32_t mantissa = random_uint32(r) >> 9;
    uint32_t bits = (exponent << 23) | mantissa;
    float result = *(float *)&bits - 1.0f;
    return result;
}

// returns random [-1, 1) float
static inline float random_bilateral(random_series_t *r)
{
    return -1.0f + 2.0f*random_unilateral(r);
}

// returns random number in range [0, range)
static inline uint32_t random_choice(random_series_t *r, uint32_t range)
{
    uint32_t result = random_uint32(r) % range;
    return result;
}

// returns random number in range [1, sides]
static inline uint32_t dice_roll(random_series_t *r, uint32_t sides)
{
    uint32_t result = 1 + random_choice(r, sides);
    return result;
}

// returns random number in range [min, max]
static inline int32_t random_range_i32(random_series_t *r, int32_t min, int32_t max)
{
    if (max < min)
        max = min;

    int32_t result = min + (int32_t)random_uint32(r) % (max - min + 1);
    return result;
}

// returns random float in range [min, max)
static inline float random_range_f32(random_series_t *r, float min, float max)
{
    float range = random_unilateral(r);
    float result = min + range*(max - min);
    return result;
}

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

// put in math.h trig, sqrt, log INTERNALs for now
#include <math.h>
INTERNAL f32
sqrt_f32(f32 x)
{
  return sqrtf(x);
}

INTERNAL f32
sin_f32(f32 x)
{
  return sinf(x);
}
28:#define STBTT_pow(x, y)  __builtin_powf(x, y)
29:#define STBTT_fmod(x, y) __builtin_fmodf(x, y)
31:#define STBTT_acos(x)    __builtin_acosf(x)

INTERNAL f32
cos_f32(f32 x)
{
  return cosf(x);
}

INTERNAL f32
tan_f32(f32 x)
{
  return tanf(x);
}

INTERNAL f32
ln_f32(f32 x)
{
  return logf(x);
}

INTERNAL f64
sqrt_f64(f64 x)
{
  return sqrt(x);
}

TODO(Ryan): Investigate retro-fps math sse routines
INTERNAL f32 sqrt_ss(f32 x)
{
  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set1_ps(x)));
}

static inline float rsqrt_ss(float x)
{
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set1_ps(x)));
}

INTERNAL f64
sin_f64(f64 x)
{
  return sin(x);
}

INTERNAL f64
cos_f64(f64 x)
{
  return cos(x);
}

INTERNAL f64
tan_f64(f64 x)
{
  return tan(x);
}

INTERNAL f64
ln_f64(f64 x)
{
  return log(x);
}

INTERNAL f32
lerp(f32 a, f32 b, f32 t)
{
  f32 result = 0.f;
  result = ((b - a) * t) + a; 
  return result;
}

// unlerp(10.f, lerp(10.f, .5f, 100.f), 100.f);
INTERNAL f32
unlerp(f32 a, f32 b, f32 t)
{
  f32 result = 0.f;
  if (a != b)
  {
    result = (t - a)/(b - a);
  }
  return result;
}

// TODO(Ryan): Perhaps include round_to, ceil functions etc.

#define M(mat, row, col) (mat)->e[(mat)->n*(col) + (row)]

// IMPORTANT(Ryan): although typing out tedious, better than codegen complexity
// i.e. prefer tedious over complexity
union V2S32
{
  struct
  {
    s32 x, y;
  };
  s32 v[2];
};

union V2F32
{
  struct
  {
    f32 x, y;
  };
  f32 v[2];
};

union V3F32
{
  struct
  {
    f32 x, y, z;
  };
  f32 v[3];
};

union V4F32
{
  struct
  {
    f32 x, y, z, w;
  };
  f32 v[4];
};

// IMPORTANT: an axis-aligned rect
union I1F32
{
  struct
  {
    f32 min, max;
  };
  f32 v[2];
};

// TODO(Ryan): useful for memory ranges?
union I1U64
{
  struct
  {
    u64 min, max;
  };
  struct
  {
    u64 first, op1;
  };
  u64 v[2];
};

union I1U64
{
  struct
  {
    u64 min, max;
  };
  u64 v[2];
};

union I2F32
{
  struct
  {
    V2F32 min, max;
  };
  struct
  {
    V2F32 p0, p1;
  };
  struct
  {
    f32 x0, y0, x1, y1;
  };
  V2F32 p[2];
  F32 v[4];
};

INTERNAL V2S32
v2s32(s32 x, s32 y)
{
  V2S32 result = {x, y};
  return result;
}

INTERNAL V2S32
v2s32_vec(V2F32 x, V2F32 y);

INTERNAL V2S32
v2s32_range(I1F32 x, I1F32 y);

INTERNAL I2S32
i2s32(s32 x0, s32 y0, s32 x1, s32 y1)
{
  I2S32 result = {};
  if (x1 < x0)
  {
    result.x0 = x1;
    result.x1 = x0;
  }
}


// IMPORTANT(Ryan): Only works on multiples of 2
#define VECTOR_SIZE(amount, type) \
  vector_size(amount * sizeof(type))

typedef r32 v2 __attribute__((VECTOR_SIZE(2, r32)));
typedef union V2
{
  v2 vec;
  struct
  {
    r32 x, y;
  };
  r32 e[2];
} V2;

V2S32 operator+(const V2S32 &a, const V2S32 &b);
V2S32 operator-(const V2S32 &a, const V2S32 &b);
V2S32 operator*(const V2S32 &a, s32 b);

INTERNAL f32
vec_hadamard(V2F32 a, V2F32 b);
INTERNAL f32
vec_dot(V2F32 a, V2F32 b);
INTERNAL b32
interval_overlaps(I1F32 a, I1F32 b)
{
  b32 result = false;

  result = (b.min < b.max && a.min < b.max);

  return result;
}
INTERNAL b32
interval_contains(I1F32 r, F32 x)
{
  b32 result = false;

  result = (r.min <= x && x < r.max);

  return result;
}
INTERNAL f32
interval_dim(I1F32 r)
{
  f32 result = 0.f;

  result = (r.max - r.min);

  return result;
}
INTERNAL f32
interval_centre(I1F32 r)
{
  (r.min + r.max)*0.5f;
}
INTERNAL I1F32
interval_axis(I2F32 r, AXIS axis)
{
  I1F32 result = {};
  result.p[0] = r.p[0].v[axis];
  result.p[1] = r.p[1].v[axis];
  return result;
}



// IMPORTANT(Ryan): This maps macros to enum constants
INTERNAL OS
os_from_context(void)
{
  OS result = OS_NULL;

// IMPORTANT(Ryan): As we 0 define macros, can use like this
#if OS_WINDOWS
  result = OS_WINDOWS;
#elif OS_LINUX
  result = OS_LINUX;
#elif OS_MAC
  result = OS_MAC;
#endif

  return result;
}

// IMPORTANT(Ryan): This is a string table
// If C99 designated initialisers, perhaps use arr[] = { [OS_WINDOWS] = "Windows" };
INTERNAL char*
string_from_os(OS os)
{
  char *result = "(null)";

  switch (os)
  {
    case OS_WINDOWS:
    {
      result = "Windows";
    } break;
  }

  return result;
}
INTERNAL char*
string_from_arch(ARCH arch)
{

}
INTERNAL char*
string_from_month(MONTH month)
{

}
INTERNAL char*
string_from_day_of_week(DAY_OF_WEEK day_of_week)
{

}


// IMPORTANT(Ryan): Rarely encounter places where actually want a generic type
// However, linked lists is one
// Macro approach for type generics can be pretty buggy, however in this case good
// Better than templates as no complicated type checking or generation of little functions

// issues of pointer serialisation (writing to disk)

// LINKED LISTS (many criticisms assume implementation details not inherent in linked list definition):
// O(1) access not always important to choose arrays ➞ If know only size 10, then don't care
// Serially-dependent, i.e. have to stall CPU waiting for next node ➞ nodes themselves could store large amounts of data thereby making compute-bound rather then memory bound
// non-locality ➞ arena allocation
//
// benefits of non-continuity are we can push multiple data types, i.e. multiple linked list types


// IMPORTANT(Ryan): Function pointers define a sort of plugin system

#define MEM_DEFAULT_RESERVE_SIZE GB(1)
#define MEM_COMMIT_BLOCK_SIZE MB(64)

// IMPORTANT(Ryan): No explicit pointer to memory, just end of struct so to speak 
struct MemArena
{
  u64 cap;
  u64 pos;
  u64 commit_pos;
};

#define COMMIT_SIZE KB(4)
#define DECOMMIT_SIZE KB(64)

STATIC_ASSERT(sizeof(MemArena) <= COMMIT_SIZE, arena_size_check);
 
INTERNAL MemArena *
mem_arena_allocate(u64 cap)
{
  // reserve to GB
  // void *ptr = VirtualAlloc(0, gb_snapped_size, MEM_RESERVE, PAGE_NOACCESS);
  MemArena *result = mem_reserve(cap);
  mem_commit(result, COMMIT_SIZE);


  // commit to nearest page size
  // TODO(Ryan): Investigate obtaining POSIX information with sysconf() and related functions
  // VirtualAlloc(ptr, page_snapped_size, MEM_COMMIT, PAGE_READWRITE);


  //VirtualFree(ptr, size, MEM_DECOMMIT);
  // release
  // VirtualFree(ptr, 0, MEM_RELEASE);

  return result;
}


INTERNAL void*
mem_arena_push(MemArena *arena, u64 size)
{
  // TODO(Ryan): Is the intention of the system to remove all error codes
  // why have a commit stage, i.e. why not just commit straight away?
  // so memory available for other processes?
  //
  // I think just to determine maximum memory usage
  // we are assuming never get memory issue only in development
  
  // video 4: 8:22
}


INTERNAL void
mem_arena_align(MemArena *arena, u64 pow2_align)
{
  u64 p = arena->pos;
  u64 p_aligned = ALIGN_UP_POW2(p, pow2_align);
  u64 z = p_aligned - p;
  if (z > 0)
  {
    mem_arena_push(arena, z);
  }
}

INTERNAL void
mem_arena_align_zero(MemArena *arena, u64 pow2_align)
{
  u64 p = arena->pos;
  u64 p_aligned = ALIGN_UP_POW2(p, pow2_align);
  u64 z = p_aligned - p;
  if (z > 0)
  {
    mem_arena_push_zero(arena, z);
  }
}

INTERNAL void
mem_arena_zero(MemArena *arena, u64 size)
{
  void *result = mem_arena_push(arena, size);
  MEMORY_ZERO(result, size);
  return result;
}

// IMPORTANT(Ryan): Using memory arenas changes the mentality of memory usage
// i.e. can only free things in reverse order
// just learn to program this way, more benefits than negatives

// IMPORTANT(Ryan): assert() when never want to handle in production

// IMPORTANT(Ryan): base layer is to make easy things easy. not for most optimal solution 
// strings much better with length than null terminator

// IMPORTANT(Ryan): Here we treat strings as immutable.
// If we need to modify in place, work out on a case-by-case basis
struct String8
{
  u8 *str;
  u64 size;
};

// String8 match = s8_consume_first_by_delim(&src, delim)
// For say split, we are fine with just lazy loading
// This means we save time not having to do expensive allocations up front

// This is something that has been allocated and we can add to
struct String8Buf
{

};

struct String8Node
{
  String8Node *next;
  String8 string;
};

// join strings, divide into strings, etc.
struct String8List
{
  String8Node *first;
  String8Node *last;
  u64 node_count;
  u64 total_size; 
};

#define STR8_LIT(s) str8((u8 *)(s), sizeof(s) - 1)
#define STR8_EXPAND(s) (int)((s).size), ((s).str) 

INTERNAL String8
str8_prefix(String8 str, u64 size)
{
  u64 size_clamped = MIN(size, str.size);
  String8 result = {str.str, size_clamped};
  return result;
}

INTERNAL String8 
str8_chop(String8 str, u64 amount) 
{ u64 amount_clamped = clampTop(amount, str.size); 
  U64 remaining_size = str.size - amount_clamped;
  String result = {str. str, remaining_size};
  return(result);
}
INTERNAL String8 
str8_postfix(String8 str, u64 size) 
{ 
  u64 size_clamped = clampTop(size, str.size); 
  u64 skip_to = str.size - size_clamped; 
  String8 result = {str.str + skip_to, size_clamped}; 
  return(result);
}
INTERNAL String8 
str8_skip(String8 str, u64 amount)
{ 
  u64 amount_clamped = clampTop(amount, str.size);
  U64 remaining_size = str.size - amount clamped;
  String8 result = {str.str + amount_clamped, remaining_size};
  return result;
}

// stores memory say on stack without arenas
function void 
str8_list_push_explicit(String8List *list, String8 string, String8Node *node_memory)
{ 
  node_memory->string = string;
  SLLQueuepush(list->first, list->last, node_memory);
  list->node_count += 1;
  list->total_size += string. size;
}
function void 
str8_list_push(MemArena *arena, String8List *list, String8 string) 
{ 
  String8Node *node = MEM_PUSH_ARRAY(arena, String8Node, 1);
  str8_list_push_explicit(list, string, node);
}

// TODO: what exactly is thread context?

// vsnprintf() just for printing human readable strings (not performant!)

// IMPORTANT: just pass arenas in to anything that requires an allocator? how about files?

// utf-16 windows
// utf-8 linux
// utf-32 easiest as everything is 32bits

INTERNAL u32
round_r32_to_u32(r32 real32)
{
  u32 result = 0;

  result = (u32)roundf(real32);

  return result;
}

INTERNAL s32
round_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = (s32)roundf(real32);

  return result;
}

INTERNAL u32
floor_r32_to_u32(r32 real32)
{
  u32 result = 0;

  result = (u32)floorf(real32);

  return result;
}

INTERNAL s32
floor_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = (s32)floorf(real32);

  return result;
}

INTERNAL u32
ceil_r32_to_u32(r32 real32)
{
  u32 result = 0;

  result = ceilf(real32);

  return result;
}

INTERNAL s32
ceil_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = ceilf(real32);

  return result;
}

INTERNAL r32
square(r32 x)
{
  r32 result = 0.0f;

  result = x * x;

  return result;
}

INTERNAL r32
square_root(r32 val)
{
    r32 result = 0.0f;

    result = sqrt(val);

    return result;
}

INTERNAL r32
lerp(r32 start, r32 end, r32 p)
{
  r32 result = 0.0f;

  result = ((end - start) * p) + start;

  return result;
}

#endif

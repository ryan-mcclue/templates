// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

// NOTE(Ryan): On windows, clang tries to emulate cl and so some defines are shared on them both
// ∴, do clang first

// CONTEXT-CRACKING, i.e. determine the context of execution
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
// could use these as unique ids

// can't read or write from this. only as an abstraction of the member, e.g. can get sizeof member 
#define ABSTRACT_MEMBER(s, member) (((s *)0)->member)

#define OFFSET_OF_MEMBER(s, member) INT_FROM_PTR(&ABSTRACT_MEMBER(s, member))

// TODO: CAST_FROM_MEMBER()? (from video 2 comments)

#define MIN(x, y)
#define MAX(x, y)

// clamptop() is min()
#define ALIGN_UP_POW2(x, p) ((x) + (p) - 1) & ~((p) - 1)
#define ALIGN_DOWN_POW2(x, p) ((x) & ~((p) - 1)) 

// Allows to search for all of them easily
#define GLOBAL static
#define LOCAL static
#define INTERNAL static

// avoid confusing auto-indenter
#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

// use i+=1 in for loop syntax?
// use separate line for each for loop

// IMPORTANT(Ryan): all array macros assume static array
//
#include <string.h>
#define MEMORY_ZERO(p, n) memset((p), 0, (n))
#define MEMORY_ZERO_STRUCT(p) MEMORY_ZERO((p), sizeof(*(p)))
#define MEMORY_ZERO_ARRAY(a) MEMORY_ZERO((a), sizeof(a[0]))

#define MEMORY_COPY(d, s, n) memmove((d), (s), (n))
#define MEMORY_COPY_STRUCT(d, s, n) MEMORY_COPY((d), (s), sizeof(*(s)))
#define MEMORY_COPY_ARRAY(d, s, n) MEMORY_COPY((d), (s), sizeof((s)))

#define MEMORY_MATCH(a, b, n) (memcmp((a), (b), (n)) == 0)

#define KB(x) ((x) * 1024LL) // alternatively x << 10, x << 20, etc.
#define MB(x) (KB(x) * 1024LL)
#define GB(x) (GB(x) * 1024LL)
#define TB(x) (TB(x) * 1024LL)

// IMPORTANT(Ryan): No tests, doesn't work!
// However, important to recognise can just have through-away tests
// i.e. no need for long living regression testing as whenever there is a bug
// in these, they will manifest themselves outwardly

#include <stdio.h>
#define PRINT_INT(i) printf("%s = %d\n", STRINGIFY(i), (int)(i))

#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float f32;
typedef double f64;

// IMPORTANT(Ryan): C99 diverged from C++ C, so as these defined in C99, perhaps not in C++
GLOBAL s8 MIN_S8 = (s8)0x80; 
GLOBAL s16 MIN_S16 = (s16)0x8000; 
GLOBAL s32 MIN_S32 = (s32)0x80000000; 
GLOBAL s64 MIN_S64 = (s64)0x8000000000000000ll; 

GLOBAL s8 MAX_S8 = (s8)0x7f; 
GLOBAL s16 MAX_S16 = (s16)0x7fff; 
GLOBAL s32 MAX_S32 = (s32)0x7fffffff; 
GLOBAL s64 MAX_S64 = (s64)0x7fffffffffffffffll; 

GLOBAL u8 MAX_U8 = (u8)0xff; 
GLOBAL u16 MAX_U16 = (u16)0xffff; 
GLOBAL u32 MAX_U32 = (u32)0xffffffff; 
GLOBAL u64 MAX_U64 = (u64)0xffffffffffffffffllu; 

// IMPORTANT: float 7 decimal places, double 15
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

// symbolic constants
enum AXIS
{
  // getting info out of vectors
  AXIS_X,
  AXIS_Y,
  AXIS_Z,
  AXIS_W,
};
enum SIDE
{
  // left-right, top-bottom
  SIDE_MIN,
  SIDE_MAX,
};
enum OS
{
  OS_NULL,
  OS_WINDOWS,
  OS_LINUX,
  OS_MAC,
  OS_COUNT
};
enum ARCH
{
  ARCH_NULL,
  ARCH_X64,
  ARCH_X32,
  ARCH_ARM,
  ARCH_ARM64,
  ARCH_COUNT,
};
// dealing with file information
enum MONTH
{
  MONTH_JAN,
  MONTH_FEB,
  MONTH_MAR,
  MONTH_APR,
  MONTH_MAY,
  MONTH_JUN,
  MONTH_JUL,
  MONTH_AUG,
  MONTH_SEP,
  MONTH_OCT,
  MONTH_NOV,
  MONTH_DEC,
};
enum DAY_OF_WEEK
{
  DAY_OF_WEEK_SUN,
  DAY_OF_WEEK_MON,
  DAY_OF_WEEK_TUE,
  DAY_OF_WEEK_WED,
  DAY_OF_WEEK_THU,
  DAY_OF_WEEK_FRI,
  DAY_OF_WEEK_SAT,
};

// IMPORTANT(Ryan): Only have globals for constants...

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

// technically procedure and INTERNAL pointers not same size (some compilers throw warnings)
// so use this when declaring a INTERNAL pointer to ensure has same size
typedef void VoidFunc(void);

// type conversion using unions is undefined behaviour (however seems to work on all major arches; however custom compilers on embedded)

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

// TODO(Ryan): Perhaps include round_to() functions


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

// IMPORTANT(Ryan): Rarely encounter places where actually want a generic type
// However, linked lists is one
// Macro approach for type generics can be pretty buggy, however in this case good
// Better than templates as no complicated type checking or generation of little functions

struct Node
{
  Node *next;
  Node *prev;

  int x;
};
Node nodes[10] = {};
Node *first = NULL, *last = NULL;
DLL_PUSH_BACK(first, last, &nodes[i]);
for (Node *node = first; node != NULL; node = node->next)
{

}

// doubly linked list
// macro particularly cryptic if trying to keep as one expression (use ternary, commas and expression assignment to link)
#define DLL_PUSH_BACK(f, l, n) 
// video [q2] 9:13  

// factor in bug from comments:
//#define DLL_REMOVE_NP(f,l,n,next,prev) ((f)==(l)&&(f)==(n)?\
                                        ((f)=(l)=0):\
                                        ((f)==(n)?\
                                        ((f)=(f)->next,(f)->prev=0):\
                                        ((l)==(n)?\
                                        ((l)=(l)->prev,(l)->next=0):\
                                        ((n)->prev->next=(n)->next,\
                                        (n)->next->prev=(n)->prev))))

// singly linked list queue
// video [q2] 10:54  

// singly linked list stack
// video [q2] 11:44  

// IMPORTANT(Ryan): The function pointers define a sort of plugin system
typedef void mem_reserve_func(void *ctx, u64 size);
typedef void mem_commit_func(void *ctx, void *ptr, u64 size);
typedef void mem_decommit_func(void *ctx, void *ptr, u64 size);
typedef void mem_release_func(void *ctx, void *ptr, u64 size);

struct BaseMemory
{
  // IMPORTANT(Ryan): Cleaner function pointer typedef syntax
  mem_reserve_func *reserve; 
  mem_commit_func *commit; 
  mem_decommit_func *decommit;
  mem_release_func *release; 
  void *ctx;
};

// IMPORTANT(Ryan): Here we implement a malloc memory plugin
INTERNAL void*
mem_malloc_reserve(void *ctx, u64 size)
{
  return malloc(size);
}
INTERNAL void*
mem_malloc_commit(void *ctx, void *ptr, u64 size) {}
INTERNAL void*
mem_malloc_decommit(void *ctx, void *ptr, u64 size) {}
INTERNAL void*
mem_malloc_release(void *ctx, void *ptr, u64 size)
{
  free(ptr);
}

INTERNAL *BaseMemory
mem_malloc_base_memory(void)
{
  LOCAL BaseMemory memory = {};

  memory.reserve = mem_malloc_reserve;
  memory.commit = mem_malloc_commit;
  memory.decommit = mem_malloc_decommit;
  memory.release = mem_malloc_release;

  return &memory;
}

#define MEM_DEFAULT_RESERVE_SIZE GB(1)
#define MEM_COMMIT_BLOCK_SIZE MB(64)

// IMPORTANT(Ryan): Now over to arena
struct MemArena
{
  BaseMemory *base;
  u8 *memory;
  u64 cap;
  u64 pos;
  u64 commit_pos;
};

INTERNAL MemArena
mem_make_arena_reserve(BaseMemory *base, u64 reserve_size)
{
  MemArena arena = {};

  arena.base = base;
  arena.memory = base->reserve(base->ctx, reserve_size);
  arena.cap = reserve_size;

  return arena;
}

INTERNAL MemArena
mem_make_arena(BaseMemory *base)
{
  return mem_make_arena_reserve(base, MEM_DEFAULT_RESERVE_SIZE); 
}

INTERNAL void
mem_arena_release(MemArena *arena)
{
  BaseMemory *base = arena->base;

  base->release(base->ctx, arena->memory, arena->cap);
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

INTERNAL
mem_arena_pop_to()

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

// BaseMemory *base = malloc_base_memory();
// MemArena arena = mem_make_arena(base);
// Node *nodes = (Node *)mem_arena_push(&arena, sizeof(Node) * node_count);


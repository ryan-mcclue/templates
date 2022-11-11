// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#pragma mark - M_CONTEXT_CRACKING

#if defined(__GNUC__)
  #define COMPILER_GCC 1
  #if defined(__gnu_linux__)
    #define OS_LINUX 1
  #else
    #error OS not supported
  #endif
  #if defined(__x86_64__)
    #define ARCH_X86_64 1
  #else
    #error Arch not supported
  #endif

  #define THREAD_VAR __thread
#else
  #error Compiler not supported
#endif

// NOTE(Ryan): If decide to port, zero out
#if !defined(COMPILER_GCC)
  #define COMPILER_GCC 0
#endif
#if !defined(OS_LINUX)
  #define OS_LINUX 0
#endif
#if !defined(ARCH_X86_64)
  #define ARCH_X86_64 0
#endif

#pragma mark - M_TYPES_AND_CONSTANTS

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



#pragma mark - M_BREAKPOINTS_AND_ASSERTS

#define STATEMENT(s) do { s } while (0);

#if defined(MAIN_DEBUG)
  #include <stdio.h>
  #include <stdlib.h>
  INTERNAL void __bp(const char *type, const char *file_name, const char *func_name, 
                     int line_num, const char *optional_message)
  { 
    fprintf(stderr, "%s BREAKPOINT TRIGGERED! (%s:%s:%d)\n\"%s\"\n", type, file_name, 
            func_name, line_num, optional_message);
    #if !defined(GUI_DEBUGGER)
      exit(1);
    #endif
  }
  #define 
  #define BP(msg) __bp("", __FILE__, __func__, __LINE__, msg)
  #define EBP(msg) __bp("ERRNO", __FILE__, __func__, __LINE__, msg)
  #define ASSERT(c) STATEMENT(if (!(c)) { BP("ASSERTION"); })
#else
  // TODO(Ryan): Replace as logging functions
  #define BP(msg)
  #define EBP(msg)
  #define ASSERT(c)
#endif

// STATIC_ASSERT(sizeof(arr) < VALUE, array_check);
#define STATIC_ASSERT(cond, line) typedef u8 GLUE(line, __LINE__) [(cond)?1:-1]

#define INVALID_CODE_PATH ASSERT(!"INVALID_CODE_PATH");
#define INVALID_DEFAULT_CASE default: { INVALID_CODE_PATH }


#pragma mark - M_UTILITIES

// NOTE(Ryan): Avoid having to worry about pernicous macro expansion
#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

#define GLUE_(a, b) a##b
#define GLUE(a, b) GLUE_(a, b)

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

// NOTE(Ryan): Could use heap addresses as unique IDs
#define INT_FROM_PTR(p) ((unsigned long long)((char *)p - (char *)0))
#define PTR_FROM_INT(n) ((void *)((char *)0 + (n)))

#define ABSTRACT_MEMBER(s, member) (((s *)0)->member)
#define OFFSET_OF_MEMBER(s, member) INT_FROM_PTR(&ABSTRACT_MEMBER(s, member))
// TODO: CAST_FROM_MEMBER()? (from video 2 comments)

#define MIN(x, y)
#define MAX(x, y)
#define CLAMP(min,x,max) (((x)<(min))?(min):((max)<(x))?(max):(x))
#define CLAMP_TOP(a,b) MIN(a,b)
#define CLAMP_BOT(a,b) MAX(a,b)

#define ALIGN_UP_POW2(x, p) ((x) + (p) - 1) & ~((p) - 1)
#define ALIGN_DOWN_POW2(x, p) ((x) & ~((p) - 1)) 

// Allows to search for all of them easily
#define GLOBAL static
#define LOCAL static
#define INTERNAL static

// NOTE(Ryan): Avoid confusing auto-indenter
#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

// use i+=1 in for loop syntax?
// use separate line for each for loop

// IMPORTANT(Ryan): all array macros assume static array
//
#include <string.h>

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

// reusing memory can be acheived by composing arenas
// conflicts are any arenas that are used for persistent allocations
// ArenaTemp GetScratch(Arena **conflicts, U64 conflict_count); // grabs a thread-local scratch arena
//
// arena can grow and shrink (effectively a dynamic array)
// if on embedded, abort when memory exceeds
// if on OS, can grow

#define MEMORY_ZERO(p, n) memset((p), 0, (n))
#define MEMORY_ZERO_STRUCT(p) MEMORY_ZERO((p), sizeof(*(p)))
#define MEMORY_ZERO_ARRAY(a) MEMORY_ZERO((a), sizeof(a[0]))

#define MEMORY_COPY(d, s, n) memmove((d), (s), (n))
#define MEMORY_COPY_STRUCT(d, s, n) MEMORY_COPY((d), (s), sizeof(*(s)))
#define MEMORY_COPY_ARRAY(d, s, n) MEMORY_COPY((d), (s), sizeof((s)))

#define MEMORY_MATCH(a, b, n) (memcmp((a), (b), (n)) == 0)

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)

#define THOUSAND(x) ((x)*1000LL)
#define MILLION(x)  ((x)*1000000LL)
#define BILLION(x)  ((x)*1000000000LL)
#define TRILLION(x) ((x)*1000000000000LL)

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

// IMPORTANT: Linked lists, require keeping first and last
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

struct MemTemp
{
  MemArena *arena;
  u64 pos;
};

struct MemTempBlock
{
  MemTemp temp;

  MemTempBlock(MemArena *arena);
  ~MemTempBlock();
  void reset();
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
{
  // video 4: 14:43
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

#define MEM_PUSH_ARRAY(a, t, c) (t *)mem_arena_push((a), sizeof(t) * (c))
#define MEM_PUSH_ARRAY_ZERO(a, t, c) (t *)mem_arena_push_zero((a), sizeof(t) * (c))

INTERNAL MemTemp
mem_begin_temp(MemArena *arena)
{
  MemTemp temp = {arena, arena->pos};
  return temp;
}

INTERNAL void
mem_end_temp(MemTemp temp)
{
  mem_arena_pop_to(temp.arena, temp.pos);
}

MemTempBlock::MemTempBlock(MemArena *arena)
{
  this->temp = mem_begin_temp(arena);
}

MemTempBlock::~MemTempBlock(void)
{
  mem_end_temp(this->temp);
}

MemTempBlock::reset(void)
{
  mem_end_temp(this->temp);
}

// BaseMemory *base = malloc_base_memory();
// MemArena arena = mem_make_arena(base);
// MemTemp temp = mem_begin_temp(&arena);
// Node *nodes = MEM_PUSH_ARRAY(&arena, Node, 64);
// ...
// mem_end_temp(&temp);

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

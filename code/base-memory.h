// SPDX-License-Identifier: zlib-acknowledgement

#pragma once

// IMPORTANT(Ryan): Using memory arenas changes the mentality of memory usage
// i.e. can only free things in reverse order
// just learn to program this way, more benefits than negatives

// malloc designed for arbitrary sizes and lifetimes
// this can lead to rats nests of lifetimes and computational issues freeing small nodes
// by enforcing generalities (i.e. very little constraints on interface), 
// allow for many possible solutions, which leads to complexity!
// also, doesn't allow for performance, and creates many wild usage patterns
// (developers stuck in managing program inertia, e.g. bug-fixing)

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



#include <string.h>



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

INTERNAL u64
linux_get_page_size(void)
{
  u64 result = 0;
  
  // TODO(Ryan): Investigate obtaining POSIX information with sysconf() and related functions
  i64 sysconf_result = sysconf(_SC_PAGESIZE);
  ERRNO_ASSERT(sysconf_result != -1);

  result = (u64)sysconf_result;

  return result;
}

#define MEM_DEFAULT_ALLOCATE_QUANTA GB(1)

// IMPORTANT(Ryan): The memory disconuitity of linked lists lends themselves well for arena allocators
// Also, usually iterate sequentially

typedef struct MemArena MemArena;
struct MemArena
{
  // TODO(Ryan): Investigate growable arenas from ryan fluery
  // I believe these are for growable arenas? 
  MemArena *first; 
  MemArena *last; 
  MemArena *next;
  MemArena *prev;
  MemArena *parent;
  void *memory;
  u64 commit_pos;
  u64 max;
  u64 pos;
  u64 align;
};

INTERNAL void *
mem_allocate(u64 size)
{
  u64 rounded_size = round_to_nearest(size, MEM_DEFAULT_ALLOCATE_QUANTA);
  void *result = malloc(rounded_size)
  ERRNO_ASSERT(result != NULL);

  return result;
}

INTERNAL void
mem_dellocate(void *ptr, u64 size)
{
  free(ptr, size);
}
 
INTERNAL MemArena *
mem_arena_allocate(u64 cap)
{
  MemArena *result = (MemArena *)mem_reserve(cap);

  result->memory = result + sizeof(MemArena);
  result->max = cap;
  result->pos = sizeof(MemArena);
  result->align = 8;

  return result;
}

// TODO(Ryan): Make thread safe when required
#if defined(MAIN_DEBUG)
  GLOBAL u64 debug_mem_max = 0, debug_mem_current = 0;
  GLOBAL SourceLocation debug_mem_max_info = ZERO_STRUCT;
  // TODO(Ryan): Store a linked list of allocations so we can print them out and see where memory is going?
  #define MEM_ARENA_PUSH_ARRAY(a,T,c) (T*)mem_arena_push((a), sizeof(T)*(c), LITERAL_SOURCE_LOCATION)
  #define MEM_ARENA_PUSH_ARRAY_ZERO(a,T,c) (T*)mem_arena_push_zero((a), sizeof(T)*(c), LITERAL_SOURCE_LOCATION)
  #define MEM_ARENA_POP_ARRAY(a,T,c) mem_arena_pop((a), sizeof(T)*(c))
#else
  #define MEM_ARENA_PUSH_ARRAY(a,T,c) (T*)mem_arena_push((a), sizeof(T)*(c))
  #define MEM_ARENA_PUSH_ARRAY_ZERO(a,T,c) (T*)mem_arena_push_zero((a), sizeof(T)*(c))
  #define MEM_ARENA_POP_ARRAY(a,T,c) mem_arena_pop((a), sizeof(T)*(c))
#endif

INTERNAL void *
#if defined(MAIN_DEBUG)
mem_arena_push_aligned(MemArena *arena, u64 size, u64 align, SourceLocation source_location)
#else
mem_arena_push_aligned(MemArena *arena, u64 size, u64 align)
#endif
{
  void *result = NULL;

  u64 clamped_align = CLAMP_BOTTOM(align, arena->align);

  u64 pos = arena->pos;

  u64 pos_address = INT_FROM_PTR(arena) + pos;
  u64 aligned_pos = ALIGN_POW2_UP(pos_address, clamped_align);
  u64 alignment_size = aligned_pos - pos_address;

  if (pos + alignment_size + size <= arena->max)
  {
    u8 *mem_base = (u8 *)arena;
    result = mem_base + pos + alignment_size;
    u64 new_pos = pos + alignment_size + size;
    arena->pos = new_pos;

#if defined(MAIN_DEBUG)
    debug_mem_current += (new_pos - debug_mem_current);
    if (debug_mem_current >= debug_mem_max)
    {
      debug_mem_max = debug_mem_current;
      debug_mem_max_info = source_location;
    }
#endif

    if (new_pos > arena->commit_pos)
    {
      u64 commit_grow = new_pos - arena->commit_pos;
      u64 commit_size = round_to_nearest(commit_grow, MEM_COMMIT_BLOCK_SIZE);
      linux_mem_commit(mem_base + arena->commit_pos, commit_grow);
      arena->commit_pos += commit_grow;
    }
  }

  return result;
}

INTERNAL void *
#if defined(MAIN_DEBUG)
mem_arena_push(MemArena *arena, u64 size, SourceLocation source_location)
#else
mem_arena_push(MemArena *arena, u64 size)
#endif
{

#if defined(MAIN_DEBUG)
  return mem_arena_push_aligned(arena, size, arena->align, source_location);
#else
  return mem_arena_push_aligned(arena, size, arena->align);
#endif
}

INTERNAL void *
#if defined(MAIN_DEBUG)
mem_arena_push_zero(MemArena *arena, u64 size, SourceLocation source_location)
#else
mem_arena_push_zero(MemArena *arena, u64 size)
#endif
{

#if defined(MAIN_DEBUG)
  void *memory = mem_arena_push(arena, size, source_location);
#else
  void *memory = mem_arena_push(arena, size);
#endif

  MEMORY_ZERO(memory, size);

  return memory;
}

INTERNAL void
mem_arena_set_pos_back(MemArena *arena, u64 pos)
{
  u64 clamped_pos = CLAMP_BOTTOM(sizeof(*arena), pos);

  if (arena->pos > clamped_pos)
  {
    arena->pos = clamped_pos;

#if defined(MAIN_DEBUG)
    debug_mem_current -= (clamped_pos - debug_mem_current);
#endif

    u64 decommit_pos = round_to_nearest(clamped_pos, MEM_COMMIT_BLOCK_SIZE);
    u64 over_committed = arena->commit_pos - decommit_pos;
    over_committed -= over_committed % MEM_COMMIT_BLOCK_SIZE;
    if (decommit_pos > 0 && over_committed >= MEM_DECOMMIT_THRESHOLD)
    {
      linux_mem_decommit((u8 *)arena + decommit_pos, over_committed);
      arena->commit_pos -= over_committed;
    }
  }
}


INTERNAL void
mem_arena_pop(MemArena *arena, u64 size)
{
  mem_arena_set_pos_back(arena, arena->pos - size);
}

INTERNAL void
mem_arena_clear(MemArena *arena)
{
  mem_arena_set_pos_back(arena, arena->pos);
}


INTERNAL void
mem_arena_release(MemArena *arena)
{
  MemArena *next = NULL;
  for (MemArena *child = arena->first; child != NULL; child = next)
  {
    next = child->next;
    mem_arena_release(child);
  }
  mem_arena_release(arena);
}

typedef struct MemArenaTemp MemArenaTemp;
struct MemArenaTemp
{
  MemArena *arena;
  u64 pos;
};

INTERNAL MemArenaTemp
mem_arena_scratch_get(MemArena **conflicts, u64 conflict_count)
{
  MemArenaTemp scratch = ZERO_STRUCT;
  ThreadContext *tctx = thread_context_get();

  for (u64 tctx_idx = 0; tctx_idx < ARRAY_COUNT(tctx->arenas); tctx_idx += 1)
  {
    b32 is_conflicting = 0;
    for (MemArena **conflict = conflicts; conflict < conflicts+conflict_count; conflict += 1)
    {
      if (*conflict == tctx->arenas[tctx_idx])
      {
        is_conflicting = 1;
        break;
      }
    }

    if (is_conflicting == 0)
    {
      scratch.arena = tctx->arenas[tctx_idx];
      scratch.pos = scratch.arena->pos;
      break;
    }
  }

  return scratch;
}

INTERNAL void
mem_arena_scratch_release(MemArenaTemp *temp)
{
  mem_arena_set_pos_back(temp->arena, temp->arena->pos);
}











// get_nprocs() - 1
typedef struct ThreadContext ThreadContext;
struct ThreadContext
{
  MemArena *arenas[2];  
  const char *file_name;
  u64 line_number;
};

THREAD_LOCAL ThreadContext *tl_thread_context = NULL;

INTERNAL ThreadContext
thread_context_create(void)
{
  ThreadContext result = ZERO_STRUCT;

  for (u32 arena_i = 0; arena_i < ARRAY_COUNT(result.arenas); ++arena_i)
  {
    result.arenas[arena_i] = mem_arena_allocate(GB(8));
  }

  return result;
}

INTERNAL void
thread_context_set(ThreadContext *tcx)
{
  // TODO(Ryan): How exactly does this work multithreaded?
  // metadesk multithreaded not same way
  tl_thread_context = tcx;
}

INTERNAL ThreadContext *
thread_context_get(void)
{
  return tl_thread_context; 
}

#define THREAD_CONTEXT_REGISTER_FILE_AND_LINE \
  __thread_context_register_file_and_line(__FILE__, __LINE__)
INTERNAL void
__thread_context_register_file_and_line(char *file, int line)
{
  ThreadContext *tctx = thread_context_get();
  tctx->file_name = file;
  tctx->line_number = (u64)line;
}

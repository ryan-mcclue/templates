// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

// IMPORTANT(Ryan): Although using memory arenas restricts arbitrary lifetimes, this provides more benefits than negatives.
// In majority of cases, a large number of allocations can be bucketed into same arena

// IMPORTANT(Ryan): In essence, OS is ultimate garbage collector as it releases page table for us.
// So, we generally shouldn't have to perform manual garbage collection.
// That is, no periodic code interuptions to determine lifetimes and possibly free

#include <string.h>
#include <stdlib.h>

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

#define MEM_DEFAULT_ALLOCATE_QUANTA GB(1)

typedef struct MemArena MemArena;
struct MemArena
{
  void *memory;
  u64 commit_pos;
  u64 max;
  u64 pos;
  u64 align;
};

INTERNAL MemArena *
mem_arena_allocate(u64 cap)
{
  u64 rounded_size = round_to_nearest(cap, MEM_DEFAULT_ALLOCATE_QUANTA);
  MemArena *result = (MemArena *)malloc(rounded_size);
  if (result == NULL)
  {
    FATAL_ERROR("Result", strerror(errno), "restart");
  }


  result->memory = result + sizeof(MemArena);
  result->max = cap;
  result->pos = sizeof(MemArena);
  result->align = 8;

  return result;
}

INTERNAL void
mem_arena_deallocate(MemArena *arena)
{
  free(arena);
}
 
#define MEM_ARENA_PUSH_ARRAY(a,T,c) (T*)mem_arena_push((a), sizeof(T)*(c))
#define MEM_ARENA_PUSH_ARRAY_ZERO(a,T,c) (T*)mem_arena_push_zero((a), sizeof(T)*(c))
#define MEM_ARENA_POP_ARRAY(a,T,c) mem_arena_pop((a), sizeof(T)*(c))

#define MEM_ARENA_PUSH_STRUCT(a,T) (T*)mem_arena_push((a), sizeof(T))
#define MEM_ARENA_PUSH_STRUCT_ZERO(a,T) (T*)mem_arena_push_zero((a), sizeof(T))


INTERNAL void *
mem_arena_push_aligned(MemArena *arena, u64 size, u64 align)
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
  }

  return result;
}

INTERNAL void *
mem_arena_push(MemArena *arena, u64 size)
{
  return mem_arena_push_aligned(arena, size, arena->align);
}

INTERNAL void *
mem_arena_push_zero(MemArena *arena, u64 size)
{
  void *memory = mem_arena_push(arena, size);

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
  mem_arena_pop(arena, arena->pos);
}


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

typedef struct MemArenaTemp MemArenaTemp;
struct MemArenaTemp
{
  MemArena *arena;
  u64 pos;
};

// IMPORTANT(Ryan): Require 2 scratches as code may not know if *arena is scratch or permanent
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
mem_arena_scratch_release(MemArenaTemp temp)
{
  mem_arena_set_pos_back(temp.arena, temp.pos);
}

// SPDX-License-Identifier: zlib-acknowledgement

#define STB_SPRINTF_IMPLEMENTATION 1
#include "stb/stb_sprintf.h"

// .h includes
#include "base.h"

// .c includes

#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

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

#define MEM_DEFAULT_RESERVE_SIZE GB(1)
#define MEM_COMMIT_BLOCK_SIZE KB(4)
#define MEM_DECOMMIT_THRESHOLD KB(64)

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
STATIC_ASSERT(sizeof(MemArena) <= MEM_COMMIT_BLOCK_SIZE, arena_size_check);


INTERNAL void *
linux_mem_reserve(u64 size)
{
  u64 gb_rounded_size = round_to_nearest(size, GB(1));
  void *result = mmap(NULL, gb_rounded_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, (off_t)0);
  return result;
}

INTERNAL b32
linux_mem_commit(void *ptr, u64 size)
{
  u64 page_rounded_size = round_to_nearest(size, linux_get_page_size());
  b32 result = (mprotect(ptr, page_rounded_size, PROT_READ | PROT_WRITE) == 0);
  return result;
}

INTERNAL void
linux_mem_decommit(void *ptr, u64 size)
{
  mprotect(ptr, size, PROT_NONE);
  madvise(ptr, size, MADV_DONTNEED);
}

INTERNAL void
linux_mem_release(void *ptr, u64 size)
{
  munmap(ptr, size);
}
 
INTERNAL MemArena *
mem_arena_allocate(u64 cap)
{
  MemArena *result = (MemArena *)linux_mem_reserve(cap);
  linux_mem_commit(result, MEM_COMMIT_BLOCK_SIZE);

  result->first = result->last = result->next = result->prev = result->parent = NULL;
  result->memory = result + sizeof(MemArena);
  result->max = cap;
  result->pos = sizeof(MemArena);
  result->commit_pos = MEM_COMMIT_BLOCK_SIZE;
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

typedef struct Array Array;
struct Array
{
	u64 capacity;
	u64 len;
	u8 *buffer;
};

#if defined(MAIN_DEBUG)
  #define ARRAY_CREATE(arena, type, len) \
    (type *)array_create(arena, sizeof(type), len, SOURCE_LOCATION)
#else
  #define ARRAY_CREATE(arena, type, len) \
    (type *)array_create(arena, sizeof(type), len)
#endif

INTERNAL void *
#if defined(MAIN_DEBUG)
array_create(MemArena *arena, u64 elem_size, u64 elem_count, SourceLocation source_location)
#else
array_create(MemArena *arena, u64 elem_size, u64 elem_count)
#endif
{
#if defined(MAIN_DEBUG)
  Array *array = (Array *)mem_arena_push_zero(arena, sizeof(Array) + (elem_count * elem_size), source_location);
#else
  Array *array = (Array *)mem_arena_push_zero(arena, sizeof(Array) + (elem_count * elem_size));
#endif

  array->buffer = (u8 *)array + OFFSET_OF_MEMBER(Array, buffer);
  array->len = 0;
  array->capacity = elem_count;

  return array->buffer;
}

#define ARRAY_LEN(buf) \
  (((buf) != NULL) ? (CAST_FROM_MEMBER(Array, buffer, buf))->len : 0)

#define ARRAY_CAPACITY(buf) \
  (((buf) != NULL) ? (CAST_FROM_MEMBER(Array, buffer, buf))->capacity : 0)

#define ARRAY_PUSH(buf, elem) \
	(ARRAY_LEN(buf) + 1 <= ARRAY_CAPACITY(buf)) ? \
    (buf)[(CAST_FROM_MEMBER(Array, buffer, buf))->len++] = (elem) : \
    FATAL_ERROR("Pushing element onto array exceeds its capacity")

#define ARRAY_REMOVE_ORDERED(buf, index) \
  do { \
    for (u32 UNIQUE_NAME(var) = index; UNIQUE_NAME(var) < ARRAY_LEN(buf) - 1; UNIQUE_NAME(var)++) { \
      (buf)[UNIQUE_NAME(var)] = (buf)[UNIQUE_NAME(var) + 1]; \
    } \
    ARRAY_LEN(buf)--; \
  } while (0)

#define ARRAY_REMOVE_UNORDERED(buf, index) \
  (((buf) != NULL) ? (buf)[index] = (buf)[--ARRAY_LEN(buf)] : 0)

// IMPORTANT(Ryan): In general though, in most programming situations we know how much data is available in advance 
// Yet, can still achieve growable memory with arenas.
// Most efficient is with a pool allocator using linked-lists
// With arrays, can be implemented with duplicated memory
/*
struct Entity
{
  Entity *next;
  Vec2F32 position;
  Vec2F32 velocity;
  // some more stuff in here
};

struct GameState
{
  Arena *permanent_arena;
  Entity *first_free_entity;
};

Entity *EntityAlloc(GameState *game_state)
{
  // first, try to grab the top of the free list...
  Entity *result = game_state->first_free_entity;
  if(result != 0)
  {
    game_state->first_free_entity = game_state->first_free_entity->next;
    MemoryZeroStruct(result);
  }

  // if the free list was empty, push a new entity onto the arena
  else
  {
    result = PushStructZero(game_state->permanent_arena, Entity);
  }

  return result;
}

void EntityRelease(GameState *game_state, Entity *entity)
{
  // releasing -> push onto free list. next allocation
  // will take the top of the free list, not push onto
  // the arena.
  entity->next = game_state->first_free_entity;
  game_state->first_free_entity = entity;
}
*/


// IMPORTANT(Ryan): For complete binary trees, better off storing it as an array
// In other tree structures, better of storing in nodes to account for holes, e.g. say root node only has left children

#define HEAP_CHILD0_INDEX(i) (2 * (i) + 1)
#define HEAP_CHILD1_INDEX(i) (2 * (i) + 2)
#define HEAP_PARENT_INDEX(i) math_floor_f32_to_u32((f32)(i - 1) / 2.0f)
#define HEAP_LAST_PARENT_INDEX(size) math_floor_f32_to_u32((f32)(size - 2) / 2.0f)

INTERNAL void
min_heap_create(u32 *array)
{
  for (u32 i = 0; i < ARRAY_LEN(array); i += 1)
  {
     
  }
}

// remove root: swap with last, sift down (swap with minimum of child nodes)

// insert: append, walk up/sift up swapping with parent

// create: iteratively sift down on parent nodes starting from end


typedef struct String8 String8;
struct String8
{
  u8 *str;
  u64 size;
};

INTERNAL String8
s8(u8 *str, u64 size)
{
  String8 result = ZERO_STRUCT;

  result.str = str;
  result.size = size;

  return result;
}

#define s8_from_lit(s) s8((u8 *)(s), sizeof(s) - 1)
#define s8_from_cstring(s) s8((u8 *)(s), strlen(s))
#define s8_varg(s) (int)(s).size, (s).str
// use like: \"%.*s\"", s8_varg(string)

INTERNAL String8
s8_from_range(u8 *start, u8 *one_past_last)
{
  String8 result = ZERO_STRUCT;

  return result;
}

INTERNAL b32
s8_match(String8 s1, String8 s2)
{
  return strncmp((char *)s1.str, (char *)s2.str, s1.size);
}

INTERNAL String8
s8_copy(MemArena *arena, String8 string)
{
  String8 result = ZERO_STRUCT;

  result.size = string.size;
  result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, string.size + 1);
  MEMORY_COPY(result.str, string.str, string.size);
  result.str[string.size] = '\0';

  return result;
}

INTERNAL String8
s8_fmt(MemArena *arena, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  String8 result = ZERO_STRUCT;
  u64 needed_bytes = (u64)stbsp_vsnprintf(NULL, 0, fmt, args) + 1;
  result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, needed_bytes);
  result.size = needed_bytes - 1;
  result.str[needed_bytes - 1] = 0;
  stbsp_vsnprintf((char *)result.str, (int)needed_bytes, fmt, args);

  va_end(args);
  return result;
}

INTERNAL String8
s8_substring(String8 str, u64 min, u64 max)
{
  if (max > str.size)
  {
    max = str.size;
  }

  if (min > str.size)
  {
    min = str.size;
  }

  if (min > max)
  {
    u64 swap = min;
    min = max;
    max = swap;
  }

  str.size = max - min;
  str.str += min;

  return str;
}

INTERNAL String8 
s8_read_entire_file(MemArena *arena, String8 file_name)
{
  String8 result = ZERO_STRUCT;

  FILE *file = fopen((char *)file_name.str, "rb");

  if (file != NULL)
  {
    fseek(file, 0, SEEK_END);
    u64 file_size = (u64)ftell(file);
    fseek(file, 0, SEEK_SET);
    result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, file_size + 1);
    if (result.str != NULL)
    {
      result.size = file_size;
      fread(result.str, 1, file_size, file);
      result.str[result.size] = '\0';
    }
    fclose(file);
  }

  return result;
}

INTERNAL void
s8_write_entire_file(String8 file_name, String8 data)
{
	FILE *file = fopen((char *)file_name.str, "w+");

  if (file != NULL)
  {
	  fputs((char *)data.str, file);
	  fclose(file);
  }
}

INTERNAL void
s8_append_to_file(String8 file_name, String8 data)
{
	FILE *file = fopen((char *)file_name.str, "a");

  if (file != NULL)
  {
	  fputs((char *)data.str, file);
	  fclose(file);
  }
}

#if 0
INTERNAL b32
os_file_rename(String8 og_name, String8 new_name){
    // convert name
    M_ArenaTemp scratch = m_get_scratch(0, 0);
    String16 og_name16 = str16_from_str8(scratch.arena, og_name);
    String16 new_name16 = str16_from_str8(scratch.arena, new_name);
    // rename file
    B32 result = MoveFileW((WCHAR*)og_name16.str, (WCHAR*)new_name16.str);
    m_release_scratch(scratch);
    return(result);
}

typedef MD_u32 MD_FileFlags;
enum
{
    MD_FileFlag_Directory = (1<<0),
};

typedef U32 DataAccessFlags;
enum{
  DataAccessFlag_Read    = (1 << 0),
  DataAccessFlag_Write   = (1 << 1),
  DataAccessFlag_Execute = (1 << 2),
};

typedef struct MD_FileInfo MD_FileInfo;
struct MD_FileInfo
{
    MD_FileFlags flags;
    MD_String8 filename;
    MD_u64 file_size;
  u64 create_time;
  u64 modify_time;
  DataAccessFlags access;
};

typedef struct MD_FileIter MD_FileIter;
struct MD_FileIter
{
    // This is opaque state to store OS-specific file-system iteration data.
    MD_u8 opaque[640];
};

// b32 file_start = file_iter_begin(&file_iter, path);
// FileInfo file_info = file_iter_next(&file_iter);
typedef struct MD_LINUX_FileIter MD_LINUX_FileIter;
struct MD_LINUX_FileIter
{
    int dir_fd;
    DIR *dir;
};
MD_StaticAssert(sizeof(MD_LINUX_FileIter) <= sizeof(MD_FileIter), file_iter_size_check);

static MD_b32
MD_LINUX_FileIterIncrement(MD_Arena *arena, MD_FileIter *opaque_it, MD_String8 path,
                           MD_FileInfo *out_info)
{
    MD_b32 result = 0;
    
    MD_LINUX_FileIter *it = (MD_LINUX_FileIter *)opaque_it;
    if(it->dir == 0)
    {
        it->dir = opendir((char*)path.str);
        it->dir_fd = open((char *)path.str, O_PATH|O_CLOEXEC);
    }
    
    if(it->dir != 0 && it->dir_fd != -1)
    {
        struct dirent *dir_entry = readdir(it->dir);
        if(dir_entry)
        {
            out_info->filename = MD_S8Fmt(arena, "%s", dir_entry->d_name);
            out_info->flags = 0;
            
            struct stat st; 
            if(fstatat(it->dir_fd, dir_entry->d_name, &st, AT_NO_AUTOMOUNT|AT_SYMLINK_NOFOLLOW) == 0)
            {
                if((st.st_mode & S_IFMT) == S_IFDIR)
                {
                    out_info->flags |= MD_FileFlag_Directory;
                }
                out_info->file_size = st.st_size;
            }
            result = 1;
        }
    }
    
    if(result == 0)
    {
        if(it->dir != 0)
        {
            closedir(it->dir);
            it->dir = 0;
        }
        if(it->dir_fd != -1)
        {
            close(it->dir_fd);
            it->dir_fd = -1;
        }
    }
    
    return result;
}

#if 0
internal b32
LinuxCopyFile(char *dest, char *source)
{
	FILE *source_fp;
	fseek(source_fp, 0, SEEK_END);
	u32 fsize = ftell(source_fp);
	rewind(source_fp);

	char *source_data = malloc(fsize + 1);
	fread(source_data, 1, fsize, source_fp);
	fclose(source_fp);

	FILE *dest_fp;
	dest_fp = fopen(dest, "w+");
	fputs(dest_fp, source_data);
	fclose(dest_fp);

	// Return 0 on error
	return 1;
}

function FileProperties
os_file_properties(String8 file_name){

linux_delete_file();

INTERNAL b32
linux_create_directory(String8 directory_name)
{
	if (mkdir(path) == 0)
	{
		return 1;
	}
	return 0;
}

INTERNAL b32
LinuxDoesFileExist(char *path)
{
	// This probably isn't the best way
	if (access(path, F_OK) != -1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

internal b32
LinuxDoesDirectoryExist(char *path)
{
	return LinuxDoesFileExist(path);
}

#endif
#endif


typedef struct String8List String8List;
struct String8List
{
  String8 *first;
  String8 *last;
  u64 node_count;
  u64 total_size;
};

#if 0
typedef struct State State;
struct State
{
  MemArena *arena;
  MemArena *frame_arenas[2];
  b32 should_quit;
  u32 frame_idx;
  //APP_Window *first_window; // these will have a field that is updated to indicate whether the window is active or not
  //APP_Window *last_window;
  //APP_Window *free_window;
  //R_Backend render_backend;
  //R_Handle render_eqp;
};

GLOBAL State *state = NULL;

INTERNAL MemArena *
state_get_frame_arena(void)
{
  return state->frame_arenas[state->frame_idx % ARRAY_COUNT(state->frame_arenas)];
}

INTERNAL void
get_events(MemArena *events_arena)
{
  SDL_Event event = {};

  // W32_WindowProc
  
  b32 is_release = false;
  while (SDL_PollEvent(&event))
  {

  }
}

INTERNAL void
update_and_render(void)
{
  begin_frame_timer();

  mem_arena_clear(state_get_frame_arena());

  EventList events = get_events(state_get_frame_arena());

  for(OS_Event *event = events.first;
      event != 0;
      event = event->next)
  {
    if (event->kind == OS_EventKind_WindowClose)
    {

    }
    eat_event();
  }

  // render part
  MemArenaTemp scratch = ;

  for(APP_Window *w = app_state->first_window; w != 0; w = w->next)
  {
    app_state->render_backend.Begin(app_state->render_eqp, w->render_eqp, w->handle);
    {
      // NOTE(rjf): Submit R_Layer's to the backend here, for this window
      R_Layer dummy_layer = {0};
      // IMPORTANT(Ryan): this submit function will be unique to each window
      // so, could pass in event state particular to a window
      app_state->render_backend.Submit(app_state->render_eqp, w->render_eqp, w->handle,
          Dim2F32(OS_ClientRectFromWindow(w->handle)),
          dummy_layer);
    }
    app_state->render_backend.Finish(app_state->render_eqp, w->render_eqp, w->handle);
  }

  state->frame_idx += 1;

  platform->current_time += 1.f / platform->target_frames_per_second;

  // the application will have the ability to queue and wait for job completion
  // the platform will execute the jobs and update their status at the end of every
  // update iteration?
  // TODO: threads: arcane/source/telescope/tsfoundation/tsfoundation_linux_threads.c
  end_frame_timer();
}

function APP_Window *
APP_WindowOpen(String8 title, Vec2S64 size)
{
    APP_Window *window = app_state->free_window;
    if(window == 0)
    {
        window = PushArrayZero(app_state->arena, APP_Window, 1);
    }
    else
    {
        StackPop(app_state->free_window);
        MemoryZeroStruct(window);
    }
    window->handle = OS_WindowOpen(title, size);
    DLLPushBack(app_state->first_window, app_state->last_window, window);
    window->render_eqp = app_state->render_backend.EquipWindow(app_state->render_eqp, window->handle);
    return window;
}

function void
APP_WindowClose(APP_Window *window)
{
    app_state->render_backend.UnequipWindow(app_state->render_eqp, window->render_eqp, window->handle);
    OS_WindowClose(window->handle);
    DLLRemove(app_state->first_window, app_state->last_window, window);
    StackPush(app_state->free_window, window);
}

INTERNAL void
push_event()
{

}

INTERNAL void
eat_event(OS_EventList *events, OS_Event *event)
{
    OS_Event *next = event->next;
    OS_Event *prev = event->prev;
    DLLRemove(events->first, events->last, event);
    events->count -= 1;
    event->next = next;
    event->prev = prev;
}

// dijkstra's only for weighted, directed (e.g. only concerned with outgoing edges), positive weight graphs  

typedef struct Vertex Vertex;
struct Vertex
{
  u32 data;
};

typedef struct VertexEdge VertexEdge;
struct VertexEdge
{
  VertexEdge *next;

  Vertex *dest_vertex;
  u64 weight;
};

typedef struct VertexEdges VertexEdges;
struct VertexEdges
{
  VertexEdges *next;

  VertexEdge *first; 
  VertexEdge *last; 

  Vertex *vertex;
};

typedef struct AdjacencyList AdjacencyList;
struct AdjacencyList
{
  VertexEdges *first;
  VertexEdges *last;
};

INTERNAL AdjacencyList *
adjacency_list_create(MemArena *arena)
{
  AdjacencyList *result = NULL;

  result = MEM_ARENA_PUSH_ARRAY_ZERO(arena, AdjacencyList, 1);

  return result;
}

vertex_create();

vertex_edge_create(dest_vertex, weight);

vertex_edges_create(vertex);
vertex_edges_push_vertex_edge();

adjacency_list_push_vertex_edges();

DijkstraResult dijkstra_result = compute_dijstrka(adjacency_list, start_vertex);

// could compute naively with say DFS to compute all possible paths
// when finding min value, log(n) with min-heap (which would have associated push/pop interfaces)
// ∴ get O((v + e) · log(v))
#endif

GLOBAL MemArena *linux_mem_arena_perm = NULL;

typedef struct Timer Timer;
struct Timer
{
  u64 start;
  u64 end;
  u64 ticks_per_sec; 
};

INTERNAL Timer
linux_timer_get(void)
{
  Timer result = ZERO_STRUCT;

  result.ticks_per_sec = NANO_TO_SEC(1);

  return result;
}

INTERNAL void
linux_timer_start(Timer *timer)
{
	struct timespec t_spec = ZERO_STRUCT;
	ERRNO_ASSERT(clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec) != -1);
  // NOTE(Ryan): time_t can be negative as it represents offset from epoch
	timer->start = (u64)t_spec.tv_sec * timer->ticks_per_sec + (u64)t_spec.tv_nsec;
}

INTERNAL void
linux_timer_end(Timer *timer)
{
	struct timespec t_spec = ZERO_STRUCT;
	ERRNO_ASSERT(clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec) != -1);
	timer->end = (u64)t_spec.tv_sec * timer->ticks_per_sec + (u64)t_spec.tv_nsec;
}

#define EVAL_PRINT_U32(var) printf("%s = %ld \n", STRINGIFY(var), var)

// IMPORTANT(Ryan): If we put everything in the tree node at the start of the structure,
// we can write a function that will have a void *, and then cast to this structure
// although types different, they are addresses, so equivalent
// struct Node {
//  Node *parent;
//  Node *first_child, *last_child;
//  Node *next, *prev;
// }

int
main(int argc, char *argv[])
{
  IGNORED(argc); IGNORED(argv);

  Timer timer = linux_timer_get();
  linux_timer_start(&timer);

  // IMPORTANT(Ryan): For switch statements, put default at top 

  // TODO(Ryan): Drawing UI: arcane/game/source/arc/ui.ui.c: DrawEditorUI(); 

  // thread init
  ThreadContext tcx = thread_context_create();
  thread_context_set(&tcx);

  // os init
  linux_mem_arena_perm = mem_arena_allocate(GB(1)); 

  // record timer start here
  // command line arguments
  //for (int i = 0; i < argc; i += 1){
  //    String8 arg = str8_cstring((U8*)argv[i]);
  //    str8_list_push(linux_mem_arena_perm, &linux_cmd_line, arg);
  //}
  // binary dir: readlink("/proc/self/exe");
  // cwd: getcwd();
  
  // app init
  //MemArena *arena = mem_arena_allocate(GB(16));
  //state = PushArrayZero(arena, APP_State, 1);
  //state->arena = arena;
  //for(int i = 0; i < ArrayCount(state->frame_arenas); i += 1)
  //{
  //  state->frame_arenas[i] = M_ArenaAlloc(Gigabytes(16));
  //}

  // renderer init
  // find refresh rate
  //while (state->want_to_run)
  //{
  //  update_and_render();
  //}
  
  //AdjacencyList *adjacency_list = adjacency_list_create(linux_mem_arena_perm);

  linux_timer_end(&timer);
  fprintf(stdout, "%f\n", (f64)(timer.end - timer.start) / (f64)timer.ticks_per_sec);

#if MAIN_DEBUG
  fprintf(stdout, "Max heap usage: %ldbytes, %s:%s():%ld\n", debug_mem_max,
          debug_mem_max_info.file_name, debug_mem_max_info.function_name, 
          debug_mem_max_info.line_number);
#endif

  return 0;
}


#if 1

typedef struct MapKey MapKey;
struct MapKey
{
  u64 hash;
  // size is used to determine if original key was a string?
  u64 size;
  // the ptr is the original key value?
  void *ptr;
};

// IMPORTANT(Ryan): Using chaining
typedef struct MapSlot MapSlot;
struct MapSlot
{
  MapSlot *next;
  MapKey key;
  void *val;
};

typedef struct MapBucket MapBucket;
struct MapBucket
{
  MapSlot *first;
  MapSlot *last;
};

typedef struct Map Map;
struct Map
{
  MapBucket *buckets;
  u64 bucket_count;
};

INTERNAL u64
map_hash_str(String8 string)
{
  u64 result = 5381;

  for (u64 i = 0; i < string.size; i += 1)
  {
    result = ((result << 5) + result) + string.str[i];
  }

  return result;
}

INTERNAL MapKey
map_key_str(String8 string)
{
  MapKey result = ZERO_STRUCT;

  if (string.size != 0)
  {
    result.hash = map_hash_str(string);
    result.size = string.size;
    if (string.size > 0)
    {
      result.ptr = string.str;
    }
  }

  return result;
}

INTERNAL u64 
map_hash_ptr(void *p)
{
  u64 h = (u64)p;

  h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9;
  h = (h ^ (h >> 27)) * 0x94d049bb133111eb;
  h = h ^ (h >> 31);

  return h;
}

INTERNAL MapKey
map_key_ptr(void *ptr)
{
  MapKey result = ZERO_STRUCT;

  if (ptr != NULL)
  {
    result.hash = map_hash_ptr(ptr);
    result.size = 0;
    result.ptr = ptr;
  }

  return result;
}

INTERNAL Map
map_create(MemArena *arena, u64 bucket_count)
{
  Map result = ZERO_STRUCT;

  result.bucket_count = bucket_count;
  result.buckets = MEM_ARENA_PUSH_ARRAY_ZERO(arena, MapBucket, bucket_count);

  return result;
}

INTERNAL MapSlot *
map_scan(MapSlot *first_slot, MapKey key)
{
  MapSlot *result = NULL;

  if (first_slot != NULL)
  {
    b32 ptr_kind = (key.size == 0);
    String8 key_string = s8((u8 *)key.ptr, key.size);
    for (MapSlot *slot = first_slot; slot != NULL; slot = slot->next)
    {
      if (slot->key.hash == key.hash)
      {
        if (ptr_kind)
        {
          if (slot->key.size == 0 && slot->key.ptr == key.ptr)
          {
            result = slot;
            break;
          }
        }
        else
        {
          String8 slot_string = s8((u8 *)slot->key.ptr, slot->key.size);
          if (s8_match(slot_string, key_string))
          {
            result = slot;
            break;
          }
        }
      }
    }
  }

  return result;
}

INTERNAL MapSlot *
map_lookup(Map *map, MapKey key)
{
  MapSlot *result = NULL;

  if (map->bucket_count > 0)
  {
    u64 index = key.hash % map->bucket_count;
    result = map_scan(map->buckets[index].first, key);
  }

  return result;
}


INTERNAL MapSlot *
map_insert(MemArena *arena, Map *map, MapKey key, void *val)
{
  MapSlot *result = NULL;

  if (map->bucket_count > 0)
  {
    u64 index = key.hash % map->bucket_count;
    MapSlot *slot = MEM_ARENA_PUSH_ARRAY(arena, MapSlot, 1);
    MapBucket *bucket = &map->buckets[index];
    SLL_QUEUE_PUSH(bucket->first, bucket->last, slot);
    slot->key = key;
    slot->val = val;
    result = slot;
  }

  return result;
}

INTERNAL MapSlot *
map_overwrite(MemArena *arena, Map *map, MapKey key, void *val)
{
  MapSlot *result = map_lookup(map, key);

  if (result != NULL)
  {
    result->val = val;
  }
  else
  {
    result = map_insert(arena, map, key, val);
  }

  return result;
}

// have to know to cast to particular type
//map_insert(arena, &map, map_key_from_str(node->string), (void*)(u64)eval_result);

typedef struct TreeNode TreeNode;
struct TreeNode
{
  TreeNode *next;
  TreeNode *prev;
  TreeNode *parent;
  TreeNode *first_child;
  TreeNode *last_child;
};

#endif


#if 0
/*
Table generation, i.e. enum to string table, often use X-Macros; however lose search features in editor (not necessarily a deal breaker)
    Programmer’s Thoughts → Text in C Language (our authorship)

    Text in C Language → C Tokens

    C Tokens → C Abstract Syntax Tree

    C Abstract Syntax Tree → C Type-Checked Tree

    C Type-Checked Tree → IR Data Structure

    IR Data Structure → Machine Code
In reality, any language does not provide level of expressiveness for all problems
Instead of overcomplicating compiler by providing more features, introduce a second stage of authorship
So, compile a program before hand that will generate C code

DEFINITION NODE:
@table(name, str) MyEnumTable:
{
  { A "A" }
  { B "B" }
  { C "C" }
}

GENERATION NODE:
@table_gen_enum
MyEnum:
{
  @expand(MyEnumTable a)
    `MyEnum_$(a.name),`;
  `MyEnum_COUNT`;
}

@table_gen_data(`char *`)
myenum_string_table:
{
  @expand(MyEnumTable a)
    `"$(a.str)",`;
}
*/

#endif




#if 0

INTERNAL u64
linux_get_ms()

#define M4X4_IDENTITY_INIT { \
    1, 0, 0, 0,  \
    0, 1, 0, 0,  \
    0, 0, 1, 0,  \
    0, 0, 0, 1,  \
}
static const m4x4_t m4x4_identity = M4X4_IDENTITY_INIT;


/*
Most ideas are bad, realise this uncomfortable truth
So what’s the solution? In my mind, it has to do with directly engaging with this uncomfortable reality. 
And, in doing so, ideas and projects will be shaped by reality, rather than divorced from it. 
It begins with simply asking the question—“are you measuring your project against markets—against the perceptions and values of others—or aren’t you?”

seems that static analysis tools give us some of these metrics? 
GENERAL PRINCIPLE: YOU GET WHAT YOU MEASURE
memory usage
lines of code
cyclomatic complexity? (perhaps count number of dependencies in a function to determine complexity?)
(this is because cyclomatic just tells us how risky to change a function)
longest function lengths?
for qualitative analysis, perhaps require a linter?

If you never measure memory usage in your program, it’s far more likely that your program will have wasteful memory usage. 
If you never measure a frame’s performance in your game, it’s far more likely that your game will drop frames. 
If you never measure the size of your codebase, it’s far more likely that it grows indefinitely. 
And if you never measure your idea against the values and needs of others, it’s far more likely that—to everyone but you—it’s a big waste of time. 
And if it’s a big waste of time for everyone but you, then your project does not deserve—for instance—anyone else’s money.

the important thing is simply that I’m actively writing, thinking, and engaging with an audience, even if it’s imperfect, sloppy, and uncomfortable.

a follow on from this is how I view open source.
not as the altruistic community participiation, but rather from tit-for-tat as a game theory for making an efficient product
all users add features they want for themselves. the rule is they have to give them back.
*/


// Cryptic expression comma chaining necessary for returning a value

void *RendererLibrary = dlopen("libHandmadeOpenGL.so", RTLD_NOW | RTLD_LOCAL);
linux_load_renderer *LinuxLoadOpenGLRenderer = (linux_load_renderer *)dlsym(RendererLibrary, LinuxRendererFunctionTableNames[0]);

internal void
LinuxMakeQueue(platform_work_queue *Queue, u32 ThreadCount, linux_thread_startup *Startups)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    
    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;
    
    uint32 InitialCount = 0;
    sem_init(&Queue->SemaphoreHandle, 0, InitialCount);
    
    for(uint32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        linux_thread_startup *Startup = Startups + ThreadIndex;
        Startup->Queue = Queue;
        
        pthread_attr_t Attr;
        pthread_t ThreadID;
        pthread_attr_init(&Attr);
        // TODO(michiel): Check return values
        pthread_attr_setdetachstate(&Attr, PTHREAD_CREATE_DETACHED);
        if (pthread_attr_setstacksize(&Attr, 0x100000))
        {
            fprintf(stderr, "Failed to set the thread stack size to 1MB\n");
        }
        int result = pthread_create(&ThreadID, &Attr, ThreadProc, Startup);
        pthread_attr_destroy(&Attr);
    }
}


// TODO(casey): Does LLVM have real read-specific barriers yet?
#define CompletePreviousReadsBeforeFutureReads asm volatile("" ::: "memory")
#define CompletePreviousWritesBeforeFutureWrites asm volatile("" ::: "memory")
inline uint32 AtomicCompareExchangeUInt32(uint32 volatile *Value, uint32 New, uint32 Expected)
{
    uint32 Result = __sync_val_compare_and_swap(Value, Expected, New);
    
    return(Result);
}
inline u64 AtomicExchangeU64(u64 volatile *Value, u64 New)
{
    u64 Result = __sync_lock_test_and_set(Value, New);
    
    return(Result);
}
inline u64 AtomicAddU64(u64 volatile *Value, u64 Addend)
{
    // NOTE(casey): Returns the original value _prior_ to adding
    u64 Result = __sync_fetch_and_add(Value, Addend);
    
    return(Result);
}
inline u32 GetThreadID(void)
{
    u32 ThreadID;
#if defined(__APPLE__) && defined(__x86_64__)
    asm("mov %%gs:0x00,%0" : "=r"(ThreadID));
#elif defined(__i386__)
    asm("mov %%gs:0x08,%0" : "=r"(ThreadID));
#elif defined(__x86_64__)
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
#else
#error Unsupported architecture
#endif
    
    return(ThreadID);
}

// UIs exist to transfer information between user and program (decide what is useful information)
// So, when making a UI decision, if requires less information to be sent, then a good one (e.g. button press over typing a long string)
// Also, how quickly the user can enter this information is important to

// use well known design elements, e.g. buttons, boxes
// provide usage hints, e.g. buttons embossed, dynamically animate (in time suitable for human brain) in response to user input

// when creating a good interface, implementation and user must agree on how something is inputted and recieved

// core, builder (majority of code), escape hatch code

// common UI code API has building and response code segmented, i.e. callbacks or event messages
// also use retained-mode, i.e. individually manage widget lifetime to add/remove from heirarchy

// NOTE(Ryan): Can simplify code by making a parameter to all functions a global
// immediate mode has all this in one spot, with widget hierarchy constructed on every frame (instead of stateful tree)
// we are flexible to mutations, and display + interaction code together

//UI_Widget *parent = ...;
//UI_PushParent(parent);
//if(UI_Button("Foo"))
//{
//  // clicked
//}
//if(UI_Button("Bar"))
//{
//  // clicked
//}
//UI_PopParent(parent);

// indentation hierarchy acheived with stack
// hierarchy of widgets, not layouts
// widget rendering in IM is deferred; 
// require frame of delay to perform offline (given all data, solve problem in one) autolayout

#endif

#define UI DEFER_LOOP(ui_begin(), ui_end(), UNIQUE_INT)

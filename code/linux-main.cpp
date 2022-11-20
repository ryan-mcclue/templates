// SPDX-License-Identifier: zlib-acknowledgement

#define STB_SPRINTF_IMPLEMENTATION 1
#include "stb/stb_sprintf.h"

#include "base.h"

#include <unistd.h>
#include <sys/mman.h>

INTERNAL u64
linux_get_page_size(void)
{
  u64 result = 0;
  
  // TODO(Ryan): Investigate obtaining POSIX information with sysconf() and related functions
  result = (u64)sysconf(_SC_PAGESIZE);

  return result;
}

#define MEM_DEFAULT_RESERVE_SIZE GB(1)
#define MEM_COMMIT_BLOCK_SIZE KB(4)
#define MEM_DECOMMIT_THRESHOLD KB(64)

typedef struct MemArena MemArena;
struct MemArena
{
  // I believe these are for growable arenas? 
  MemArena *first; // tree node first child?
  MemArena *last; // tree node last child?
  MemArena *next; // linked list?
  MemArena *prev; // linked list?
  MemArena *parent; // tree node root? 
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

INTERNAL void
mem_arena_set_pos_back(MemArena *arena, u64 pos)
{
  u64 clamped_pos = CLAMP_BOTTOM(sizeof(*arena), pos);

  if (arena->pos > clamped_pos)
  {
    arena->pos = clamped_pos;

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

#define MEM_ARENA_PUSH_ARRAY(a,T,c)     (T*)mem_arena_push((a), sizeof(T)*(c))
#define MEM_ARENA_PUSH_ARRAY_ZERO(a,T,c) (T*)mem_arena_push_zero((a), sizeof(T)*(c))
#define MEM_ARENA_POP_ARRAY(a,T,c) mem_arena_pop((a), sizeof(T)*(c))

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

#define s8_lit(s) s8((u8 *)(s), sizeof(s) - 1)
#define s8_cstring(s) s8((u8 *)(s), strlen(s))
#define s8_varg(s) (int)(s).size, (s).str
// use like: \"%.*s\"", s8_varg(string)

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
load_entire_file(MemArena *arena, String8 file_name)
{
  String8 result = ZERO_STRUCT;

  MemArenaTemp scratch = mem_arena_scratch_get(&arena, 1);
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

  mem_arena_scratch_release(&scratch);

  return result;
}

typedef struct Node Node;
struct Node
{
  Node *prev;
  Node *next;

  u32 x;
  u32 y;
};

typedef struct NodeList NodeList;
struct NodeList
{
  Node *first;
  Node *last;
  u64 node_count;
};

// ParseNodeSet -> TokenFromString 

typedef struct String8List String8List;
struct String8List
{
  String8 *first;
  String8 *last;
  u64 node_count;
  u64 total_size;
};


typedef struct State State;
struct State
{
  MemArena *arena;
  MemArena *frame_arenas[2];
  b32 should_quit;
  u32 frame_idx;
  //APP_Window *first_window;
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

GLOBAL MemArena *linux_mem_arena_perm = NULL;

int
main(int argc, char *argv[])
{
  IGNORED(argc); IGNORED(argv);

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
  // read binary path with readlink("/proc/self/exe")?
  
  // app init
  MemArena *arena = mem_arena_allocate(GB(16));
  state = PushArrayZero(arena, APP_State, 1);
  state->arena = arena;
  for(int i = 0; i < ArrayCount(state->frame_arenas); i += 1)
  {
    state->frame_arenas[i] = M_ArenaAlloc(Gigabytes(16));
  }

  // renderer init
  // find refresh rate
  while (state->want_to_run)
  {
    update_and_render();
  }
  
  return 0;
}

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
/*
Most ideas are bad, realise this uncomfortable truth
So what’s the solution? In my mind, it has to do with directly engaging with this uncomfortable reality. 
And, in doing so, ideas and projects will be shaped by reality, rather than divorced from it. 
It begins with simply asking the question—“are you measuring your project against markets—against the perceptions and values of others—or aren’t you?”

GENERAL PRINCIPLE: YOU GET WHAT YOU MEASURE

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

enum UI_SizeKind
{
  UI_SizeKind_Null,
  UI_SizeKind_Pixels,
  UI_SizeKind_TextContent,
  UI_SizeKind_PercentOfParent,
  UI_SizeKind_ChildrenSum,
};

struct UI_Size
{
  UI_SizeKind kind;
  F32 value;

  // percentage of how close to perfect pixel size willing to go, 
  // 1.0 would be only allowing entire perfect pixel size
  F32 strictness; 
};

enum Axis2
{
  Axis2_X,
  Axis2_Y,
  Axis2_COUNT
};

struct UI_Widget
{
  // ...
  UI_Size semantic_size[Axis2_COUNT];
  // ...
};

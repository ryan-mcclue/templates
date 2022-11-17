// SPDX-License-Identifier: zlib-acknowledgement

#include "base.h"

#include <unistd.h>
#include <sys/mman.h>

struct String8
{
  u8 *str;
  u64 size;
};

INTERNAL String8
s8(u8 *str, u64 size)
{
  String8 result = {};

  result.str = str;
  result.size = size;

  return result;
}

#define S8_LIT(s) s8((u8 *)s, sizeof(s) - 1)


INTERNAL u64
linux_get_page_size(void)
{
  u64 result = 0;
  
  // TODO(Ryan): Investigate obtaining POSIX information with sysconf() and related functions
  result = (u64)sysconf(_SC_PAGESIZE);

  return result;
}

#define MEM_DEFAULT_RESERVE_SIZE GB(1)
#define MEM_COMMIT_BLOCK_SIZE MB(64)

struct MemArena
{
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
  u64 page_snapped_size = round_to_nearest(size, linux_get_page_size());
  b32 result = (mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0);
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
  ThreadContext result = {};

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

// from caller's perspective might have scratch but calle view as permanent
// give requestor ability to specify other arena's its using for permanent allocation
// so calle can say it wants another scratch that it doesn't want to conflict with the another arena it's treating as permanent (which is scratch from the caller's perspective)

// As long as only a single persistent arena is present at any point in any codepath, 
// you will not need more than two scratch arenas.
// Will alternate between the two scratches for arbitrarily deep call stacks



int
main(int argc, char *argv[])
{
  IGNORED(argc); IGNORED(argv);

  ThreadContext tcx = thread_context_create();
  thread_context_set(&tcx);

  String8 s = S8_LIT("hello world");

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

struct TreeNode
{
  TreeNode *first_child;
  TreeNode *last_child;
  TreeNode *next_sibling;
};

struct Node
{
  Node *next;
  Node *prev;

  int x;
};

INTERNAL void
dll_push_front(void *first, void *last, void *node)
{
  if (first == NULL) 
  {
    first = last = node;
    node->next = node->prev = NULL;
  } 
  else 
  {
    node->prev = NULL;
    node->next = first;
    first->prev = node;
    first = node;
  }
}

INTERNAL void
dll_push_back(void *first, void *last, void *node)
{
  if (first == NULL) 
  {
    first = last = node;
    node->next = node->prev = NULL;
  } 
  else 
  {
    node->prev = last;
    node->next = NULL;
    last->next = node;
    last = node;
  }
}

INTERNAL void
dll_remove(void *first, void *last, void *node)
{
  if (node == first) 
  {
    if (first == last) 
    {
      first = last = NULL;
    } 
    else 
    {
      first = first->next;
      first->prev = NULL;
    }
  } 
  else if (node == last) 
  {
    last = last->prev;
    last->next = NULL;
  } 
  else 
  {
    node->next->prev = node->prev;
    node->prev->next = node->next;
  }
}

// IMPORTANT(Ryan): Macro wrapper necessary for C++ version as no implicit void * casts
#define DLL_PUSH_FRONT(first, last, node, type) \
  dll_push_front((type *)(first), (type *)(last), (type *)(node))
#define DLL_PUSH_BACK(first, last, node, type) \
  dll_push_back((type *)(first), (type *)(last), (type *)(node))
#define DLL_REMOVE(first, last, node, type) \
  dll_remove((type *)(first), (type *)(last), (type *)(node))


INTERNAL void
sll_queue_push(void *first, void *last, void *node)
{
  if (first == NULL) 
  {
    first = last = node;
    node->next = NULL;
  } 
  else 
  {
    node->next = first;
    first = node;
  }
}

INTERNAL void *
sll_queue_pop(void *first, void *last)
{
  void *result = first;

  if (first == last)
  {
    first = last = NULL;
  }
  else
  {
    first = first->next;
  }

  return result;
}

#define SLL_QUEUE_PUSH(first, last, node, type) \
  sll_queue_push((type *)(first), (type *)(last), (type *)(node))
#define SLL_QUEUE_POP(first, last, type) \
  (type *)sll_queue_pop((type *)(first), (type *)(last))

INTERNAL void
sll_stack_push(void *first, void *node)
{
  node->next = first;
  first = node;
}

INTERNAL void *
sll_stack_pop(void *first)
{
  void *result = first;

  if (first != NULL)
  {
    first = first->next;
  }

  return result;
}

#define SLL_STACK_PUSH(first, node, type) \
  sll_stack_push((type *)(first), (type *)(node))
#define SLL_STACK_POP(first, type) \
  (type *)sll_stack_pop((type *)(first))




// UIs exist to transfer information between user and program (decide what is useful information)
// So, when making a UI decision, if requires less information to be sent, then a good one (e.g. button press over typing a long string)
// Also, how quickly the user can enter this information is important to

// use well known design elements, e.g. buttons, boxes
// provide usage hints, e.g. buttons embossed, dynamically animate (in time suitable for human brain) in response to user input

// when creating a good interface, implementation and user must agree on how something is inputted and recieved

// core, buildler (majority of code), escape hatch code

// common UI code API has building and response code segmented, i.e. callbacks or event messages
// also use retained-mode, i.e. individually manage widget lifetime to add/remove from heirarchy

// immediate mode has all this in one spot, with widget hierarchy constructed on every frame
// indentation hierarchy acheived with stack
// hierarchy of widgets, not layouts
// widget rendering in IM is deferred; require frame of delay to perform offline (given all data, solve problem in one) autolayout

#endif

// SPDX-License-Identifier: zlib-acknowledgement

#define LCOV_EXCL_LINE
#define LCOV_EXCL_START
#define LCOV_EXCL_STOP

// use i+=1 in for loop syntax?
// use separate line for each for loop

// TODO(Ryan): Investigate obtaining POSIX information with sysconf() and related functions

// .h includes
#include "base-inc.h"

// .c includes

GLOBAL MemArena *linux_mem_arena_perm = NULL;

typedef struct TreeNode TreeNode;
struct TreeNode
{
  TreeNode *next;
  TreeNode *prev;
  TreeNode *parent;
  TreeNode *first_child;
  TreeNode *last_child;
};

int
main(int argc, char *argv[])
{
  IGNORED(argc); IGNORED(argv);

  // IMPORTANT(Ryan): For switch statements, put default at top 

  // TODO(Ryan): Drawing UI: arcane/game/source/arc/ui.ui.c: DrawEditorUI(); 

  // thread init
  ThreadContext tcx = thread_context_create();
  thread_context_set(&tcx);

  // os init
  linux_mem_arena_perm = mem_arena_allocate(GB(1)); 

  String8List linux_cmd_line = ZERO_STRUCT;

  // record timer start here
  // command line arguments
  for (i32 i = 0; i < argc; i += 1)
  {
    String8 arg = str8_cstring((u8 *)argv[i]);
    str8_list_push(linux_mem_arena_perm, &linux_cmd_line, arg);
  }
  // binary dir: readlink("/proc/self/exe");
  // cwd: getcwd();
  
  // IMPORTANT(Ryan): Instead of choosing the right data structure
  // design the right data structure for the job, i.e. data structure composition
  // linked lists allow for seamless data structure composition?
  
  return 0;
}


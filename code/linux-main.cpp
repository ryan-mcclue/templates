// SPDX-License-Identifier: zlib-acknowledgement

// .h includes
#include "base-inc.h"

// .c includes

GLOBAL MemArena *linux_mem_arena_perm = NULL;

INTERNAL ThreadContext
create_thread_local_temporary_arenas(void)
{
  ThreadContext result = ZERO_STRUCT;

  result = thread_context_create();
  thread_context_set(&result);

  return result;
}

int
main(int argc, char *argv[])
{
  IGNORED(argc); IGNORED(argv);

  // IMPORTANT(Ryan): For switch statements, put default at top 

  // NOTE(Ryan): Arena allocations
  linux_mem_arena_perm = mem_arena_allocate(GB(1)); 

  ThreadContext tcx = create_thread_local_temporary_arenas();

  //String8List linux_cmd_line = ZERO_STRUCT;

  // record timer start here
  // command line arguments
  //for (i32 i = 0; i < argc; i += 1)
  //{
  //  String8 arg = s8_cstring(argv[i]);
  //  s8_list_push(linux_mem_arena_perm, &linux_cmd_line, arg);
  //}

  // binary dir: readlink("/proc/self/exe");
  // cwd: getcwd();
  
  // IMPORTANT(Ryan): Instead of choosing the right data structure
  // design the right data structure for the job, i.e. data structure composition
  // linked lists allow for seamless data structure composition?
  
  LSAN_RUN();
  
  return 0;
}


/*
#define LCOV_EXCL_LINE
#define LCOV_EXCL_START
#define LCOV_EXCL_STOP

use i+=1 in for loop syntax?
use separate line for each for loop

// TODO(Ryan): Investigate obtaining POSIX information with sysconf() and related functions

typedef struct TreeNode TreeNode;
struct TreeNode
{
  TreeNode *next;
  TreeNode *prev;
  TreeNode *parent;
  TreeNode *first_child;
  TreeNode *last_child;
};
*/

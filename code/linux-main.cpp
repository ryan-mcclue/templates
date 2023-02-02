// SPDX-License-Identifier: zlib-acknowledgement

// .h includes
#include "base-inc.h"

#include <raylib.h>
// .c includes

#include <sys/stat.h>
#include <dlfcn.h>

GLOBAL MemArena *linux_mem_arena_perm = NULL;

INTERNAL u64 
linux_get_file_mod_time(String8 file_name)
{
  u64 result = 0;

  struct stat file_stat = ZERO_STRUCT;
  if (stat((char *)file_name.str, &file_stat) == 0)
  {
    result = (u64)file_stat.st_mtime;
  }

  return result;
}

int
main(int argc, char *argv[])
{
  IGNORED(argc); IGNORED(argv);

  // IMPORTANT(Ryan): For switch statements, put default at top 

  // NOTE(Ryan): Arena allocations
  linux_mem_arena_perm = mem_arena_allocate(GB(1)); 

  ThreadContext tctx = thread_context_create();
  thread_context_set(&tctx);

  MemArenaTemp mem_temp_arena = mem_arena_scratch_get(NULL, 0);

  i32 window_width = 1280;
  i32 window_height = 720;

  InitWindow(window_width, window_height, "Test");        
  SetTargetFPS(60);

  u64 last_app_reload_time = 0;
  String8 app_name = s8_lit("app.so");
  String8 app_temp_name = s8_lit("app-temp.so");
  void *app_lib = NULL;
  void (*app)(void) = NULL;

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(LITERAL(Color){0, 0, 0, 255}); 

    u64 app_mod_time = linux_get_file_mod_time(app_name);
    if (app_mod_time > last_app_reload_time)
    {
      if (app_lib != NULL) 
      {
        dlclose(app_lib);
      }

      s8_copy_file(mem_temp_arena.arena, app_name, app_temp_name);
      struct stat app_stat = ZERO_STRUCT;
      stat((char *)app_name.str, &app_stat);
      chmod((char *)app_temp_name.str, app_stat.st_mode);

      do {
        app_lib = dlopen((char *)app_temp_name.str, RTLD_NOW);
        u32 x = 10;
      } while (errno == 11);

      // 11 resource temporarily unavailable
      if (app_lib != NULL) 
      {
        app = (void (*)(void))dlsym(app_lib, "app");
        if (app != NULL)
        {
          last_app_reload_time = app_mod_time;
        }
        else
        {
          errno_log();
        }
      }
      else
      {
        errno_log();
      }
    }

    if (app != NULL)
    {
      app();
    }
    else
    {
      DrawText("Failed to load app.", 200, 200, 20, RED);
    }

    // mem_arena_pop

    EndDrawing();
  }

  CloseWindow();


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

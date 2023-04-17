// SPDX-License-Identifier: zlib-acknowledgement

// .h includes
#include "base-inc.h"

#include <raylib.h>
// .c includes

#include <sys/stat.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>

#include "app.h"

#include <syslog.h>

GLOBAL MemArena *linux_mem_arena_perm = NULL;

// TODO(Ryan): linux_run_command_block/fork()
/*
INTERNAL void
run_command_in_background(const char *cmd)
{
  pid_t pid = vfork();
  if (pid != -1)
  {
    if (pid == 0)
    {
      if (prctl(PR_SET_PDEATHSIG, SIGTERM) != -1)
      {
        execl("/bin/bash", "bash", "-c", cmd, NULL);

        EBP("Failed to execute background command in fork");
      }
      else
      {
        EBP("Failed to set death of background command when parent process terminates");
      }
  
      exit(127);
    }
  }
  else
  {
    EBP("Failed to fork to run background command");
  }
}
 */

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

// perhaps change to get_walltime_ms()
INTERNAL u64
linux_get_ms(void)
{
  u64 result = 0;

  struct timespec time_spec = {0};
  // not actually time since epoch, 1 jan 1970
  // rather time since some unspecified period in past
  clock_gettime(CLOCK_MONOTONIC_RAW, &time_spec);

  result = ((u32)time_spec.tv_sec * 1000) + (u32)((f32)time_spec.tv_nsec / 1000000.0f);

  return result;
}

INTERNAL u32
linux_get_seed_u32(void)
{
  u32 result = 0;

  ERRNO_ASSERT(getentropy(&result, sizeof(result)) != -1);

  return result;
}

int
main(int argc, char *argv[])
{
  IGNORED(argc); IGNORED(argv);

  // IMPORTANT(Ryan): For release, don't rely on LOG_LOCAL
  // In this case, use LOG_USER but only put error messages
  // For logging, put in own file with fopen()

  // syslog_r() for threadsafe
  openlog("log-file", LOG_PID | LOG_CONS, LOG_USER); // sets a prefix for log file messages
  setlogmask(LOG_UPTO(LOG_ERR));
  u32 val = 10;
  // syslog(LOG_INFO | LOG_LOCAL2, "started logging with %d", val);
  syslog(LOG_INFO, "started logging with %d", val); // TODO(Ryan): perhaps cleaner to use %m instead of strerror(errno)?
  closelog();
  // output in /var/log/syslog (or /var/log/messages)
  // Note that your syslogd configuration might not be set up to keep messages of log level LOG_INFO
  // syslogd can be configured to send logs to a server

  // want to log stacktrace on assertion/crash?
  // https://unix.stackexchange.com/questions/11953/make-a-log-file
  // https://stackify.com/syslog-101

  // /etc/rsyslog.conf?

  // IMPORTANT(Ryan): For switch statements, put default at top 

  // NOTE(Ryan): Arena allocations
  linux_mem_arena_perm = mem_arena_allocate(GB(1)); 

  ThreadContext tctx = thread_context_create();
  thread_context_set(&tctx);

  // TODO(Ryan): Add pools to memory arenas
  MemArenaTemp mem_arena_temp = mem_arena_scratch_get(NULL, 0);

  i32 window_width = 1280;
  i32 window_height = 720;

  InitWindow(window_width, window_height, "Test");        
  SetTargetFPS(60);

  // how different to argv[0]?
  // binary dir: readlink("/proc/self/exe");
  // char buf[256] = {0};
  // readlink("/proc/self/exe", buf, sizeof(buf));

  u32 cwd_path_size = KB(32);
  u8 *cwd_path_buffer = MEM_ARENA_PUSH_ARRAY_ZERO(linux_mem_arena_perm, u8, cwd_path_size);
  ERRNO_ASSERT(getcwd((char *)cwd_path_buffer, cwd_path_size) != NULL);

  String8List app_temp_name_list = ZERO_STRUCT;
  s8_list_push(linux_mem_arena_perm, &app_temp_name_list, s8_cstring(cwd_path_buffer));
  s8_list_push(linux_mem_arena_perm, &app_temp_name_list, LITERAL(s8_lit("/app-temp.so")));

  // IMPORTANT(Ryan): Absolute path name required for dlopen() as doesn't honour relative $CWD
  String8 app_temp_abs_path = s8_list_join(linux_mem_arena_perm, app_temp_name_list, NULL);

  u64 last_app_reload_time = 0;
  String8 app_name = s8_lit("app.so");
  void *app_lib = NULL;
  app_func app = NULL;

  AppState *app_state = MEM_ARENA_PUSH_STRUCT(linux_mem_arena_perm, AppState);
  app_state->ui_state.delta = 1.0f / 60.0f;

  Color clear_colour = {0, 0, 0, 255};

  // DrawRectangleGradientH(), DrawRectangleGradientEx()

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(clear_colour); 

    u64 app_mod_time = linux_get_file_mod_time(app_name);
    if (app_mod_time > last_app_reload_time)
    {
      if (app_lib != NULL) 
      {
        dlclose(app_lib);
      }

      s8_copy_file(mem_arena_temp.arena, app_name, app_temp_abs_path);
      struct stat app_stat = ZERO_STRUCT;
      stat((char *)app_name.str, &app_stat);
      chmod((char *)app_temp_abs_path.str, app_stat.st_mode);

      // TODO(Ryan): This will fail as it seems detects file change before can actually load.
      // So, will successfully load on subsequent calls
      // However, get brief flicker
      app_lib = dlopen((char *)app_temp_abs_path.str, RTLD_NOW);

      if (app_lib != NULL) 
      {
        app = (app_func)dlsym(app_lib, "app");
        if (app != NULL)
        {
          last_app_reload_time = app_mod_time;
        }
        else
        {
          errno_inspect();
          app = NULL;
        }
      }
      else
      {
        errno_inspect();
        app = NULL;
      }
    }

    if (app != NULL)
    {
      app_state->ms = linux_get_ms();

      app_state->window_width = GetScreenWidth();
      app_state->window_height = GetScreenHeight(); 

      app_state->ui_state.mouse_x = GetMouseX();
      app_state->ui_state.mouse_y = GetMouseY();
      app_state->ui_state.mouse_is_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

      app(app_state, linux_mem_arena_perm, &mem_arena_temp);
    }
    else
    {
      //DrawText("Failed to load app.", 200, 200, 20, RED);
    }

    mem_arena_scratch_release(mem_arena_temp);

    EndDrawing();
  }

  CloseWindow();

  
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

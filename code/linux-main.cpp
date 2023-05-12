// SPDX-License-Identifier: zlib-acknowledgement

// TODO(Ryan): Create a base-linux.c
#include "base-inc.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <sys/stat.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>

#include "app.h"

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

  if (getentropy(&result, sizeof(result)) == -1)
  {
    WARN("getentropy failed", strerror(errno));
  }

  return result;
}

INTERNAL b32
linux_was_launched_by_gdb(void)
{
  b32 result = false;

  pid_t ppid = getppid(); 
  char path[64] = {0};
  char buf[64] = {0};
  snprintf(path, sizeof(path), "/proc/%d/exe", ppid);
  readlink(path, buf, sizeof(buf));
  if (strncmp(buf, "/usr/bin/gdb", sizeof("/usr/bin/gdb")) == 0)
  {
    result = true;
  }

  return result;
}

INTERNAL u32
sdl2_get_refresh_rate(SDL_Window *window)
{
  u32 result = 60;

  SDL_DisplayMode display_mode = ZERO_STRUCT;

  i32 display_index = SDL_GetWindowDisplayIndex(window);
  if (SDL_GetCurrentDisplayMode(display_index, &display_mode) == 0)
  {
    // IMPORTANT(Ryan): This doesn't handle a variable refresh rate monitor
    if (display_mode.refresh_rate > 0)
    {
      result = (u32)display_mode.refresh_rate;
    }
  }
  else
  {
    WARN("Failed to retrieve current refresh rate. Defaulting to 60", SDL_GetError());
  }

  return result;
}

INTERNAL void
sdl2_map_window_mouse_to_render_mouse(Renderer *renderer, Input *input)
{
  i32 window_mouse_x, window_mouse_y = 0;
  u32 mouse_state = SDL_GetMouseState(&window_mouse_x, &window_mouse_y);

  if (mouse_state & SDL_BUTTON(1))
  {
    if (input->mouse_pressed)
    {
      input->mouse_pressed = false;
      input->mouse_held = true;
    }
    else
    {
      input->mouse_pressed = true;
      input->mouse_held = false;
    }
  }
  else
  {
    input->mouse_pressed = false;
    input->mouse_held = false;
  }

  f32 desired_window_width = (f32)renderer->window_height * ((f32)renderer->render_width / (f32)renderer->render_height);
  f32 desired_window_height = (f32)renderer->window_width * ((f32)renderer->render_height / (f32)renderer->render_width);

  f32 mouse_x_norm = (f32)window_mouse_x / (f32)renderer->window_width;
  f32 mouse_y_norm = (f32)window_mouse_y / (f32)renderer->window_height;

  if (desired_window_width > (f32)renderer->window_width)
  {
    // NOTE(Ryan): Width constrained, i.e top and bottom black bars
    f32 black_bar_height = ((f32)renderer->window_height - desired_window_height) * 0.5f;

    if (window_mouse_y < black_bar_height)
    {
      window_mouse_y = 0.0f;
    }
    else if (window_mouse_y > black_bar_height + desired_window_height)
    {
      window_mouse_y = (i32)renderer->window_height;
    }
    else
    {
      window_mouse_y -= black_bar_height;
    }

    mouse_y_norm = (f32)window_mouse_y / desired_window_height;
  }
  else
  {
    // NOTE(Ryan): Height constrained, i.e left and right black bars
    f32 black_bar_width = ((f32)renderer->window_width - desired_window_width) * 0.5f;

    if (window_mouse_x < black_bar_width)
    {
      window_mouse_x = 0.0f;
    }
    else if (window_mouse_x > black_bar_width + desired_window_width)
    {
      window_mouse_x = (i32)renderer->window_width;
    }
    else
    {
      window_mouse_x -= black_bar_width;
    }

    mouse_x_norm = (f32)window_mouse_x / desired_window_width;
  }

  input->mouse_x = round_f32_to_i32(mouse_x_norm * renderer->render_width); 
  input->mouse_y = round_f32_to_i32(mouse_y_norm * renderer->render_height); 
}

int
main(int argc, char *argv[])
{
  IGNORED(argc); IGNORED(argv);

  global_debugger_present = linux_was_launched_by_gdb();

#if defined(MAIN_DEBUG)
  // IMPORTANT(Ryan): Content in /etc/rsyslog.d/app.conf:
  //  if $programname == 'app' and $syslogseverity-text == 'error' then /var/log/app.log
  // And subsequent daemon restart: $(sudo service rsyslog restart)
  // global(parser.escapeControlCharactersOnReceive="off") 
  openlog("app", LOG_CONS | LOG_PERROR, LOG_USER);
  setlogmask(LOG_UPTO(LOG_DEBUG));
#else
  openlog("app", 0, LOG_USER);
  setlogmask(LOG_UPTO(LOG_WARNING));
#endif

  // IMPORTANT(Ryan): For switch statements, put default at top 

  // NOTE(Ryan): Arena allocations
  linux_mem_arena_perm = mem_arena_allocate(GB(1)); 

  ThreadContext tctx = thread_context_create();
  thread_context_set(&tctx);

  // TODO(Ryan): Add pools to memory arenas
  MemArenaTemp mem_arena_temp = mem_arena_scratch_get(NULL, 0);

  i32 render_width = 1280;
  i32 render_height = 720;
  i32 window_width = render_width;
  i32 window_height = render_height;


  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    FATAL_ERROR("Failed to initialise all SDL2 subsystems.", SDL_GetError(), "");
  }

  SDL_version sdl2_version_compiled = ZERO_STRUCT;
  SDL_VERSION(&sdl2_version_compiled);
  DBG("Compiled with SDL2 %u.%u.%u\n", 
       sdl2_version_compiled.major, sdl2_version_compiled.minor, sdl2_version_compiled.patch);

  SDL_version sdl2_version_linked = ZERO_STRUCT;
  SDL_GetVersion(&sdl2_version_linked);
  DBG("Linked with SDL2 %u.%u.%u\n", 
       sdl2_version_linked.major, sdl2_version_linked.minor, sdl2_version_linked.patch);

  SDL_Window *window = SDL_CreateWindow("app", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        window_width, window_height, SDL_WINDOW_RESIZABLE);

  if (window == NULL)
  {
    FATAL_ERROR("Failed to create SDL2 window.", SDL_GetError(), "");
  }

  SDL_Renderer *sdl2_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (sdl2_renderer == NULL)
  {
    FATAL_ERROR("Failed to create SDL2 renderer.", SDL_GetError(), "");
  }

  if (SDL_RenderSetLogicalSize(sdl2_renderer, render_width, render_height) != 0)
  {
    FATAL_ERROR("Failed to set SDL2 renderer size.", SDL_GetError(), "");
  }

  if (SDL_SetRenderDrawBlendMode(sdl2_renderer, SDL_BLENDMODE_BLEND) != 0)
  {
    FATAL_ERROR("Failed to set SDL2 renderer blend mode.", SDL_GetError(), "");
  }

  int sdl2_image_flags = IMG_INIT_PNG;
  if ((IMG_Init(sdl2_image_flags) & sdl2_image_flags) == 0)
  {
    FATAL_ERROR("Failed to initialise SDL2_image.", IMG_GetError(), "");
  }

  u32 cwd_path_size = KB(32);
  u8 *cwd_path_buffer = MEM_ARENA_PUSH_ARRAY_ZERO(linux_mem_arena_perm, u8, cwd_path_size);
  if (getcwd((char *)cwd_path_buffer, cwd_path_size) == NULL)
  {
    FATAL_ERROR("Failed to get current working directory.", strerror(errno), "");
  }

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
  // IMPORTANT(Ryan): Required to be called again if in fullscreen mode
  u32 refresh_rate = sdl2_get_refresh_rate(window);
  app_state->delta = 1.0f / refresh_rate;

  app_state->rand_seed = linux_get_seed_u32();

  app_state->debugger_present = global_debugger_present;

  Renderer *renderer = MEM_ARENA_PUSH_STRUCT(linux_mem_arena_perm, Renderer);
  renderer->renderer = sdl2_renderer;
  renderer->render_width = (u32)render_width;
  renderer->render_height = (u32)render_height;
  renderer->window_width = (u32)window_width;
  renderer->window_height = (u32)window_height;

  Input *input = MEM_ARENA_PUSH_ARRAY_ZERO(linux_mem_arena_perm, Input, 1);

  b32 want_to_run = true;
  while (want_to_run)
  {
    SDL_SetRenderDrawColor(sdl2_renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(sdl2_renderer);

    SDL_Event sdl2_event = ZERO_STRUCT;
    while (SDL_PollEvent(&sdl2_event) != 0)
    {
      switch (sdl2_event.type)
      {
        default: break;
        case SDL_QUIT:
        {
          want_to_run = false;
        } break;
        case SDL_KEYDOWN:
        {
          if (sdl2_event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
          {
            want_to_run = false;
          }

          if (sdl2_event.key.repeat == 0 && 
              sdl2_event.key.keysym.scancode == SDL_SCANCODE_D &&
              sdl2_event.key.keysym.mod & KMOD_LSHIFT)
          {
            app_state->debug_overlay = !app_state->debug_overlay;
          }

          if (sdl2_event.key.keysym.scancode == SDL_SCANCODE_F &&
              sdl2_event.key.keysym.mod & KMOD_LSHIFT)
          { 
            // TODO(Ryan): 'Real' fullscreen that changes resolution with SDL_WINDOW_FULLSCREEN
            // Would also require updating refresh rate
            if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
            {
              SDL_SetWindowFullscreen(window, 0);
            }
            else
            {
              SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
          }
        } break;
        case SDL_KEYUP:
        {
          if (sdl2_event.key.keysym.scancode == SDL_SCANCODE_S)
          {
            input->bullet_fired = true;
          }
        } break;
        case SDL_MOUSEBUTTONUP:
        {
          if (sdl2_event.button.button == SDL_BUTTON_LEFT)
          {
            input->mouse_clicked = true;
          }
        } break;
      }

    }

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
          WARN("Failed to load app from shared library.", strerror(errno));
          app = NULL;
        }
      }
      else
      {
        WARN("Failed to open app shared library.", strerror(errno));
        app = NULL;
      }
    }

    if (app != NULL)
    {
      app_state->ms = linux_get_ms();

      SDL_GetWindowSize(window, &window_width, &window_height);
      renderer->window_width = (u32)window_width;
      renderer->window_height = (u32)window_height;

      const u8 *keyboard_state = SDL_GetKeyboardState(NULL);
      input->move_left = keyboard_state[SDL_SCANCODE_LEFT];
      input->move_right = keyboard_state[SDL_SCANCODE_RIGHT];
      input->move_up = keyboard_state[SDL_SCANCODE_SPACE];

      sdl2_map_window_mouse_to_render_mouse(renderer, input);

      // fps calculation?
      // vsync more accurate than OS scheduler granularity
      // IMPORTANT(Ryan): Still technically variable-delta-time, so not deterministic
      app(app_state, renderer, input, linux_mem_arena_perm);

      input->bullet_fired = false;
      input->mouse_clicked = false;
    }
    else
    {
      //DrawText("Failed to load app.", 200, 200, 20, RED);
    }

    mem_arena_scratch_release(mem_arena_temp);

    // NOTE(Ryan): Double buffering to prevent 'glitching' also applicable in embedded
    SDL_RenderPresent(sdl2_renderer);
  }

  SDL_Quit();

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

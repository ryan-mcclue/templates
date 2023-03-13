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

  // IMPORTANT(Ryan): For switch statements, put default at top 

  // NOTE(Ryan): Arena allocations
  linux_mem_arena_perm = mem_arena_allocate(GB(1)); 

  ThreadContext tctx = thread_context_create();
  thread_context_set(&tctx);

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

  UICache *cache = MEM_ARENA_PUSH_STRUCT(linux_mem_arena_perm, UICache);
  // preserve previous elements for animation? 
	// stable_table_init(UI_Key, UI_Box, &ui_cache->cache, 64);

  // load font
	// UI_ClippingRectPush(ui_cache, rect_init(0, 0, window->width, window->height));
	// UI_FontPush(ui_cache, &ui_cache->default_font);
	// UI_BoxColorPush(ui_cache, 0x111111FF);
	// UI_HotColorPush(ui_cache, 0x131313FF);
	// UI_ActiveColorPush(ui_cache, 0x131313FF);
	// UI_EdgeColorPush(ui_cache, 0x9A5EBDFF);
	// UI_TextColorPush(ui_cache, 0xFFAAFFFF);
	// UI_RoundingPush(ui_cache, 5.f);
	// UI_EdgeSoftnessPush(ui_cache, 2.f);
	// UI_EdgeSizePush(ui_cache, 2.f);
	// UI_PrefWidthPush(ui_cache, UI_Pixels(100));
	// UI_PrefHeightPush(ui_cache, UI_Pixels(100));
	// UI_LayoutAxisPush(ui_cache, axis2_y);


  AppState *app_state = MEM_ARENA_PUSH_STRUCT(linux_mem_arena_perm, AppState);
  // TODO(Ryan): just call getWindowWidth() to handle resizing?
  app_state->window_width = (f32)window_width;
  app_state->window_height = (f32)window_height;

  app_state->font = LoadFont("");
  ASSERT(app_state->font.texture.id != 0);
  app_state->font_size = 34.0f; 
  app_state->font_scale = 1.0f; 

  // boxes have hashes computed by builder code? (distinction between builder code and ...?)

  // 'slack' is how much size willing to give to satisfy layout constraints (used in combination with excess size in parent)

  // stack based styling doesn't easily allow for modifying subtree styles 
  // For example, we want to style all buttons with the text “OK” that are children of boxes named “dialog”. 
  
  // optional arguments: one arg for struct, other arg for flags to obtain from this struct

  // styling pass: 

  // these are the 'per-box' styling properties that can be overridden
  // 'size' is flexible constraint, e.g. size of the box’s text, as a number of pixels, etc.
  // 'layout' is flexible, e.g. layout axis (vertical or horizontal), the spacing of boxes along the layout axis, etc.
  // 'floating' indicates if not be included in layout ('floating_target' is location)
  // 'animation' controls property transition to new styles, i.e. doesn't happen immediately ('animation_mask' is what properties)
  app_state->border_width = 2;
		.borderWidth = 2,
		.borderWidthHover = 2,
		.borderWidthActive = 4,
		.viewBorderWidth = 2,
		.frameRoundness = 0,
		.elementRoundess = 0,
		.scrollbarRoundness = 1,
		.margin = 10,
		.titleHeight = 80,

		.colors = {
			[MP_GUI_STYLE_COLOR_BG] = {0.2, 0.2, 0.2, 1},
			[MP_GUI_STYLE_COLOR_BG_HOVER] = {0.2, 0.2, 0.2, 1},
			[MP_GUI_STYLE_COLOR_BG_ACTIVE] = {0.2, 0.2, 0.2, 1},

			[MP_GUI_STYLE_COLOR_BORDER] = {1, 1, 1, 1},
			[MP_GUI_STYLE_COLOR_BORDER_HOVER] = {1, 1, 1, 1},
			[MP_GUI_STYLE_COLOR_BORDER_ACTIVE] = {1, 1, 1, 1},

			[MP_GUI_STYLE_COLOR_ELT] = {0.5, 0.5, 0.5, 1},
			[MP_GUI_STYLE_COLOR_ELT_HOVER] = {0.5, 0.5, 0.5, 1},
			[MP_GUI_STYLE_COLOR_ELT_ACTIVE] = {0.1, 0.1, 0.1, 1},

			[MP_GUI_STYLE_COLOR_TEXT] = {1, 1, 1, 1},
			[MP_GUI_STYLE_COLOR_TEXT_HOVER] = {1, 1, 1, 1},
			[MP_GUI_STYLE_COLOR_TEXT_ACTIVE] = {1, 1, 1, 1},
			[MP_GUI_STYLE_COLOR_TEXT_SELECTION_BG] = {0, 0, 1, 1},
			[MP_GUI_STYLE_COLOR_TEXT_SELECTION_FG] = {1, 1, 1, 1},

			[MP_GUI_STYLE_COLOR_VIEW_BG] = {0.3, 0.3, 0.3, 1},
			[MP_GUI_STYLE_COLOR_VIEW_TITLE_BG] = {0.1, 0.1, 0.1, 1},
			[MP_GUI_STYLE_COLOR_VIEW_TITLE_TEXT] = {1, 1, 1, 1},
			[MP_GUI_STYLE_COLOR_VIEW_BORDER] = {1, 1, 1, 1},

			[MP_GUI_STYLE_COLOR_SCROLLBAR] = {0.2, 0.2, 0.2, 1},
			[MP_GUI_STYLE_COLOR_SCROLLBAR_ACTIVE] = {0.1, 0.1, 0.1, 1},
		}
	};


  Color clear_colour = {0, 0, 0, 255};

// DrawRectangleGradientH(), DrawRectangleGradientEx()
// MeasureTextEx()
//    positions[i].x = screenWidth/2.0f - MeasureTextEx(fonts[i], messages[i], fonts[i].baseSize*2.0f, (float)spacings[i]).x/2.0f;
//    positions[i].y = 60.0f + fonts[i].baseSize + 45.0f*i; 
//    DrawTextEx()

  // IsKeyDown(); IsMouseButtonPressed();
  // GetMouseWheelMove() * scroll_speed;
  // SetMouseCursor(MOUSE_CURSOR_IBEAM);
  //      if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
  //      else mouseOnText = false;

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(clear_colour); 

    MemArenaTemp mem_temp_arena = mem_arena_scratch_get(NULL, 0);

    u64 app_mod_time = linux_get_file_mod_time(app_name);
    if (app_mod_time > last_app_reload_time)
    {
      if (app_lib != NULL) 
      {
        dlclose(app_lib);
      }

      s8_copy_file(mem_temp_arena.arena, app_name, app_temp_abs_path);
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


      app(app_state);
    }
    else
    {
      //DrawText("Failed to load app.", 200, 200, 20, RED);
    }

    mem_arena_scratch_release(mem_temp_arena);

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

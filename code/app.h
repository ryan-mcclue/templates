// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(APP_H)
#define APP_H

struct SpaceObject
{
  Vec2F32 position, velocity, size;
  f32 angle; // TODO(Ryan): Replace with x, y axis?
};

IGNORE_WARNING_PADDED()
typedef struct AppState AppState;
struct AppState
{
  b32 is_initialised;

  f32 delta;

  u64 ms;

  u32 rand_seed;

  SpaceObject asteroid;
  SpaceObject player;
};
IGNORE_WARNING_POP()

IGNORE_WARNING_PADDED()
typedef struct Renderer Renderer;
struct Renderer
{
  SDL_Renderer *renderer;
  u32 window_width, window_height;
  u32 render_width, render_height;
};
IGNORE_WARNING_POP()

IGNORE_WARNING_PADDED()
typedef struct Input Input;
struct Input
{
  b32 move_left, move_right, move_up; 
};
IGNORE_WARNING_POP()

typedef void (*app_func)(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena, MemArenaTemp *temp_arena);

EXPORT void
app(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena, MemArenaTemp *temp_arena);

#endif

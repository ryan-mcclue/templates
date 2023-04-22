// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(APP_H)
#define APP_H

struct SpaceObject
{
  Vec2F32 position, velocity;
  f32 angle;
  f32 scale;

  Vec2F32 *points;
  u32 num_points;
};

struct SpaceObjectDLL
{
  SpaceObject object;
  SpaceObjectDLL *next, *prev;
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

  SpaceObjectDLL *first_free_bullet;
  SpaceObjectDLL *first_bullet;
  SpaceObjectDLL *last_bullet;

  u32 map_width, map_height;
  u8 *map;
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
  b32 move_left, move_right, move_up, bullet_fired;
};
IGNORE_WARNING_POP()

typedef void (*app_func)(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena);

EXPORT void
app(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena);

#endif

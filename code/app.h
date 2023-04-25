// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(APP_H)
#define APP_H

struct SpaceObject
{
  Vec2F32 position, velocity, acceleration;
  f32 angle;
  
  f32 radius; // collision
  b32 stable; // not moving (so know when to pass control over?)

  f32 scale;
  Vec2F32 *points;
  u32 num_points;
};

struct Entity
{
  Vec2F32 position, velocity, acceleration;
  f32 angle;
  
  f32 radius; // collision
  b32 stable; // not moving (so know when to pass control over?)

  f32 scale;
  Vec2F32 *points;
  u32 num_points;

  Entity *next;
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

  Entity *first_test_entity;
  Entity *last_test_entity;

  Vec2F32 camera;

  Vec2F32 grid_offset;
  Vec2F32 grid_pan; 

  Vec2F32 grid_scale;
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
  f32 mouse_x, mouse_y; 
  b32 mouse_clicked;
  b32 mouse_pressed;
  b32 mouse_held;
};
IGNORE_WARNING_POP()

typedef void (*app_func)(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena);

EXPORT void
app(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena);

#endif

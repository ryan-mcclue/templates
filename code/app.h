// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(APP_H)
#define APP_H

typedef u8 MAP_TYPE;
enum
{
  MAP_TYPE_EMPTY,
  MAP_TYPE_LAND,
};

IGNORE_WARNING_PADDED()
typedef struct AppState AppState;
struct AppState
{
  b32 is_initialised;

  i32 window_width, window_height;

  SDL_Renderer *renderer;
  u32 render_width, render_height;

  f32 delta;

  u64 ms;

  u32 rand_seed;

  u32 map_width, map_height;
  u8 *map;
};
IGNORE_WARNING_POP()

typedef void (*app_func)(AppState *state, MemArena *perm_arena, MemArenaTemp *temp_arena);

EXPORT void
app(AppState *state, MemArena *perm_arena, MemArenaTemp *temp_arena);

#endif

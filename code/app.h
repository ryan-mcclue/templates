// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(APP_H)
#define APP_H

#include "ui.h"

IGNORE_WARNING_PADDED()
typedef struct AppState AppState;
struct AppState
{
  b32 is_initialised;

  f32 window_width, window_height;
  u64 ms;

  UIState ui_state;
};
IGNORE_WARNING_POP()

typedef void (*app_func)(AppState *state, MemArena *perm_arena, MemArenaTemp *temp_arena);

EXPORT void
app(AppState *state, MemArena *perm_arena, MemArenaTemp *temp_arena);

#endif

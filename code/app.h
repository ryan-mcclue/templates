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

  UICache *ui_cache;

  u64 t;
  u32 x, y;

  u64 ms;
};
IGNORE_WARNING_POP()

typedef void (*app_func)(AppState *state);

EXPORT void
app(AppState *state);

#endif

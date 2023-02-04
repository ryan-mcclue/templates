// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(APP_H)
#define APP_H

IGNORE_WARNING_PADDED()
typedef struct AppState AppState;
struct AppState
{
  b32 is_initialised;

  i32 width, height;

  u64 t;
  u32 x, y;

  u64 ms;
};
IGNORE_WARNING_POP()

EXPORT void
app(AppState *state);

#endif

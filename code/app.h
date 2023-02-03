// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(APP_H)
#define APP_H

IGNORE_WARNING_PADDED()
typedef struct AppState AppState;
struct AppState
{
  u64 t;
  b32 is_initialised;
  u32 x, y;
};
IGNORE_WARNING_POP()

EXPORT void
app(AppState *state);

#endif

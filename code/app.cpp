// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <raylib.h>

#include "app.h"

EXPORT void
app(AppState *state)
{
  if (!state->is_initialised)
  {
    state->x = 10;
    state->y = 20;
    state->t = 30;

    state->is_initialised = true;
  }

  DrawCircle(400, 500, 80.0f, YELLOW);
}

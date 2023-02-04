// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <raylib.h>

#include "app.h"

void
draw_rect(Vec2F32 min, Vec2F32 max, Vec4F32 colour)
{
  Vector2 position = {min.x, min.y};
  Vector2 size = {max.x - min.x, max.y - min.y};
  Vec4F32 scaled = vec4_f32_mul(colour, 255.0f);
  Color color = {(u8)color.r, (u8)color.g, (u8)color.b, (u8)color.a};
  DrawRectangleV(position, size, color);
}

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

  DrawRectangle(0, 0, state->width, state->height, DARKBLUE);
  DrawRectangle(0, state->height - 150, state->width, state->height, GREEN);

  // TODO(Ryan): Perhaps use i32 whenever used in calculation for drawing
  i32 snow_num_x = 10, snow_width = 10, snow_gutter_x = 30;
  i32 snow_num_y = 20, snow_height = 10, snow_gutter_y = 50;

#if 0
  for (u32 y = 0; y < state->height; ++y)
  {
    for (u32 x = 0; x < state->width; ++x)
    {
      i32 snow_coord_x = 20 + (snow_x * snow_width + snow_gutter_x * snow_x);
      i32 snow_coord_y = (i32)((snow_y * snow_height + snow_gutter_y * snow_y) + (i32)state->ms * 0.2f) % state->height;
      DrawRectangle(snow_coord_x, snow_coord_y,
                    snow_width, snow_height,
                    WHITE);
      // DrawRectangleV();
    }
  }
#endif

}

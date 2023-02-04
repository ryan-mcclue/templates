// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <raylib.h>

#include "app.h"

void
draw_rect(Vec2F32 min, Vec2F32 max, Vec3F32 colour)
{
  Vector2 position = {min.x, min.y};
  Vector2 size = {max.x - min.x, max.y - min.y};
  Vec3F32 scaled = vec3_f32_mul(colour, 255.0f);
  Color color = {(u8)scaled.r, (u8)scaled.g, (u8)scaled.b, 255};
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

  for (i32 y = 0; y < (i32)state->height; ++y)
  {
    for (i32 x = 0; x < (i32)state->width; ++x)
    {
      Vec3F32 colour = vec3_f32_dup((x ^ y) / (state->width + state->height));
      draw_rect(vec2_f32(x, y), vec2_f32(x + 5, y + 1), colour);
    }
  }


  //DrawRectangle(0, 0, state->width, state->height, DARKBLUE);
  //DrawRectangle(0, state->height - 150, state->width, state->height, GREEN);

  // TODO(Ryan): Perhaps use i32 whenever used in calculation for drawing
  //i32 snow_num_x = 10, snow_width = 10, snow_gutter_x = 30;
  //i32 snow_num_y = 20, snow_height = 10, snow_gutter_y = 50;


}

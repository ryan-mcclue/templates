// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <raylib.h>

#include "app.h"

INTERNAL void
draw_rect(Vec2F32 min, Vec2F32 max, Vec3F32 colour)
{
  Vector2 position = {min.x, min.y};
  Vector2 size = {max.x - min.x, max.y - min.y};
  Vec3F32 scaled = vec3_f32_mul(colour, 255.0f);
  Color color = {(u8)scaled.r, (u8)scaled.g, (u8)scaled.b, 255};
  DrawRectangleV(position, size, color);
}

EXPORT void
app(AppState *state, MemArena *perm_arena, MemArenaTemp *temp_arena)
{
  if (!state->is_initialised)
  {
    state->is_initialised = true;
  }

  active_t = 0.0f;

	f32 fast_rate = 1 - pow_f32(2.0f, -50.0f * state->delta);
	f32 slow_rate = 1 - pow_f32(2.0f, -30.0f * state->delta);

  // I more-or-less always recommend self-correcting exponential animation curves for animating these two values, because their sharp initial motion fits with the user’s expectation of instantaneous feedback.
  // current = current + (target - current) * rate;
  // fastest on first frame, slowest on last frame
	curr->active_t  += ((f32)!!is_active - curr->active_t) * fast_rate;

	bg_color = ColorLerp(bg_color, box->active_color, box->active_t);

  Vec2F32 min = vec2_f32(state->ui_state.mouse_x - 32, state->ui_state.mouse_y - 32);
  Vec2F32 max = vec2_f32(min.x + 64, min.y + 64);
  Vec3F32 base_color = vec3_f32_dup(0.8f);
  Vec3F32 active_color = vec3_f32_dup(0.1f);
  Vec3F32 color = vec3_f32_lerp(base_color, active_color, 0.5f * state->ui_state.is_mouse_down); 

  draw_rect(min, max, color);
}

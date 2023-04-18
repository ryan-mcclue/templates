// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <SDL2/SDL.h>

#include "app.h"


// http://solhsa.com/imgui/

#define GEN_UI_ID __LINE__

INTERNAL SDL_Colour
vec4_f32_to_sdl_colour(Vec4F32 colour)
{
  SDL_Colour result = {0};

  result.r = round_f32_to_i32(255.0f * colour.r);
  result.g = round_f32_to_i32(255.0f * colour.g);
  result.b = round_f32_to_i32(255.0f * colour.b);
  result.a = round_f32_to_i32(255.0f * colour.a);

  return result;
}

INTERNAL void
draw_rect(SDL_Renderer *renderer, Vec2F32 pos, Vec2F32 dim, Vec4F32 colour)
{
  SDL_Colour sdl_colour = vec4_f32_to_sdl_colour(colour);

  SDL_SetRenderDrawColor(renderer, sdl_colour.r, sdl_colour.g, sdl_colour.b, sdl_colour.a);

  SDL_Rect render_rect = {0};
  render_rect.x = pos.x;
  render_rect.y = pos.y;
  render_rect.w = dim.w;
  render_rect.h = dim.h;
  SDL_RenderFillRect(renderer, &render_rect);
}

/*
INTERNAL b32
ui_region_hot(UIState *ui_state, Vec2F32 min, Vec2F32 max)
{
  b32 result = false;

  if (ui_state->mouse_x >= min.x && ui_state->mouse_x <= max.x &&
      ui_state->mouse_y >= min.y && ui_state->mouse_y <= max.y)
  {
    result = true;
  }

  return result;
}

INTERNAL b32
ui_button(UIState *ui_state, Vec2F32 min, Vec2F32 max)
{
  b32 is_active = false;

  Vec3F32 base_color = vec3_f32_dup(0.9f);
  Vec3F32 hot_color = vec3_f32_dup(0.5f);
  Vec3F32 active_color = vec3_f32_dup(0.1f);

  Vec3F32 final_color = base_color;

  if (ui_region_hot(ui_state, min, max))
  {
	  f32 slow_rate = 1.0f - pow_f32(2.0f, -30.0f * ui_state->delta);
	  f32 fast_rate = 1.0f - pow_f32(2.0f, -50.0f * ui_state->delta);

    if (ui_state->mouse_is_down)
    {
	    ui_state->active_t += (1.0f - ui_state->active_t) * slow_rate;
      final_color = vec3_f32_lerp(hot_color, active_color, ui_state->active_t); 
      is_active = true;
      ui_state->hot_t = 0.0f;
    }
    else
    {
	    ui_state->hot_t += (1.0f - ui_state->hot_t) * fast_rate;
      final_color = vec3_f32_lerp(base_color, hot_color, ui_state->hot_t); 
      ui_state->active_t = 0.0f;
    }
  }
  else
  {
    ui_state->active_t = 0.0f;
    ui_state->hot_t = 0.0f;
  }

  draw_rect(min, max, final_color);

  return is_active;
}
*/


EXPORT void
app(AppState *state, MemArena *perm_arena, MemArenaTemp *temp_arena)
{
  if (!state->is_initialised)
  {
    state->is_initialised = true;
  }

  draw_rect(state->renderer, vec2_f32(50.0f, 50.0f), vec2_f32(400.0f, 400.0f), vec4_f32(1.0f, 0.8f, 0.8f, 1.0f));

  // IMPORTANT(Ryan): Anything that is animated, i.e. varies over time use a _t varible

  // NOTE(Ryan): Self-correcting, exponential. Fastest on first frame to satisfy user requirement of instantaneous feedback
  // current = current + (target - current) * rate;
  //Vec2F32 min = vec2_f32(64, 64);
  //Vec2F32 max = vec2_f32(min.x + 256, min.y + 256);
  //if (ui_button(&state->ui_state, min, max))
  //{
  //  // DrawText("Button pressed", 300, 300, 24, {255, 255, 255, 255});  
  //}

}

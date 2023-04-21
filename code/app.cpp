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

#define PERLIN_SIZE 256
#define PERLIN_OCTAVES 8
// NOTE(Ryan): Higher values will give smoother results as contains more lower frequency parts
#define PERLIN_SCALE 2.0f
INTERNAL void
perlin_like_noise(AppState *state, Renderer *renderer)
{
  // NOTE(Ryan): Allows pitch to be halved evenly
  ASSERT(IS_POW2(PERLIN_SIZE));
  // NOTE(Ryan): Ensure don't have a pitch less than 1
  ASSERT(PERLIN_SIZE >> PERLIN_OCTAVES >= 1);
  
  f32 noise[PERLIN_SIZE] = ZERO_STRUCT;

  f32 seed[PERLIN_SIZE] = ZERO_STRUCT;
  for (u32 i = 0; i < PERLIN_SIZE; i += 1)
  {
    seed[i] = rand_unilateral_f32(&state->rand_seed);
  }

  for (u32 elem_i = 0; elem_i < PERLIN_SIZE; elem_i += 1)
  {
    f32 elem_noise_accum = 0.0f;
    f32 scale = 1.0f;
    f32 scale_accum = 0.0f;

    for (u32 octave_i = 0; octave_i < PERLIN_OCTAVES; octave_i += 1)
    {
      u32 pitch = PERLIN_SIZE >> octave_i;
      // NOTE(Ryan): First sample multiple of pitch
      u32 sample_one_i = (elem_i / pitch) * pitch;
      u32 sample_two_i = (sample_one_i + pitch) % PERLIN_SIZE;
      
      // NOTE(Ryan): How far into pitch
      f32 blend = (f32)(elem_i - sample_one_i) / (f32)pitch;

      f32 sample = lerp_f32(seed[sample_one_i], seed[sample_two_i], blend);

      elem_noise_accum += sample * scale;

      scale_accum += scale;
      scale = scale / PERLIN_SCALE;
    }

    // NOTE(Ryan): Division to ensure between 0 and 1
    noise[elem_i] = elem_noise_accum / scale_accum;
  }

  u32 box_width = renderer->render_width / PERLIN_SIZE;
  for (u32 x = 0; x < renderer->render_width / box_width; x += 1)
  {
    f32 box_height = (renderer->render_height * 0.5f) * noise[x];
    draw_rect(renderer->renderer, 
              vec2_f32(x * box_width, 0.0f), vec2_f32(box_width, box_height), 
              vec4_f32(1.0f, 0.8f, 0.8f, 1.0f));
  }


  return;
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

INTERNAL void
draw_rect_on_axis(SDL_Renderer *renderer, V2 origin, V2 x_axis, V2 y_axis, V4 colour)
{
  SDL_Colour sdl2_colour = v4_to_sdl_colour(colour);

  SDL_Vertex vertices[6] = {0};
  vertices[0].position.x = origin.x;
  vertices[0].position.y = origin.y;
  vertices[0].color = sdl2_colour;

  vertices[1].position.x = origin.x + x_axis.x;
  vertices[1].position.y = origin.y + x_axis.y;
  vertices[1].color = sdl2_colour;

  vertices[2].position.x = origin.x + y_axis.x;
  vertices[2].position.y = origin.y + y_axis.y;
  vertices[2].color = sdl2_colour;

  vertices[3].position.x = origin.x + y_axis.x;
  vertices[3].position.y = origin.y + y_axis.y;
  vertices[3].color = sdl2_colour;

  vertices[4].position.x = origin.x + x_axis.x + y_axis.x;
  vertices[4].position.y = origin.y + x_axis.y + y_axis.y;
  vertices[4].color = sdl2_colour;

  vertices[5].position.x = origin.x + x_axis.x;
  vertices[5].position.y = origin.y + x_axis.y;
  vertices[5].color = sdl2_colour;

  SDL_RenderGeometry(renderer, NULL, vertices, 6, NULL, 0);
}
*/


INTERNAL Vec2F32
toroidal_position_wrap(Vec2F32 position, u32 render_width, u32 render_height)
{
  Vec2F32 result = position;

  if (position.x < 0.0f)
  {
    result.x = position.x + render_width;
  }
  else if (position.x > render_width)
  {
    result.x = position.x - render_width;
  }

  if (position.y < 0.0f)
  {
    result.y = position.y + render_height;
  }
  else if (position.y > render_height)
  {
    result.y = position.y - render_height;
  }

  return result;
}

EXPORT void
app(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena, MemArenaTemp *temp_arena)
{
  if (!state->is_initialised)
  {
    state->is_initialised = true;

    state->asteroid.position = {20.0f, 20.0f};
    state->asteroid.velocity = {50.0f, -6.0f};
    state->asteroid.size = {128.0f, 128.0f};
    state->asteroid.angle = 0.0f; 

    state->player.position = {renderer->render_width * 0.5f, renderer->render_height * 0.5f};
    state->player.velocity = {0.0f, 0.0f};
    state->player.size = {64.0f, 64.0f};
    state->player.angle = 0.0f; 
  }

  if (input->move_left)
  {
    state->player.angle -= (5.0f * state->delta);
  }
  if (input->move_right)
  {
    state->player.angle += (5.0f * state->delta);
  }
  if (input->move_up)
  {
    f32 acceleration = 20.0f;
    // changing velocity is acceleration
    state->player.velocity.x += (sin(state->player.angle) * acceleration * state->delta);
    // NOTE(Ryan): Negative to account for 0 being at top of screen
    state->player.velocity.y += (-cos(state->player.angle) * acceleration * state->delta);
  }

  Vec2F32 asteroid_modulated_velocity = vec2_f32_mul(state->asteroid.velocity, state->delta);
  Vec2F32 asteroid_new_position = vec2_f32_add(state->asteroid.position, asteroid_modulated_velocity);
  state->asteroid.position = toroidal_position_wrap(asteroid_new_position, renderer->render_width, renderer->render_height);

  Vec2F32 player_modulated_velocity = vec2_f32_mul(state->player.velocity, state->delta);
  Vec2F32 player_new_position = vec2_f32_add(state->player.position, player_modulated_velocity);
  state->player.position = toroidal_position_wrap(player_new_position, renderer->render_width, renderer->render_height);

  draw_rect(renderer->renderer, state->asteroid.position, state->asteroid.size, vec4_f32(0.5f, 0.9f, 0.2f, 0.0f));

  draw_rect(renderer->renderer, state->player.position, state->player.size, vec4_f32(0.3f, 0.3f, 0.8f, 0.0f));

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

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

  SDL_FRect render_rect = ZERO_STRUCT;
  render_rect.x = pos.x;
  render_rect.y = pos.y;
  render_rect.w = dim.w;
  render_rect.h = dim.h;

  SDL_RenderFillRectF(renderer, &render_rect);
}

#define PERLIN_SIZE 256
#define PERLIN_OCTAVES 8
// NOTE(Ryan): Higher values will give smoother results as contains more lower frequency parts
#define PERLIN_SCALE 2.0f
INTERNAL f32 *
perlin_1d(MemArena *perm_arena, u32 *rand_seed, u32 size, u32 octaves, f32 smoothness)
{
  // NOTE(Ryan): Allows pitch to be halved evenly
  ASSERT(IS_POW2(size));
  // NOTE(Ryan): Ensure don't have a pitch less than 1
  ASSERT(size >> octaves >= 1);

  MemArenaTemp temp_arena = mem_arena_scratch_get(NULL, 0);
  f32 *seed = MEM_ARENA_PUSH_ARRAY_ZERO(temp_arena.arena, f32, size);
  for (u32 i = 0; i < size; i += 1)
  {
    seed[i] = rand_unilateral_f32(rand_seed);
  }

  f32 *noise = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, f32, size);

  for (u32 elem_i = 0; elem_i < size; elem_i += 1)
  {
    f32 elem_noise_accum = 0.0f;
    f32 scale = 1.0f;
    f32 scale_accum = 0.0f;

    for (u32 octave_i = 0; octave_i < octaves; octave_i += 1)
    {
      u32 pitch = size >> octave_i;
      // NOTE(Ryan): First sample multiple of pitch
      u32 sample_one_i = (elem_i / pitch) * pitch;
      u32 sample_two_i = (sample_one_i + pitch) % size;
      
      // NOTE(Ryan): How far into pitch
      f32 blend = (f32)(elem_i - sample_one_i) / (f32)pitch;

      f32 sample = lerp_f32(seed[sample_one_i], seed[sample_two_i], blend);

      elem_noise_accum += sample * scale;

      scale_accum += scale;
      scale = scale / smoothness;
    }

    // NOTE(Ryan): Division to ensure between 0 and 1
    noise[elem_i] = elem_noise_accum / scale_accum;
  }

  mem_arena_scratch_release(temp_arena);

  return noise;
}

INTERNAL void
draw_wire_frame(SDL_Renderer *renderer, Vec2F32 *points, u32 num_points, Vec2F32 origin, f32 rotation, f32 scale, Vec4F32 colour)
{
  MemArenaTemp temp_arena = mem_arena_scratch_get(NULL, 0);

  SDL_FPoint *transformed_points = MEM_ARENA_PUSH_ARRAY_ZERO(temp_arena.arena, SDL_FPoint, num_points + 1);

  for (u32 point_i = 0; point_i < num_points; point_i += 1)
  {
    // NOTE(Ryan): Rotation
    transformed_points[point_i].x = points[point_i].x * cos_f32(rotation) - points[point_i].y * sin_f32(rotation);
    transformed_points[point_i].y = points[point_i].x * sin_f32(rotation) + points[point_i].y * cos_f32(rotation);

    // NOTE(Ryan): Scaling
    transformed_points[point_i].x *= scale;
    transformed_points[point_i].y *= scale;
    
    // NOTE(Ryan): Translation
    transformed_points[point_i].x += origin.x;
    transformed_points[point_i].y += origin.y;
  }

  // NOTE(Ryan): Ensure closed
  transformed_points[num_points] = transformed_points[0];

  SDL_Colour sdl_colour = vec4_f32_to_sdl_colour(colour);
  SDL_SetRenderDrawColor(renderer, sdl_colour.r, sdl_colour.g, sdl_colour.b, sdl_colour.a);

  SDL_RenderDrawLinesF(renderer, transformed_points, (i32)(num_points + 1));

  mem_arena_scratch_release(temp_arena);
}

INTERNAL SpaceObjectDLL*
push_bullet(SpaceObjectDLL *first_free_bullet, MemArena *perm_arena, Vec2F32 position, Vec2F32 velocity)
{
  SpaceObjectDLL *result = NULL;

  if (first_free_bullet == NULL)
  {
    result = MEM_ARENA_PUSH_STRUCT_ZERO(perm_arena, SpaceObjectDLL);
  }
  else
  {
    result = first_free_bullet;
    MEMORY_ZERO_STRUCT(result);

    first_free_bullet = first_free_bullet->next;
  }

  result->object.position = position;
  result->object.velocity = velocity;

  return result;
}

INTERNAL void 
release_bullet(SpaceObjectDLL *first_free_bullet, SpaceObjectDLL *bullet)
{
  bullet->next = first_free_bullet;
  first_free_bullet = bullet;
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

INTERNAL b32
is_point_in_circle(Vec2F32 point, f32 radius, Vec2F32 circle)
{
  return (sqrt_f32(SQUARE((circle.x - point.x)) + SQUARE((circle.y - point.y))) < radius); 
}

ThreadContext global_tctx;

EXPORT void
app(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena)
{
  if (!state->is_initialised)
  {
    state->is_initialised = true;

    global_tctx = thread_context_create();
    thread_context_set(&global_tctx);

    state->asteroid.position = {20.0f, 20.0f};
    state->asteroid.velocity = {50.0f, -6.0f};
    state->asteroid.scale = 32.0f;
    state->asteroid.angle = 0.0f; 
    state->asteroid.num_points = 20;
    state->asteroid.points = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, Vec2F32, state->asteroid.num_points);
    for (u32 asteroid_point_i = 0; asteroid_point_i < state->asteroid.num_points; asteroid_point_i += 1)
    {
      f32 radius = 3.0f + rand_unilateral_f32(&state->rand_seed) * 2.0f;

      f32 a = ((f32)asteroid_point_i / (f32)state->asteroid.num_points) * TAU_F32; 
      state->asteroid.points[asteroid_point_i] = {radius * sin_f32(a), radius * cos_f32(a)};
    }

    state->player.position = {renderer->render_width * 0.5f, renderer->render_height * 0.5f};
    state->player.velocity = {0.0f, 0.0f};
    state->player.scale = 8.0f;
    state->player.angle = 0.0f; 
    state->player.num_points = 3;
    state->player.points = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, Vec2F32, state->player.num_points);
    state->player.points[0] = {0.0f, -5.0f};
    state->player.points[1] = {-2.5f, 2.5f};
    state->player.points[2] = {2.5f, 2.5f};

    state->map_width = 1024;
    state->map_height = 512;
    // 1 with land, 0 without
    state->map = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, u8, state->map_width * state->map_height);
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

  if (input->bullet_fired)
  {
    Vec2F32 bullet_position = state->player.position;
    Vec2F32 bullet_velocity = ZERO_STRUCT;
    bullet_velocity.x = 50.0f * sinf(state->player.angle);
    bullet_velocity.y = -50.0f * cosf(state->player.angle);
    f32 bullet_scale = 1.0f;

    SpaceObjectDLL *bullet = push_bullet(state->first_free_bullet, perm_arena, bullet_position, bullet_velocity);
    DLL_PUSH_FRONT(state->first_bullet, state->last_bullet, bullet);
  }

  Vec2F32 asteroid_modulated_velocity = vec2_f32_mul(state->asteroid.velocity, state->delta);
  Vec2F32 asteroid_new_position = vec2_f32_add(state->asteroid.position, asteroid_modulated_velocity);
  state->asteroid.position = toroidal_position_wrap(asteroid_new_position, renderer->render_width, renderer->render_height);
  state->asteroid.angle += 0.5f * state->delta;

  Vec2F32 player_modulated_velocity = vec2_f32_mul(state->player.velocity, state->delta);
  Vec2F32 player_new_position = vec2_f32_add(state->player.position, player_modulated_velocity);
  state->player.position = toroidal_position_wrap(player_new_position, renderer->render_width, renderer->render_height);

  for (SpaceObjectDLL *bullet = state->first_bullet; bullet != NULL; bullet = bullet->next)
  {
    Vec2F32 bullet_modulated_velocity = vec2_f32_mul(bullet->object.velocity, state->delta);
    bullet->object.position = vec2_f32_add(bullet->object.position, bullet_modulated_velocity);

    // check collision with asteroid and create 2 child asteroids
    // f32 angle1 = rand_f32_unilateral() * TAU_F32;

    // if (player_dies) reset_game();
  }

  // NOTE(Ryan): Remove bullets that are off screen
  // TODO(Ryan): Compose linked lists rather than separate  
  SpaceObjectDLL *bullet = state->first_bullet;
  SpaceObjectDLL *bullet_next = NULL;
  while (bullet != NULL)
  {
    // NOTE(Ryan): Cache next prempting release
    bullet_next = bullet->next;  

    if (bullet->object.position.x <= 0 || bullet->object.position.x >= renderer->render_width ||
        bullet->object.position.y <= 0 || bullet->object.position.y >= renderer->render_height)
    {
      DLL_REMOVE(state->first_bullet, state->last_bullet, bullet);
      release_bullet(state->first_free_bullet, bullet);
    }

    bullet = bullet_next;
  }

  for (SpaceObjectDLL *bullet = state->first_bullet; bullet != NULL; bullet = bullet->next)
  {
    draw_rect(renderer->renderer, bullet->object.position, vec2_f32(8.0f, 8.0f), vec4_f32(1.0f, 1.0f, 1.0f, 0.0f));
  }

  draw_wire_frame(renderer->renderer,
                  state->asteroid.points, state->asteroid.num_points, 
                  state->asteroid.position, state->asteroid.angle, state->asteroid.scale, 
                  vec4_f32(0.8f, 0.2f, 0.1f, 0.0f));

  draw_wire_frame(renderer->renderer,
                  state->player.points, state->player.num_points, 
                  state->player.position, state->player.angle, state->player.scale, 
                  vec4_f32(0.5f, 0.8f, 0.3f, 0.0f));
  
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



// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <SDL2/SDL.h>

#include "app.h"

// NOTE(Ryan): Solarized light colours
#define YELLOW_COLOUR	vec4_f32(0.71f, 0.54f, 0.00f, 0.0f)
#define ORANGE_COLOUR	vec4_f32(0.80f, 0.29f, 0.09f, 0.0f)
#define RED_COLOUR vec4_f32(0.86f, 0.20f, 0.18f, 0.0f)
#define MAGENTA_COLOUR vec4_f32(0.83f, 0.21f, 0.05f, 0.0f)
#define VIOLET_COLOUR	vec4_f32(0.42f, 0.44f, 0.77f, 0.0f)
#define BLUE_COLOUR vec4_f32(0.15f, 0.55f, 0.82f, 0.0f)
#define CYAN_COLOUR vec4_f32(0.16f, 0.63f, 0.60f, 0.0f)
#define GREEN_COLOUR vec4_f32(0.52f, 0.60f, 0.00f, 0.0f)

// trees if don't care loop with recursion
// however, loop is faster and more reliable

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

INTERNAL f32 *
perlin_1d(MemArena *arena, f32 *seed, u32 size, u32 octaves, f32 smoothness)
{
  // NOTE(Ryan): Allows pitch to be halved evenly
  ASSERT(IS_POW2(size));
  // NOTE(Ryan): Ensure don't have a pitch less than 1
  ASSERT(size >> octaves >= 1);

  f32 *noise = MEM_ARENA_PUSH_ARRAY_ZERO(arena, f32, size);

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


  return noise;
}

// IMPORTANT(Ryan): Rotation angle can be determined via: atan2f(vx, vy);
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

INTERNAL Entity *
create_test_entity(MemArena *arena, f32 x, f32 y)
{
  Entity *result = MEM_ARENA_PUSH_STRUCT_ZERO(arena, Entity);

  result->position.x = x;
  result->position.y = y;
  result->radius = 16.0f;

  result->num_points = 10;
  result->points = MEM_ARENA_PUSH_ARRAY_ZERO(arena, Vec2F32, result->num_points);
  for (u32 p = 0; p < result->num_points - 1; p += 1)
  {
    f32 a = ((f32)p / (f32)(result->num_points - 2)) * TAU_F32; 
    result->points[p] = vec2_f32_arm(a) * result->radius;
  }
  result->points[result->num_points - 1] = {0.0f, 0.0f};

  return result;
}

// Always work in 'world units' and convert to render when drawing?
INTERNAL Vec2F32
to_render_coord(Vec2F32 world, Vec2F32 offset, Vec2F32 scale)
{
  return vec2_f32_hadamard((world + offset), scale);
}

INTERNAL Vec2F32
to_world_coord(Vec2F32 render, Vec2F32 offset, Vec2F32 scale)
{
  Vec2F32 result = ZERO_STRUCT;

  result.x = render.x / scale.x;
  result.y = render.y / scale.y;

  result -= offset;

  return result;
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

    /*
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
    */

    state->map_width = 1024;
    state->map_height = 512;
    // 1 with land, 0 without
    state->map = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, u8, state->map_width * state->map_height);

    MemArenaTemp temp_arena = mem_arena_scratch_get(NULL, 0);
    f32 *seed = MEM_ARENA_PUSH_ARRAY_ZERO(temp_arena.arena, f32, state->map_width);
    for (u32 i = 0; i < state->map_width; i += 1)
    {
      seed[i] = rand_unilateral_f32(&state->rand_seed);
    }
    // NOTE(Ryan): Ensure start and end are halfway up
    seed[0] = 0.5f;
    f32 *map_heights = perlin_1d(temp_arena.arena, seed, state->map_width, 8, 2.0f); 

    for (u32 x = 0; x < state->map_width; x += 1)
    {
      for (u32 y = 0; y < state->map_height; y += 1)
      {
        if (y >= (map_heights[x] * state->map_height))
        {
          state->map[state->map_width * y + x] = 1;
        }
        else
        {
          state->map[state->map_width * y + x] = 0;
        }
      }
    }

    mem_arena_scratch_release(temp_arena);

    state->grid_offset = {renderer->render_width * 0.5f, renderer->render_height * 0.5f};
    state->grid_scale = {1.0f, 1.0f};
  } 

  /*
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
  */

#if 0

  // IMPORTANT(Ryan): If cannot map larger than screen, require camera
  f32 map_edge_scroll_speed = 400.0f;
  if (input->mouse_x < 5)
  {
    state->camera.x -= map_edge_scroll_speed * state->delta; 
  }
  if (input->mouse_x > renderer->render_width - 5)
  {
    state->camera.x += map_edge_scroll_speed * state->delta; 
  }
  if (input->mouse_y < 5)
  {
    state->camera.y -= map_edge_scroll_speed * state->delta; 
  }
  if (input->mouse_y > renderer->render_height - 5)
  {
    state->camera.y += map_edge_scroll_speed * state->delta; 
  }

  if (state->camera.x < 0)
  {
    state->camera.x = 0;
  }
  if (state->camera.x >= state->map_width - renderer->render_width)
  {
    state->camera.x = state->map_width - renderer->render_width; 
  }
  if (state->camera.y < 0)
  {
    state->camera.y = 0;
  }
  if (state->camera.y >= state->map_height - renderer->render_height)
  {
    state->camera.y = state->map_height - renderer->render_height; 
  }

  if (input->mouse_clicked)
  {
    Entity *entity = create_test_entity(perm_arena, input->mouse_x + state->camera.x, input->mouse_y + state->camera.y);
    SLL_QUEUE_PUSH(state->first_test_entity, state->last_test_entity, entity);
  }


  // response vector normal is tangential to terrain at collision?

  // TODO(Ryan): Remove this and get efficient higher update rate?
  for (u32 physics_update_count = 0; physics_update_count < 10; physics_update_count += 1)
  {
    for (Entity *test_entity = state->first_test_entity; test_entity != NULL; test_entity = test_entity->next)
    {
      // gravity
      test_entity->acceleration.y += 2.0f;

      test_entity->velocity += test_entity->acceleration * state->delta;

      Vec2F32 potential_pos = test_entity->position + (test_entity->velocity * state->delta);

      test_entity->acceleration = {0.0f, 0.0f};

      // it's moving
      test_entity->stable = false;

#if 0
      test_entity->position = potential_pos;
#else
      f32 angle = vec2_f32_angle(test_entity->velocity);
      Vec2F32 response = ZERO_STRUCT;

      b32 collision = false;

      // ensure increment amount has arc length <= 1 pixel
      for (f32 r = angle - HALF_PI_F32; r < angle + HALF_PI_F32; r += PI_F32 / 8.0f)
      {
        Vec2F32 test_pos = potential_pos + (test_entity->radius * vec2_f32_arm(r));

        if (test_pos.x >= state->map_width) 
        {
          test_pos.x = state->map_width - 1;
        }
        if (test_pos.x < 0) 
        {
          test_pos.x = 0;
        }
        if (test_pos.y >= state->map_height) 
        {
          test_pos.y = state->map_height - 1;
        }
        if (test_pos.y < 0)
        {
          test_pos.y = 0;
        }

        if (state->map[(u32)test_pos.y * state->map_width + (u32)test_pos.x] != 0)
        {
          // accumulate to get response vector? semicircle gives resultant?  
          // response vector is our normal to collision surface
          response += potential_pos - test_pos;
          collision = true;
        }
      }

      if (collision)
      {
        // so, won't actually collide as stopping before
        test_entity->stable = true;
        f32 velocity_magnitude = vec2_f32_length(test_entity->position);
        Vec2F32 normal = vec2_f32_normalise(response);

        // reflection vector with dot product
        test_entity->velocity += -2.0f * (vec2_f32_dot(test_entity->velocity, normal)) * normal;
        
      }
      else
      {
        test_entity->position = potential_pos;
      }
#endif
    }
  }


  f32 block_size = 8.0f;
  for (u32 x = 0; x < (renderer->render_width / block_size); x += 1)
  {
    for (u32 y = 0; y < (renderer->render_height / block_size); y += 1)
    {
      u8 map_val = state->map[(y + (u32)state->camera.y)*state->map_width + (x + (u32)state->camera.x)];
      if (map_val == 1)
      {
        draw_rect(renderer->renderer, vec2_f32(x * block_size, y * block_size), vec2_f32(block_size, block_size), GREEN_COLOUR);
      }
      else
      {
        draw_rect(renderer->renderer, vec2_f32(x * block_size, y * block_size), vec2_f32(block_size, block_size), BLUE_COLOUR);
      }
    }
  }


  for (Entity *test_entity = state->first_test_entity; test_entity != NULL; test_entity = test_entity->next)
  {
    draw_wire_frame(renderer->renderer,
                    test_entity->points, test_entity->num_points, 
                    test_entity->position - state->camera, vec2_f32_angle(test_entity->velocity), 
                    1.0f, 
                    RED_COLOUR);
  }
  
/*
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
*/

  // IMPORTANT(Ryan): Anything that is animated, i.e. varies over time use a _t varible

  // NOTE(Ryan): Self-correcting, exponential. Fastest on first frame to satisfy user requirement of instantaneous feedback
  // current = current + (target - current) * rate;
  //Vec2F32 min = vec2_f32(64, 64);
  //Vec2F32 max = vec2_f32(min.x + 256, min.y + 256);
  //if (ui_button(&state->ui_state, min, max))
  //{
  //  // DrawText("Button pressed", 300, 300, 24, {255, 255, 255, 255});  
  //}
#else
  if (input->mouse_pressed)
  {
    state->grid_pan = {input->mouse_x, input->mouse_y};
  }
  if (input->mouse_held)
  {
    state->grid_offset.x += (input->mouse_x - state->grid_pan.x) / state->grid_scale.x;
    state->grid_offset.y += (input->mouse_y - state->grid_pan.y) / state->grid_scale.y;

    state->grid_pan = {input->mouse_x, input->mouse_y};
  }
  
  Vec2F32 mouse_grid_before_zoom = to_world_coord(vec2_f32(input->mouse_x, input->mouse_y), state->grid_offset, state->grid_scale);
  if (input->move_right)
  {
    state->grid_scale *= 1.010f;
  }
  if (input->move_left)
  {
    state->grid_scale *= 0.990f;
  }
  Vec2F32 mouse_grid_after_zoom = to_world_coord(vec2_f32(input->mouse_x, input->mouse_y), state->grid_offset, state->grid_scale);
  state->grid_offset -= (mouse_grid_before_zoom - mouse_grid_after_zoom);

  // IMPORTANT(Ryan): Whenever using mouse, use after zoom variant

  SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 0);

  f32 line_spacing = 10;
  for (u32 line_i = 0; line_i < 10; line_i += 1)
  {
    Vec2F32 line_start = {(f32)(line_i * line_spacing), 0.0f};
    Vec2F32 line_end = {(f32)(line_i * line_spacing), 100.0f};

    Vec2F32 line_start_render = to_render_coord(line_start, state->grid_offset, state->grid_scale);
    Vec2F32 line_end_render = to_render_coord(line_end, state->grid_offset, state->grid_scale);

    SDL_RenderDrawLineF(renderer->renderer, line_start_render.x, line_start_render.y, line_end_render.x, line_end_render.y);
  }
#endif

}



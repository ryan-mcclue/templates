// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "app.h"

// NOTE(Ryan): Solarized light colours
#define YELLOW_COLOUR	vec4_f32(0.71f, 0.54f, 0.00f, 1.0f)
#define ORANGE_COLOUR	vec4_f32(0.80f, 0.29f, 0.09f, 1.0f)
#define RED_COLOUR vec4_f32(0.86f, 0.20f, 0.18f, 1.0f)
#define MAGENTA_COLOUR vec4_f32(0.83f, 0.21f, 0.05f, 1.0f)
#define VIOLET_COLOUR	vec4_f32(0.42f, 0.44f, 0.77f, 1.0f)
#define BLUE_COLOUR vec4_f32(0.15f, 0.55f, 0.82f, 1.0f)
#define CYAN_COLOUR vec4_f32(0.16f, 0.63f, 0.60f, 1.0f)
#define GREEN_COLOUR vec4_f32(0.52f, 0.60f, 0.00f, 1.0f)

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

INTERNAL SDL_FRect
vec2_f32_to_sdl_frect(Vec2F32 a, Vec2F32 b)
{
  SDL_FRect result = ZERO_STRUCT;

  result.x = a.x;
  result.y = a.y;
  result.w = b.x;
  result.h = b.y;

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

/*
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

// Always work in 'world units' and convert to render when drawing?
INTERNAL Vec2F32
to_render_coord(Vec2F32 world, Vec2F32 offset, Vec2F32 scale)
{
  return vec2_f32_hadamard(world + offset, scale);
}
/* PANNING/ZOOMING
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
*/

INTERNAL Vec2F32
to_world_coord(Vec2F32 render, Vec2F32 offset, Vec2F32 scale)
{
  return vec2_f32_div(render, scale) - offset;
}

ThreadContext global_tctx;

// sorting should happen a layer above storage?

// we are not doing procedural animation



// ECS data-oriented
// performance orientated?
// so NOT:
//   I'm interested in how much faster ECS than a big heterogeneous linked list like: 
//   list<IGameObject> scene;
//
// these entity ids would be indexes into component arrays
// entity: id
// components: contingous memory structs (we want data-orientated design, i.e. how CPU accesses)
// systems: logic

// rigid body (can't be deformed)

// Store transform values in matrices?
// Using those matrices those modifications would just end being something like rotationMatrix * sizeMatrix * positionMatrix.

// inheritance: multiple inheritance
// component: 

INTERNAL Entity *
alloc_pool_entity(Entity **first_free_entity, MemArena *perm_arena)
{
  Entity *result = NULL;

  if (*first_free_entity == NULL)
  {
    result = MEM_ARENA_PUSH_STRUCT_ZERO(perm_arena, Entity);
  }
  else
  {
    result = (*first_free_entity);
    MEMORY_ZERO_STRUCT(result);

    (*first_free_entity) = (*first_free_entity)->next;
  }

  return result;
}

INTERNAL void 
release_pool_entity(Entity **first_free_entity, Entity *entity)
{
  entity->next = (*first_free_entity);
  (*first_free_entity) = entity;
}

INTERNAL Entity *
push_entity(Entity **first_free_entity, MemArena *mem_arena, Entity **first_entity, Entity **last_entity, ENTITY_COMPONENT_FLAG flags)
{
  Entity *result = NULL;  

  result = alloc_pool_entity(first_free_entity, mem_arena);
  // IMPORTANT(Ryan): Pointer variables copies just like normal variables; they go away
  // To actual reassign, must change where that pointer points at, e.g. *a = val;
  // So, if wanting to what passed in pointer points to, require another layer of indirection
  DLL_PUSH_BACK((*first_entity), (*last_entity), result);

  result->component_flags = flags;

  return result;
}

struct PACKED TileMap
{
  u8 width, height;
  u8 *tile_types;
};

INTERNAL void
asset_store_add_texture(SDL_Renderer *renderer, Map *texture_map, MemArena *mem_arena, 
                        const char *key, const char *file_name)
{
  SDL_Surface *surface = IMG_Load(file_name);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  if (texture == NULL)
  {
    WARN("Failed to load texture", SDL_GetError());
  }

  map_insert(mem_arena, texture_map, map_key_str(s8_cstring(key)), texture);
}

// merge sort for linked list
// o(nlogn) best for comparison based sort
// o(n) best for counting


// recursion for readable solutions, but always go for non-recursive e.g. iterative, mathematical formula etc.

INTERNAL Entity *
get_middle_entity(Entity *first)
{
  Entity *slow = first, *fast = first->next;

  while (fast != NULL && fast->next != NULL)
  {
    slow = slow->next;
    fast = fast->next->next;
  }

  return slow;
}

 // logarithmic to split
 // linear to merge
INTERNAL Entity *
sorted_merge(Entity *left, Entity *right)
{
  Entity *first = NULL, *last = NULL;

  while (left != NULL && right != NULL)
  {
    if (left->sprite_component.z_index < right->sprite_component.z_index)
    {
      DLL_PUSH_BACK(first, last, left);
      left = left->next;
    }
    else
    {
      DLL_PUSH_BACK(first, last, right);
      right = right->next;
    }
  }

  while (left != NULL)
  {
    DLL_PUSH_BACK(first, last, left);
    left = left->next;
  }

  while (right != NULL)
  {
    DLL_PUSH_BACK(first, last, right);
    right = right->next;
  }

  return first;
}

INTERNAL Entity *
sort_entities_by_z_index(Entity *first)
{
  if (first == NULL || first->next == NULL)
  {
    return first;
  }
  else
  {
    Entity *left = first;
    Entity *one_before_midpoint = get_middle_entity(first);
    Entity *tmp = one_before_midpoint->next;
    // NOTE(Ryan): To 'separate' lists
    one_before_midpoint->next = NULL;
    Entity *right = tmp;

    left = sort_entities_by_z_index(left);
    right = sort_entities_by_z_index(right);

    return sorted_merge(left, right);
  }
}


EXPORT void
app(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena)
{
  if (!state->is_initialised)
  {
    global_debugger_present = state->debugger_present;

    state->is_initialised = true;

    // TODO(Ryan): More platform agnostic if these are passed in as 2 frame arenas
    global_tctx = thread_context_create();
    thread_context_set(&global_tctx);

    state->asset_store.textures = map_create(perm_arena);
   
    // 320 x 96 pixels
    // 10 x 3 tiles
    asset_store_add_texture(renderer->renderer, &state->asset_store.textures, perm_arena,
                            "tile-map", "./jungle.png");
    String8 tile_map_file = s8_read_entire_file(perm_arena, s8_lit("jungle.bin")); 
    TileMap *tile_map = (TileMap *)tile_map_file.str;

#pragma mark LOAD_LEVEL_START
    asset_store_add_texture(renderer->renderer, &state->asset_store.textures, perm_arena,
                            "tank-image", "./tank-panther-right.png");
    asset_store_add_texture(renderer->renderer, &state->asset_store.textures, perm_arena,
                            "truck-image", "./truck-ford-right.png");


    Entity *tank = push_entity(&state->first_free_entity, perm_arena, &state->first_entity, &state->last_entity, 
                               ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_RIGID_BODY | ENTITY_COMPONENT_FLAG_SPRITE);
    tank->transform_component.position = {100.0f, 100.0f};
    tank->transform_component.scale = {1.0f, 1.0f};
    tank->rigid_body_component.velocity = {8.0f, 2.0f};
    tank->sprite_component.dimensions = {32.0f, 32.0f}; // actual image dimensions
    tank->sprite_component.texture_key = map_key_str(s8_lit("tank-image"));
    tank->sprite_component.z_index = 5;

    Entity *tank2 = push_entity(&state->first_free_entity, perm_arena, &state->first_entity, &state->last_entity, 
                               ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_RIGID_BODY | ENTITY_COMPONENT_FLAG_SPRITE);
    tank2->transform_component.position = {200.0f, 100.0f};
    tank2->transform_component.scale = {2.0f, 2.0f};
    tank2->rigid_body_component.velocity = {-8.0f, 2.0f};
    tank2->sprite_component.dimensions = {32.0f, 32.0f};
    tank2->sprite_component.texture_key = map_key_str(s8_lit("tank-image"));
    tank2->sprite_component.z_index = 1;

#pragma mark LOAD_LEVEL_END

  } 

  for (Entity *entity = state->first_entity; entity != NULL; entity = entity->next)
  {
    if (HAS_FLAGS_ALL(entity->component_flags, (ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_RIGID_BODY)))
    {
      entity->transform_component.position += entity->rigid_body_component.velocity * state->delta;
    }
  }

  // TODO(Ryan): map render, cache texture from map asset store if doing subsets?

  // TODO(Ryan): update state->last_entity as well
  state->first_entity = sort_entities_by_z_index(state->first_entity);
  for (Entity *entity = state->first_entity; entity != NULL; entity = entity->next)
  {
    if (HAS_FLAGS_ALL(entity->component_flags, (ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_SPRITE)))
    {
      SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 255);

      SDL_FRect dst_rect = vec2_f32_to_sdl_frect(entity->transform_component.position,
                                                 vec2_f32_hadamard(entity->sprite_component.dimensions, entity->transform_component.scale));
      MapSlot *map_slot = map_lookup(&state->asset_store.textures, entity->sprite_component.texture_key);
      if (map_slot != NULL)
      {
        SDL_Texture *texture = (SDL_Texture *)map_slot->val;
        // much slower than normal render?
        SDL_RenderCopyExF(renderer->renderer, texture, NULL, &dst_rect, entity->transform_component.rotation, NULL, SDL_FLIP_NONE);
      }
      //draw_rect(renderer->renderer, entity->transform_component.position, entity->sprite_component.dimensions, RED_COLOUR);
    }
  }
}



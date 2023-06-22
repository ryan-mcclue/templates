// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

// TODO(Ryan): UI elements: https://mattiasgustavsson.itch.io/yarnspin/devlog/544215/coding-an-ad-hoc-ui   

// TODO(Ryan): Use "", as still implementation defined, however allows for easier overriding if required
// So, only use for stdlib?
// TODO(Ryan): Link ctags for library
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

  colour.r = CLAMP(0.0f, colour.r, 1.0f); 
  colour.g = CLAMP(0.0f, colour.g, 1.0f); 
  colour.b = CLAMP(0.0f, colour.b, 1.0f); 
  colour.a = CLAMP(0.0f, colour.a, 1.0f); 

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

INTERNAL SDL_Rect
vec2_f32_to_sdl_rect(Vec2F32 a, Vec2F32 b)
{
  SDL_Rect result = ZERO_STRUCT;

  result.x = (i32)a.x;
  result.y = (i32)a.y;
  result.w = (i32)b.x;
  result.h = (i32)b.y;

  return result;
}

INTERNAL void
draw_texture(SDL_Renderer *renderer, Map *texture_map, MapKey texture_key, Vec2F32 pos, Vec2F32 dim, 
             Vec2I32 texture_offset = vec2_i32(0, 0), f32 rotation = 0.0f,
             Vec4F32 colour = vec4_f32(1.0f, 1.0f, 1.0f, 1.0f))
{
  SDL_FRect dst_rect = vec2_f32_to_sdl_frect(pos, dim);

  MapSlot *map_slot = map_lookup(texture_map, texture_key);
  if (map_slot != NULL)
  {
    SDL_Texture *texture = (SDL_Texture *)map_slot->val;

    SDL_Color texture_colour_mod = vec4_f32_to_sdl_colour(colour);
    SDL_SetTextureColorMod(texture, texture_colour_mod.r, texture_colour_mod.g, texture_colour_mod.b);
    SDL_SetTextureAlphaMod(texture, texture_colour_mod.a);

    SDL_Rect src_rect = ZERO_STRUCT;
    if (texture_offset.x == 0)
    {
      src_rect = {0, 0, (i32)dim.w, (i32)dim.h};
    }
    else
    {
      src_rect = {texture_offset.x, texture_offset.y, (i32)dim.w, (i32)dim.h};
    }

    SDL_RenderCopyExF(renderer, texture, &src_rect, &dst_rect, rotation, NULL, SDL_FLIP_NONE);
  }
}


// TODO(Ryan): Call this when window size changes, e.g. set font size to (window_height / 24)
INTERNAL void
asset_store_add_font(SDL_Renderer *renderer, Map *font_map, MemArena *mem_arena, 
                     const char *key, const char *file_name, u32 font_size)
{
  Font *font = MEM_ARENA_PUSH_STRUCT_ZERO(mem_arena, Font);
  font->font = TTF_OpenFont(file_name, (i32)font_size);
  if (font->font == NULL)
  {
    WARN("Failed to load font", SDL_GetError());
  }
  else
  {
    font->character_height = TTF_FontHeight(font->font);
    // TTF_SetFontStyle(font->font, renderstyle); // TTF_STYLE_BOLD, TTF_STYLE_ITALIC 
    map_insert(mem_arena, font_map, map_key_str(s8_cstring(key)), font);
  }
}

INTERNAL PreparedText
prepare_text(SDL_Renderer *renderer, Font *font, String8 text)
{
  PreparedText result = ZERO_STRUCT;

  SDL_Colour text_colour = {0xff, 0xff, 0xff, 0xff};
  SDL_Surface *surface = TTF_RenderText_Blended(font->font, (char *)text.str, text_colour);

  result.texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  if (result.texture != NULL)
  {
    SDL_QueryTexture(result.texture, NULL, NULL, &result.width, &result.height);
  }

  return result;
}

INTERNAL void
draw_prepared_text(SDL_Renderer *renderer, PreparedText *prepared_text, Vec2F32 pos,
                   Vec4F32 colour = vec4_f32(1.0f, 1.0f, 1.0f, 1.0f), 
                   b32 free_after_draw = true, 
                   f32 rotation = 0.0f)
{
  SDL_FRect dst_rect = {pos.x, pos.y, (f32)prepared_text->width, (f32)prepared_text->height};

  SDL_Color texture_colour_mod = vec4_f32_to_sdl_colour(colour);
  SDL_SetTextureColorMod(prepared_text->texture, texture_colour_mod.r, texture_colour_mod.g, texture_colour_mod.b);
  SDL_SetTextureAlphaMod(prepared_text->texture, texture_colour_mod.a);

  SDL_RenderCopyExF(renderer, prepared_text->texture, NULL, &dst_rect, rotation, NULL, SDL_FLIP_NONE);

  if (free_after_draw)
  {
    SDL_DestroyTexture(prepared_text->texture);
  }
}


INTERNAL void
draw_rect(SDL_Renderer *renderer, Vec2F32 pos, Vec2F32 dim, Vec4F32 colour)
{
  SDL_Colour sdl_colour = vec4_f32_to_sdl_colour(colour);

  SDL_SetRenderDrawColor(renderer, sdl_colour.r, sdl_colour.g, sdl_colour.b, sdl_colour.a);

  SDL_FRect render_rect = vec2_f32_to_sdl_frect(pos, dim);

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

/* IMPORTANT(Ryan): If using a fixed array, just have a queue of ids, e.g:
  if (first_free_id == NULL)
  {
    free_id = num_ids++;
  }
  else
  {
    free_id = SLL_QUEUE_POP(first_free_id, last_free_id);
  }
*/

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
  // TODO(Ryan): Update next and prev of state->first_entity list
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
  else
  {
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    map_insert(mem_arena, texture_map, map_key_str(s8_cstring(key)), texture);
  }
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

INTERNAL void
print_entity_linked_list(Entity *first)
{
  printf("[");
  for (Entity *entity = first; entity != NULL; entity = entity->next)
  {
    printf("%d%s", (u32)entity->sprite_component.z_index, entity->next != NULL ? ", " : "");
  }
  printf("]\n");
}

 // logarithmic to split
 // linear to merge
INTERNAL Entity *
sorted_merge(Entity *left, Entity *right, u32 indent)
{
  Entity *first = NULL, *last = NULL;

  while (left != NULL && right != NULL)
  {
    if (left->sprite_component.z_index < right->sprite_component.z_index)
    {
      Entity *next = left->next;
      DLL_PUSH_BACK(first, last, left);
      left = next;
    }
    else
    {
      Entity *next = right->next;
      DLL_PUSH_BACK(first, last, right);
      right = next;
    }
  }

  while (left != NULL)
  {
    Entity *next = left->next;
    DLL_PUSH_BACK(first, last, left);
    left = next;
  }

  while (right != NULL)
  {
    Entity *next = right->next;
    DLL_PUSH_BACK(first, last, right);
    right = next;
  }

  //printf("%*smerged: ", indent * 4, "");
  //print_entity_linked_list(first);

  return first;
}


INTERNAL Entity *
sort_entities_by_z_index(Entity *first, u32 indent)
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

    left = sort_entities_by_z_index(left, indent + 1);
    //printf("%*ssplit left: ", indent * 4, "");
    //print_entity_linked_list(left);

    right = sort_entities_by_z_index(right, indent + 1);
    //printf("%*ssplit right: ", indent * 4, "");
    //print_entity_linked_list(right);

    return sorted_merge(left, right, indent + 1);
  }
}

#if 0
struct TreeMapNode 
{
  // TODO(Ryan): combine display information into single struct and have two is_dirty flags for each

  String8 name; // this is full file name
  f32 size;

  TreeMapNode *parent;
  TreeMapNode *first_child, *last_child;
  TreeMapNode *next, *prev;

  // for putting random stuff on node, e.g.
  // node.user_data_u32 = NODE_TYPE_FILE,NODE_TYPE_ALLOCATION, etc. (could colour them differently)
  void *user_data_ptr;
  u32 user_data_u32;
  f32 user_data_f32;

  // store index to parent rather than pointer to avoid dereferences that are costly for cache locality?
  // also don't have to worry about pointer stability on dynamic array resizes
  // i32 parent; // signed as we set to -1

  u32 index; // not settable by user
};

struct TreeMap 
{
  // IMPORTANT(Ryan): Working with trees, storing as doubly linked list
  // allows for simple forwards a reverse iteration (in conjunction with node flags)
  // e.g. can iterate from leaves upwards
  // This iteration can give DFS variants
  // So, prefer when tree structure static, i.e. no sorts taken place
  TreeMapNode *first_node, *last_node; // first_node is root

  // IMPORTANT(Ryan): Many cases if linked list, also add a hash map with it?
  TreeMapNode *first_leaf, *last_leaf;
  Map leaves[TreeMapNode *node, b32]; // TODO(Ryan): hash tables used everywhere

  // IMPORTANT(Ryan): Could have pointer to node as key to hash table storing say texture/colour information

  b32 is_dirty;
};

// refresh this part, and keep the 'data' part static (much quicker?)
struct DisplayNode 
{
  u32 treemap_node_index;
  Vec4F32 colour;

  Vec2F32 corner; // in render coordinates
  Vec2F32 size; // in render coordinates

};
struct TreeMapDisplay 
{
  u32 node_count;
  DisplayNode nodes[MAX_NUM_NODES]; // parallel to tree_map_nodes

  TreeMap *treemap;

  b32 is_dirty;
};

INTERNAL TreeMapNode *
child_node_at_index(TreeMapNode *first, u32 index)
{
  TreeMapNode *result = MD_NilNode();
  if(n >= 0)
  {
    int idx = 0;
    for(TreeMapNode *node = first; !TreeMapNodeIsNil(node); node = node->next, idx += 1)
    {
      if(idx == n)
      {
        result = node;
        break;
      }
    }
  }
  return result;
}

INTERNAL u32
index_of_child_node(TreeMapNode *node)
{
    int idx = 0;
    for(TreeMapNode *last = node->prev; !TreeMapNodeIsNil(last); last = last->prev, idx += 1);
    return idx;
}

INTERNAL TreeMapNode *
MD_RootFromNode(TreeMapNode *node)
{
    TreeMapNode *parent = node;
    for(TreeMapNode *p = parent; !TreeMapNodeIsNil(p); p = p->parent)
    {
        parent = p;
    }
    return parent;
}

INTERNAL MD_i64
MD_ChildCountFromNode(TreeMapNode *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(child, node->first_child))
    {
        result += 1;
    }
    return result;
}

INTERNAL TreeMapNode *
add_node(Arena *arena, TreeMapNode *parent, String8 name)
{
  TreeMapNode *node = PushArrayZero(arena, TreeMapNode, 1);
  node->name = s8_copy(arena, name);
  node->parent = parent;
  node->flags = NODE_FLAG_LEAF;
  // TODO(Ryan): Should we copy string over to allow for mutability?
  // TODO(Ryan): Introduce @speed, @memory, @nochecking, @copypasta, @cleanup comment modifications (interwoven with not just writing green-day code)
  // TODO(Ryan): Improve comment usage
  // TODO(Ryan): Have single line if with no braces to reduce indentation
  if (parent != NULL)
  {
    DLL_QUEUE_PUSH(parent->first, parent->last, node);
    parent &= ~NODE_FLAG_LEAF;
  }

  return result;
}

// Use these locally in a function if repeated calls
// undef later in file
#define EACH_TREEMAP_NODE(it, first) TreeMapNode *it = (first); (it != NULL); it = it->next
#define ENUMERATE_TREEMAP_NODE(it, first) \
  struct {TreeMapNode *it; u32 i} e = {(first), 0}; (e.it != NULL); e.it = e.it->next, e.i++;

f32 global_zoom_target = 1.0f;
f32 global_zoom_current = 1.0f;
f32 global_zoom_factor_max = 50.0f;

handle_mouse_wheel_zoom {
  f32 factor = SIGN_OF(event.wheel.y);
  f32 zoom = pow_f32(1.2, factor);
  global_zoom_target *= zoom;
  CLAMP(global_zoom_target, global_zoom_factor, 0.0f);

  inside_of_draw {
    global_zoom_current = \
      // tweaking last parameter to adjust speed of zoom
      move_toward_f32(global_zoom_current, global_zoom_target, dt, global_zoom_current * 3); 
  }
}

b32 up, up_alt;
handle_keyboard_pan {
  Vec2F32 dir;

  f32 up = (f32)(up || up_alt);

  // create unit vector for movement direction 
  dir = vec2_f32(right - left, up - down);

  delta = SPEED_BASE * (dir * dt) / zoom_current;

  offset += delta;
}

INTERNAL void
recompute_if_dirty(TreeMapDisplay *tree_map_display, Vec2F32 size, b32 force = false)
{
  // TODO(Ryan): Set is dirty if window size changes
  if (!tree_map_display->is_dirty || force) return;

  tree_map_display->is_dirty = false;

  TreeMap *tree_map = tree_map_display->tree_map;

  if (tree_map->nodes->count == 0) return;

  display_root->corner = ZERO_STRUCT;
  display_root->size = size;

  size_children(root, tree_map_display, 0);
}


INTERNAL void
size_children(TreeMapNode *root, TreeMapDisplay *display, u32 depth)
{
  TreeMapDisplayNode *display_node = display->first

  TreeMapDisplayNode *root_display = display->nodes->first;
  f32 root_area = root_display->size.x * root_display->size.y;
  f32 root_size = root_display->size;

  Vec2F32 corner = root_display->corner;
  Vec2F32 size = root_display->size;

  for (TreeMapchild *child = root->first; child != NULL; child = child->next)
  {
    if (display_node == NULL)
    {
      display_node = PUSH();
      SLL_QUEUE_INSERT();
    }

    f32 node_fraction = (child->size / root_size);
    f32 node_area = root_area * node_fraction; 

    display_node->corner = corner;

    // Alternate between vertical and horizontal
    f32 width, height = 0.0f;
    if (depth & 1)
    {
      height = node_area / size.x;
      width = size.x;

      corner.y += height;
    }
    else
    {
      width = node_area / size.y;
      height = size.y; // as h = a / w

      corner.x += width;
    }

    display_node->size = {width, height};
  }
  
  // 2nd pass
  for (child = root->first)
  {
    size_children(child, display, depth + 1);
  }
}


// lists over arrays generally, as copying in lists free
// could add parallel hash map if require random access

INTERNAL void
recompute_if_dirty(TreeMap *tree_map)
{
  if (!tree_map->is_dirty) 
  {
    return;
  }
  else
  {
    tree_map->dirty = false;

    // pass 1. clear all non-leaf sizes
    for (Node *node = node_list_first; node != NULL; node = node->next) 
    {
      if !(node->flags & LEAF) node->size = 0;
    }

    // pass 2. iterate from leaves, adding sizes to parents
    for (Node *node = node_list_last; node != NULL; node = node->prev) 
    {
      if (node->parent != NULL) node->parent.size += node->size;
    }

    // pass 3. sort children by size
    for (Node *node = node_list_first; node != NULL; node = node->next) 
    {
      // TODO(Ryan): have quick_sort() also for arrays?
      merge_sort(node->children, tree_map_size_cmp);
    }
  }
}

// TODO(Ryan): HH next particle video to look at curve animations?

// STD statically transmittable diseases

// i.e. files are not added to this map
GLOBAL Map global_directory_map;

INTERNAL void
add_directory_node(String8 full_name)
{
  path, basename = decomp(full_name);
  // remove path end slash

  MapSlot *map_slot = map_lookup(directory_map, );
  if (map_slot != NULL)
  {
    parent = (TreeNode *)map_slot->val;
  }
  else
  {
    // error
  }

  // TODO(Ryan): When adding a file_node, why is the size a float as its in bytes?

  TreeNode *node = add_node(arena, parent, full_name);
  map_insert(directory_map, full_name, node);
}

INTERNAL void
treemap_visit(MemArena *arena, FileInfo *file_info, void *user_data)
{
  if (file_info->flags & FILE_INFO_FLAG_DIRECTORY)
  {
    // add_directory_node(file_info->full_name);
  }
  else
  {
    // add_file_node(file_info->short_name);
  }
}

INTERNAL TreeMapNode *
tree_map_init(MemArena *arena)
{
  String8 directory = s8_lit("/home/ryan/prog/personal/sim");

  TreeMapNode *result = add_node(arena, );
  insert(global_directory_map, global_tree_map);
  
  linux_visit_files(arena, directory, treemap_visit, NULL, true);


  return result;
}

INTERNAL TreeMapDisplay
tree_map_display_init(TreeMap *tree_map)
{
  TreeMapDisplay display = ZERO_STRUCT;
  display.tree_map = tree_map;

  return display
}

// IMPORTANT(Ryan): Seems that drawing UI element, pass in rectangle region as first arg
// The base rectangle arg will be modified down the chain to other elements
// A final arg will be a UI element specific theme struct, e.g. LabelTheme.alignment = RIGHT, .font = ; 
// Furthermore, make units relative to say window_width, font_height etc.

// Load small/medium/large font variants
ui_techniques {
  draw_metrics(region) {
    graph_left_margin = 0.14f;
    graph_right_margin = 0.02f;
    graph_y_margin = 0.05f;
    graph_region = get_rect();

    draw_graph(graph_region) {
      line_height = 0.15f * window_height;
      line_stride = line_height * 1.2f;

      label(something);
      line.y -= line_height;
    }

    draw_graph_footer(region);
  }
}

whiten(fg, 0.8);
darken(Vec4F32 colour, f32 amount)
{
  Vec4F32 result = ZERO_STRUCT;
  result.r = lerp(colour.r, 0, amount);
  result.g = lerp(colour.g, 0, amount);
  result.b = lerp(colour.b, 0, amount);
}

ui_options = ENUM {NONE, THIN, MEDIUM, THICK};
ui_option_names = "None", "Thin", etc.
ui_something {

}

pressed = checkbox(r, "");
changed_val = number_input(r, text, 0, 90000);

// the rectange region passed is just for top option
ui_dropdown {
  d_rect = get_rect(region.x + label_width, y, width, height);
  l_rect.x = d.rect.x - label_width;
  l_rect.w = d.rect.x - l_rect.x;
  dropdown(d_rect, names_array, name_cur_index, dropdown_theme) {
    if (pressed) state.open = !state.open;
    if (state.open) state.open_t = move_toward(arrow_flip_down_rate);
    render_drawing_arrow();
  }
  label(l_rect);
}

dropdown_theme {
  flip_up_rate;
  color_over;
  color_down;
  color_flash;
}

// TODO(Ryan): Vec2F32 cut_right(), cut_left();

ui_click_select {
  // at end
  if (click_func != NULL && mouse_clicked)
  {
    click_func();
  }
}

// TODO(Ryan): Begin work in new files (and then add to git)
// TODO(Ryan): Begin work in functions, e.g init, compute, test, etc.
// TODO(Ryan): Error handling, try and give the user something instead of aborting
// TODO(Ryan): Always code to handle the worst case scenarios, e.g. divide-by-zero, NULL passed, etc. 
// Anyone can write "sunny day" code. Being able to write code to handle failures is the mark of a mature engineer.
// Take responsibility, and never make excuses! My absolute personal favorite excuse: "it worked yesterday". Don't ever say that! It's just downright embarrassing!
// Test, test. and test!!!  
// Never make assumptions!
INTERNAL void
draw_tree_map(RectF32 entire_region, Vec4F32 fg)
{
  Vec2F32 p0 = {
      renderer->render_width / 20.0f,
      renderer->render_width / 20.0f
    };
  Vec2F32 p1 = {
    renderer->render_width - p0.x,
    renderer->render_height - p0.y
  };

  // recompute_if_dirty(tree_map);
  // recompute_if_dirty(display_tree_map, p1 - p0);

  draw_rect(renderer->renderer, p0, p1 - p0, vec4_f32(0.12f, 0.12f, 0.12f, 1.0f));

  current_node = NULL;
  Vec2F32 screen_corner = p0;
  Vec2F32 centre = (p1 - p0) * 0.5;
  // for zooming coordinates
  centre_transform = (p - centre) * zoom_current + centre;

  for (iterate_nodes)
  {
    p0 = screen_corner + node->corner;
    p1 = p0 + node->size;

    if (mouse_in_node_rect)
    {
      current_node = node;
    }
  }

  Vec4F32 font_colour = vec4_f32(1.0f, 0.07f, 1.0f, 1.0f);

  Font *font = (Font *)map_val_assert(&state->asset_store.fonts, "droid-sans");

  if (current_node != NULL)
  {
    text = current_node->name;
  }

  PreparedText text = prepare_text(renderer->renderer, font, s8_lit("hi there"));

  Vec2F32 text_pos = {
    5.0f, 
    (renderer->render_height - 5.0f - (u32)font->character_height),
  };

  draw_prepared_text(renderer->renderer, &text, text_pos, font_colour);
}


NOTE(Ryan): Drawing widgets often have margin which are calculated depending on some property, e.g. width

NOTE(Ryan): Drawing widgets encompass there own update and then actual rendering
draw_widget()
{
   update_dt_value(); 
   now draw...
}

font = get_font_at_size();

text_input {
  if (text_input_active) SDL_StartTextInput();

  case BACKSPACE:
  {
    text_len--;
  }

  case SDL_TEXTINPUT:
  {
    if (text.count < MAX_BUFFER_SIZE)
    {
      text += event.text.text;
      last_keypress_time = get_ms();
    }
  }
       //Handle backspace
  if( e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0 )
  {
    //lop off character
    inputText.pop_back();
    renderText = true;
  }
  //Handle copy
  else if( e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
  {
    SDL_SetClipboardText( inputText.c_str() );
  }
  //Handle paste
  else if( e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
  {
    inputText = SDL_GetClipboardText();
    renderText = true;
  }

  draw_rect(input_rect);
  text_x = input_rect.x + font.height * left_pad;
  text_y = input_rect.y + font.height * top_pad;

  // text backdrop
  b = font_height * 0.05f;
  if (b < 2) b = 2;
  k = 0.05f;
  bg_color = {k, k, k, 0.9};
  draw_text(text_rect + b, bg_colour);
  draw_text(text_rect);

  // TODO(Ryan): Make cursor a line instead of box
  // this subtraction means it will pulse on keypresses
  t = cos_f32((time_now - last_keypress_time) * 3);
  t *= t;

  cursor_colour = lerp(white, non_white, t);

  cursor.x = text_rect.x + text_width;
  cursor.w = cursor.h = font_height;
}




#endif

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
    state->asset_store.fonts = map_create(perm_arena);
    state->asset_store.audio = map_create(perm_arena);
   
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
    asset_store_add_texture(renderer->renderer, &state->asset_store.textures, perm_arena,
                            "chopper-image", "./chopper.png");

    asset_store_add_font(renderer->renderer, &state->asset_store.fonts, perm_arena,
                            "droid-sans", "./DroidSans.ttf", 24);

    Entity *tank = push_entity(&state->first_free_entity, perm_arena, &state->first_entity, &state->last_entity, 
                               ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_RIGID_BODY | ENTITY_COMPONENT_FLAG_SPRITE);
    tank->transform_component.position = {100.0f, 100.0f};
    tank->transform_component.scale = {1.0f, 1.0f};
    tank->transform_component.rotation = 1.0f;
    tank->rigid_body_component.velocity = {8.0f, 2.0f};
    tank->sprite_component.dimensions = {32.0f, 32.0f}; // actual image dimensions
    tank->sprite_component.texture_key = map_key_str(s8_lit("tank-image"));
    tank->sprite_component.z_index = 5;

    Entity *tank2 = push_entity(&state->first_free_entity, perm_arena, &state->first_entity, &state->last_entity, 
                               ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_RIGID_BODY | ENTITY_COMPONENT_FLAG_SPRITE);
    tank2->transform_component.position = {200.0f, 100.0f};
    tank2->transform_component.scale = {2.0f, 2.0f};
    tank2->transform_component.rotation = 2.0f;
    tank2->rigid_body_component.velocity = {-8.0f, 2.0f};
    tank2->sprite_component.dimensions = {32.0f, 32.0f};
    tank2->sprite_component.texture_key = map_key_str(s8_lit("tank-image"));
    tank2->sprite_component.z_index = 1;

    Entity *truck = push_entity(&state->first_free_entity, perm_arena, &state->first_entity, &state->last_entity, 
                               ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_RIGID_BODY | ENTITY_COMPONENT_FLAG_SPRITE | ENTITY_COMPONENT_FLAG_BOX_COLLIDER);
    truck->transform_component.position = {300.0f, 300.0f};
    truck->transform_component.scale = {1.0f, 1.0f};
    truck->transform_component.rotation = 3.0f;
    truck->rigid_body_component.velocity = {0.0f, 0.0f};
    truck->sprite_component.dimensions = {32.0f, 32.0f};
    truck->sprite_component.texture_key = map_key_str(s8_lit("truck-image"));
    truck->sprite_component.z_index = 1;
    truck->box_collider_component.size = truck->sprite_component.dimensions;

    Entity *chopper = push_entity(&state->first_free_entity, perm_arena, &state->first_entity, &state->last_entity, 
                               ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_RIGID_BODY | ENTITY_COMPONENT_FLAG_SPRITE | ENTITY_COMPONENT_FLAG_ANIMATION);
    chopper->transform_component.position = {600.0f, 300.0f};
    chopper->transform_component.scale = {1.0f, 1.0f};
    chopper->transform_component.rotation = 4.0f;
    chopper->rigid_body_component.velocity = {0.0f, 0.0f};
    chopper->sprite_component.dimensions = {32.0f, 32.0f};
    chopper->sprite_component.texture_key = map_key_str(s8_lit("chopper-image"));
    chopper->sprite_component.z_index = 1;
    chopper->animation_component.num_frames = 2;
    chopper->animation_component.current_frame = 0;
    chopper->animation_component.frame_rate = 10;
    chopper->animation_component.should_loop = true;
    chopper->animation_component.start_time = state->ms;

#pragma mark LOAD_LEVEL_END
  } 

  for (Entity *entity = state->first_entity; entity != NULL; entity = entity->next)
  {

    if (HAS_FLAGS_ALL(entity->component_flags, (ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_RIGID_BODY)))
    {
      // Introducing 'equations of motion' essentially means giving momentum, i.e. no instantaneous direction changes
      // p = 0.5f * acceleration * SQUARE(time) + velocity * time + position;
      // v = acceleration * t + v
      // if (down) acceleration = 1.0f;
      // acceleration += -0.7f * v (friction)

      // TODO(Ryan): add acceleration
      entity->transform_component.position += entity->rigid_body_component.velocity * state->delta;

      // TODO(Ryan): add rotation matrices (from app_template)
    }


    if (HAS_FLAGS_ALL(entity->component_flags, (ENTITY_COMPONENT_FLAG_SPRITE | ENTITY_COMPONENT_FLAG_ANIMATION)))
    {
      AnimationComponent *anim = &entity->animation_component;

      u32 time_elapsed = (state->ms - anim->start_time);

      anim->current_frame = (u32)(time_elapsed * (anim->frame_rate / 1000.0f)) % anim->num_frames;

      entity->sprite_component.texture_offset.x = anim->current_frame * entity->sprite_component.dimensions.w;
    }


    if (HAS_FLAGS_ALL(entity->component_flags, (ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_BOX_COLLIDER)))
    {
      for (Entity *collision_entity = entity->next; collision_entity != NULL; collision_entity = collision_entity->next)
      {
        SDL_Rect entity_bounding = \
          vec2_f32_to_sdl_rect(entity->transform_component.position, entity->box_collider_component.size);
        SDL_Rect collision_entity_bounding = \
          vec2_f32_to_sdl_rect(collision_entity->transform_component.position, collision_entity->box_collider_component.size);

        // AABB
        if (SDL_HasIntersection(&entity_bounding, &collision_entity_bounding))
        {
          // TODO(Ryan): remove both entities
          CollisionEvent collision_event = ZERO_STRUCT;
        }
      }
    }
  }

  // TODO(Ryan): Convert pixels/s to m/s

  // TODO(Ryan): process event queues here?

  // collision map for pixel perfect (mesh collider for 3d)

  // TODO(Ryan): map render, cache texture from map asset store if doing subsets?

  // TODO(Ryan): update state->last_entity as well
  //printf("starting: ");
  //print_entity_linked_list(state->first_entity);
  state->first_entity = sort_entities_by_z_index(state->first_entity, 0);
  //printf("final: ");
  //print_entity_linked_list(state->first_entity);

#if 0
  for (Entity *entity = state->first_entity; entity != NULL; entity = entity->next)
  {
    if (HAS_FLAGS_ALL(entity->component_flags, (ENTITY_COMPONENT_FLAG_TRANSFORM | ENTITY_COMPONENT_FLAG_SPRITE)))
    {

      if (state->debug_overlay)
      {
        if (HAS_FLAGS_ALL(entity->component_flags, (ENTITY_COMPONENT_FLAG_BOX_COLLIDER)))
        {
          SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 255);

          SDL_FRect dst_rect = vec2_f32_to_sdl_frect(entity->transform_component.position, entity->box_collider_component.size);
          SDL_RenderDrawRectF(renderer->renderer, &dst_rect);
        }
      }

      Vec2F32 texture_dim = vec2_f32_hadamard(entity->sprite_component.dimensions, entity->transform_component.scale);
      draw_texture(renderer->renderer, &state->asset_store.textures, entity->sprite_component.texture_key,
                   entity->transform_component.position, texture_dim, entity->sprite_component.texture_offset);
    }
  }

  // NOTE(Ryan): Particle system test
  // more randomness the better
#define PARTICLE_DENSITY_COUNT 1
  for (u32 particle_spawn_i = 0; particle_spawn_i < PARTICLE_DENSITY_COUNT; particle_spawn_i++)
  {
    Particle *particle = &state->particles[state->next_particle++];
    if (state->next_particle == ARRAY_COUNT(state->particles))
    {
      state->next_particle = 0;
    }

    particle->position = vec2_f32(400.0f, 400.0f);

    particle->velocity = vec2_f32(rand_bilateral_f32(&state->rand_seed) * 100.0f, -100.0f);
    particle->acceleration = vec2_f32(0.0f, 220.0f);
    particle->colour = vec4_f32(0.8f, 0.5f, 0.9f, 1.0f);
    particle->colour_velocity = vec4_f32(0.0f, 0.0f, 0.0f, -0.5f);
  }

  for (u32 particle_i = 0; particle_i < ARRAY_COUNT(state->particles); particle_i++)
  {
    Particle *particle = &state->particles[particle_i];

    particle->position += particle->velocity * state->delta;
    if (particle->position.y > 405.0f)
    {
      f32 restitution = 0.5f;
      particle->position.y = 405.0f;
      particle->velocity.y = restitution * -particle->velocity.y;
    }

    particle->velocity += particle->acceleration * state->delta;
    particle->colour += particle->colour_velocity * state->delta;

    // TODO(Ryan): Store texture width and height
    draw_texture(renderer->renderer, &state->asset_store.textures, map_key_str(s8_lit("tank-image")),
                 particle->position, vec2_f32(32.0f, 32.0f), vec2_i32(0, 0), 0.0f, particle->colour);
  }
#endif
}

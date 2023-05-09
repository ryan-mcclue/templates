// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(APP_H)
#define APP_H

typedef u32 ENTITY_COMPONENT_FLAG;
enum
{
  ENTITY_COMPONENT_FLAG_TRANSFORM = (1 << 0),
  ENTITY_COMPONENT_FLAG_SPRITE = (1 << 1),
  ENTITY_COMPONENT_FLAG_RIGID_BODY = (1 << 2),
  ENTITY_COMPONENT_FLAG_PROJECTILE = (1 << 3),
  ENTITY_COMPONENT_FLAG_ANIMATION = (1 << 4),
  // ...
};

// COMBINATORIC DATA:
// IMPORTANT: “what something is” and how its memory should be interpreted are different
// “why do I even care what ‘is a button’ or what ‘is a slider’, to the degree that I have to store it at each node in the hierarchy?”
// higher level features can be described at a lower level with what data we have, what data we need to produce from that data, and a function (in the mathematical sense) taking our inputs to our outputs.
// IMPORTANT: no discriminated unions now (okay to have 'waste' data)

struct AnimationComponent
{
  u32 num_frames, current_frame, frame_rate; // frames per second
  b32 should_loop;
  u32 start_time;
};

struct Entity
{
  ENTITY_COMPONENT_FLAG component_flags;

  Entity *next, *prev;

  struct TransformComponent
  {
    Vec2F32 position, scale;
    f32 rotation;
  } transform_component;

  struct SpriteComponent
  {
    Vec2F32 dimensions;
    SDL_Rect src_rect; 
    // TODO(Ryan): OPTIMAL SOLUTION IS TO HAVE LAYERS, E.G. VEGETATION LAYER, BULLET LAYER, ETC.
    u32 z_index;
    MapKey texture_key;
  } sprite_component;

  struct RigidBodyComponent
  {
    Vec2F32 velocity;
  } rigid_body_component;

  AnimationComponent animation_component;
};

struct AssetStore
{
  Map textures;
};

IGNORE_WARNING_PADDED()
typedef struct AppState AppState;
struct AppState
{
  b32 debugger_present;
  b32 is_initialised;
  f32 delta;
  u64 ms;
  u32 rand_seed;

  Entity *first_free_entity;
  Entity *first_entity;
  Entity *last_entity;

  AssetStore asset_store;
};
IGNORE_WARNING_POP()

IGNORE_WARNING_PADDED()
typedef struct Renderer Renderer;
struct Renderer
{
  SDL_Renderer *renderer;
  u32 window_width, window_height;
  u32 render_width, render_height;
};
IGNORE_WARNING_POP()

IGNORE_WARNING_PADDED()
typedef struct Input Input;
struct Input
{
  b32 move_left, move_right, move_up, bullet_fired;
  f32 mouse_x, mouse_y; 
  b32 mouse_clicked;
  b32 mouse_pressed;
  b32 mouse_held;
};
IGNORE_WARNING_POP()

typedef void (*app_func)(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena);

EXPORT void
app(AppState *state, Renderer *renderer, Input *input, MemArena *perm_arena);

#endif

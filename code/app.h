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
  ENTITY_COMPONENT_FLAG_BOX_COLLIDER = (1 << 5),
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

// could also have circle, pixel-perfect etc.
struct BoxColliderComponent
{
  Vec2F32 size, offset;
};

struct SpriteComponent
{
  Vec2F32 dimensions;
  Vec2I32 texture_offset; 
  // TODO(Ryan): OPTIMAL SOLUTION IS TO HAVE LAYERS, E.G. VEGETATION LAYER, BULLET LAYER, ETC.
  u32 z_index;
  MapKey texture_key;
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

  SpriteComponent sprite_component;

  BoxColliderComponent box_collider_component;

  struct RigidBodyComponent
  {
    Vec2F32 velocity;
  } rigid_body_component;

  AnimationComponent animation_component;
};

struct Font
{
  TTF_Font *font;
  i32 character_height;
};

struct PreparedText
{
  SDL_Texture *texture;
  i32 width, height;
};

struct AssetStore
{
  Map textures;
  Map fonts;
  Map audio;
};


struct CollisionEvent
{
  Entity *a, *b;
};

struct Particle
{
  Vec2F32 position, velocity, acceleration;
  Vec4F32 colour, colour_velocity;
};

IGNORE_WARNING_PADDED()
typedef struct AppState AppState;
struct AppState
{
  b32 debugger_present;
  b32 is_initialised;
  f32 delta;
  u64 ms;
  u32 rand_seed; // perhaps rename effects_series?

  b32 debug_overlay;

  Entity *first_free_entity;
  Entity *first_entity;
  Entity *last_entity;

  u32 next_particle;
  Particle particles[64];

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

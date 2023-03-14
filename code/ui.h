// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(UI_H)
#define UI_H

/*
// IMPORTANT(Ryan): Rename Widget to Box, so as to extend UI decomposition, e.g:
 List Box Region (child layout on x)
| * Scrollable Region (fill space, clip rectangle, overflow y)
| | * List Item 1 (clickable text)
| | * List Item 2 (clickable text)
| | * List Item 3 (clickable text)
| | * List Item 4 (clickable text)
| | * (etc.)
|
| * Scroll Bar Region (fixed size, child layout on y)
| | * Scroll-Up Button (clickable button with up-arrow)
| | * Space Before Scroller
| | * Scroller (fixed size, proportional to view - draggable)
| | * Space After Scroller
| | * Scroll-Down Button (clickable button with down-arrow) 

Single-line text field:
* Container (allow overflow in x, clip rectangle, scrollable, clickable)
| * Text Content (text + extra rendering for cursor/selection)

* Menu Bar Container (layout children in x)
| * File Menu Bar Button (clickable. if the menu bar is not 
|                         activated, then activate the menu bar
|                         and load this button's menu on *press*. |                         if it is activated, then activate on
|                         *hover*.)
| * Window Menu Bar Button
| * Panel Menu Bar Button
| * View Menu Bar Button
| * Control Menu Bar Button

* Button "Container" (clickable, but click happens after children)
| * Icon Label
| * Text Label
| * Space, to align to right-hand-side
| * Binding Button (clickable, with other special behavior)
*/

enum AXIS2
{
  AXIS2_X,
  AXIS2_Y,
  AXIS2_COUNT
};

typedef struct UIKey UIKey;
struct UIKey
{
  u64 key;
};

typedef u32 UI_SIZE_KIND;
enum
{
  UI_SIZE_KIND_NULL,
  UI_SIZE_KIND_PIXELS,
  UI_SIZE_KIND_TEXT_CONTENT,
  UI_SIZE_KIND_PERCENT_OF_PARENT,
  UI_SIZE_KIND_CHILDREN_SUM,
};
typedef struct UISize UISize;
struct UISize
{
	UI_SIZE_KIND kind;
	f32 value; // 0 for ChildrenSum
	f32 strictness; // 1 == no budge
};

typedef u32 UI_BOX_FLAG;
enum
{
	UI_BOX_FLAG_CLICKABLE       =  (1 << 0),    // @done
	UI_BOX_FLAG_VIEW_SCROLL      = (1 << 1),  // TODO hard
	UI_BOX_FLAG_DRAW_TEXT        = (1 << 2),  // @done
	UI_BOX_FLAG_DRAW_BORDER      = (1 << 3),  // @done
	UI_BOX_FLAG_DRAW_BACKGROUND  = (1 << 4),  // @done
	UI_BOX_FLAG_DRAW_DROP_SHADOW = (1 << 5),   // @done
	UI_BOX_FLAG_CLIP            =  (1 << 6), // @done
	UI_BOX_FLAG_HOT_ANIMATION    = (1 << 7),  // @done
	UI_BOX_FLAG_ACTIVE_ANIMATION = (1 << 8),  // @done
	UI_BOX_FLAG_CUSTOM_RENDERER  = (1 << 9),  // @done
};

typedef struct UICache UICache;
typedef struct UIBox UIBox;
typedef void (*ui_render_function)(UICache *cache, UIBox *box);


struct UIBox
{
  // NOTE(Ryan): Per-frame
  UIBox *first;
  UIBox *last;
  UIBox *next;
  UIBox *prev;
  UIBox *parent;

  // NOTE(Ryan): Persistent
  // TODO(Ryan): What exactly are these pointing to? used at begin and end frame
  UIBox *hash_next;
  UIBox *hash_prev;
  UIKey key;
  u64 last_frame_touched_index; // if < current_frame_index then 'pruned'
  b32 direct_set; // ??

  // NOTE(Ryan): From builders
  UI_BOX_FLAG flags;
  String8 identifier;

  b32 pressed_on_this; // ??

	// layouting
	UISize semantic_size[AXIS2_COUNT];
	f32 computed_size[AXIS2_COUNT]; // final pixel size
	AXIS2 layout_axis;
	f32 computed_rel_position[AXIS2_COUNT]; // size relative to parent

  // final display coordinates.
  // this will be used next frame for input event consumption
  // this will be used this frame for rendering
	Vec2F32 target_bounds;
	Vec2F32 bounds;
	Vec2F32 clipped_bounds;
	
	f32 hot_t;
	u32 hot_color;
  // exponential animation curves for animating these two values (_t for transition)
	f32 active_t;
	u32 active_color;
	b8 is_on;
	
  // styling (would these be active styles, as oppose to what is in UICache?)
	Font font;
	u32 color;
	u32 edge_color;
	u32 text_color;
	f32 rounding;
	f32 softness;
	f32 edge_size;

	ui_render_function* custom_render_function;
};

typedef struct UISignal UISignal;
struct UISignal
{
  //UIBox *box;
  //Vec2F32 mouse;
  //Vec2F32 drag_delta;

	b32 clicked;        // @done
	b32 double_clicked; // TODO
	b32 right_clicked;  // @done
	b32 pressed;        // @done
	b32 released;       // @done
	b32 dragging;       // TODO
	b32 hovering;       // @done
};

typedef struct RectangleStack RectangleStack;
struct RectangleStack
{
  RectangleStack *next;
  Rectangle val;
};
typedef struct U32Stack U32Stack;
struct U32Stack
{
  U32Stack *next;
  u32 val;
};
typedef struct F32Stack F32Stack;
struct F32Stack
{
  F32Stack *next;
  f32 val;
};
typedef struct UIBoxStack UIBoxStack;
struct UIBoxStack
{
  UIBoxStack *next;
  UIBox val;
};
typedef struct FontStack FontStack;
struct FontStack
{
  FontStack *next;
  Font val;
};
typedef struct ColorStack ColorStack;
struct ColorStack
{
  ColorStack *next;
  Color val;
};
typedef struct UISizeStack UISizeStack;
struct UISizeStack
{
  UISizeStack *next;
  UISize val;
};
typedef struct AXIS2Stack AXIS2Stack;
struct AXIS2Stack
{
  AXIS2Stack *next;
  AXIS2 val;
};
typedef struct UIRenderFunctionStack UIRenderFunctionStack;
struct UIRenderFunctionStack
{
  UIRenderFunctionStack *next;
  ui_render_function val;
};

struct UICache
{
  // NOTE(Ryan): This is temp arena
  MemArenaTemp style_stack_arena;

  // map_create(closest prime to power of 2?);
  // this is a hashmap (Map from base)
  // UIKey -> UIBox
  UIBox *cache_elems;
  u32 cache_len; // this is num_slots

  // NOTE(Ryan): Style stacks
  struct parent_stack { UIBoxStack *first; } parent_stack;
  struct font_stack { FontStack *first; } font_stack;
  struct bg_color_stack { ColorStack *first; } bg_color_stack;
  struct hot_color_stack { ColorStack *first; } hot_color_stack;
  struct active_color_stack { ColorStack *first; } active_color_stack;
  struct rounding_stack { F32Stack *first; } rounding_stack;
  struct softness_stack { F32Stack *first; } softness_stack;
  struct edge_size_stack { F32Stack *first; } edge_size_stack;
  struct edge_color_stack { U32Stack *first; } edge_color_stack;
  struct text_color_stack { U32Stack *first; } text_color_stack;
  struct pref_width_stack { UISizeStack *first; } pref_width_stack;
  struct pref_height_stack { UISizeStack *first; } pref_height_stack;
  struct layout_axis_stack { AXIS2Stack *first; } layout_axis_stack;
  struct clip_rect_stack { RectangleStack *first; } clip_rect_stack;
  struct render_function_stack { UIRenderFunctionStack *first; } render_function_stack;

	Font default_font;
  f32 default_font_size;
	
	UIBox* root;
	u64 current_frame_index;
	
	UIKey hot_key;
	UIKey active_key;
};

INTERNAL void 
ui_push_clip_rect(UICache *cache, Rectangle clip_rect)
{
  RectangleStack *window_rect = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, RectangleStack);
  window_rect->val = clip_rect; 
  SLL_STACK_PUSH(cache->clip_rect_stack.first, window_rect);
}

INTERNAL void
ui_pop_clip_rect(UICache *cache)
{
  SLL_STACK_POP(cache->clip_rect_stack.first);
}

INTERNAL void 
ui_push_font(UICache *cache, Font font)
{
  FontStack *font_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, FontStack);
  font_stack->val = font; 
  SLL_STACK_PUSH(cache->font_stack.first, font_stack);
}

INTERNAL void
ui_pop_font(UICache *cache)
{
  SLL_STACK_POP(cache->font_stack.first);
}

INTERNAL void
ui_cache_init(UICache *cache, u32 window_width, u32 window_height)
{
  cache->default_font = LoadFont("DroidSans.ttf");
  ASSERT(cache->default_font.texture.id != 0);
  cache->default_font_size = 24.0f; 

  ui_push_clip_rect(cache, {0, 0, (f32)window_width, (f32)window_height});
  ui_push_font(cache, cache->default_font);
  ui_push_box_color(cache, 0x111111FF); // bg color
  ui_push_hot_color(cache, 0x131313FF);
  ui_push_active_color(cache, 0x131313FF);
  ui_push_edge_color(cache, 0x9A5EBDFF);
  ui_push_text_color(cache, 0xFFAAFFFF);
  ui_push_rounding(cache, 5.0f);
  ui_push_edge_softness(cache, 2.0f);
  ui_push_edge_size(cache, 2.0f);
  ui_push_pref_width(cache, ui_pixels(100));
  ui_push_height_width(cache, ui_pixels(100));
  ui_push_layout_axis(cache, AXIS2_Y);
  ui_push_custom_render_function(cache, NULL);
}


#endif

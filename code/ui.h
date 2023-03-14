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
	f32 value;
	// TODO(Ryan):  f32 strictness; // 1 == no budge
};

INTERNAL UISize 
ui_size_pixels(f32 pixels)
{
  UISize result = ZERO_STRUCT;

  result.kind = UI_SIZE_KIND_PIXELS;
  result.value = pixels;

  return result;
}

INTERNAL UISize 
ui_size_text_content(f32 padding)
{
  UISize result = ZERO_STRUCT;

  result.kind = UI_SIZE_KIND_TEXT_CONTENT;
  result.value = padding;

  return result;
}

INTERNAL UISize
ui_size_percentage(f32 pct) 
{
  UISize result = ZERO_STRUCT;

  result.kind = UI_SIZE_KIND_PERCENT_OF_PARENT;
  result.value = pct;

  return result;
}

INTERNAL UISize
ui_size_children_sum(void) 
{
  UISize result = ZERO_STRUCT;

  result.kind = UI_SIZE_KIND_CHILDREN_SUM;

  return result;
}

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
  MemArenaTemp style_stack_arena;

  // map_create(closest prime to power of 2?);
  // this is a hashmap (Map from base)
  // UIKey -> UIBox
  // so, this should be:
  // Map persistent_elems; ?
  UIBox *cache_elems;
  u32 cache_len; // this is num_slots

  // NOTE(Ryan): Style stacks
  struct parent_stack { UIBoxStack *first; } parent_stack;
  struct font_stack { FontStack *first; } font_stack;
  struct bg_color_stack { ColorStack *first; } bg_color_stack;
  struct hot_color_stack { ColorStack *first; } hot_color_stack;
  struct active_color_stack { ColorStack *first; } active_color_stack;
  struct edge_rounding_stack { F32Stack *first; } edge_rounding_stack;
  struct edge_softness_stack { F32Stack *first; } edge_softness_stack;
  struct edge_size_stack { F32Stack *first; } edge_size_stack;
  struct edge_color_stack { ColorStack *first; } edge_color_stack;
  struct text_color_stack { ColorStack *first; } text_color_stack;
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
ui_push_bg_color(UICache *cache, Color bg_color)
{
  ColorStack *bg_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  bg_color_stack->val = bg_color; 
  SLL_STACK_PUSH(cache->bg_color_stack.first, bg_color_stack);
}

INTERNAL void
ui_pop_bg_color(UICache *cache)
{
  SLL_STACK_POP(cache->bg_color_stack.first);
}

INTERNAL void 
ui_push_hot_color(UICache *cache, Color hot_color)
{
  ColorStack *hot_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  hot_color_stack->val = hot_color; 
  SLL_STACK_PUSH(cache->hot_color_stack.first, hot_color_stack);
}

INTERNAL void
ui_pop_hot_color(UICache *cache)
{
  SLL_STACK_POP(cache->hot_color_stack.first);
}

INTERNAL void 
ui_push_active_color(UICache *cache, Color active_color)
{
  ColorStack *active_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  active_color_stack->val = active_color; 
  SLL_STACK_PUSH(cache->active_color_stack.first, active_color_stack);
}

INTERNAL void
ui_pop_active_color(UICache *cache)
{
  SLL_STACK_POP(cache->active_color_stack.first);
}

INTERNAL void 
ui_push_edge_color(UICache *cache, Color edge_color)
{
  ColorStack *edge_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  edge_color_stack->val = edge_color; 
  SLL_STACK_PUSH(cache->edge_color_stack.first, edge_color_stack);
}

INTERNAL void
ui_pop_edge_color(UICache *cache)
{
  SLL_STACK_POP(cache->edge_color_stack.first);
}

INTERNAL void 
ui_push_text_color(UICache *cache, Color text_color)
{
  ColorStack *text_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  text_color_stack->val = text_color; 
  SLL_STACK_PUSH(cache->text_color_stack.first, text_color_stack);
}

INTERNAL void
ui_pop_text_color(UICache *cache)
{
  SLL_STACK_POP(cache->text_color_stack.first);
}

INTERNAL void 
ui_push_edge_rounding(UICache *cache, f32 edge_rounding)
{
  F32Stack *edge_rounding_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, F32Stack);
  edge_rounding_stack->val = edge_rounding;
  SLL_STACK_PUSH(cache->edge_rounding_stack.first, edge_rounding_stack);
}

INTERNAL void
ui_pop_edge_rounding(UICache *cache)
{
  SLL_STACK_POP(cache->edge_rounding_stack.first);
}

INTERNAL void 
ui_push_edge_softness(UICache *cache, f32 edge_softness)
{
  F32Stack *edge_softness_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, F32Stack);
  edge_softness_stack->val = edge_softness;
  SLL_STACK_PUSH(cache->edge_softness_stack.first, edge_softness_stack);
}

INTERNAL void
ui_pop_edge_softness(UICache *cache)
{
  SLL_STACK_POP(cache->edge_softness_stack.first);
}

INTERNAL void 
ui_push_edge_size(UICache *cache, f32 edge_size)
{
  F32Stack *edge_size_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, F32Stack);
  edge_size_stack->val = edge_size;
  SLL_STACK_PUSH(cache->edge_size_stack.first, edge_size_stack);
}

INTERNAL void
ui_pop_edge_size(UICache *cache)
{
  SLL_STACK_POP(cache->edge_size_stack.first);
}

INTERNAL void 
ui_push_pref_width(UICache *cache, UISize pref_width)
{
  UISizeStack *pref_width_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, UISizeStack);
  pref_width_stack->val = pref_width;
  SLL_STACK_PUSH(cache->pref_width_stack.first, pref_width_stack);
}

INTERNAL void
ui_pop_pref_width(UICache *cache)
{
  SLL_STACK_POP(cache->pref_width_stack.first);
}

INTERNAL void 
ui_push_pref_height(UICache *cache, UISize pref_height)
{
  UISizeStack *pref_height_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, UISizeStack);
  pref_height_stack->val = pref_height;
  SLL_STACK_PUSH(cache->pref_height_stack.first, pref_height_stack);
}

INTERNAL void
ui_pop_pref_height(UICache *cache)
{
  SLL_STACK_POP(cache->pref_height_stack.first);
}

INTERNAL void 
ui_push_layout_axis(UICache *cache, AXIS2 layout_axis)
{
  AXIS2Stack *layout_axis_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, AXIS2Stack);
  layout_axis_stack->val = layout_axis;
  SLL_STACK_PUSH(cache->layout_axis_stack.first, layout_axis_stack);
}

INTERNAL void
ui_pop_layout_axis(UICache *cache)
{
  SLL_STACK_POP(cache->layout_axis_stack.first);
}

INTERNAL void 
ui_push_render_function(UICache *cache, ui_render_function render_function)
{
  UIRenderFunctionStack *render_function_stack = \
    MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, UIRenderFunctionStack);
  render_function_stack->val = render_function;
  SLL_STACK_PUSH(cache->render_function_stack.first, render_function_stack);
}

INTERNAL void
ui_pop_render_function(UICache *cache)
{
  SLL_STACK_POP(cache->render_function_stack.first);
}

#define UI_SET_PARENT(ui_cache, parent) \
  DEFER_LOOP(ui_push_parent(ui_cache, parent), ui_pop_parent(ui_cache))

#define UI_SET_FONT(ui_cache, font) \
  DEFER_LOOP(ui_push_font(ui_cache, font), ui_pop_font(ui_cache))

#define UI_SET_BG_COLOR(ui_cache, bg_color) \
  DEFER_LOOP(ui_push_bg_color(ui_cache, bg_color), ui_pop_bg_color(ui_cache))

#define UI_SET_HOT_COLOR(ui_cache, hot_color) \
  DEFER_LOOP(ui_push_hot_color(ui_cache, hot_color), ui_pop_hot_color(ui_cache))

#define UI_SET_ACTIVE_COLOR(ui_cache, active_color) \
  DEFER_LOOP(ui_push_active_color(ui_cache, active_color), ui_pop_active_color(ui_cache))

#define UI_EDGE_ROUNDING(ui_cache, rounding) \
  DEFER_LOOP(ui_push_edge_rounding(ui_cache, rounding), ui_pop_edge_rounding(ui_cache))

#define UI_EDGE_SOFTNESS(ui_cache, softness) \
  DEFER_LOOP(ui_push_edge_softness(ui_cache, softness), ui_pop_edge_softness(ui_cache))

#define UI_EDGE_SIZE(ui_cache, edge_size) \
  DEFER_LOOP(ui_push_edge_size(ui_cache, edge_size), ui_pop_edge_size(ui_cache))

#define UI_EDGE_COLOR(ui_cache, edge_color) \
  DEFER_LOOP(ui_push_edge_color(ui_cache, edge_color), ui_pop_edge_color(ui_cache))

#define UI_TEXT_COLOR(ui_cache, text_color) \
  DEFER_LOOP(ui_push_text_color(ui_cache, text_color), ui_pop_text_color(ui_cache))

#define UI_PREF_WIDTH(ui_cache, pref_width) \
  DEFER_LOOP(ui_push_pref_width(ui_cache, pref_width), ui_pop_pref_width(ui_cache))

#define UI_PREF_HEIGHT(ui_cache, pref_height) \
  DEFER_LOOP(ui_push_pref_height(ui_cache, pref_height), ui_pop_pref_height(ui_cache))

#define UI_LAYOUT_AXIS(ui_cache, layout_axis) \
  DEFER_LOOP(ui_push_layout_axis(ui_cache, layout_axis), ui_pop_layout_axis(ui_cache))

#define UI_RENDER_FUNCTION(ui_cache, render_function) \
  DEFER_LOOP(ui_push_render_function(ui_cache, render_function), ui_pop_render_function(ui_cache))

#define UI_CLIP_RECT(ui_cache, clipping_rect) \
  DEFER_LOOP(ui_push_clip_rect(ui_cache, clipping_rect), ui_pop_clip_rect(ui_cache))


INTERNAL void
ui_cache_init(UICache *cache, u32 window_width, u32 window_height)
{
  cache->default_font = LoadFont("DroidSans.ttf");
  ASSERT(cache->default_font.texture.id != 0);
  cache->default_font_size = 24.0f; 

  ui_push_clip_rect(cache, {0, 0, (f32)window_width, (f32)window_height});

  ui_push_font(cache, cache->default_font);

  ui_push_bg_color(cache, {17, 17, 17, 255});
  ui_push_hot_color(cache, {19, 19, 19, 255});
  ui_push_active_color(cache, {19, 19, 19, 255});
  ui_push_edge_color(cache, {154, 94, 189, 255});
  ui_push_text_color(cache, {255, 170, 255, 255});

  ui_push_edge_rounding(cache, 5.0f);
  ui_push_edge_softness(cache, 2.0f);
  ui_push_edge_size(cache, 2.0f);

  ui_push_pref_width(cache, ui_size_pixels(100));
  ui_push_pref_height(cache, ui_size_pixels(100));

  ui_push_layout_axis(cache, AXIS2_Y);

  ui_push_render_function(cache, NULL);
}

// TODO(Ryan): rename to ui_box_instance
INTERNAL UIBox *
ui_make_box(UICache *cache, UI_BOX_FLAGS flags, String8 str)
{
  if (str.size == 0)
  {
    // TODO(Ryan): Allocate in temporary memory, i.e. not in cache
  }
  else
  {

	// Check cache and return Box from last frame if hit
	// Else add a new Box to the cache
  MapKey key = map_key_str(str);
	
	UIBox *result = map_lookup(cache->box_map, key);
  if (result != NULL)
  {
    result->last_frame_touched_index = cache->current_frame_index; 

    UIBox *parent = cache->parent_stack.first;
    if (parent != NULL)
    {
      // add this box to parents children
      result->parent = parent;
      DLL_QUEUE_PUSH(parent->first, parent->last, result);
      result->prev = parent-> ;
    }

    result->direct_set = false; // ??
    // clear children list
    result->next = NULL;
    result->first = NULL;
    result->last = NULL;
  }
  else
  {
    result = new_box;
    add_to_cache_map();

    result->last_frame_touched_index = cache->current_frame_index;
    UIBox *parent = cache->parent_stack.first;
    if (parent != NULL)
    {
      result->parent = parent;
      result->flags = flags;
      result->identifier = str;

      // add this box to parents children
      result->parent = parent;
      DLL_QUEUE_PUSH(parent->first, parent->last, result);
      result->prev = parent-> ;

      result->direct_set = true; // ??
      result->font = cache->font_stack.first;
      result->color = cache->bg_color_stack.first;
      result->hot_color = cache->hot_color_stack.first;
      result->active_color = cache->active_color_stack.first;
      result->edge_color =cache->edge_color_stack.first;
      result->text_color =cache->text_color_stack.first;
      result->rounding = cache->edge_rounding_stack.first;
      result->softness = cache->edge_softness_stack.first;
      result->edge_size = cache->edge_size_stack.first;
      result->custom_render = cache->render_function_stack.first;
		  result->semantic_size[AXIS2_X] = cache->pref_width_stack.first;
		  result->semantic_size[AXIS2_Y] = cache->pref_height_stack.first;
		  result->layout_axis = cache->layout_axis_stack.first;
    }
  }
}

INTERNAL void
ui_begin_frame(UICache *cache)
{
	// NOTE(Ryan): EVICTION PASS
  // Essentially going through hashmap of boxes existing in previous frame and seeing if they should be kept around
  // TODO(Ryan): Replace with actual hashmap
  // IMPORTANT(Ryan): This caching necessary to allow for centralisation of creation and input handling code  
  for (u32 box_i = 0; i < cache->cache_len; box_i += 1)
  {
    UIBox *box = &cache->cache_elems[i];

    if (ui_key_is_null(box->key))
    {
      continue;
    }

    while (box != NULL)
    {
      if (!ui_key_is_null(box->key))
      {
        if (box->last_frame_touched_index < cache->current_frame_index)
        {
          UIKey o = box->key;
          box = box->hash_next;
          // remove
					// stable_table_del(UI_Key, UI_Box, &ui_cache->cache, o);
          continue;
        }
      }

      box = box->hash_next;
    }
  }

  cache->current_frame_index++;
	
	// NOTE(Ryan): Reset all the stacks and push default values
  // TODO(Ryan): put most of ui_cache_init() here
	UI_POP_ALL_STACKS_TO_ONE;
	UI_ParentPop(ui_cache); // Reset parent stack
	
	// NOTE(Ryan): Default parent
	UIBox *container = ui_make_box(ui_cache, BoxFlag_Clip, s8_lit("__MainContainer"));
	container->computed_size[0] = window->width;
	container->computed_size[1] = window->height;
	container->layout_axis = AXIS2_Y;
	container->bounds = { 0.f, 0.f, window->width, window->height };
	ui_push_parent(ui_cache, container);

	cache->root = container;
}



//- Layouting Helpers TODO(voxel): @rework replace recursives with queue/stack 
//                                 Callstack size may become an issue
static void UI_LayoutRecurseForward(UI_Cache* ui_cache, UI_Box* box, u32 axis) {
  f32 edge_correction_factor = 0.0f;
  if (box->parent != NULL)
  {
    edge_correction_factor = box->parent->edge_size + (box->parent->edge_size)*0.25f;
  }

	f32 edge_correction_factor = box->parent ?
		box->parent->edge_size + (box->parent->edge_size)*0.25 : 0;
	
	if (box->semantic_size[axis].kind == SizeKind_Pixels) {
		box->computed_size[axis] = box->semantic_size[axis].value;
	} else if (box->semantic_size[axis].kind == SizeKind_TextContent) {
		if (axis == axis2_x) {
			box->computed_size[axis] = UI_GetStringSize(box->font, box->identifier) + box->semantic_size[axis].value * 2;
		} else {
			box->computed_size[axis] = box->font->font_size + box->semantic_size[axis].value * 2;
		}
	} else if (box->semantic_size[axis].kind == SizeKind_PercentOfParent) {
		box->computed_size[axis] = (box->parent->computed_size[axis] - edge_correction_factor*2) * 
			box->semantic_size[axis].value / 100.f;
	}
	
	UI_Box* curr = box->first;
	while (curr) {
		UI_LayoutRecurseForward(ui_cache, curr, axis);
		curr = curr->next;
	}
}

static void UI_LayoutRecurseBackward(UI_Cache* ui_cache, UI_Box* box, u32 axis) {
	UI_Box* curr = box->first;
	f32 size = 0.f;
	while (curr) {
		UI_LayoutRecurseBackward(ui_cache, curr, axis);
		size += curr->computed_size[axis];
		
		curr = curr->next;
	}
	if (box->semantic_size[axis].kind == SizeKind_ChildrenSum) {
		box->computed_size[axis] = size;
	}
}

// https://www.enjoyalgorithms.com/blog/iterative-binary-tree-traversals-using-stack

// iterative tree searching: stack DFS, queue BFS
INTERNAL void
dfs(UIBox *box)
{
  UIBox *first = NULL;
  SLL_STACK_PUSH(first, box);
  while (first != NULL)
  {
    UIBox *cur = SLL_STACK_POP(first);
    for (u32 i = len(cur->children) - 1; i >= 0; i--)
    {
      SLL_STACK_PUSH(first, cur->children[i]);
    }
  }
}

INTERNAL void
ui_end_frame(UICache *cache, f32 delta)
{
	// TODO(voxel): @rework Use a queue and stack to bypass recusive functions
	//              I have yet to implement a queue in ds.h so I'll hold off on that for now
	for (u32 i = AXIS2_X; i < AXIS2_COUNT; i++) 
  {
    // upwards-dependent (pre-order)
		UI_LayoutRecurseForward(ui_cache, ui_cache->root, i);
    // downwards-dependent (post-order)
		UI_LayoutRecurseBackward(ui_cache, ui_cache->root, i);
		// NOTE(voxel): What the hell is this supposed to do even. I'll implement this
		//              if I find things to be weird
		//UI_LayoutRecurseSolveViolations(ui_cache, ui_cache->root, i);
	}
	UI_LayoutRecursePositionForward(ui_cache, ui_cache->root, 0.f);
	UI_LayoutRecurseCalculateBounds(ui_cache, ui_cache->root, 0.f, 0.f);

}


#endif

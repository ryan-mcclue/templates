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
  UIBox *parent;
  UIBox *first_child;
  UIBox *last_child;
  UIBox *next;
  UIBox *prev;

  // NOTE(Ryan): Persistent
  UIKey key;
  u64 last_frame_touched_index; 
  b32 direct_set; // seems to be if newly created?

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
	Rectangle target_bounds;
	Rectangle bounds;
	Rectangle clipped_bounds;
	
	f32 hot_t;
	u32 hot_color;
  // exponential animation curves for animating these two values (_t for transition)
	f32 active_t;
	u32 active_color;
	b8 is_on;
	
	Font font;
	u32 bg_color;
	u32 text_color;
	u32 edge_color;
	f32 edge_softness;
	f32 edge_size;
	f32 edge_rounding;

	ui_render_function* render_function;
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

  UIBoxStack *parent_style_stack;
  FontStack *font_style_stack;
  ColorStack *bg_color_style_stack;
  ColorStack *hot_color_style_stack;
  ColorStack *active_color_style_stack;
  F32Stack *edge_rounding_style_stack;
  F32Stack *edge_softness_style_stack;
  F32Stack *edge_size_style_stack;
  ColorStack *edge_color_style_stack;
  ColorStack *text_color_style_stack;
  UISizeStack *pref_width_style_stack;
  UISizeStack *pref_height_style_stack;
  AXIS2Stack *layout_axis_style_stack;
  RectangleStack *clip_rect_style_stack;
  UIRenderFunctionStack *render_function_style_stack;

  Map box_map;

	Font default_font;
  f32 default_font_size;
	
	UIBox *root_box;
	u64 current_frame_index;
	
	UIKey hot_key;
	UIKey active_key;
};

INTERNAL void 
ui_push_style_clip_rect(UICache *cache, Rectangle clip_rect)
{
  RectangleStack *window_rect = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, RectangleStack);
  window_rect->val = clip_rect; 
  SLL_STACK_PUSH(cache->clip_rect_style_stack, window_rect);
}

INTERNAL void
ui_pop_style_clip_rect(UICache *cache)
{
  SLL_STACK_POP(cache->clip_rect_style_stack);
}

INTERNAL void 
ui_push_style_font(UICache *cache, Font font)
{
  FontStack *font_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, FontStack);
  font_stack->val = font; 
  SLL_STACK_PUSH(cache->font_style_stack, font_stack);
}

INTERNAL void
ui_pop_style_font(UICache *cache)
{
  SLL_STACK_POP(cache->font_style_stack);
}

INTERNAL void 
ui_push_style_bg_color(UICache *cache, Color bg_color)
{
  ColorStack *bg_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  bg_color_stack->val = bg_color; 
  SLL_STACK_PUSH(cache->bg_color_style_stack, bg_color_stack);
}

INTERNAL void
ui_pop_style_bg_color(UICache *cache)
{
  SLL_STACK_POP(cache->bg_color_style_stack);
}

INTERNAL void 
ui_push_style_hot_color(UICache *cache, Color hot_color)
{
  ColorStack *hot_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  hot_color_stack->val = hot_color; 
  SLL_STACK_PUSH(cache->hot_color_style_stack, hot_color_stack);
}

INTERNAL void
ui_pop_style_hot_color(UICache *cache)
{
  SLL_STACK_POP(cache->hot_color_style_stack);
}

INTERNAL void 
ui_push_style_active_color(UICache *cache, Color active_color)
{
  ColorStack *active_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  active_color_stack->val = active_color; 
  SLL_STACK_PUSH(cache->active_color_style_stack, active_color_stack);
}

INTERNAL void
ui_pop_style_active_color(UICache *cache)
{
  SLL_STACK_POP(cache->active_color_style_stack);
}

INTERNAL void 
ui_push_style_edge_color(UICache *cache, Color edge_color)
{
  ColorStack *edge_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  edge_color_stack->val = edge_color; 
  SLL_STACK_PUSH(cache->edge_color_style_stack, edge_color_stack);
}

INTERNAL void
ui_pop_style_edge_color(UICache *cache)
{
  SLL_STACK_POP(cache->edge_color_style_stack);
}

INTERNAL void 
ui_push_style_text_color(UICache *cache, Color text_color)
{
  ColorStack *text_color_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, ColorStack);
  text_color_stack->val = text_color; 
  SLL_STACK_PUSH(cache->text_color_style_stack, text_color_stack);
}

INTERNAL void
ui_pop_style_text_color(UICache *cache)
{
  SLL_STACK_POP(cache->text_color_style_stack);
}

INTERNAL void 
ui_push_style_edge_rounding(UICache *cache, f32 edge_rounding)
{
  F32Stack *edge_rounding_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, F32Stack);
  edge_rounding_stack->val = edge_rounding;
  SLL_STACK_PUSH(cache->edge_rounding_style_stack, edge_rounding_stack);
}

INTERNAL void
ui_pop_style_edge_rounding(UICache *cache)
{
  SLL_STACK_POP(cache->edge_rounding_style_stack);
}

INTERNAL void 
ui_push_style_edge_softness(UICache *cache, f32 edge_softness)
{
  F32Stack *edge_softness_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, F32Stack);
  edge_softness_stack->val = edge_softness;
  SLL_STACK_PUSH(cache->edge_softness_style_stack, edge_softness_stack);
}

INTERNAL void
ui_pop_style_edge_softness(UICache *cache)
{
  SLL_STACK_POP(cache->edge_softness_style_stack);
}

INTERNAL void 
ui_push_style_edge_size(UICache *cache, f32 edge_size)
{
  F32Stack *edge_size_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, F32Stack);
  edge_size_stack->val = edge_size;
  SLL_STACK_PUSH(cache->edge_size_style_stack, edge_size_stack);
}

INTERNAL void
ui_pop_style_edge_size(UICache *cache)
{
  SLL_STACK_POP(cache->edge_size_style_stack);
}

INTERNAL void 
ui_push_style_pref_width(UICache *cache, UISize pref_width)
{
  UISizeStack *pref_width_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, UISizeStack);
  pref_width_stack->val = pref_width;
  SLL_STACK_PUSH(cache->pref_width_style_stack, pref_width_stack);
}

INTERNAL void
ui_pop_style_pref_width(UICache *cache)
{
  SLL_STACK_POP(cache->pref_width_style_stack);
}

INTERNAL void 
ui_push_style_pref_height(UICache *cache, UISize pref_height)
{
  UISizeStack *pref_height_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, UISizeStack);
  pref_height_stack->val = pref_height;
  SLL_STACK_PUSH(cache->pref_height_style_stack, pref_height_stack);
}

INTERNAL void
ui_pop_style_pref_height(UICache *cache)
{
  SLL_STACK_POP(cache->pref_height_style_stack);
}

INTERNAL void 
ui_push_style_layout_axis(UICache *cache, AXIS2 layout_axis)
{
  AXIS2Stack *layout_axis_stack = MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, AXIS2Stack);
  layout_axis_stack->val = layout_axis;
  SLL_STACK_PUSH(cache->layout_axis_style_stack, layout_axis_stack);
}

INTERNAL void
ui_pop_style_layout_axis(UICache *cache)
{
  SLL_STACK_POP(cache->layout_axis_style_stack);
}

INTERNAL void 
ui_push_style_render_function(UICache *cache, ui_render_function render_function)
{
  UIRenderFunctionStack *render_function_stack = \
    MEM_ARENA_PUSH_STRUCT_ZERO(cache->style_stack_arena.arena, UIRenderFunctionStack);
  render_function_stack->val = render_function;
  SLL_STACK_PUSH(cache->render_function_style_stack, render_function_stack);
}

INTERNAL void
ui_pop_render_function(UICache *cache)
{
  SLL_STACK_POP(cache->render_function_style_stack);
}

#define UI_STYLE_PARENT(ui_cache, parent) \
  DEFER_LOOP(ui_push_style_parent(ui_cache, parent), ui_pop_style_parent(ui_cache))

#define UI_STYLE_FONT(ui_cache, font) \
  DEFER_LOOP(ui_push_style_font(ui_cache, font), ui_pop_style_font(ui_cache))

#define UI_STYLE_BG_COLOR(ui_cache, bg_color) \
  DEFER_LOOP(ui_push_style_bg_color(ui_cache, bg_color), ui_pop_style_bg_color(ui_cache))

#define UI_STYLE_HOT_COLOR(ui_cache, hot_color) \
  DEFER_LOOP(ui_push_style_hot_color(ui_cache, hot_color), ui_pop_style_hot_color(ui_cache))

#define UI_STYLE_ACTIVE_COLOR(ui_cache, active_color) \
  DEFER_LOOP(ui_push_style_active_color(ui_cache, active_color), ui_pop_style_active_color(ui_cache))

#define UI_STYLE_EDGE_ROUNDING(ui_cache, rounding) \
  DEFER_LOOP(ui_push_style_STYLE_EDGE_rounding(ui_cache, rounding), ui_pop_style_STYLE_EDGE_rounding(ui_cache))

#define UI_STYLE_EDGE_SOFTNESS(ui_cache, softness) \
  DEFER_LOOP(ui_push_style_STYLE_EDGE_softness(ui_cache, softness), ui_pop_style_STYLE_EDGE_softness(ui_cache))

#define UI_STYLE_EDGE_SIZE(ui_cache, STYLE_EDGE_size) \
  DEFER_LOOP(ui_push_style_STYLE_EDGE_size(ui_cache, STYLE_EDGE_size), ui_pop_style_STYLE_EDGE_size(ui_cache))

#define UI_STYLE_EDGE_COLOR(ui_cache, STYLE_EDGE_color) \
  DEFER_LOOP(ui_push_style_edge_color(ui_cache, edge_color), ui_pop_style_edge_color(ui_cache))

#define UI_STYLE_TEXT_COLOR(ui_cache, text_color) \
  DEFER_LOOP(ui_push_style_text_color(ui_cache, text_color), ui_pop_style_text_color(ui_cache))

#define UI_STYLE_PREF_WIDTH(ui_cache, pref_width) \
  DEFER_LOOP(ui_push_style_pref_width(ui_cache, pref_width), ui_pop_style_pref_width(ui_cache))

#define UI_STYLE_PREF_HEIGHT(ui_cache, pref_height) \
  DEFER_LOOP(ui_push_style_pref_height(ui_cache, pref_height), ui_pop_style_pref_height(ui_cache))

#define UI_STYLE_LAYOUT_AXIS(ui_cache, layout_axis) \
  DEFER_LOOP(ui_push_style_layout_axis(ui_cache, layout_axis), ui_pop_style_layout_axis(ui_cache))

#define UI_STYLE_RENDER_FUNCTION(ui_cache, render_function) \
  DEFER_LOOP(ui_push_style_render_function(ui_cache, render_function), ui_pop_style_render_function(ui_cache))

#define UI_STYLE_CLIP_RECT(ui_cache, clip_rect) \
  DEFER_LOOP(ui_push_style_clip_rect(ui_cache, clip_rect), ui_pop_style_clip_rect(ui_cache))


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
	
  MapSlot *slot = map_lookup(cache->box_map, key);
  if (slot != NULL)
  {
    result = (UIBox *)slot->val;

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
    result = ui_make_box();
    map_insert(cache->box_arena, cache->box_map, key, result);

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





/*
 * In general, if need to traverse entire tree use DFS as less state/memory required and simpler.
 *
 * Consider shape of tree (really only if large search space?)
 * DFS:
 *   - target close to bottom
 *   - tree very wide
 *
 * BFS:
 *   - target close to root
 *   - very deep tree
 *
 * in tail recursion, final answer computed on last invocation
 * tail recursion simply changed to iterative
 * more efficient, especially with tail call optimisation in compilers
   u32 sum(x) {
       if (x == 0) {
         return 0;
       } else {
         return x + sum(x - 1);
       }
   }
   u32 tail_sum(x, running_total) {
    if (x == 0) {
        return running_total;
    } else {
        return tail_sum(x - 1, running_total + x);
    }
   }

  DFS traversals:
    In recursive implementations: consider root node, left subtree, right subtree 
    * pre-order: 
    * post-order: property dependent on children, e.g. children_sum
                  traversal is not tail recursive

  level-order is same as BFS
 */
INTERNAL void
ui_autolayout_preorder_sizes(UICache *cache, UIBox *box, AXIS2 axis)
{
  if (box == NULL)
  {
    return;
  }

  f32 edge_correction_factor = 0.0f;
  if (box->parent != NULL)
  {
    edge_correction_factor = box->parent->edge_size + (box->parent->edge_size)*0.25f;
  }

	if (box->semantic_size[axis].kind == UI_SIZE_KIND_PIXELS)
  {
		box->computed_size[axis] = box->semantic_size[axis].value;
	} 
  else if (box->semantic_size[axis].kind == UI_SIZE_KIND_TEXT_CONTENT)
  {
		if (axis == AXIS2_X) 
    {
			box->computed_size[axis] = UI_GetStringSize(box->font, box->identifier) + box->semantic_size[axis].value * 2;
		} 
    else 
    {
			box->computed_size[axis] = box->font->font_size + box->semantic_size[axis].value * 2;
		}
	}
  else if (box->semantic_size[axis].kind == UI_SIZE_KIND_PERCENT_OF_PARENT)
  {
		box->computed_size[axis] = (box->parent->computed_size[axis] - edge_correction_factor*2) * 
			box->semantic_size[axis].value / 100.f;
	}

  for (UIBox *box_child = box->first_child; box_child != NULL; box_child = box_child->next)
  {
    ui_autolayout_preorder_sizes(cache, box_child, axis);
  }
}

INTERNAL void
ui_autolayout_postorder_sizes(UICache *cache, UIBox *box, AXIS2 axis)
{
  if (box == NULL)
  {
    return;
  }

  f32 size = 0.0f;
  for (UIBox *box_child = box->first_child; box_child != NULL; box_child = box_child->next)
  {
    ui_autolayout_postorder_sizes(cache, box_child, axis);
    size += box_child->computed_size[axis];
  }

	if (box->semantic_size[axis].kind == UI_SIZE_KIND_CHILDREN_SUM)
  {
		box->computed_size[axis] = size;
	}
}

INTERNAL void
ui_autolayout_preorder_relative_positions(UICache *cache, UIBox *box, f32 depth)
{
	f32 edge_correction_factor = box->parent ? (box->parent->edge_size)*0.25 : 0;
	f32 child_depth = box->edge_size + edge_correction_factor;
	UI_Box* curr = box->first;
	while (curr) {
		UI_LayoutRecursePositionForward(ui_cache, curr, child_depth);
		child_depth += curr->computed_size[box->layout_axis];
		curr = curr->next;
	}
	
	if (box->parent) {
		box->computed_rel_position[box->parent->layout_axis] = depth;
		box->computed_rel_position[!box->parent->layout_axis] = box->parent->edge_size + edge_correction_factor;
	}
}

INTERNAL void
ui_autolayout_preorder_screen_positions(UICache *cache, UIBox *box, f32 xoff, f32 yoff) 
{
	xoff += box->computed_rel_position[axis2_x];
	yoff += box->computed_rel_position[axis2_y];
	
	box->target_bounds.x = xoff;
	box->target_bounds.y = yoff;
	box->target_bounds.w = box->computed_size[axis2_x];
	box->target_bounds.h = box->computed_size[axis2_y];
	
	rect clippable_bounds = box->bounds;
	if (box->flags & BoxFlag_DrawBorder) {
		f32 edge_correction_factor = box->parent ? (box->parent->edge_size)*0.25 : 0;
		clippable_bounds.x += box->edge_size + edge_correction_factor;
		clippable_bounds.y += box->edge_size + edge_correction_factor;
		clippable_bounds.w -= box->edge_size * 2 + edge_correction_factor;
		clippable_bounds.h -= box->edge_size * 2 + edge_correction_factor;
	}
	
	rect clipping_quad = UI_ClippingRectPeek(ui_cache);
	box->clipped_bounds = rect_get_overlap(clippable_bounds, clipping_quad);
	
	if (box->flags & BoxFlag_Clip) {
		UI_ClippingRectPush(ui_cache, box->clipped_bounds);
	}
	
	UI_Box* curr = box->first;
	while (curr) {
		UI_LayoutRecurseCalculateBounds(ui_cache, curr, xoff, yoff);
		curr = curr->next;
	}
	
	if (box->flags & BoxFlag_Clip) {
		UI_ClippingRectPop(ui_cache);
	}
}

static b8 UI_StateRecurseCheckHotAndActive(UI_Cache* ui_cache, UI_Box* box) {
	UI_Box* curr = box->first;
	while (curr) {
		if (UI_StateRecurseCheckHotAndActive(ui_cache, curr)) return true;
		curr = curr->next;
	}
	
	if (!box->parent || UI_KeyIsNull(box->key)) return false;
	if (rect_contains_point(box->clipped_bounds, v2(OS_InputGetMouseX(), OS_InputGetMouseY()))) {
		if (OS_InputButton(Input_MouseButton_Left)) {
			ui_cache->hot_key = (UI_Key) {0};
			ui_cache->active_key = box->key;
		} else {
			ui_cache->hot_key = box->key;
			ui_cache->active_key = (UI_Key) {0};
		}
		return true;
	}
	return false;
}

static void UI_PushBox(UI_Cache* ui_cache, UI_Box* box) {
	if (box->flags & BoxFlag_CustomRenderer) {
		box->custom_render(ui_cache, box);
		return;
	}
	
	if (box->flags & BoxFlag_DrawDropShadow) {
		UI_PushQuad(ui_cache, (rect) {box->bounds.x + 5, box->bounds.y + 5, box->bounds.w, box->bounds.h}, rect_init(0, 0, 1, 1),
					&ui_cache->white_texture, UI_Vec4ColorSetUniform(v4(0.05, 0.05, 0.05, 1.0)),
					box->rounding, 5,
					0.f);
	}
	
	if (box->flags & BoxFlag_DrawBackground) {
		UI_QuadVec4ColorSet color = UI_ColorToVec4Set(box->color);
		if (box->flags & BoxFlag_HotAnimation) {
			UI_QuadVec4ColorSet hot_color = UI_ColorToVec4Set(box->hot_color);
			color = UI_ColorSetLerp(&color, &hot_color, box->hot_t);
		}
		if (box->flags & BoxFlag_ActiveAnimation) {
			UI_QuadVec4ColorSet active_color = UI_ColorToVec4Set(box->active_color);
			color = UI_ColorSetLerp(&color, &active_color, box->active_t);
		}
		
		UI_PushQuad(ui_cache, box->bounds, rect_init(0, 0, 1, 1),
					&ui_cache->white_texture, color, box->rounding, box->softness,
					0.f);
	}
	
	if (box->flags & BoxFlag_DrawText) {
		f32 string_size = UI_GetStringSize(box->font, box->identifier);
		vec2 pos = v2(box->bounds.x + (box->bounds.w / 2.f), box->bounds.y + (box->bounds.h / 2.f));
		pos.x -= string_size / 2.f;
		pos.y += box->font->baseline / 2.f;
		vec4 vcolor = color_code_to_vec4(box->text_color);
		
		for (u32 i = 0; i < box->identifier.size; i++) {
			if (box->identifier.str[i] >= 32 && box->identifier.str[i] < 128) {
				stbtt_packedchar* info = &box->font->cdata[box->identifier.str[i] - 32];
				rect uvs = {
					info->x0 / 512.f,
					info->y0 / 512.f,
					(info->x1 - info->x0) / 512.f,
					(info->y1 - info->y0) / 512.f
				};
				rect loc = { pos.x + info->xoff, pos.y + info->yoff, info->x1 - info->x0, info->y1 - info->y0 };
				UI_PushQuad(ui_cache, loc, uvs, &box->font->font_texture,
							UI_Vec4ColorSetUniform(vcolor), 0.f, 0.f, 0.f);
				pos.x += info->xadvance;
			}
		}
	}
	
	if (box->flags & BoxFlag_DrawBorder) {
		vec4 bcolor = color_code_to_vec4(box->edge_color);
		UI_QuadVec4ColorSet bcolor_set = { bcolor, bcolor, bcolor, bcolor };
		UI_PushQuad(ui_cache, box->bounds, rect_init(0, 0, 1, 1),
					&ui_cache->white_texture, bcolor_set, box->rounding,
					box->softness, box->edge_size);
	}
}


static void UI_PushBoxRecursive(UI_Cache* ui_cache, UI_Box* box) {
	UI_PushBox(ui_cache, box);
	
	if (box->flags & BoxFlag_Clip) {
		UI_ClippingRectPush(ui_cache, box->clipped_bounds);
	}
	
	UI_Box* curr = box->first;
	while (curr) {
		UI_PushBoxRecursive(ui_cache, curr);
		curr = curr->next;
	}
	
	if (box->flags & BoxFlag_Clip) {
		UI_ClippingRectPop(ui_cache);
	}
}

INTERNAL void
ui_end_frame(UICache *cache, f32 delta)
{
	for (u32 axis = AXIS2_X; axis < AXIS2_COUNT; axis++) 
  {
    ui_autolayout_preorder_sizes(cache, cache->root, axis);
    ui_autolayout_postorder_sizes(cache, cache->root, axis);
    // TODO(Ryan): solve violations step with strictness
  }

  // TODO(Ryan): Should these be in axis loop?
  ui_autolayout_preorder_relative_positions(cache, cache->root, 0.0f);
  ui_autolayout_preorder_screen_positions(cache, cache->root, 0.0f, 0.0f);


	ui_cache->hot_key = (UI_Key) {0};
	ui_cache->active_key = (UI_Key) {0};
	UI_StateRecurseCheckHotAndActive(ui_cache, ui_cache->root);
	
	// UI_Animate inlined
	f32 fast_rate = 1 - pow(2.f, -50.f * delta_time);
	f32 slow_rate = 1 - pow(2.f, -30.f * delta_time);
	Iterate (ui_cache->cache, i) {
		UI_Box* curr = &ui_cache->cache.elems[i];
		if (UI_KeyIsNull(curr->key)) continue;
		while (curr) {
			if (!UI_KeyIsNull(curr->key)) {
				b8 is_hot        = UI_KeyEquals(ui_cache->hot_key, curr->key);
				b8 is_active     = UI_KeyEquals(ui_cache->active_key, curr->key);
				curr->hot_t     += ((f32)!!is_hot - curr->hot_t) * fast_rate;
				curr->active_t  += ((f32)!!is_active - curr->active_t) * fast_rate;
				
				if (curr->direct_set) {
					curr->bounds.x += curr->target_bounds.x;
					curr->bounds.y += curr->target_bounds.y;
					curr->bounds.w += curr->target_bounds.w;
					curr->bounds.h += curr->target_bounds.h;
				} else {
					curr->bounds.x += (curr->target_bounds.x - curr->bounds.x) * slow_rate;
					curr->bounds.y += (curr->target_bounds.y - curr->bounds.y) * slow_rate;
					curr->bounds.w += (curr->target_bounds.w - curr->bounds.w) * slow_rate;
					curr->bounds.h += (curr->target_bounds.h - curr->bounds.h) * slow_rate;
				}
			}
			curr = curr->hash_next;
		}
	}
	
	// NOTE(Ryan): Rendering 
	UI_PushBoxRecursive(ui_cache, ui_cache->root);
}


#endif

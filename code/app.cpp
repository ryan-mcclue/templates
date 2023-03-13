// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <raylib.h>

#include "app.h"

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

typedef struct UIBox UIBox;
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
	axis2 layout_axis;
	f32 computed_rel_position[AXIS2_COUNT]; // size relative to parent

  // final display coordinates.
  // this will be used next frame for input event consumption
  // this will be used this frame for rendering
	rect target_bounds;
	rect bounds;
	rect clipped_bounds;
	
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

	UI_RenderFunction* custom_render;
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

typedef struct UICache UICache;
struct UICache
{
  MemArena arena;

  // map_create(closest prime to power of 2?);
  // this is a hashmap (Map from base)
  // UIKey -> UIBox
  UIBox *cache_elems;
  u32 cache_len; // this is num_slots

  // NOTE(Ryan): Style stacks
  struct parent_stack { UIBox *first, *last; };
  struct font_stack { Font *first, *last; };
  struct bg_color_stack { Color *first, *last; };
  struct hot_color_stack { Color *first, *last; };
  struct active_color_stack { Color *first, *last; };
  struct rounding_stack { f32 *first, *last; };
  struct softness_stack { f32 *first, *last; };
  struct edge_size_stack { f32 *first, *last; };
  struct edge_color_stack { u32 *first, *last };
  struct text_color_stack { u32 *first, *last };
  struct pref_width_stack { UISize *first, *last; };
  struct pref_height_stack { UISize *first, *last; };
  struct layout_axis_stack { AXIS2 *first, *last; };
  struct clipping_rect_stack { V2F32 *first, *last; };
  struct render_function_stack { ui_render_function *first, *last };

	Font default_font;
	
	UIBox* root;
	u64 current_frame_index;
	
	UIKey hot_key;
	UIKey active_key;
};

void
draw_rect(Vec2F32 min, Vec2F32 max, Vec3F32 colour)
{
  Vector2 position = {min.x, min.y};
  Vector2 size = {max.x - min.x, max.y - min.y};
  Vec3F32 scaled = vec3_f32_mul(colour, 255.0f);
  Color color = {(u8)scaled.r, (u8)scaled.g, (u8)scaled.b, 255};
  DrawRectangleV(position, size, color);
}

EXPORT void
app(AppState *state)
{
  /*
   * GUI all about communicating information to user
   * Ideally, want to not waste users time:
   *    - minimal bytes sent to max bytes recieved ratio
   *    - user reuse existing knowledge (Buttons, Checkboxes, Radio Buttons, Sliders, Text Fields, Scroll Bars, etc.)
   *    - give confirmation (hover state, clicking down state, etc.)  
   *
   * Core code (general styles, ability for custom styles) (development accelerates early on in project, then slows if done well)
   * Builder code arranging widgets (majority is this)
   * ================================================================================
   *
   * We use immediate mode to mean UI hierarchy built every frame (there can still be some retained state)
   * So, appearance and interaction code in one place
   * Everything is a widget, i.e. tree hierarchy of widgets
    1. Begin a row that fills the available space (one descent in the hierarchy).
    2. File button, sized just enough to fit the text, with some extra padding.
    3. Window button, sized the same way.
    4. Panel button, sized the same way.
    5. View button, sized the same way.
    6. Control button, sized the same way.
    7. Enough space to fill the screen, leaving enough room for the following buttons.
IMPORTANT:
Need to account for parts of widget tree we don't have yet
Rendering deferred
Widgets rendered last, consume input events first
build hierarchy -> autolayout -> render
Input delay one frame, rendering immediately
    8. “Play” button, only being big enough for the “Play” icon.
    9. “Pause” button, the same size as the previous.
    10. “X” button, the same size as the previous.
    11. End the row (one ascent in the hierarchy).

  DFS has pre-order (node before children) and post-order (node after children) traversals
Autolayout for each exis:
  1. Standalone sizes, e.g. UI_SizeKind_Pixels, UI_SizeKind_TextContent 
  2. (pre-order) 'upwards-dependent' UI_SizeKind_PercentOfParent
  3. (post-order) 'downwards-dependent' UI_SizeKind_ChildrenSum 
  4. (pre-order) Solve violations using 'strictness'
  5. (pre-order) Given the calculated sizes of each widget, compute the relative positions of each widget
   * ================================================================================
   
   Core implements certain effects, e.g:
    Clickability—taking mouse events and hovering, responding to clicks and drags
    Mouse wheel scrolling to shift the “view offset”
    Keyboard focusing behavior—another path for producing “clicks” or “presses” in a keyboard-driven fashion
    Being, and appearing, disabled and non-interactive to the user
    “Floating” on the X axis—skipping layout on this axis, effectively
    “Floating” on the Y axis
    Allowing overflow on the X axis—skipping size-constraint-solving on this axis
    Allowing overflow on the Y axis
    Requiring a drop shadow to be rendered
    Requiring a background to be rendered
    Requiring a border to be rendered
    Requiring text to be rendered
    Hot animation visualization
    Active animation visualization
    Arbitrary “render command buffer” attachment
    Center-aligned text
    Clipping rectangle for children widgets
    The X-axis position being smoothly animated across frames
    The Y-axis position being smoothly animated across frames
    Bypassing text-truncation with ellipses

commonly useful to have a “make a widget that has text, border, background rendering, hot/active animations, is clickable, and has a specific hovering cursor icon” (effectively your standard, everyday button). But, we should also then immediately realize that an identical button but without the border is not an impossibility or even an improbability.

IMPORTANT(Ryan): Design for combinatoric, i.e. many combinations
BAD (WE WANT TO FOCUS ON HIGH LEVEL TRANSFORMATIONS NOT SAY WHAT A BUTTON IS):
struct UI_Widget
{
  UI_WidgetKind kind;
  // ...
  union
  {
    struct { ... } button;
    struct { ... } checkbox;
    struct { ... } slider;
    // ...
  };
};

GOOD:
typedef U32 UI_WidgetFlags;
enum
{
  UI_WidgetFlag_Clickable       = (1<<0),
  UI_WidgetFlag_ViewScroll      = (1<<1),
  UI_WidgetFlag_DrawText        = (1<<2),
  UI_WidgetFlag_DrawBorder      = (1<<3),
  UI_WidgetFlag_DrawBackground  = (1<<4),
  UI_WidgetFlag_DrawDropShadow  = (1<<5),
  UI_WidgetFlag_Clip            = (1<<6),
  UI_WidgetFlag_HotAnimation    = (1<<7),
  UI_WidgetFlag_ActiveAnimation = (1<<8),
  // ...
};

struct UI_Widget
{
  // ...
  UI_WidgetFlags flags;
  // ...
};
================================================================================

Spacing box has no layout parametisations. The hash should be some NULL-id
UI_Box *spacer = UI_BoxMakeF(0, "");

IMPORTANT: Defer loop useful for when stacks are scope based
Using style stacks:
#define UI_TextColor(v) UI_DeferLoop(UI_PushTextColor(v), UI_PopTextColor())
UI_TextColor(V4(1, 0, 0, 1))
{
  UI_ButtonF("Red Button A");
  UI_ButtonF("Red Button B");
  UI_ButtonF("Red Button C");
}

================================================================================

Solid rectangles, text, icons, bitmaps, gradients (embossed, debossed), rounded corners (useful for drop shadows), hollow rectangles

For icons, Fontello to produce unicode codepoints for icons inside of font
================================================================================

The builder code already manages state, i.e. whatever the application the UI is interfacing with

struct Window
{
  Window *next;
  Window *prev;
  Rect rect;
  String8 title;
  // ... extra info for which interface is active?
};

struct AppState
{
  // doubly-linked list of opened windows
  Window *first_window;
  Window *last_window;
  
  // singly-linked free-list of closed windows
  Window *free_window;
};

void WindowBuildUI(AppState *app_state, Window *window)
{
  UI_Window(window->rect, window->title)
  {
    // build title bar
    UI_TitleBar
    {
      UI_Label(window->title);
      UI_Spacer(UI_Pct(1, 0));
      if(UI_CloseButton(...).clicked)
      {
        WindowClose(app_state, window);
      }
    }

    // build contents
    {
      // build the instantiated interface
    }
  }
}

Use command buffer for storing important stateful mutations, e.g. allocation, deallocation

struct Panel
{
  Panel *first;
  Panel *last;
  Panel *next;
  Panel *prev;
  Panel *parent;

  Axis2 split_axis;
  F32 pct_of_parent;

  // (any other state to encode view info)
};

A tab is a combination of a Window and Panel
================================================================================
*/

  

/*
 * TODO: replace cache with UIContext?
         1. font init (load and set size)
         2. default style stacks (perhaps with stack add len field)
            SLL_STACK_PUSH(ui_cache.clipping_rect.first, ui_cache.clipping_rect.last, rect(0, 0, window_width, window_height))
            SLL_STACK_PUSH(ui_cache.fonts.first, ui_cache.fonts.last, default_font)
	          UI_BoxColorPush(ui_cache, 0x111111FF);
	          UI_HotColorPush(ui_cache, 0x131313FF);
	          UI_ActiveColorPush(ui_cache, 0x131313FF);
	          UI_EdgeColorPush(ui_cache, 0x9A5EBDFF);
	          UI_TextColorPush(ui_cache, 0xFFAAFFFF);
	          UI_RoundingPush(ui_cache, 5.f);
	          UI_EdgeSoftnessPush(ui_cache, 2.f);
	          UI_EdgeSizePush(ui_cache, 2.f);
	          UI_PrefWidthPush(ui_cache, UI_Pixels(100));
	          UI_PrefHeightPush(ui_cache, UI_Pixels(100));
	          UI_LayoutAxisPush(ui_cache, axis2_y);
	          UI_CustomRenderFunctionPush(ui_cache, nullptr);
         3. UIBeginFrame:
            - pruning using persistent hashing?
            - reset style stacks to default
            - create new parent/root box for entire screen
              MakeBox():
                1. handle spacers
                2. check if box in cache (will be in cache if existed previous frame)
                   and only create if it isn't
                   (cache is just container for boxes?)
         4. gui application specific
         5. UIEndFrame:
            - recursively autolayout position and bounds
            - check hot/active and animate values
            - recurse hierarchy and render based on box flags 
             

         graphics init (clear colour)
         set event process callback (on frame refresh event call app)
         enter event loop
*/
    
  if (!state->is_initialised)
  {
    state->x = 10;
    state->y = 20;
    state->t = 30;

    state->is_initialised = true;
  }

  // update
  
  // render

  for (i32 y = 0; y < (i32)state->height; ++y)
  {
    for (i32 x = 0; x < (i32)state->width; ++x)
    {
      f32 coord = sin_f32(x) + sin_f32(y);
      //Vec3F32 colour = vec3_f32_dup(((coord * (state->ms)) % 256) / 256.0f);
      //draw_rect(vec2_f32(x, y), vec2_f32(x + 5, y + 1), colour);
    }
  }

  //DrawRectangle(0, 0, state->width, state->height, DARKBLUE);
  //DrawRectangle(0, state->height - 150, state->width, state->height, GREEN);

  // TODO(Ryan): Perhaps use i32 whenever used in calculation for drawing
  //i32 snow_num_x = 10, snow_width = 10, snow_gutter_x = 30;
  //i32 snow_num_y = 20, snow_height = 10, snow_gutter_y = 50;


}

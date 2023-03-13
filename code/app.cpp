// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <raylib.h>

#include "app.h"

enum UI_SizeKind
{
  UI_SizeKind_Null,
  UI_SizeKind_Pixels,
  UI_SizeKind_TextContent,
  UI_SizeKind_PercentOfParent,
  UI_SizeKind_ChildrenSum,
};

struct UI_Size
{
  UI_SizeKind kind;
  F32 value; // 0 for ChildrenSum
  F32 strictness; // 1 == no budge
};

enum Axis2
{
  Axis2_X,
  Axis2_Y,
  Axis2_COUNT
};

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

// information representing how user interacted with widget
struct UI_Comm
{
  UI_Widget *widget;
  Vec2F32 mouse;
  Vec2F32 drag_delta;
  B8 clicked;
  B8 double_clicked;
  B8 right_clicked;
  B8 pressed;
  B8 released;
  B8 dragging;
  B8 hovering;
};

B32 UI_Button(String8 string)
{
  UI_Widget *widget = UI_WidgetMake(UI_WidgetFlag_Clickable|
                                    UI_WidgetFlag_DrawBorder|
                                    UI_WidgetFlag_DrawText|
                                    UI_WidgetFlag_DrawBackground|
                                    UI_WidgetFlag_HotAnimation|
                                    UI_WidgetFlag_ActiveAnimation,
                                    string);
  UI_Comm comm = UI_CommFromWidget(widget);
  return comm.clicked;
}

// IMPORTANT(Ryan): Rename Widget to Box, so as to extend UI decomposition, e.g:
/* List Box Region (child layout on x)
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
	f32 strictness;
};

typedef struct UICache UICache;
struct UICache
{

};

typedef u32 UI_BOX_FLAG;
enum
{
	UI_BOX_FLAG_CLICKABLE       = 0x1,    // @done
	UI_BOX_FLAG_VIEW_SCROLL      = 0x2,    // TODO hard
	UI_BOX_FLAG_DRAW_TEXT        = 0x4,    // @done
	UI_BOX_FLAG_DRAW_BORDER      = 0x8,    // @done
	UI_BOX_FLAG_DRAW_BACKGROUND  = 0x10,   // @done
	UI_BOX_FLAG_DRAW_DROP_SHADOW  = 0x20,   // @done
	UI_BOX_FLAG_CLIP            = 0x40,   // @done
	UI_BOX_FLAG_HOT_ANIMATION    = 0x80,   // @done
	UI_BOX_FLAG_ACTIVE_ANIMATION = 0x100,  // @done
	UI_BOX_FLAG_CUSTOM_RENDERER  = 0x200,  // @done
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
	UISize semantic_size[axis2_count];
	f32 computed_size[axis2_count]; // final pixel size
	axis2 layout_axis;
	f32 computed_rel_position[axis2_count]; // size relative to parent

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
  UI_Key_UI_Box cache; // ??

  // style stacks
  struct parent_stack
  { 
    UI_Box* elems[UI_STACK_MAX]; UI_Box* *top; u64 len; b32 auto_pop; 
  };
  // ...

	UI_FontInfo default_font;
	
	UI_Box* root;
	u64 current_frame_index;
	
	UI_Key hot_key;
	UI_Key active_key;
};




#if 0

typedef struct q_app
{

	mp_gui_style style; // colours, button sizes, etc.

	mp_gui_io input;

	mp_gui_context* gui;
} q_app;


typedef struct mp_gui_context
{
	mp_graphics_context graphics;

	mp_gui_io input;

	u64 frameCounter;

	mp_gui_id prevHovered;
	mp_gui_id prevActive;
	mp_gui_id prevFocus;
	mp_gui_id hovered;
	mp_gui_id active;
	mp_gui_id focus;

	mp_gui_view* hoveredRoot;
	mp_gui_view* nextHoveredRoot;
	mp_gui_view* hoveredView;
	mp_gui_view* nextHoveredView;

	mp_gui_view* frontView;

	u32 nextZ;

	bool nextViewFrameSet;
	mp_aligned_rect nextViewFrame;

	mp_mouse_cursor mouseCursor;

	//NOTE(martin): text input
	f64 editCursorBlinkStart;
	u32 firstDisplayedChar;
	u32 editCursor;
	u32 editMark;
	u32 editBufferSize; // _not_ including a null codepoint
	u32 editBuffer[MP_GUI_EDIT_BUFFER_MAX_SIZE];

	//NOTE(martin): stacks
	mp_gui_id idStack[MP_GUI_ID_STACK_MAX_DEPTH];
	u32 idStackSize;

	mp_gui_view* viewStack[MP_GUI_VIEW_STACK_MAX_DEPTH];
	u32 viewStackSize;

	mp_aligned_rect clipStack[MP_GUI_CLIP_STACK_MAX_DEPTH];
	u32 clipStackSize;

	mp_gui_transform transformStack[MP_GUI_TRANSFORM_STACK_MAX_DEPTH];
	u32 transformStackSize;

	mp_gui_style styleStack[MP_GUI_STYLE_STACK_MAX_DEPTH];
	u32 styleStackSize;
	mp_gui_style defaultStyle;

	//NOTE(martin): root views and view move-to-front list
	mp_gui_view* rootViews[MP_GUI_VIEW_STACK_MAX_DEPTH];
	u32 rootViewCount;

	list_info views;
	mp_gui_view viewPool[MP_GUI_VIEW_MAX_COUNT];

} mp_gui_context;



typedef struct demo_info
{
	mp_graphics_surface surface;
	mp_graphics_context graphics;
	mp_graphics_font font;
	mp_gui_context* gui;

	f32 windowWidth;
	f32 windowHeight;

} demo_info;

void demo_gui(demo_info* demo)
{
	mp_gui_context* gui = demo->gui;

	mp_gui_begin_frame(gui);
	{
		mp_gui_view_flags flags = MP_GUI_VIEW_FLAG_ROOT
		                        | MP_GUI_VIEW_FLAG_TITLED
		                        | MP_GUI_VIEW_FLAG_RESIZEABLE
		                        | MP_GUI_VIEW_FLAG_SCROLL;

		mp_gui_begin_view(gui, "Test view 3", (mp_aligned_rect){700, 200, 600, 800}, flags);
		{
			const int optionCount = 4;
			mp_string options[4] = {mp_string_lit("option one"),
			                        mp_string_lit("option two"),
			                        mp_string_lit("option three"),
			                        mp_string_lit("option four")};

			static int optionIndex = 0;

			if(mp_gui_popup_menu(gui, "popup", (mp_aligned_rect){300, 600, 300, 50}, optionCount, options, &optionIndex))
			{
				printf("selected option #%i (%.*s)\n", optionIndex, (int)options[optionIndex].len, options[optionIndex].ptr);
			}

		} mp_gui_end_view(gui);

		mp_gui_begin_view(gui, "Test view 2", (mp_aligned_rect){500, 400, 600, 450}, flags);
		{
			static utf32 codePoints[256];
			static u32 codePointsSize = 0;
			if(mp_gui_text_field(gui, "text", (mp_aligned_rect){50, 50, 500, 80}, 256, &codePointsSize, codePoints, 0))
			{
				//...
			}
			//TODO: show modifiers keys and last key pressed...

			static bool check = false;
			mp_gui_checkbox(gui, "check", (mp_aligned_rect){50, 150, 50, 50}, &check);

		} mp_gui_end_view(gui);

		mp_gui_begin_view(gui, "Test view 1", (mp_aligned_rect){200, 100, 600, 450}, flags);
		{
			if(mp_gui_text_button(gui, "Button 1", (mp_aligned_rect){50, 40, 200, 80}))
			{
			}

			if(mp_gui_text_button(gui, "Button 2", (mp_aligned_rect){50, 140, 200, 80}))
			{
			}

			if(mp_gui_text_button(gui, "Button 3", (mp_aligned_rect){50, 240, 200, 80}))
			{
			}

			static f32 slider1 = 0;
			static f32 slider2 = 0;
			static f32 slider3 = 0;

			mp_gui_slider(gui, "slider_1", (mp_aligned_rect){300, 40, 200, 50}, 80, &slider1);
			mp_gui_slider(gui, "slider_2", (mp_aligned_rect){300, 140, 200, 50}, 80, &slider2);
			mp_gui_slider(gui, "slider_3", (mp_aligned_rect){300, 240, 200, 50}, 80, &slider3);

		} mp_gui_end_view(gui);


	} mp_gui_end_frame(gui);
}
#endif




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



  
  /*    Gui init:
	      * mp_aligned_rect windowRect = {.x = 100, .y = 100, .w = 0.5*Q_WINDOW_DEFAULT_WIDTH, .h = 0.5*Q_WINDOW_DEFAULT_HEIGHT};
	      * app->window = mp_window_create(windowRect, "Quadrant", 0);
        * font init (load and set size)
        * graphics init (clear colour)
        * set event process callback (on frame refresh event call app)
        * enter event loop
  */
  if (!state->is_initialised)
  {
    state->x = 10;
    state->y = 20;
    state->t = 30;

    state->is_initialised = true;
  }


  // hierarchy of boxes. each box ability to turn off bg, text and border
  // overall layout computed at end of frame
  app_state->begin_frame_time = get_time();
  app_state->mouse_cursor = CURSOR_UP;
  app_state->frame_counter++;

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

  // https://www.forkingpaths.dev/posts/23-03-10/rule_based_styling_imgui.html

  //DrawRectangle(0, 0, state->width, state->height, DARKBLUE);
  //DrawRectangle(0, state->height - 150, state->width, state->height, GREEN);

  // TODO(Ryan): Perhaps use i32 whenever used in calculation for drawing
  //i32 snow_num_x = 10, snow_width = 10, snow_gutter_x = 30;
  //i32 snow_num_y = 20, snow_height = 10, snow_gutter_y = 50;


}

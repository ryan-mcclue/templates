// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <raylib.h>

#include "app.h"

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

    ui_cache_init(state->ui_cache, state->window_width, state->window_height);

    state->is_initialised = true;
  }

  // update
  ui_begin_frame(state->ui_cache);

  UI_SET_PREF_WIDTH(state->ui_cache, ui_size_percentage(100))
  UI_SET_PREF_HEIGHT(state->ui_cache, ui_size_percentage(100))
  UI_SET_LAYOUT_AXIS(state->ui_cache, AXIS2_X)
	{
		UI_Box* full_container = UI_BoxMake(ui_cache, BoxFlag_DrawBackground | BoxFlag_DrawBorder | BoxFlag_Clip, str_lit("foo"));
		UI_SET_PARENT(ui_cache, full_container)
    {
	    ui_spacer_instance(ui_cache, UI_Percentage(35));
				
		  UI_LayoutAxis(ui_cache, axis2_y)
		  UI_PrefWidth(ui_cache, UI_Percentage(30))
			UI_PrefHeight(ui_cache, UI_Percentage(100))
      {
			  UI_Box* vert = UI_BoxMake(ui_cache, 0, str_lit("VerticalCheckboxContainer"));
					
				UI_Parent(ui_cache, vert)
				UI_PrefWidth(ui_cache, UI_Percentage(100))
				UI_PrefHeight(ui_cache, UI_Pixels(35))
				UI_LayoutAxis(ui_cache, axis2_x) 
        {
					UI_Box* pm = UI_BoxMake(ui_cache, 0, str_lit("PlusMinusContainer"));
					
					UI_Parent(ui_cache, pm)
					UI_PrefWidth(ui_cache, UI_Percentage(50))
					UI_PrefHeight(ui_cache, UI_Pixels(35))
          {
							if (UI_Button(ui_cache, str_lit("+##AddCheckbox")).clicked)
								show_btn ++;
							if (UI_Button(ui_cache, str_lit("-##SubCheckbox")).clicked)
								show_btn --;
							if (show_btn < 0) show_btn = 0;
					}
						
					UI_Spacer(ui_cache, UI_Pixels(15));
						
					UI_ActiveColor(ui_cache, 0x9A5EBDFF)
          {
            for (i32 i = show_btn; i > 0; i--)
              UI_CheckboxF(ui_cache, "Checkbox##%d", i);
					}
				}
			}
		}
  }

  ui_end_frame(state->ui_cache, delta);

  // render

  DrawRectangle(0, 0, state->window_width, state->window_height, DARKBLUE);
  DrawRectangle(0, state->window_height - 150, state->window_width, state->window_height, GREEN);

  // TODO(Ryan): Perhaps use i32 whenever used in calculation for drawing
  //i32 snow_num_x = 10, snow_width = 10, snow_gutter_x = 30;
  //i32 snow_num_y = 20, snow_height = 10, snow_gutter_y = 50;


}

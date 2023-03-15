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
*/
    
  if (!state->is_initialised)
  {
    state->x = 10;
    state->y = 20;
    state->t = 30;

    state->show_button = 0;

    ui_cache_init(state->ui_cache, state->window_width, state->window_height);

    state->is_initialised = true;
  }

  ui_begin_frame(state->ui_cache);

  UI_STYLE_PREF_WIDTH(state->ui_cache, ui_size_percentage(100))
  UI_STYLE_PREF_HEIGHT(state->ui_cache, ui_size_percentage(100))
  UI_STYLE_LAYOUT_AXIS(state->ui_cache, AXIS2_X)
	{
		UIBox *full_container = ui_make_box(state->ui_cache, 
                                        UI_BOX_FLAG_DRAW_BACKGROUND | UI_BOX_FLAG_DRAW_BORDER | UI_BOX_FLAG_CLIP, 
                                        s8_lit("foo"));
		UI_STYLE_PARENT(state->ui_cache, full_container)
    {
	    ui_spacer(state->ui_cache, ui_style_percentage(35));
				
		  UI_STYLE_LAYOUT_AXIS(state->ui_cache, AXIS2_Y)
		  UI_STYLE_PREF_WIDTH(state->ui_cache, ui_style_percentage(30))
			UI_STYLE_PREF_HEIGHT(state->ui_cache, ui_style_percentage(100))
      {
			  UIBox *vert = ui_make_box(state->ui_cache, 0, s8_lit("VerticalCheckboxContainer"));
					
				UI_STYLE_PARENT(state->ui_cache, vert)
				UI_STYLE_PREF_WIDTH(state->ui_cache, ui_style_percentage(100))
				UI_STYLE_PREF_HEIGHT(state->ui_cache, ui_style_pixels(35))
				UI_STYLE_LAYOUT_AXIS(state->ui_cache, AXIS2_X) 
        {
					UIBox *pm = ui_make_box(state->ui_cache, 0, s8_lit("PlusMinusContainer"));
					
					UI_STYLE_PARENT(state->ui_cache, pm)
					UI_STYLE_PREF_WIDTH(state->ui_cache, ui_style_percentage(50))
					UI_STYLE_PREF_HEIGHT(state->ui_cache, ui_style_pixels(35))
          {
							if (ui_button(state->ui_cache, s8_lit("+##AddCheckbox")).clicked)
								state->show_btn++;
							if (ui_button(state->ui_cache, s8_lit("-##SubCheckbox")).clicked)
								state->show_btn--;
							if (state->show_btn < 0) state->show_btn = 0;
					}
						
					ui_spacer(state->ui_cache, ui_style_pixels(15));
						
					UI_STYLE_ACTIVE_COLOR(state->ui_cache, 0x9A5EBDFF)
          {
            for (i32 i = state->show_btn; i > 0; i--)
              ui_checkbox_fmt(state->ui_cache, "Checkbox##%d", i);
					}
				}
			}
		}
  }

  ui_end_frame(state->ui_cache, delta);
}

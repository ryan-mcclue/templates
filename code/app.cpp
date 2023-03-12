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

struct UI_Widget
{
  // ...
  UI_Size semantic_size[Axis2_COUNT];

  F32 computed_rel_position[Axis2_COUNT]; // size relative to parent
  F32 computed_size[Axis2_COUNT]; // final pixel size
  Rng2F32 rect; // final display coordinates.
  // this will be used next frame for input event consumption
  // this will be used this frame for rendering
  
  // hot_t and active_t (_t for transition)
  // exponential animation curves for animating these two values 
  
  // simple n-ary tree
  // no requirement for caching
  // computed each frame
  UI_Widget *first;
  UI_Widget *last;
  UI_Widget *next; // children
  UI_Widget *prev; // children
  UI_Widget *parent;

  // for caching persistent data across frames
  UI_Widget *hash_next;
  UI_Widget *hash_prev;
  // key+generation info
  // IMPORTANT(Ryan): To generate keys, hash of string passed to widget builder
  UI_Key key;
  U64 last_frame_touched_index; // if < current_frame_index then 'pruned'

};






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


void demo_event_callback(mp_event* event, void* userData)
{
	demo_info* demo = (demo_info*)userData;

	mp_gui_process_event(mp_gui_context_get_io(demo->gui), event);
	switch(event->type)
	{
		case MP_EVENT_WINDOW_CLOSE:
		{
			mp_do_quit();
		} break;

		case MP_EVENT_WINDOW_RESIZE:
		{
			demo->windowWidth = event->frame.rect.w * 2;
			demo->windowHeight = event->frame.rect.h * 2;
		} break;

		case MP_EVENT_FRAME:
		{
			mp_graphics_set_clear_color(demo->graphics, DEMO_COLOR_CLEAR);
			mp_graphics_clear(demo->graphics);

			demo_gui(demo);

			mp_graphics_context_flush(demo->graphics, demo->surface);
			mp_graphics_surface_present(demo->surface);
		} break;

		default:
			break;
	}
}

void mp_gui_process_event(mp_gui_io* input, mp_event* event)
{
	switch(event->type)
	{
		case MP_EVENT_WINDOW_RESIZE:
		{
			input->displayWidth = event->frame.rect.w * 2;
			input->displayHeight = event->frame.rect.h * 2;
		} break;

		case MP_EVENT_MOUSE_MOVE:
		{
			input->mods = event->move.mods;

			input->mouse.deltaX += event->move.x * 2 - input->mouse.x;
			input->mouse.deltaY += (input->displayHeight - event->move.y * 2) - input->mouse.y;
			input->mouse.x = event->move.x * 2;
			input->mouse.y = input->displayHeight - event->move.y * 2;

		} break;

		case MP_EVENT_MOUSE_WHEEL:
		{
			input->mouse.wheelX += event->move.deltaX * MP_GUI_SCROLL_SCALE;
			input->mouse.wheelY += event->move.deltaY * MP_GUI_SCROLL_SCALE;
		} break;

		case MP_EVENT_MOUSE_BUTTON:
		{
			input->mods = event->key.mods;

			if(event->key.action == MP_KEY_PRESS)
			{
				input->mouse.buttons[event->key.code] |= MP_KEY_STATE_PRESSED;
				input->mouse.buttons[event->key.code] |= MP_KEY_STATE_DOWN;

				if(event->key.clickCount > 1)
				{
					input->mouse.buttons[event->key.code] |= MP_KEY_STATE_DOUBLE_CLICKED;
				}
				else
				{
					input->mouse.buttons[event->key.code] |= MP_KEY_STATE_SIMPLE_CLICKED;
				}
			}
			else
			{
				input->mouse.buttons[event->key.code] &= ~MP_KEY_STATE_DOWN;
				input->mouse.buttons[event->key.code] |= MP_KEY_STATE_RELEASED;
			}

		} break;

		case MP_EVENT_KEYBOARD_CHAR:
		{
			//TODO(martin): shouldn't check seqLen while we only store 1 codepoint at a time ?
			if(input->text.count + event->character.seqLen < MP_GUI_MAX_INPUT_CHAR_PER_FRAME)
			{
				input->text.codePoints[input->text.count] = event->character.codepoint;
				input->text.count++;
			}
		} break;

		case MP_EVENT_KEYBOARD_MODS:
			input->mods = event->key.mods;
			break;

		case MP_EVENT_KEYBOARD_KEY:
		{
			input->mods = event->key.mods;

			if(event->key.action == MP_KEY_RELEASE)
			{
				input->keys[event->key.code] &= ~MP_KEY_STATE_DOWN;
				input->keys[event->key.code] |= MP_KEY_STATE_RELEASED;
			}
			else if(  event->key.action == MP_KEY_PRESS
			       || event->key.action == MP_KEY_REPEAT)
			{
				//TODO: maybe distinguish pressed and repeat ?
				input->keys[event->key.code] |= (MP_KEY_STATE_DOWN | MP_KEY_STATE_PRESSED);
			}
		} break;

		default:
			break;
	}
}

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
   * Core code (general styles, ability for custom styles)
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

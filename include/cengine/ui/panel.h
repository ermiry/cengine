#ifndef _CENGINE_UI_PANEL_H_
#define _CENGINE_UI_PANEL_H_

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"

#include "cengine/video.h"
#include "cengine/renderer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/layout.h"

typedef struct Panel {

	UIElement *ui_element;

	// background
	bool colour;
	RGBA_Color bg_colour;
	SDL_Texture *bg_texture;
	SDL_Rect bg_texture_rect;

	// outline
	bool outline;
	RGBA_Color outline_colour;
	float outline_scale_x;
	float outline_scale_y;

	// layout
	LayoutType layout_type;
	void *layout;

	// media
	u32 original_w, original_h;

} Panel;

extern void ui_panel_delete (void *panel_ptr);

// sets the background colour of the panel
extern void ui_panel_set_bg_colour (Panel *panel, Renderer *renderer, RGBA_Color colour);

// removes the background from the panel
extern void ui_panel_remove_background (Panel *panel);

// sets the panel's outline colour
extern void ui_panel_set_ouline_colour (Panel *panel, RGBA_Color colour);

// sets the panel's outline scale
extern void ui_panel_set_ouline_scale (Panel *panel, float x_scale, float y_scale);

// removes the ouline form the panel
extern void ui_panel_remove_outline (Panel *panel);

// sets the layout for the panel
extern void ui_panel_layout_set (Panel *panel, LayoutType type);

// removes the existing layout form the panel
extern void ui_panel_layout_remove (Panel *panel);

// adds a new ui elment to the layout of the panel
extern void ui_panel_layout_add_element (Panel *panel, UIElement *ui_element);

// removes a ui element form the panel layout
extern void ui_panel_layout_remove_element (Panel *panel, UIElement *ui_element);

// creates a new panel
// x and y for position
extern Panel *ui_panel_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer);

// rezises the panel based on window size
extern void ui_panel_resize (Panel *panel, WindowSize window_original_size, WindowSize window_new_size);

// draws the panel to the screen
extern void ui_panel_draw (Panel *panel, Renderer *renderer);

#endif
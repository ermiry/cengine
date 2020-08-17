#ifndef _CENGINE_UI_PANEL_H_
#define _CENGINE_UI_PANEL_H_

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/config.h"
#include "cengine/renderer.h"
#include "cengine/video.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/layout.h"
#include "cengine/ui/layout/horizontal.h"
#include "cengine/ui/layout/vertical.h"
#include "cengine/ui/layout/grid.h"

struct _Renderer;

struct _Panel {

	// 08/02/2020 -- aux pointer
	struct _Renderer *renderer;

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

	DoubleList *children;

	// media
	u32 original_w, original_h;

};

typedef struct _Panel Panel;

CENGINE_PUBLIC void ui_panel_delete (void *panel_ptr);

// sets the panel's UI position
CENGINE_EXPORT void ui_panel_set_pos (Panel *panel, UIRect *ref_rect, UIPosition pos, Renderer *renderer);

// sets the panel's UI position offset
CENGINE_EXPORT void ui_panel_set_pos_offset (Panel *panel, int x_offset, int y_offset);

// updates one panel's child position
CENGINE_EXPORT void ui_panel_child_update_pos (Panel *panel, UIElement *child);

// updates the panel's children positions
CENGINE_EXPORT void ui_panel_children_update_pos (Panel *panel);

// sets the background colour of the panel
CENGINE_EXPORT void ui_panel_set_bg_colour (Panel *panel, Renderer *renderer, RGBA_Color colour);

// removes the background from the panel
CENGINE_EXPORT void ui_panel_remove_background (Panel *panel);

// sets the panel's outline colour
CENGINE_EXPORT void ui_panel_set_ouline_colour (Panel *panel, RGBA_Color colour);

// sets the panel's outline scale
CENGINE_EXPORT void ui_panel_set_ouline_scale (Panel *panel, float x_scale, float y_scale);

// removes the ouline form the panel
CENGINE_EXPORT void ui_panel_remove_outline (Panel *panel);

// get the amount of elements that are inside the panel's layout
CENGINE_EXPORT size_t ui_panel_layout_get_elements_count (Panel *panel);

// sets the layout for the panel
CENGINE_EXPORT void ui_panel_layout_set (Panel *panel, LayoutType type, struct _Renderer *renderer);

// removes the existing layout form the panel
CENGINE_EXPORT void ui_panel_layout_remove (Panel *panel);

// adds a new ui element to the panel's layout's in the specified position (0 indexed)
CENGINE_EXPORT void ui_panel_layout_add_element_at_pos (Panel *panel, UIElement *ui_element, u32 pos);

// adds a new ui element to the panel's layout's END
CENGINE_EXPORT void ui_panel_layout_add_element_at_end (Panel *panel, UIElement *ui_element);

 // returns the ui element that is at the required position in the panel's layout
CENGINE_EXPORT UIElement *ui_panel_layout_get_element_at (Panel *panel, unsigned int pos);

// removes a ui element form the panel layout
CENGINE_EXPORT u8 ui_panel_layout_remove_element (Panel *panel, UIElement *ui_element);

// adds a new child to the panel
CENGINE_EXPORT void ui_panel_child_add (Panel *panel, UIElement *ui_element);

// removes a child from the panel (the dlist uses a ui element ids comparator)
// returns the ui element that was removed
CENGINE_EXPORT UIElement *ui_panel_child_remove (Panel *panel, UIElement *ui_element);

// creates a new panel
// x and y for position
CENGINE_EXPORT Panel *ui_panel_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer);

// rezises the panel based on window size
CENGINE_PRIVATE void ui_panel_resize (Panel *panel, WindowSize window_original_size, WindowSize window_new_size);

// draws the panel to the screen
CENGINE_PRIVATE void ui_panel_draw (Panel *panel, Renderer *renderer);

#endif
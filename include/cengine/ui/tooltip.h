#ifndef _CENGINE_UI_TOOLTIP_H_
#define _CENGINE_UI_TOOLTIP_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>

#include "cengine/collections/dlist.h"

#include "cengine/config.h"
#include "cengine/renderer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"

#include "cengine/ui/layout/layout.h"
#include "cengine/ui/layout/vertical.h"

struct _Renderer;
struct _UIElement;

typedef struct Tooltip {

    struct _Renderer *renderer;

    struct _UIElement *ui_element;

    // background
	bool colour;
	RGBA_Color bg_colour;
	SDL_Texture *bg_texture;
	SDL_Rect bg_texture_rect;

    VerticalLayout *vertical;

    DoubleList *children;

} Tooltip;

CENGINE_PUBLIC void ui_tooltip_delete (void *tooltip_ptr);

// sets the background colour of the tooltip
CENGINE_EXPORT void ui_tooltip_set_bg_colour (Tooltip *tooltip, struct _Renderer *renderer, RGBA_Color colour);

// removes the background from the tooltip
CENGINE_EXPORT void ui_tooltip_remove_background (Tooltip *tooltip);

// updates the tooltip's children positions
CENGINE_EXPORT void ui_tooltip_children_update_pos (Tooltip *tooltip);

// adds a new ui element to the tooltip's layout's in the specified position (0 indexed)
CENGINE_EXPORT void ui_tooltip_add_element_at_pos (Tooltip *tooltip, struct _UIElement *ui_element, u32 pos);

// adds a new ui element to the tooltip's layout's END
CENGINE_EXPORT void ui_tooltip_add_element_at_end (Tooltip *tooltip, struct _UIElement *ui_element);

// returns the ui element that is at the required position in the tooltip's layout
CENGINE_EXPORT struct _UIElement *ui_tooltip_get_element_at (Tooltip *tooltip, unsigned int pos);

// removes a ui element form the tooltip's layout
CENGINE_EXPORT u8 ui_tooltip_remove_element (Tooltip *tooltip, struct _UIElement *ui_element);

CENGINE_EXPORT Tooltip *ui_tooltip_create (u32 w, u32 h, struct _Renderer *renderer);

// draws the tooltip to the screen
CENGINE_PRIVATE void ui_tooltip_draw (Tooltip *tooltip, struct _Renderer *renderer);

#endif
#ifndef _CENGINE_UI_PANEL_H_
#define _CENGINE_UI_PANEL_H_

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"

typedef struct Panel {

    UIElement *ui_element;
    UITransform *transform;

     // background
    bool colour;
    RGBA_Color bg_colour;
    SDL_Texture *bg_texture;
    SDL_Rect bg_texture_rect;

    // outline
    bool outline;
    RGBA_Color outline_colour;

} Panel;

extern void ui_panel_delete (void *panel_ptr);

// sets the background colour of the panel
extern void ui_panel_set_bg_colour (Panel *panel, RGBA_Color colour);

// removes the background from the panel
extern void ui_panel_remove_background (Panel *panel);

// sets the panel's outline colour
extern void ui_panel_set_ouline_colour (Panel *panel, RGBA_Color colour);

// removes the ouline form the panel
extern void ui_panel_remove_outline (Panel *panel);

// creates a new panel
// x and y for position
extern Panel *ui_panel_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos);

// draws the panel to the screen
extern void ui_panel_draw (Panel *panel);

#endif
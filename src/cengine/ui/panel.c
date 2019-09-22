#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"

#include "cengine/renderer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/panel.h"
#include "cengine/ui/components/transform.h"

static Panel *ui_panel_new (void) {

    Panel *panel = (Panel *) malloc (sizeof (Panel));
    if (panel) {
        memset (panel, 0, sizeof (Panel));
        panel->ui_element = NULL;
        panel->transform = NULL;

        panel->colour = false;
        panel->bg_texture = NULL;
        panel->outline = false;
    }

    return panel;

}

void ui_panel_delete (void *panel_ptr) {

    if (panel_ptr) {
        Panel *panel = (Panel *) panel_ptr;
        panel->ui_element = NULL;
        ui_transform_component_delete (panel->transform);
        if (panel->bg_texture) SDL_DestroyTexture (panel->bg_texture);
        free (panel);
    }

}

// sets the background colour of the panel
void ui_panel_set_bg_colour (Panel *panel, RGBA_Color colour) {

    if (panel) {
        panel->colour = true;
        panel->bg_colour = colour;
        if (colour.a < 255) {
            panel->bg_texture = render_complex_transparent_rect (&panel->transform->rect, colour);
            panel->bg_texture_rect.w = panel->transform->rect.w;
            panel->bg_texture_rect.h = panel->transform->rect.h;
        }
    } 

}

// removes the background from the panel
void ui_panel_remove_background (Panel *panel) {

    if (panel) {
        if (panel->bg_texture) {
            SDL_DestroyTexture (panel->bg_texture);
            panel->bg_texture = NULL;
        }

        memset (&panel->bg_colour, 0, sizeof (RGBA_Color));
        panel->colour = false;
    }

}

// sets the panel's outline colour
void ui_panel_set_ouline_colour (Panel *panel, RGBA_Color colour) {

    if (panel) {
        panel->outline = true;
        panel->outline_colour = colour;
    }

}

// removes the ouline form the panel
void ui_panel_remove_outline (Panel *panel) {

    if (panel) {
        memset (&panel->outline_colour, 0, sizeof (RGBA_Color));
        panel->outline = false;
    }

}

// creates a new panel
// x and y for position
Panel *ui_panel_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos) {

    Panel *panel = NULL;

    UIElement *ui_element = ui_element_new (UI_PANEL);
    if (ui_element) {
        panel = ui_panel_new ();
        if (panel) {
            panel->ui_element = ui_element;
            panel->transform = ui_transform_component_create (x, y, w, h);
            ui_transform_component_set_pos (panel->transform, NULL, pos, true);

            ui_element->element = panel;
        }
    }

    return panel;

}

// draws the panel to the screen
void ui_panel_draw (Panel *panel) {

    // render the background
    if (panel->colour) {
        if (panel->bg_texture) {
            SDL_RenderCopyEx (main_renderer->renderer, panel->bg_texture, 
                &panel->bg_texture_rect, &panel->transform->rect, 
                0, 0, SDL_FLIP_NONE);
        }

        else {
            render_basic_filled_rect (&panel->transform->rect, panel->bg_colour);
        }
    }

    // render the outline
    if (panel->outline) 
        render_basic_outline_rect (&panel->transform->rect, panel->outline_colour);

}
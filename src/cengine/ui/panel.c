#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"

#include "cengine/video.h"
#include "cengine/renderer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/panel.h"
#include "cengine/ui/components/transform.h"

#include "cengine/ui/layout/layout.h"
#include "cengine/ui/layout/horizontal.h"
#include "cengine/ui/layout/vertical.h"
#include "cengine/ui/layout/grid.h"

void ui_panel_layout_remove (Panel *panel);

static Panel *ui_panel_new (void) {

    Panel *panel = (Panel *) malloc (sizeof (Panel));
    if (panel) {
        memset (panel, 0, sizeof (Panel));
        panel->ui_element = NULL;

        panel->colour = false;
        panel->bg_texture = NULL;
        panel->outline = false;

        panel->layout = NULL;
    }

    return panel;

}

void ui_panel_delete (void *panel_ptr) {

    if (panel_ptr) {
        Panel *panel = (Panel *) panel_ptr;

        panel->ui_element = NULL;
        if (panel->bg_texture) SDL_DestroyTexture (panel->bg_texture);

        ui_panel_layout_remove (panel);

        free (panel);
    }

}

// sets the background colour of the panel
void ui_panel_set_bg_colour (Panel *panel, Renderer *renderer, RGBA_Color colour) {

    if (panel) {
        panel->colour = true;
        panel->bg_colour = colour;
        if (colour.a < 255) {
            render_complex_transparent_rect (renderer, &panel->bg_texture, &panel->ui_element->transform->rect, colour);
            panel->bg_texture_rect.w = panel->ui_element->transform->rect.w;
            panel->bg_texture_rect.h = panel->ui_element->transform->rect.h;
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

// sets the panel's outline scale
void ui_panel_set_ouline_scale (Panel *panel, float x_scale, float y_scale) {

    if (panel) {
        panel->outline_scale_x = x_scale;
        panel->outline_scale_y = y_scale;
    }

}

// removes the ouline form the panel
void ui_panel_remove_outline (Panel *panel) {

    if (panel) {
        memset (&panel->outline_colour, 0, sizeof (RGBA_Color));
        panel->outline = false;
    }

}

// sets the layout for the panel
void ui_panel_layout_set (Panel *panel, LayoutType type) {

    if (panel) {
        if (panel->layout) ui_panel_layout_remove (panel);

        panel->layout_type = type;
        switch (panel->layout_type) {
            case LAYOUT_TYPE_HORIZONTAL: 
                panel->layout = ui_layout_horizontal_create (panel->ui_element->transform->rect.y, panel->ui_element->transform->rect.x, 
                    panel->ui_element->transform->rect.w, panel->ui_element->transform->rect.h);
                break;
            case LAYOUT_TYPE_VERTICAL:
                panel->layout = ui_layout_vertical_create (panel->ui_element->transform->rect.y, panel->ui_element->transform->rect.x, 
                    panel->ui_element->transform->rect.w, panel->ui_element->transform->rect.h);
                break;
            case LAYOUT_TYPE_GRID: 
                panel->layout = ui_layout_grid_create (panel->ui_element->transform->rect.x, panel->ui_element->transform->rect.y, 
                    panel->ui_element->transform->rect.w, panel->ui_element->transform->rect.h);
                break;

            default: break;
        }
    }

}

// removes the existing layout form the panel
void ui_panel_layout_remove (Panel *panel) {

    if (panel) {
        if (panel->layout) {
            switch (panel->layout_type) {
                case LAYOUT_TYPE_HORIZONTAL: ui_layout_horizontal_delete (panel->layout); break;
                case LAYOUT_TYPE_VERTICAL: ui_layout_vertical_delete (panel->layout); break;
                case LAYOUT_TYPE_GRID: ui_layout_grid_delete (panel->layout); break;

                default: break;
            }

            panel->layout = NULL;
            panel->layout_type = LAYOUT_TYPE_NONE;
        }
    }

}

// adds a new ui elment to the layout of the panel
void ui_panel_layout_add_element (Panel *panel, UIElement *ui_element) {

    if (panel && ui_element) {
        if (panel->layout) {
            switch (panel->layout_type) {
                case LAYOUT_TYPE_HORIZONTAL: break;
                case LAYOUT_TYPE_VERTICAL: break;
                case LAYOUT_TYPE_GRID: 
                    ui_layout_grid_add_element ((GridLayout *) panel->layout, ui_element); 
                    break;

                default: break;
            }
        }
    }

}

// removes a ui element form the panel layout
void ui_panel_layout_remove_element (Panel *panel, UIElement *ui_element) {

    if (panel && ui_element) {
        if (panel->layout) {
            switch (panel->layout_type) {
                case LAYOUT_TYPE_HORIZONTAL: break;
                case LAYOUT_TYPE_VERTICAL: break;
                case LAYOUT_TYPE_GRID: 
                    ui_layout_grid_remove_element ((GridLayout *) panel->layout, ui_element); 
                    break;

                default: break;
            }
        }
    }

}

// creates a new panel
// x and y for position
Panel *ui_panel_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer) {

    Panel *panel = NULL;

    UIElement *ui_element = ui_element_create (renderer->ui, UI_PANEL);
    if (ui_element) {
        panel = ui_panel_new ();
        if (panel) {
            panel->ui_element = ui_element;
            ui_transform_component_set_values (panel->ui_element->transform, x, y, w, h);
            ui_transform_component_set_pos (panel->ui_element->transform, renderer, NULL, pos, true);

            ui_element->element = panel;

            panel->outline_scale_x = 1;
            panel->outline_scale_y = 1;

            panel->original_w = w;
            panel->original_h = h;
        }
    }

    return panel;

}

// rezises the panel based on window size
void ui_panel_resize (Panel *panel, WindowSize window_original_size, WindowSize window_new_size) {

    if (panel) {
        if ((window_original_size.width == window_new_size.width) && window_original_size.height == window_new_size.height) {
            panel->ui_element->transform->rect.w = panel->original_w;
            panel->ui_element->transform->rect.h = panel->original_h;
        }

        else {
            u32 new_width = (window_new_size.width * panel->ui_element->transform->rect.w) / window_original_size.width;
            u32 new_height = (window_new_size.height * panel->ui_element->transform->rect.h) / window_original_size.height;
            panel->ui_element->transform->rect.w = new_width;
            panel->ui_element->transform->rect.h = new_height;
        }
    }

}

// draws the panel to the screen
void ui_panel_draw (Panel *panel, Renderer *renderer) {

    if (panel && renderer) {
        if (SDL_HasIntersection (&panel->ui_element->transform->rect, &renderer->window->screen_rect)) {
            // render the background
            if (panel->colour) {
                if (panel->bg_texture) {
                    SDL_RenderCopyEx (renderer->renderer, panel->bg_texture, 
                        &panel->bg_texture_rect, &panel->ui_element->transform->rect, 
                        0, 0, SDL_FLIP_NONE);
                }

                else {
                    render_basic_filled_rect (renderer, &panel->ui_element->transform->rect, panel->bg_colour);
                }
            }

            // render the outline
            if (panel->outline) 
                render_basic_outline_rect (renderer, &panel->ui_element->transform->rect, panel->outline_colour,
                    panel->outline_scale_x, panel->outline_scale_y);

            renderer->render_count += 1;
        }
    }

}
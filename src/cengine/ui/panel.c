#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/video.h"
#include "cengine/renderer.h"
#include "cengine/textures.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/panel.h"
#include "cengine/ui/components/transform.h"

// used for children
#include "cengine/ui/inputfield.h"
#include "cengine/ui/button.h"
#include "cengine/ui/image.h"
#include "cengine/ui/textbox.h"

#include "cengine/ui/layout/layout.h"
#include "cengine/ui/layout/horizontal.h"
#include "cengine/ui/layout/vertical.h"
#include "cengine/ui/layout/grid.h"

void ui_panel_child_add (Panel *panel, UIElement *ui_element);
UIElement *ui_panel_child_remove (Panel *panel, UIElement *ui_element);
void ui_panel_children_update_pos (Panel *panel);
void ui_panel_layout_remove (Panel *panel);

static Panel *ui_panel_new (void) {

    Panel *panel = (Panel *) malloc (sizeof (Panel));
    if (panel) {
        memset (panel, 0, sizeof (Panel));

        panel->renderer = NULL;

        panel->ui_element = NULL;

        panel->colour = false;
        panel->bg_texture = NULL;
        panel->outline = false;

        panel->layout = NULL;

        panel->children = NULL;
    }

    return panel;

}

void ui_panel_delete (void *panel_ptr) {

    if (panel_ptr) {
        Panel *panel = (Panel *) panel_ptr;

        panel->ui_element = NULL;

        texture_destroy (panel->renderer, panel->bg_texture);

        ui_panel_layout_remove (panel);

        dlist_delete (panel->children);

        free (panel);
    }

}

// sets the panel's UI position
void ui_panel_set_pos (Panel *panel, UIRect *ref_rect, UIPosition pos, Renderer *renderer) {

    if (panel) {
        ui_transform_component_set_pos (panel->ui_element->transform, 
            renderer, 
            ref_rect, 
            pos, 
            false);
        ui_panel_children_update_pos (panel);
    }

}

// sets the panel's UI position offset
void ui_panel_set_pos_offset (Panel *panel, int x_offset, int y_offset) {

    if (panel) {
        panel->ui_element->transform->x_offset = x_offset;
        panel->ui_element->transform->y_offset = y_offset;
    }

}

// updates one panel's child position
void ui_panel_child_update_pos (Panel *panel, UIElement *child) {

    if (panel && child) {
         // update the element's transform values
        switch (child->type) {
            case UI_BUTTON: {
                // FIXME: make sure that all values are updated as well
                ui_button_set_pos ((Button *) child->element,
                    &panel->ui_element->transform->rect,
                    child->transform->pos,
                    NULL);
            } break;

            case UI_IMAGE: {
                ui_image_set_pos ((Image *) child->element,
                    &panel->ui_element->transform->rect, 
                    child->transform->pos, 
                    NULL);
            } break;

            case UI_INPUT: {
                // FIXME: 04/02/2020 -- 18:03 -- update children's pos
                ui_input_field_set_pos ((InputField *) child->element,
                    &panel->ui_element->transform->rect, 
                    child->transform->pos, 
                    NULL);
            } break;

            case UI_PANEL: {
                // FIXME: 04/02/2020 -- 18:03 -- update children's pos
                ui_panel_set_pos ((Panel *) child->element, 
                    &panel->ui_element->transform->rect, 
                    child->transform->pos, 
                    NULL);
                // ui_position_update_to_parent (panel->ui_element->transform,
                //     ((Panel *) child->element)->ui_element->transform, false);
            } break;

            case UI_TEXTBOX: {
                TextBox *textbox = (TextBox *) child->element;
                ui_textbox_set_pos (textbox,
                    &panel->ui_element->transform->rect, 
                    child->transform->pos, 
                    NULL);
                // FIXME:
                // ui_position_update_to_parent (panel->ui_element->transform,
                //     textbox->ui_element->transform, false);
                // ui_textbox_update_text_pos (textbox);
                // printf ("%d - %d\n", textbox->ui_element->transform->rect.x, textbox->ui_element->transform->rect.y);
            } break;

            default: break;
        }
    }

}

// updates the panel's children positions
void ui_panel_children_update_pos (Panel *panel) {

    if (panel) {
        for (ListElement *le = dlist_start (panel->children); le; le = le->next) {
            UIElement *child = (UIElement *) le->data;

            ui_panel_child_update_pos (panel, child);
        }
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
            texture_destroy (panel->renderer, panel->bg_texture);
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

// get the amount of elements that are inside the panel's layout
size_t ui_panel_layout_get_elements_count (Panel *panel) {

    size_t retval = 0;

    if (panel) {
        if (panel->layout) {
            switch (panel->layout_type) {
                case LAYOUT_TYPE_HORIZONTAL: 
                    retval = ui_layout_horizontal_get_elements_count ((HorizontalLayout *) panel->layout);
                    break;
                case LAYOUT_TYPE_VERTICAL:
                    retval = ui_layout_vertical_get_elements_count ((VerticalLayout *) panel->layout);
                    break;
                case LAYOUT_TYPE_GRID: 
                    retval = ui_layout_grid_get_elements_count ((GridLayout *) panel->layout);
                    break;

                default: break;
            }
        }
    }

    return retval;

}

// sets the layout for the panel
void ui_panel_layout_set (Panel *panel, LayoutType type, Renderer *renderer) {

    if (panel) {
        if (panel->layout) ui_panel_layout_remove (panel);

        panel->layout_type = type;
        switch (panel->layout_type) {
            case LAYOUT_TYPE_HORIZONTAL: 
                panel->layout = ui_layout_horizontal_create (panel->ui_element->transform->rect.x, panel->ui_element->transform->rect.y, 
                    panel->ui_element->transform->rect.w, panel->ui_element->transform->rect.h,
                    // panel,
                    renderer);
                break;
            case LAYOUT_TYPE_VERTICAL:
                panel->layout = ui_layout_vertical_create (panel->ui_element->transform->rect.x, panel->ui_element->transform->rect.y, 
                    panel->ui_element->transform->rect.w, panel->ui_element->transform->rect.h, 
                    renderer);
                break;
            case LAYOUT_TYPE_GRID: 
                panel->layout = ui_layout_grid_create (panel->ui_element->transform->rect.x, panel->ui_element->transform->rect.y, 
                    panel->ui_element->transform->rect.w, panel->ui_element->transform->rect.h,
                    renderer);
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

// adds a new ui element to the panel's layout's in the specified position (0 indexed)
void ui_panel_layout_add_element_at_pos (Panel *panel, UIElement *ui_element, u32 pos) {

    if (panel && ui_element) {
        if (panel->layout) {
            // add the element to the panel's children
            ui_panel_child_add (panel, ui_element);

            switch (panel->layout_type) {
                case LAYOUT_TYPE_HORIZONTAL: 
                    ui_layout_horizontal_add_at_pos ((HorizontalLayout *) panel->layout, ui_element, pos);
                    break;
                case LAYOUT_TYPE_VERTICAL: 
                    ui_layout_vertical_add_at_pos ((VerticalLayout *) panel->layout, ui_element, pos);
                    break;
                case LAYOUT_TYPE_GRID: 
                    ui_layout_grid_add_element_at_pos ((GridLayout *) panel->layout, ui_element, pos);
                    break;

                default: break;
            }

            switch (ui_element->type) {
                case UI_PANEL: ui_panel_children_update_pos ((Panel *) ui_element->element); break;
                
                default: break;
            }
        }
    }

}

// adds a new ui element to the panel's layout's END
void ui_panel_layout_add_element_at_end (Panel *panel, UIElement *ui_element) {

    if (panel && ui_element) {
        if (panel->layout) {
            // add the element to the panel's children
            ui_panel_child_add (panel, ui_element);

            switch (panel->layout_type) {
                case LAYOUT_TYPE_HORIZONTAL: 
                    ui_layout_horizontal_add_at_end ((HorizontalLayout *) panel->layout, ui_element);
                    break;
                case LAYOUT_TYPE_VERTICAL:
                    ui_layout_vertical_add_at_end ((VerticalLayout *) panel->layout, ui_element);
                    break;
                case LAYOUT_TYPE_GRID: 
                    ui_layout_grid_add_element_at_end ((GridLayout *) panel->layout, ui_element);
                    break;

                default: break;
            }

            switch (ui_element->type) {
                case UI_PANEL: ui_panel_children_update_pos ((Panel *) ui_element->element); break;
                
                default: break;
            }
        }
    }

}

// returns the ui element that is at the required position in the panel's layout
UIElement *ui_panel_layout_get_element_at (Panel *panel, unsigned int pos) {

    UIElement *retval = NULL;

    if (panel) {
        if (panel->layout) {
            switch (panel->layout_type) {
                case LAYOUT_TYPE_HORIZONTAL: 
                    retval = ui_layout_horizontal_get_element_at ((HorizontalLayout *) panel->layout, pos);
                    break;
                case LAYOUT_TYPE_VERTICAL:
                    retval = ui_layout_vertical_get_element_at ((VerticalLayout *) panel->layout, pos);
                    break;
                case LAYOUT_TYPE_GRID: 
                     retval = ui_layout_grid_get_element_at ((GridLayout *) panel->layout, pos);
                    break;

                default: break;
            }
        }
    }

    return retval;

}

// removes a ui element form the panel layout
u8 ui_panel_layout_remove_element (Panel *panel, UIElement *ui_element) {

    u8 retval = 1;

    if (panel && ui_element) {
        if (panel->layout) {
            switch (panel->layout_type) {
                case LAYOUT_TYPE_HORIZONTAL: 
                    retval = ui_layout_horizontal_remove ((HorizontalLayout *) panel->layout, ui_element);
                    break;
                case LAYOUT_TYPE_VERTICAL: 
                    retval = ui_layout_vertical_remove ((VerticalLayout *) panel->layout, ui_element);
                    break;
                case LAYOUT_TYPE_GRID: 
                    retval = ui_layout_grid_remove_element ((GridLayout *) panel->layout, ui_element); 
                    break;

                default: break;
            }

            // remove the element from the panel's children
            ui_panel_child_remove (panel, ui_element);
        }
    }

    return retval;

}

// adds a new child to the panel
// the element's position will be managed by the panel
// when the panel gets destroyed, all of its children get destroyed as well 
void ui_panel_child_add (Panel *panel, UIElement *ui_element) {

    if (panel && ui_element) {
        // remove the element from the ui elements
        ui_remove_element (panel->renderer->ui, ui_element);

        // remove element from its layers to let the panel manage how the element gets renderer
        Layer *layer = layer_get_by_pos (panel->renderer->ui->ui_elements_layers, 
            ui_element->layer_id);
        layer_remove_element (layer, ui_element);

        dlist_insert_after (panel->children, dlist_end (panel->children), ui_element);

        ui_panel_child_update_pos (panel, ui_element);

        ui_element->abs_offset_x = panel->ui_element->transform->rect.x;
        ui_element->abs_offset_y = panel->ui_element->transform->rect.y;

        ui_element->parent = panel->ui_element;
    }

}

// removes a child from the panel (the dlist uses a ui element ids comparator)
// returns the ui element that was removed
UIElement *ui_panel_child_remove (Panel *panel, UIElement *ui_element) {

    UIElement *retval = NULL;

    if (panel && ui_element) {
        retval = (UIElement *) dlist_remove (panel->children, ui_element, NULL);

        // add the element back to its original layer
        layer_add_element (layer_get_by_pos (panel->renderer->ui->ui_elements_layers,
            ui_element->layer_id), ui_element);

        // add the element back to the UI
        ui_add_element (panel->renderer->ui, ui_element);

        ui_element->parent = NULL;
    }

    return retval;

}

// creates a new panel
// x and y for position
Panel *ui_panel_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer) {

    Panel *panel = NULL;

    UIElement *ui_element = ui_element_create (renderer->ui, UI_PANEL);
    if (ui_element) {
        panel = ui_panel_new ();
        if (panel) {
            panel->renderer = renderer;

            panel->ui_element = ui_element;
            ui_transform_component_set_values (panel->ui_element->transform, x, y, w, h);
            ui_transform_component_set_pos (panel->ui_element->transform, renderer, NULL, pos, true);

            ui_element->element = panel;

            panel->outline_scale_x = 1;
            panel->outline_scale_y = 1;

            panel->children = dlist_init (ui_element_delete, ui_element_comparator);

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

            // 08/02/2020 - render the panel's children
            if (panel->children) {
                // SDL_Rect viewport;
                // SDL_RenderGetViewport (renderer->renderer, &viewport);

                if (panel->layout) SDL_RenderSetViewport (renderer->renderer, &panel->ui_element->transform->rect);
                for (ListElement *le = dlist_start (panel->children); le; le = le->next)
                    ui_render_element (renderer, (UIElement *) le->data);
                
                // renderer_set_viewport (renderer, viewport.x, viewport.y, viewport.w, viewport.h);
                if (panel->layout) renderer_set_viewport_to_window_size (renderer);
                // renderer_set_viewport_to_previous_size (renderer);
            }

            // render the outline
            if (panel->outline) {
                render_basic_outline_rect (renderer, &panel->ui_element->transform->rect, panel->outline_colour,
                    panel->outline_scale_x, panel->outline_scale_y);
            }
                

            renderer->render_count += 1;
        }
    }

}
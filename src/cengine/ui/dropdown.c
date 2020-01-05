#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/types/vector2d.h"
#include "cengine/collections/dlist.h"

#include "cengine/graphics.h"
#include "cengine/renderer.h"
#include "cengine/input.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/position.h"
#include "cengine/ui/button.h"
#include "cengine/ui/dropdown.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"
#include "cengine/ui/layout/vertical.h"

void ui_dropdown_option_set_text (DropdownOption *option, Renderer *renderer, const char *option_text,
    Font *font, u32 size, RGBA_Color color);

#pragma region Dropdown Option

static DropdownOption *ui_dropdown_option_new (void) {

    DropdownOption *option = (DropdownOption *) malloc (sizeof (DropdownOption));
    if (option) {
        memset (option, 0, sizeof (DropdownOption));
        option->button = NULL;
        option->draw_selected = false;
    }

    return option;

}

void ui_dropdown_option_delete (void *dropdown_option_ptr) {

    if (dropdown_option_ptr) {
        DropdownOption *option = (DropdownOption *) dropdown_option_ptr;
        ui_element_destroy (option->button->ui_element);
        free (option);
    }

}

// dropdown_option_comparator by option's name
int ui_dropdown_option_comparator (const void *one, const void *two) {

    if (one && two) 
        return str_compare (((DropdownOption *) one)->button->text->text, ((DropdownOption *) two)->button->text->text);

    else if (one) return -1;
    else if (two) return 1;
    return 0;

}

// creates a new dropdown option to be added to a dropdown
// options to pass text modifiers
DropdownOption *ui_dropdown_option_create (Renderer *renderer, const char *option_text,
    Font *font, u32 size, RGBA_Color color) {

    DropdownOption *option = ui_dropdown_option_new ();
    if (option) {
        option->button = ui_button_create (0, 0, 0, 0, UI_POS_FREE, renderer);
        ui_dropdown_option_set_text (option, renderer, option_text, font, size, color);
    }

    return option;

}

// sets the option's text for the dropdown option element
void ui_dropdown_option_set_text (DropdownOption *option, Renderer *renderer, const char *option_text,
    Font *font, u32 size, RGBA_Color color) {

    if (option) {
        ui_button_set_text (option->button, renderer, option_text, font, size, color);
    }

}

// sets the option's outline colour
void ui_dropdown_option_set_ouline_colour (DropdownOption *option, RGBA_Color colour) {

    if (option) ui_button_set_ouline_colour (option->button, colour);

}

// sets the option's outline scale
void ui_dropdown_option_set_outline_scale (DropdownOption *option, float x_scale, float y_scale) {

    if (option) ui_button_set_ouline_scale (option->button, x_scale, y_scale);

}

// removes the ouline form the option
void ui_dropdown_option_remove_outline (DropdownOption *option) {

    if (option) ui_button_remove_outline (option->button);
}

// sets the option's background color
void ui_dropdown_option_set_bg_color (DropdownOption *option, Renderer *renderer, RGBA_Color color) {

    if (option) ui_button_set_bg_color (option->button, renderer, color);

}

// removes the background from the option
void ui_dropdown_option_remove_background (DropdownOption *option) {

    if (option) ui_button_remove_background (option->button);

}

static void ui_dropdown_option_render (DropdownOption *option, Renderer *renderer) {

    if (option) ui_button_draw (option->button, renderer);

}

#pragma endregion

#pragma region Dropdown

static Dropdown *ui_dropdown_new (void) {

    Dropdown *dropdown = (Dropdown *) malloc (sizeof (Dropdown));
    if (dropdown) {
        memset (dropdown, 0, sizeof (dropdown));

        dropdown->ui_element = NULL;

        dropdown->active = true;
        dropdown->pressed = false;
        dropdown->extended = false;

        dropdown->colour = false;
        dropdown->bg_texture = NULL;
        dropdown->outline = false;
        dropdown->placeholder = NULL;

        dropdown->option_hover_texture = NULL;
        dropdown->option_selected = NULL;
        dropdown->options = NULL;
        dropdown->extended_panel = NULL;
        dropdown->vertical_layout = NULL;
    }

    return dropdown;

}

void ui_dropdown_delete (void *dropdown_ptr) {

    if (dropdown_ptr) {
        Dropdown *dropdown = (Dropdown *) dropdown_ptr;

        dropdown->ui_element = NULL;
        if (dropdown->bg_texture) SDL_DestroyTexture (dropdown->bg_texture);
        ui_text_component_delete (dropdown->placeholder);

        dlist_delete (dropdown->options);

        if (dropdown->extended_panel) ui_element_destroy (dropdown->extended_panel->ui_element);
        ui_layout_vertical_delete (dropdown->vertical_layout);

        if (dropdown->option_hover_texture) SDL_DestroyTexture (dropdown->option_hover_texture);

        free (dropdown);
    }

}

// creates a new dropdown menu
Dropdown *ui_dropdown_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer) {

    Dropdown *dropdown = NULL;

    UIElement *ui_element = ui_element_create (renderer->ui, UI_DROPDOWN);
    if (ui_element) {
        dropdown = ui_dropdown_new ();
        if (dropdown) {
            dropdown->ui_element = ui_element;
            ui_transform_component_set_values (ui_element->transform, x, y, w, h);
            ui_transform_component_set_pos (dropdown->ui_element->transform, renderer, NULL, pos, true);

            ui_element->element = dropdown;
            ui_element->active = true;

            dropdown->outline_scale_x = 1;
            dropdown->outline_scale_y = 1;
        }

        else ui_element_delete (ui_element);
    }

    return dropdown;

}

// sets the main options for the dropdown (this is required to correctly create the dropdown)
// x, y: set the position of the dropdown's extended panel
// with, height: set the dimensions of the dropdown's extended panel
void ui_dropdown_set_options (Dropdown *dropdown, i32 x, i32 y,
    u32 options_width, u32 options_max_height, UIPosition pos, Renderer *renderer) {

    if (dropdown) {
        dropdown->option_selected = NULL;
        dropdown->options = dlist_init (ui_dropdown_option_delete, ui_dropdown_option_comparator);

        // create the extened panel 
        dropdown->extended_panel = ui_panel_create (x, y, options_width, options_max_height, UI_POS_FREE, renderer);
        ui_transform_component_set_pos (dropdown->extended_panel->ui_element->transform, NULL, &dropdown->ui_element->transform->rect, pos, false);
        dropdown->extended_panel->ui_element->transform->rect.y += options_max_height;
        dropdown->extended_panel->ui_element->transform->rect.x += x;
        dropdown->extended_panel->ui_element->transform->rect.y += y;
        dropdown->extended_panel->ui_element->active = false;

        // the layout group contains the transforms of the options that render on top of the extended panel
        dropdown->vertical_layout = ui_layout_vertical_create (x, y, options_width, options_max_height);
        ui_transform_component_set_pos (dropdown->vertical_layout->transform, NULL, &dropdown->ui_element->transform->rect, pos, false);
        dropdown->vertical_layout->transform->rect.y += options_max_height;
    }

}

// sets the dropdown to be active depending on values
void ui_dropdown_set_active (Dropdown *dropdown, bool active) {

    if (dropdown) {
        dropdown->active = active;
        if (!dropdown->active) dropdown->extended = false;
    } 

}

// toggles the dropdown field to be active or not
void ui_dropdown_toggle_active (Dropdown *dropdown) {

    if (dropdown) {
        dropdown->active = !dropdown->active;
        if (!dropdown->active) dropdown->extended = false;
    } 

}

// sets the dropdown's outline colour
void ui_dropdown_set_ouline_colour (Dropdown *dropdown, RGBA_Color colour) {

    if (dropdown) {
        dropdown->outline = true;
        dropdown->outline_colour = colour;
    }

}

// sets the dropdown's outline scale
void ui_dropdown_set_outline_scale (Dropdown *dropdown, float x_scale, float y_scale) {

    if (dropdown) {
        dropdown->outline_scale_x = x_scale;
        dropdown->outline_scale_y = y_scale;
    }

}

// removes the ouline form the dropdown
void ui_dropdown_remove_outline (Dropdown *dropdown) {

    if (dropdown) {
        memset (&dropdown->outline_colour, 0, sizeof (RGBA_Color));
        dropdown->outline = false;
    }

}

// sets the dropdown's background color
void ui_dropdown_set_bg_color (Dropdown *dropdown, Renderer *renderer, RGBA_Color color) {

    if (dropdown) {
        dropdown->bg_colour = color;
        if (color.a < 255) {
            render_complex_transparent_rect (renderer, &dropdown->bg_texture, &dropdown->ui_element->transform->rect, color);
            dropdown->bg_texture_rect.w = dropdown->ui_element->transform->rect.w;
            dropdown->bg_texture_rect.h = dropdown->ui_element->transform->rect.h;
        }

        dropdown->colour = true;
    } 

}

// removes the background from the dropdown
void ui_dropdown_remove_background (Dropdown *dropdown) {

    if (dropdown) {
        if (dropdown->bg_texture) {
            SDL_DestroyTexture (dropdown->bg_texture);
            dropdown->bg_texture = NULL;
        }

        memset (&dropdown->bg_colour, 0, sizeof (RGBA_Color));
        dropdown->colour = false;
    }

}

// sets the dropdown's placeholder text
void ui_dropdown_set_placeholder (Dropdown *dropdown, Renderer *renderer,
    const char *text, Font *font, u32 size, RGBA_Color colour) {

    if (dropdown) {
        if (dropdown->placeholder) ui_text_component_delete (dropdown->placeholder);

        dropdown->placeholder = text ? ui_text_component_new () : NULL;
        if (dropdown->placeholder) {
            ui_text_component_init (dropdown->placeholder,
                font, size, colour, text);

            // set the text position inside the dropdown field
            dropdown->placeholder->transform->rect.x = dropdown->ui_element->transform->rect.x;
            dropdown->placeholder->transform->rect.y = dropdown->ui_element->transform->rect.y;

            ui_text_component_draw (dropdown->placeholder, renderer);
        }
    }

}

// sets the dropdown's placeholder position
void ui_dropdown_set_placeholder_pos (Dropdown *dropdown, UIPosition pos) {

    if (dropdown) {
        if (dropdown->placeholder) 
            ui_transform_component_set_pos (dropdown->placeholder->transform, NULL, &dropdown->ui_element->transform->rect, pos, true);
    }

}

// sets the dropdown's extened panel colour
void ui_dropdown_extened_set_bg_colour (Dropdown *dropdown, Renderer *renderer, RGBA_Color colour) {

    if (dropdown) ui_panel_set_bg_colour (dropdown->extended_panel, renderer, colour);

}

// adds a new dropdown option to the dropdown
void ui_dropdown_option_add (Dropdown *dropdown, DropdownOption *option) {

    if (dropdown && option) {
        dlist_insert_after (dropdown->options, dlist_end (dropdown->options), option);
        ui_button_set_pos (option->button, &dropdown->ui_element->transform->rect, UI_POS_MIDDLE_CENTER, NULL);

        ui_layout_vertical_add (dropdown->vertical_layout, option->button->ui_element);

        // FIXME: move this to vertical layout
        DropdownOption *op = NULL;
        for (ListElement *le = dlist_start (dropdown->options); le; le = le->next) {
            op = (DropdownOption *) le->data;
            // ui_position_update (NULL, op->option->transform, &op->transform->rect, false);
        }
    }

}

// gets an option from a dropdown by its name
DropdownOption *ui_dropdown_option_get (Dropdown *dropdown, const char *value) {

    DropdownOption *retval = NULL;

    if (dropdown && retval) {
        DropdownOption *option = ui_dropdown_option_new ();
        if (option) {
            option->button->text->text = str_new (value);
            retval = (DropdownOption *) dlist_search (dropdown->options, option);
            ui_dropdown_option_delete (option);
        }
    }

    return retval;

}

// TODO: 02/12/2019 -- check this is correct
// removes a dropdown option from the dropdown
void ui_dropdown_option_remove (Dropdown *dropdown, DropdownOption *option) {

    if (dropdown && option) {
        ui_layout_vertical_remove (dropdown->vertical_layout, option->button->ui_element);
        dlist_remove_element (dropdown->options, dlist_get_element (dropdown->options, option));
    }

}

// gets the value of the selected option in the dropdown
String *ui_dropdown_option_selected (Dropdown *dropdown) {

    String *retval = NULL;

    if (dropdown) {
        if (dropdown->option_selected) {
            retval = dropdown->option_selected->button->text->text;
        }
    }

    return retval;

}

// sets the hovering colour for the dropdown options
// IMPORTANT: set the colour after you have added all the options to the dropdwon!
void ui_dropdown_option_set_hover_color (Dropdown *dropdown, Renderer *renderer, RGBA_Color color) {

    if (dropdown) {
        // FIXME: set the hover option to buttons!!
    } 

}

// render the dropdown to the screen
void ui_dropdown_render (Dropdown *dropdown, Renderer *renderer) {

    if (dropdown && renderer) {
        if (SDL_HasIntersection (&dropdown->ui_element->transform->rect, &renderer->window->screen_rect)) {
            // render the background
            if (dropdown->bg_texture) {
                SDL_RenderCopyEx (renderer->renderer, dropdown->bg_texture, 
                    &dropdown->bg_texture_rect, &dropdown->ui_element->transform->rect, 
                    0, 0, SDL_FLIP_NONE);
            }

            else if (dropdown->colour) 
                render_basic_filled_rect (renderer, &dropdown->ui_element->transform->rect, dropdown->bg_colour);

            // check if the mouse is in the dropdown
            if (dropdown->active) {
                if (mousePos.x >= dropdown->ui_element->transform->rect.x && mousePos.x <= (dropdown->ui_element->transform->rect.x + dropdown->ui_element->transform->rect.w) && 
                    mousePos.y >= dropdown->ui_element->transform->rect.y && mousePos.y <= (dropdown->ui_element->transform->rect.y + dropdown->ui_element->transform->rect.h)) {
                    // the mouse is over use
                    render_basic_filled_rect (renderer, &dropdown->ui_element->transform->rect, RGBA_BLACK);

                    // check if the user pressed the left dropdown over the mouse
                    if (input_get_mouse_button_state (MOUSE_LEFT)) {
                        dropdown->pressed = true;
                    }
                    
                    else if (!input_get_mouse_button_state (MOUSE_LEFT)) {
                        if (dropdown->pressed) {
                            dropdown->pressed = false;
                            // extend and show options
                            // printf ("Pressed!\n");
                            dropdown->extended = !dropdown->extended;
                            // dropdown->extended_panel->ui_element->active = !dropdown->extended_panel->ui_element->active;
                        }
                    }
                }
            
                else dropdown->pressed = false;
            }

            // render the outline rect
            if (dropdown->outline) 
                render_basic_outline_rect (renderer, &dropdown->ui_element->transform->rect, dropdown->outline_colour, 
                    dropdown->outline_scale_x, dropdown->outline_scale_y);

            // render the placeholder text (also the selected option text)
            ui_text_component_render (dropdown->placeholder, renderer);

            // render the extended section (options)
            if (dropdown->extended) {
                for (ListElement *le = dlist_start (dropdown->options); le; le = le->next) {
                    ui_dropdown_option_render ((DropdownOption *) le->data, renderer);
                }
            }

            renderer->render_count += 1;
        }
    }

}

#pragma endregion
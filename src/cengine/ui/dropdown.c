#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/types/vector2d.h"
#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"
#include "cengine/input.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/position.h"
#include "cengine/ui/dropdown.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"
#include "cengine/ui/components/layout.h"

void ui_dropdown_option_set_text (DropdownOption *option, const char *option_text,
    Font *font, u32 size, RGBA_Color color);

#pragma region Dropdown Option

static DropdownOption *ui_dropdown_option_new (void) {

    DropdownOption *option = (DropdownOption *) malloc (sizeof (DropdownOption));
    if (option) {
        memset (option, 0, sizeof (DropdownOption));
        option->transform = NULL;
        option->option = NULL;

        option->colour = false;
        option->bg_texture = NULL;

        option->outline = false;
        option->draw_selected = false;
    }

    return option;

}

void ui_dropdown_option_delete (void *dropdown_option_ptr) {

    if (dropdown_option_ptr) {
        DropdownOption *option = (DropdownOption *) dropdown_option_ptr;
        ui_transform_component_delete (option->transform);
        ui_text_component_delete (option->option);
        if (option->bg_texture) SDL_DestroyTexture (option->bg_texture);
        free (option);
    }

}

// dropdown_option_comparator by option's name
int ui_dropdown_option_comparator (const void *one, const void *two) {

    if (one && two) 
        return str_compare (((DropdownOption *) one)->option->text, ((DropdownOption *) two)->option->text);

    else if (one) return -1;
    else if (two) return 1;
    return 0;

}

// creates a new dropdown option to be added to a dropdown
// options to pass text modifiers
DropdownOption *ui_dropdown_option_create (const char *option_text,
    Font *font, u32 size, RGBA_Color color) {

    DropdownOption *option = ui_dropdown_option_new ();
    if (option) {
        option->transform = ui_transform_component_create (0, 0, 0, 0);
        ui_dropdown_option_set_text (option, option_text, font, size, color);
    }

    return option;

}

// sets the option's text for the dropdown option element
void ui_dropdown_option_set_text (DropdownOption *option, const char *option_text,
    Font *font, u32 size, RGBA_Color color) {

    if (option) {
        if (option->option) ui_text_component_delete (option->option);

        option->option = option_text ? ui_text_component_new () : NULL;
        if (option->option) {
             ui_text_component_init (option->option, font, size, color, option_text);

            // set the option's text position inside the dropdown option element
            option->option->transform->rect.x = option->transform->rect.x;
            option->option->transform->rect.y = option->transform->rect.y;

            ui_text_component_draw (option->option);
        }
    }

}

// sets the option's outline colour
void ui_dropdown_option_set_ouline_colour (DropdownOption *option, RGBA_Color colour) {

    if (option) {
        option->outline = true;
        option->outline_colour = colour;
    }

}

// removes the ouline form the option
void ui_dropdown_option_remove_outline (DropdownOption *option) {

    if (option) {
        memset (&option->outline_colour, 0, sizeof (RGBA_Color));
        option->outline = false;
    }

}

// sets the option's background color
void ui_dropdown_option_set_bg_color (DropdownOption *option, RGBA_Color color) {

    if (option) {
        option->bg_colour = color;
        if (color.a < 255) {
            option->bg_texture = render_complex_transparent_rect (&option->transform->rect, color);
            option->bg_texture_rect.w = option->transform->rect.w;
            option->bg_texture_rect.h = option->transform->rect.h;
        }

        option->colour = true;
    } 

}

// removes the background from the option
void ui_dropdown_option_remove_background (DropdownOption *option) {

    if (option) {
        if (option->bg_texture) {
            SDL_DestroyTexture (option->bg_texture);
            option->bg_texture = NULL;
        }

        memset (&option->bg_colour, 0, sizeof (RGBA_Color));
        option->colour = false;
    }

}

static void ui_dropdown_option_render (DropdownOption *option, 
    RGBA_Color *colour, SDL_Texture *texture) {

    if (option) {
        // render hover background
        if (texture) {
            SDL_RenderCopyEx (main_renderer->renderer, texture, 
                &option->bg_texture_rect, &option->transform->rect, 
                0, 0, SDL_FLIP_NONE);
        }

        else if (colour) render_basic_filled_rect (&option->transform->rect, *colour);

        // render the normal background
        else if (option->bg_texture) {
            SDL_RenderCopyEx (main_renderer->renderer, option->bg_texture, 
                &option->bg_texture_rect, &option->transform->rect, 
                0, 0, SDL_FLIP_NONE);
        }

        else if (option->colour) render_basic_filled_rect (&option->transform->rect, option->bg_colour);

        // render the outline rect
        if (option->outline) 
            render_basic_outline_rect (&option->transform->rect, option->outline_colour);

        // render the option's text
        ui_text_component_render (option->option);
    }

}

#pragma endregion

#pragma region Dropdown

static Dropdown *ui_dropdown_new (void) {

    Dropdown *dropdown = (Dropdown *) malloc (sizeof (Dropdown));
    if (dropdown) {
        memset (dropdown, 0, sizeof (dropdown));

        dropdown->ui_element = NULL;
        dropdown->transform = NULL;

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
        dropdown->layout = NULL;
    }

    return dropdown;

}

void ui_dropdown_delete (void *dropdown_ptr) {

    if (dropdown_ptr) {
        Dropdown *dropdown = (Dropdown *) dropdown_ptr;

        dropdown->ui_element = NULL;
        ui_transform_component_delete (dropdown->transform);
        if (dropdown->bg_texture) SDL_DestroyTexture (dropdown->bg_texture);
        ui_text_component_delete (dropdown->placeholder);

        dlist_delete (dropdown->options);
        ui_layout_group_delete (dropdown->layout);

        ui_element_destroy (dropdown->extended_panel->ui_element);

        if (dropdown->option_hover_texture) SDL_DestroyTexture (dropdown->option_hover_texture);

        free (dropdown);
    }

}

// creates a new dropdown menu
Dropdown *ui_dropdown_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos) {

    Dropdown *dropdown = NULL;

    UIElement *ui_element = ui_element_new (UI_DROPDOWN);
    if (ui_element) {
        dropdown = ui_dropdown_new ();
        if (dropdown) {
            dropdown->ui_element = ui_element;
            dropdown->transform = ui_transform_component_create (x, y, w, h);
            ui_transform_component_set_pos (dropdown->transform, NULL, pos, true);

            ui_element->element = dropdown;
        }

        else ui_element_delete (ui_element);
    }

    return dropdown;

}

// sets the main options for the dropdown (this is required to correctly create the dropdown)
// x, y: set the position of the dropdown's extended panel
// with, height: set the dimensions of the dropdown's extended panel
void ui_dropdown_set_options (Dropdown *dropdown, i32 x, i32 y,
    u32 options_width, u32 options_max_height, UIPosition pos) {

    if (dropdown) {
        dropdown->option_selected = NULL;
        dropdown->options = dlist_init (ui_dropdown_option_delete, ui_dropdown_option_comparator);

        // create the extened panel 
        dropdown->extended_panel = ui_panel_create (x, y, options_width, options_max_height, UI_POS_FREE);
        ui_transform_component_set_pos (dropdown->extended_panel->transform, &dropdown->transform->rect, pos, false);
        dropdown->extended_panel->transform->rect.y += options_max_height;
        dropdown->extended_panel->transform->rect.x += x;
        dropdown->extended_panel->transform->rect.y += y;
        dropdown->extended_panel->ui_element->active = false;

        // the layout group contains the transforms of the options that render on top of the extended panel
        dropdown->layout = ui_layout_group_create (x, y, 
            options_width, options_max_height, UI_POS_FREE);
        ui_transform_component_set_pos (dropdown->layout->transform, &dropdown->transform->rect, pos, false);
        dropdown->layout->transform->rect.y += options_max_height;
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

// removes the ouline form the dropdown
void ui_dropdown_remove_outline (Dropdown *dropdown) {

    if (dropdown) {
        memset (&dropdown->outline_colour, 0, sizeof (RGBA_Color));
        dropdown->outline = false;
    }

}

// sets the dropdown's background color
void ui_dropdown_set_bg_color (Dropdown *dropdown, RGBA_Color color) {

    if (dropdown) {
        dropdown->bg_colour = color;
        if (color.a < 255) {
            dropdown->bg_texture = render_complex_transparent_rect (&dropdown->transform->rect, color);
            dropdown->bg_texture_rect.w = dropdown->transform->rect.w;
            dropdown->bg_texture_rect.h = dropdown->transform->rect.h;
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
void ui_dropdown_set_placeholder (Dropdown *dropdown,
    const char *text, Font *font, u32 size, RGBA_Color colour) {

    if (dropdown) {
        if (dropdown->placeholder) ui_text_component_delete (dropdown->placeholder);

        dropdown->placeholder = text ? ui_text_component_new () : NULL;
        if (dropdown->placeholder) {
            ui_text_component_init (dropdown->placeholder,
                font, size, colour, text);

            // set the text position inside the dropdown field
            dropdown->placeholder->transform->rect.x = dropdown->transform->rect.x;
            dropdown->placeholder->transform->rect.y = dropdown->transform->rect.y;

            ui_text_component_draw (dropdown->placeholder);
        }
    }

}

// sets the dropdown's placeholder position
void ui_dropdown_set_placeholder_pos (Dropdown *dropdown, UIPosition pos) {

    if (dropdown) {
        if (dropdown->placeholder) 
            ui_transform_component_set_pos (dropdown->placeholder->transform, &dropdown->transform->rect, pos, true);
    }

}

// sets the dropdown's extened panel colour
void ui_dropdown_extened_set_bg_colour (Dropdown *dropdown, RGBA_Color colour) {

    if (dropdown) ui_panel_set_bg_colour (dropdown->extended_panel, colour);

}

// adds a new dropdown option to the dropdown
void ui_dropdown_option_add (Dropdown *dropdown, DropdownOption *option) {

    if (dropdown && option) {
        option->transform->pos = UI_POS_BOTTOM_CENTER;
        option->option->transform->pos = UI_POS_MIDDLE_CENTER;
        ui_position_update (option->transform, &dropdown->transform->rect, false);
        dlist_insert_after (dropdown->options, dlist_end (dropdown->options), option);

        ui_layout_group_add (dropdown->layout, option->transform);

        DropdownOption *op = NULL;
        for (ListElement *le = dlist_start (dropdown->options); le; le = le->next) {
            op = (DropdownOption *) le->data;
            ui_position_update (op->option->transform, &op->transform->rect, false);
        }
    }

}

// removes a dropdown option from the dropdown
void ui_dropdown_option_remove (Dropdown *dropdown, DropdownOption *option) {

    if (dropdown && option) {
        // FIXME:
    }

}

// gets the value of the selected option in the dropdown
String *ui_dropdown_option_selected (Dropdown *dropdown) {

    String *retval = NULL;

    if (dropdown) {
        if (dropdown->option_selected) {
            retval = dropdown->option_selected->option->text;
        }
    }

    return retval;

}

// sets the hovering colour for the dropdown options
// IMPORTANT: set the colour after you have added all the options to the dropdwon!
void ui_dropdown_option_set_hover_color (Dropdown *dropdown, RGBA_Color color) {

    if (dropdown) {
        dropdown->option_hover_colour = color;
        if (color.a < 255) {
            DropdownOption *option = ((DropdownOption *) dlist_start (dropdown->options)->data);
            dropdown->option_hover_texture = render_complex_transparent_rect (&option->transform->rect, color);
            dropdown->bg_texture_rect.w = option->transform->rect.w;
            dropdown->bg_texture_rect.h = option->transform->rect.h;
        }
    } 

}

// render the dropdown to the screen
void ui_dropdown_render (Dropdown *dropdown) {

    if (dropdown) {
        // render the background
        if (dropdown->bg_texture) {
            SDL_RenderCopyEx (main_renderer->renderer, dropdown->bg_texture, 
                &dropdown->bg_texture_rect, &dropdown->transform->rect, 
                0, 0, SDL_FLIP_NONE);
        }

        else if (dropdown->colour) 
            render_basic_filled_rect (&dropdown->transform->rect, dropdown->bg_colour);

        // check if the mouse is in the dropdown
        if (dropdown->active) {
            if (mousePos.x >= dropdown->transform->rect.x && mousePos.x <= (dropdown->transform->rect.x + dropdown->transform->rect.w) && 
                mousePos.y >= dropdown->transform->rect.y && mousePos.y <= (dropdown->transform->rect.y + dropdown->transform->rect.h)) {
                // the mouse is over use
                render_basic_filled_rect (&dropdown->transform->rect, RGBA_BLACK);

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
            render_basic_outline_rect (&dropdown->transform->rect, dropdown->outline_colour);

        // render the placeholder text (also the selected option text)
        ui_text_component_render (dropdown->placeholder);

        // render the extended section (options)
        if (dropdown->extended) {
            DropdownOption *option = NULL;
            for (ListElement *le = dlist_start (dropdown->options); le; le = le->next) {
                option = (DropdownOption *) le->data;

                if (mousePos.x >= option->transform->rect.x && mousePos.x <= (option->transform->rect.x + option->transform->rect.w) && 
                mousePos.y >= option->transform->rect.y && mousePos.y <= (option->transform->rect.y + option->transform->rect.h)) {
                    // FIXME: create a colour ptr to check for colour!
                    ui_dropdown_option_render (option, &dropdown->option_hover_colour, dropdown->option_hover_texture);

                    // check if the user pressed the left button over the mouse
                    if (input_get_mouse_button_state (MOUSE_LEFT)) {
                        option->pressed = true;
                    }
                    
                    else if (!input_get_mouse_button_state (MOUSE_LEFT)) {
                        if (option->pressed) {
                            option->pressed = false;
                            dropdown->option_selected = option;
                            ui_dropdown_set_placeholder (dropdown, 
                                option->option->text->str, 
                                dropdown->placeholder->font, dropdown->placeholder->size, dropdown->placeholder->text_color);
                            ui_dropdown_set_placeholder_pos (dropdown, UI_POS_MIDDLE_CENTER);
                        }
                    }
                }

                else {
                    ui_dropdown_option_render (option, NULL, NULL);
                } 
            }
        }
    }

}

#pragma endregion
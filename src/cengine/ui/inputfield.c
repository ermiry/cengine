#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/types/vector2d.h"

#include "cengine/renderer.h"
#include "cengine/input.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/position.h"
#include "cengine/ui/inputfield.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

void ui_input_field_update (InputField *input, Renderer *renderer);

static InputField *ui_input_field_new (void) {

    InputField *input = (InputField *) malloc (sizeof (InputField));
    if (input) {
        memset (input, 0, sizeof (InputField));
        input->ui_element = NULL;

        input->bg_texture = NULL;

        input->placeholder = NULL;
        input->empty_text = true;

        input->text = NULL;
        input->password = NULL;
        input->is_password = false;

        input->pressed = false;
        input->active = true;
    }

    return input;

}

void ui_input_field_delete (void *input_ptr) {

    if (input_ptr) {
        InputField *input = (InputField *) input_ptr;

        input->ui_element = NULL;
        if (input->bg_texture) SDL_DestroyTexture (input->bg_texture);
        ui_text_component_delete (input->placeholder);
        ui_text_component_delete (input->text);
        str_delete (input->password);

        free (input);
    }

}

// sets the input field to be active depending on values
void ui_input_field_set_active (InputField *input, bool active) {

    if (input) input->active = active;

}

// toggles the input field to be active or not
void ui_input_field_toggle_active (InputField *input) {

    if (input) input->active = !input->active;

}

// sets the inputs's UI position
void ui_input_field_set_pos (InputField *input, UIRect *ref_rect, UIPosition pos, Renderer *renderer) {

    if (input) ui_transform_component_set_pos (input->ui_element->transform, renderer, ref_rect, pos, false);

}

// sets the input placeholder text
void ui_input_field_placeholder_text_set (InputField *input, Renderer *renderer, const char *text,
    Font *font, u32 size, RGBA_Color text_color) {

    if (input) {
        if (input->placeholder) ui_text_component_delete (input->placeholder);

        input->placeholder = text ? ui_text_component_new () : NULL;
        if (input->placeholder) {
            ui_text_component_init (input->placeholder,
                font, size, text_color, text);

            // set the text position inside the input field
            input->placeholder->transform->rect.x = input->ui_element->transform->rect.x;
            input->placeholder->transform->rect.y = input->ui_element->transform->rect.y;

            ui_text_component_draw (input->placeholder, renderer);
        }
    }

}

// updates the input's placeholder text
void ui_input_field_placeholder_text_update (InputField *input, Renderer *renderer, const char *text) {

    if (input) {
        if (input->placeholder) {
            ui_text_component_set_text (input->placeholder, text);
            ui_text_component_draw (input->placeholder, renderer);
            ui_transform_component_set_pos (input->placeholder->transform, NULL, 
                &input->ui_element->transform->rect, input->placeholder->transform->pos, true);
        }
    }

}

// sets the input placeholder text position
void ui_input_field_placeholder_text_pos_set (InputField *input, UIPosition pos) {

    if (input) {
        if (input->placeholder) 
            ui_transform_component_set_pos (input->placeholder->transform, NULL, &input->ui_element->transform->rect, pos, true);
    }

}

// sets the input field's text
void ui_input_field_text_set (InputField *input, Renderer *renderer, const char *text,
    Font *font, u32 size, RGBA_Color text_color, bool is_password) {

    if (input) {
        if (input->text) ui_text_component_delete (input->text);

        input->text = ui_text_component_new ();
        if (input->text) {
            ui_text_component_init (input->text,
                font, size, text_color, text);

            // set the text position inside the input field
            input->text->transform->rect.x = input->ui_element->transform->rect.x;
            input->text->transform->rect.y = input->ui_element->transform->rect.y;

            ui_text_component_draw (input->text, renderer);
        }

        if (is_password) {
            input->is_password = true;
            input->password = text ? str_new (text) : str_new ("");
            ui_input_field_update (input, renderer);
        } 
    }

}

// updates the placeholder text (redraws the text component)
void ui_input_field_text_update (InputField *input, Renderer *renderer, const char *update_text) {

    if (input) {
        if (input->text) {
            ui_text_component_set_text (input->text, update_text);
            ui_text_component_draw (input->text, renderer);
            ui_transform_component_set_pos (input->text->transform, NULL,
                &input->ui_element->transform->rect, input->text->transform->pos, true);
        }
    }

}

// sets the input field's text position
void ui_input_field_text_pos_set (InputField *input, UIPosition pos) {

    if (input) {
        if (input->text) 
            ui_transform_component_set_pos (input->text->transform, NULL, &input->ui_element->transform->rect, pos, true);
    }

}

// sets the input field's text color
void ui_input_field_text_color_set (InputField *input, Renderer *renderer, RGBA_Color color) {

    if (input) {
        input->text->text_color = color;
        ui_text_component_draw (input->text, renderer);
    }

}

// returns the current input text
String *ui_input_field_input_get (InputField *input) {

    if (input) return input->text->text;

}

// returns the actual password value
String *ui_input_field_password_get (InputField *input) {

    if (input) return input->password;

}

// sets the input field's outline colour
void ui_input_field_ouline_set_colour (InputField *input, RGBA_Color colour) {

    if (input) {
        input->outline = true;
        input->outline_colour = colour;
    }

}

// sets the input field's outline scale
void ui_input_field_ouline_set_scale (InputField *input, float x_scale, float y_scale) {

    if (input) {
        input->outline_scale_x = x_scale;
        input->outline_scale_y = y_scale;
    }

}

// removes the ouline form the input field
void ui_input_field_outline_remove (InputField *input) {

    if (input) {
        memset (&input->outline_colour, 0, sizeof (RGBA_Color));
        input->outline = false;
    }

}

// sets the input field's background color
void ui_input_field_bg_color_set (InputField *input, Renderer *renderer, RGBA_Color color) {

    if (input) {
        input->bg_colour = color;
        if (color.a < 255) {
            render_complex_transparent_rect (renderer, &input->bg_texture,
                &input->ui_element->transform->rect, color);
            input->bg_texture_rect.w = input->ui_element->transform->rect.w;
            input->bg_texture_rect.h = input->ui_element->transform->rect.h;
        }

        input->colour = true;
    } 

}

// removes the background from the input field
void ui_input_field_bg_remove (InputField *input) {

    if (input) {
        if (input->bg_texture) {
            SDL_DestroyTexture (input->bg_texture);
            input->bg_texture = NULL;
        }

        memset (&input->bg_colour, 0, sizeof (RGBA_Color));
        input->colour = false;
    }

}

// sets the option to draw an outline rect when the input is selected
void ui_input_field_selected_set (InputField *input, RGBA_Color selected_color) {

    if (input) {
        input->draw_selected = true;
        input->selected_color = selected_color;
    }

}

// sets an action to be triggered every input
// works for every keystroke, paste, delete
void ui_input_field_set_on_key_input (InputField *input, Action on_key_input, void *on_key_input_args) {

    if (input) {
        input->on_key_input = on_key_input;
        input->on_key_input_args = on_key_input_args;
    }

}

// creates a new input field
InputField *ui_input_field_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer) {

    InputField *input = NULL;

    UIElement *ui_element = ui_element_create (renderer->ui, UI_INPUT);
    if (ui_element) {
        input = ui_input_field_new ();
        if (input) {
            input->ui_element = ui_element;
            ui_transform_component_set_values (ui_element->transform, x, y, w, h);
            ui_transform_component_set_pos (input->ui_element->transform, renderer, NULL, pos, true);
            ui_element->element = input;

            input->outline_scale_x = 1;
            input->outline_scale_y = 1;
        }

        else ui_element_delete (ui_element);
    }

    return input;

}

// draws the input field
void ui_input_field_draw (InputField *input, Renderer *renderer) {

    if (input && renderer) {
        if (SDL_HasIntersection (&input->ui_element->transform->rect, &renderer->window->screen_rect)) {
            // render the background
            if (input->bg_texture) {
                SDL_RenderCopyEx (renderer->renderer, input->bg_texture, 
                    &input->bg_texture_rect, &input->ui_element->transform->rect, 
                    0, 0, SDL_FLIP_NONE);
            }

            else if (input->colour) 
                render_basic_filled_rect (renderer, &input->ui_element->transform->rect, input->bg_colour);

            // check if the mouse is in the input
            if (input->active) {
                if (mousePos.x >= input->ui_element->transform->rect.x && mousePos.x <= (input->ui_element->transform->rect.x + input->ui_element->transform->rect.w) && 
                    mousePos.y >= input->ui_element->transform->rect.y && mousePos.y <= (input->ui_element->transform->rect.y + input->ui_element->transform->rect.h)) {
                    // check if the user pressed the left input over the mouse
                    if (input_get_mouse_button_state (MOUSE_LEFT)) {
                        input->pressed = true;
                    }
                    
                    else if (!input_get_mouse_button_state (MOUSE_LEFT)) {
                        if (input->pressed) {
                            input->pressed = false;
                            // make this text active
                            input_set_active_text (renderer->window->input, input);
                            // printf ("Pressed!\n");
                        }
                    }
                }
            
                else input->pressed = false;
            }

            if (input->pressed) {
                if (input->draw_selected) 
                    render_basic_outline_rect (renderer, &input->ui_element->transform->rect, input->selected_color,
                        input->outline_scale_x, input->outline_scale_y);
            }

            else if (input->outline) render_basic_outline_rect (renderer, &input->ui_element->transform->rect, input->outline_colour, 
                input->outline_scale_x, input->outline_scale_y);
            
            // draw the correct text
            if (input->empty_text) {
                // draw the placeholder text
                ui_text_component_render (input->placeholder, renderer);
            }

            else {
                // draw the input text
                ui_text_component_render (input->text, renderer);
            }

            renderer->render_count += 1;
        }
    }

}

// updates the input field with the correct values
void ui_input_field_update (InputField *input, Renderer *renderer) {

    if (input) {
        // check if the input text is empty
        if (input->is_password) {
            if (strcmp (input->password->str, "")) {
                input->empty_text = false;

                // update the input's text
                unsigned int pass_len = strlen (input->password->str);
                char *secret = (char *) calloc (pass_len, sizeof (char));
                memset (secret, '*', pass_len);
                str_delete (input->text->text);
                input->text->text = str_new (secret);
                free (secret);
            }
            
            else input->empty_text = true;
        }

        else {
            if (strcmp (input->text->text->str, ""))
                input->empty_text = false;
            else input->empty_text = true;
        }

        ui_text_component_draw (input->text, renderer);
        ui_transform_component_set_pos (input->text->transform, renderer,
            &input->ui_element->transform->rect, input->text->transform->pos, false);
    }

}
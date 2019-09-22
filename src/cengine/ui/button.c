#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/types/vector2d.h"

#include "cengine/renderer.h"
#include "cengine/textures.h"
#include "cengine/sprites.h"
#include "cengine/input.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/button.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

static Button *ui_button_new (void) {

    Button *button = (Button *) malloc (sizeof (Button));
    if (button) {
        memset (button, 0, sizeof (Button));

        button->ui_element = NULL;
        button->transform = NULL;

        button->active = true;

        // background colour
        button->colour = false;
        button->bg_texture = NULL;

        button->text = NULL;

        button->sprites = (Sprite **) calloc (BUTTON_STATE_TOTAL, sizeof (Sprite *));
        for (unsigned int  i = 0; i < BUTTON_STATE_TOTAL; i++)
            button->sprites[i] = NULL;

        button->ref_sprites = (bool *) calloc (BUTTON_STATE_TOTAL, sizeof (bool));
        for (unsigned int  i = 0; i < BUTTON_STATE_TOTAL; i++)
            button->ref_sprites[i] = false;

        button->action = NULL;
        button->args = NULL;
    }

    return button;

}

void ui_button_delete (void *button_ptr) {

    if (button_ptr) {
        Button *button = (Button *) button_ptr;

        button->ui_element = NULL;
        ui_transform_component_delete (button->transform);
        ui_text_component_delete (button->text);

        if (button->bg_texture) SDL_DestroyTexture (button->bg_texture);

        if (button->sprites && button->ref_sprites) {
            for (unsigned int  i = 0; i < BUTTON_STATE_TOTAL; i++) {
                if (!button->ref_sprites[i]) sprite_destroy (button->sprites[i]);
                else button->sprites[i] = NULL;
            }

            free (button->sprites);
            free (button->ref_sprites);
        }

        free (button);
    }

}

// sets the button to be active depending on values
void ui_button_set_active (Button *button, bool active) {

    if (button) button->active = active;

}

// toggles the button to be active or not
void ui_button_toggle_active (Button *button) {

    if (button) button->active = !button->active;

}

// sets the button text
void ui_button_set_text (Button *button, const char *text, 
    Font *font, u32 size, RGBA_Color text_color) {

    if (button) {
        if (button->text) ui_text_component_delete (button->text);

        button->text = ui_text_component_new ();
        if (button->text) {
            ui_text_component_init (button->text,
                font, size, text_color, text);

            // set the text position inside the button
            button->text->transform->rect.x = button->transform->rect.x;
            button->text->transform->rect.y = button->transform->rect.y;

            ui_text_component_draw (button->text);
        }
    }

}

// sets the button's text position
void ui_button_set_text_pos (Button *button, UIPosition pos) {

    if (button) {
        if (button->text)
            ui_transform_component_set_pos (button->text->transform, &button->transform->rect, pos, true);
    }

}

// sets the button's text color
void ui_button_set_text_color (Button *button, RGBA_Color color) {

    if (button) {
        button->text->text_color = color;
        ui_text_component_draw (button->text);
    }

}

// sets the button's outline colour
void ui_button_set_ouline_colour (Button *button, RGBA_Color colour) {

    if (button) {
        button->outline = true;
        button->outline_colour = colour;
    }

}

// removes the ouline form the button
void ui_button_remove_outline (Button *button) {

    if (button) {
        memset (&button->outline_colour, 0, sizeof (RGBA_Color));
        button->outline = false;
    }

}

// sets the background color of the button
void ui_button_set_bg_color (Button *button, RGBA_Color color) {

    if (button) {
        button->bg_colour = color;
        if (color.a < 255) {
            button->bg_texture = render_complex_transparent_rect (&button->transform->rect, color);
            button->bg_texture_rect.w = button->transform->rect.w;
            button->bg_texture_rect.h = button->transform->rect.h;
        }

        button->colour = true;
    } 

}

// removes the background from the button
void ui_button_remove_background (Button *button) {

    if (button) {
        if (button->bg_texture) {
            SDL_DestroyTexture (button->bg_texture);
            button->bg_texture = NULL;
        }

        memset (&button->bg_colour, 0, sizeof (RGBA_Color));
        button->colour = false;
    }

}

// sets an sprite for each button state
// the sprite is loaded and deleted when the button gets deleted
void ui_button_set_sprite (Button *button, ButtonState state, const char *filename) {

    if (button && filename) {
        Sprite *sprite = sprite_load (filename, main_renderer);

        if (sprite) {
            switch (state) {
                case BUTTON_STATE_MOUSE_OUT: 
                    button->sprites[BUTTON_STATE_MOUSE_OUT] = sprite;
                    button->ref_sprites[BUTTON_STATE_MOUSE_OUT] = false;
                    break;
                case BUTTON_STATE_MOUSE_OVER_MOTION: 
                    button->sprites[BUTTON_STATE_MOUSE_OVER_MOTION] = sprite;
                    button->ref_sprites[BUTTON_STATE_MOUSE_OVER_MOTION] = false;
                    break;
                case BUTTON_STATE_MOUSE_DOWN: 
                    button->sprites[BUTTON_STATE_MOUSE_DOWN] = sprite;
                    button->ref_sprites[BUTTON_STATE_MOUSE_DOWN] = false;
                    break;
                case BUTTON_STATE_MOUSE_UP: 
                    button->sprites[BUTTON_STATE_MOUSE_UP] = sprite;
                    button->ref_sprites[BUTTON_STATE_MOUSE_UP] = false;
                    break;

                default: break;
            }
        }
    }

}

// uses a refrence to the sprite and does not load or destroy it 
void ui_button_ref_sprite (Button *button, ButtonState state, Sprite *sprite) {

    if (button && sprite) {
        switch (state) {
            case BUTTON_STATE_MOUSE_OUT: 
                button->sprites[BUTTON_STATE_MOUSE_OUT] = sprite;
                button->ref_sprites[BUTTON_STATE_MOUSE_OUT] = true;
                break;
            case BUTTON_STATE_MOUSE_OVER_MOTION: 
                button->sprites[BUTTON_STATE_MOUSE_OVER_MOTION] = sprite;
                button->ref_sprites[BUTTON_STATE_MOUSE_OVER_MOTION] = true;
                break;
            case BUTTON_STATE_MOUSE_DOWN: 
                button->sprites[BUTTON_STATE_MOUSE_DOWN] = sprite;
                button->ref_sprites[BUTTON_STATE_MOUSE_DOWN] = true;
                break;
            case BUTTON_STATE_MOUSE_UP: 
                button->sprites[BUTTON_STATE_MOUSE_UP] = sprite;
                button->ref_sprites[BUTTON_STATE_MOUSE_UP] = true;
                break;

            default: break;
        }
    }

}

// sets an action to be triggered when the button is clicked
void ui_button_set_action (Button *button, Action action, void *args) {

    if (button) {
        button->action = action;
        button->args = args;
    } 

}

// creates a new button
Button *ui_button_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos) {

    Button *button = NULL;

    UIElement *ui_element = ui_element_new (UI_BUTTON);
    if (ui_element) {
        button = ui_button_new ();
        if (button) {
            button->ui_element = ui_element;
            button->transform = ui_transform_component_create (x, y, w, h);
            ui_transform_component_set_pos (button->transform, NULL, pos, true);

            ui_element->element = button;
        }

        else ui_element_delete (ui_element);
    }

    return button;

}

// draws a button
void ui_button_draw (Button *button) {

    if (button) {
        // draw the background
         if (button->bg_texture) {
            SDL_RenderCopyEx (main_renderer->renderer, button->bg_texture, 
                &button->bg_texture_rect, &button->transform->rect, 
                0, 0, SDL_FLIP_NONE);
        }

        else if (button->colour) 
            render_basic_filled_rect (&button->transform->rect, button->bg_colour);

        // render the outline border
        if (button->outline) 
            render_basic_outline_rect (&button->transform->rect, button->outline_colour);

        Sprite *selected_sprite = NULL;

        if (button->active) {
            // check if the mouse is in the button
            if (mousePos.x >= button->transform->rect.x && mousePos.x <= (button->transform->rect.x + button->transform->rect.w) && 
                mousePos.y >= button->transform->rect.y && mousePos.y <= (button->transform->rect.y + button->transform->rect.h)) {
                // check if the user pressed the left button over the mouse
                if (input_get_mouse_button_state (MOUSE_LEFT)) {
                    button->pressed = true;
                    selected_sprite = button->sprites[BUTTON_STATE_MOUSE_DOWN];
                }
                
                else if (!input_get_mouse_button_state (MOUSE_LEFT)) {
                    if (button->pressed) {
                        button->pressed = false;
                        selected_sprite = button->sprites[BUTTON_STATE_MOUSE_UP];
                        if (button->action) button->action (button->args);
                        // printf ("Pressed!\n");
                    }
                }

                // if not, the user is hovering the mouse over the button
                else selected_sprite = button->sprites[BUTTON_STATE_MOUSE_OVER_MOTION];
            }
        
            else button->pressed = false;
        }

        else selected_sprite = button->sprites[BUTTON_STATE_DISABLE];

        // the mouse is NOT hovering over the button OR we dont have any other sprite available
        if (!selected_sprite)
            selected_sprite = button->sprites[BUTTON_STATE_MOUSE_OUT];

        if (selected_sprite) {
            selected_sprite->dest_rect.x = button->transform->rect.x;
            selected_sprite->dest_rect.y = button->transform->rect.y;

            SDL_RenderCopyEx (main_renderer->renderer, selected_sprite->texture, 
                &selected_sprite->src_rect, 
                &selected_sprite->dest_rect, 
                0, 0, NO_FLIP);
        } 

        // draw button text
        ui_text_component_render (button->text);
    }

}
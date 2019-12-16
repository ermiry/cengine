#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/types/vector2d.h"

#include "cengine/video.h"
#include "cengine/graphics.h"
#include "cengine/renderer.h"
#include "cengine/textures.h"
#include "cengine/sprites.h"
#include "cengine/input.h"
#include "cengine/timer.h"

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

        button->double_click_timer = NULL;
        button->double_click_action = NULL;
        button->double_click_args = NULL;
        button->double_click_delay = BUTTON_DEFAULT_DOUBLE_CLICK_DELAY;
    }

    return button;

}

void ui_button_delete (void *button_ptr) {

    if (button_ptr) {
        Button *button = (Button *) button_ptr;

        button->ui_element = NULL;
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

// sets the buttons's UI position
void ui_button_set_pos (Button *button, UIRect *ref_rect, UIPosition pos, Renderer *renderer) {

    if (button) ui_transform_component_set_pos (button->ui_element->transform, renderer, ref_rect, pos, false);

}

// sets the button's render dimensions
void ui_button_set_dimensions (Button *button, unsigned int width, unsigned int height) {

    if (button) {
        button->ui_element->transform->rect.w = width;
        button->ui_element->transform->rect.h = height;
    }

}

// sets the button's scale factor
void ui_button_set_scale (Button *button, int x_scale, int y_scale) {

    if (button) {
        button->ui_element->transform->x_scale = x_scale;
        button->ui_element->transform->y_scale = y_scale;

        button->ui_element->transform->rect.w *= button->ui_element->transform->x_scale;
        button->ui_element->transform->rect.h *= button->ui_element->transform->y_scale;
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
void ui_button_set_text (Button *button, Renderer *renderer, const char *text, 
    Font *font, u32 size, RGBA_Color text_color) {

    if (button) {
        if (button->text) ui_text_component_delete (button->text);

        button->text = ui_text_component_new ();
        if (button->text) {
            ui_text_component_init (button->text,
                font, size, text_color, text);

            // set the text position inside the button
            button->text->transform->rect.x = button->ui_element->transform->rect.x;
            button->text->transform->rect.y = button->ui_element->transform->rect.y;

            ui_text_component_draw (button->text, renderer);
        }
    }

}

// sets the button's text position
void ui_button_set_text_pos (Button *button, UIPosition pos) {

    if (button) {
        if (button->text)
            ui_transform_component_set_pos (button->text->transform, NULL, &button->ui_element->transform->rect, pos, true);
    }

}

// sets the button's text color
void ui_button_set_text_color (Button *button, Renderer *renderer, RGBA_Color color) {

    if (button) {
        button->text->text_color = color;
        ui_text_component_draw (button->text, renderer);
    }

}

// sets the button's outline colour
void ui_button_set_ouline_colour (Button *button, RGBA_Color colour) {

    if (button) {
        button->outline = true;
        button->outline_colour = colour;
    }

}

// sets the button's outline scale
void ui_button_set_ouline_scale (Button *button, float x_scale, float y_scale) {

    if (button) {
        button->outline_scale_x = x_scale;
        button->outline_scale_y = y_scale;
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
void ui_button_set_bg_color (Button *button, Renderer *renderer, RGBA_Color color) {

    if (button) {
        button->bg_colour = color;
        if (color.a < 255) {
            render_complex_transparent_rect (renderer, &button->bg_texture, &button->ui_element->transform->rect, color);
            button->bg_texture_rect.w = button->ui_element->transform->rect.w;
            button->bg_texture_rect.h = button->ui_element->transform->rect.h;
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
void ui_button_set_sprite (Button *button, Renderer *renderer, ButtonState state, const char *filename) {

    if (button && filename) {
        Sprite *sprite = sprite_load (filename, renderer);

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

// sets an action to be executed if double click is dected
void ui_button_set_double_click_action (Button *button, Action action, void *args) {

    if (button) {
        button->double_click_action = action;
        button->double_click_args = args;
    }

}

// sets the max delay between two clicks to count as a double click (in mili secs), the default value is 500
void ui_button_set_double_click_delay (Button *button, u32 double_click_delay) {

    if (button) button->double_click_delay = double_click_delay;

}

// creates a new button
Button *ui_button_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer) {

    Button *button = NULL;

    UIElement *ui_element = ui_element_create (renderer->ui, UI_BUTTON);
    if (ui_element) {
        button = ui_button_new ();
        if (button) {
            button->ui_element = ui_element;
            ui_transform_component_set_values (ui_element->transform, x, y, w, h);
            ui_transform_component_set_pos (button->ui_element->transform, renderer, NULL, pos, true);

            ui_element->element = button;

            button->outline_scale_x = 1;
            button->outline_scale_y = 1;

            button->original_w = w;
            button->original_h = h;

            button->double_click_timer = timer_new ();
        }

        else ui_element_delete (ui_element);
    }

    return button;

}

// rezises the button based on window size
void ui_button_resize (Button *button, WindowSize window_original_size, WindowSize window_new_size) {

    if (button) {
        if ((window_original_size.width == window_new_size.width) && window_original_size.height == window_new_size.height) {
            button->ui_element->transform->rect.w = button->original_w;
            button->ui_element->transform->rect.h = button->original_h;
        }

        else {
            u32 new_width = (window_new_size.width * button->ui_element->transform->rect.w) / window_original_size.width;
            u32 new_height = (window_new_size.height * button->ui_element->transform->rect.h) / window_original_size.height;
            button->ui_element->transform->rect.w = new_width;
            button->ui_element->transform->rect.h = new_height;
        }
    }

}

// draws a button
void ui_button_draw (Button *button, Renderer *renderer) {

    if (button && renderer) {
        if (SDL_HasIntersection (&button->ui_element->transform->rect, &renderer->window->screen_rect)) {
            // draw the background
            if (button->bg_texture) {
                SDL_RenderCopyEx (renderer->renderer, button->bg_texture, 
                    &button->bg_texture_rect, &button->ui_element->transform->rect, 
                    0, 0, SDL_FLIP_NONE);
            }

            else if (button->colour) 
                render_basic_filled_rect (renderer, &button->ui_element->transform->rect, button->bg_colour);

            Sprite *selected_sprite = NULL;

            if (button->active) {
                if (renderer->window->mouse) {
                    // check if the mouse is in the button
                    if (mousePos.x >= button->ui_element->transform->rect.x && mousePos.x <= (button->ui_element->transform->rect.x + button->ui_element->transform->rect.w) && 
                        mousePos.y >= button->ui_element->transform->rect.y && mousePos.y <= (button->ui_element->transform->rect.y + button->ui_element->transform->rect.h)) {
                        renderer->ui->ui_element_hover = button->ui_element;
                            
                        // check if the user pressed the left button over the mouse
                        if (input_get_mouse_button_state (MOUSE_LEFT)) {
                            button->pressed = true;
                            selected_sprite = button->sprites[BUTTON_STATE_MOUSE_DOWN];
                        }
                        
                        else if (!input_get_mouse_button_state (MOUSE_LEFT)) {
                            if (button->pressed) {
                                if (!button->one_click) {
                                    button->one_click = true;
                                    timer_start (button->double_click_timer);
                                    // button->selected = !button->selected;
                                    if (button->action) button->action (button->args);
                                    // printf ("One click!\n");
                                }

                                else {
                                    u32 ticks = timer_get_ticks (button->double_click_timer);
                                    if (ticks <= button->double_click_delay) {
                                        button->one_click = false;
                                        if (button->double_click_action) 
                                            button->double_click_action (button->double_click_args);

                                        // image->selected = !image->selected;
                                        // printf ("Double click!\n");
                                    }

                                    else {
                                        button->one_click = true;
                                        timer_start (button->double_click_timer);
                                        // button->selected = !button->selected;
                                        if (button->action) button->action (button->args);
                                        // printf ("One click again!\n");
                                    }
                                }
                                
                                button->pressed = false;
                            }
                        }

                        // if not, the user is hovering the mouse over the button
                        else selected_sprite = button->sprites[BUTTON_STATE_MOUSE_OVER_MOTION];
                    }

                    else button->pressed = false;
                }
            
                else button->pressed = false;
            }

            else selected_sprite = button->sprites[BUTTON_STATE_DISABLE];

            // the mouse is NOT hovering over the button OR we dont have any other sprite available
            if (!selected_sprite)
                selected_sprite = button->sprites[BUTTON_STATE_MOUSE_OUT];

            if (selected_sprite) {
                selected_sprite->dest_rect.x = button->ui_element->transform->rect.x;
                selected_sprite->dest_rect.y = button->ui_element->transform->rect.y;

                SDL_RenderCopyEx (renderer->renderer, selected_sprite->texture, 
                    &selected_sprite->src_rect, 
                    &button->ui_element->transform->rect, 
                    0, 0, NO_FLIP);
            } 

            // draw button text
            ui_text_component_render (button->text, renderer);

            // render the outline border
            if (button->outline) 
                render_basic_outline_rect (renderer, &button->ui_element->transform->rect, button->outline_colour,
                    button->outline_scale_x, button->outline_scale_y);

            renderer->render_count += 1;
        }
    }

}
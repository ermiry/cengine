#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/renderer.h"
#include "cengine/textures.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/textbox.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

void ui_textbox_set_text_pos (TextBox *textbox, UIPosition pos);
void ui_textbox_update_text_pos (TextBox *textbox);

static TextBox *ui_textbox_new (void) {

    TextBox *textbox = (TextBox *) malloc (sizeof (TextBox));
    if (textbox) {
        memset (textbox, 0, sizeof (TextBox));
        textbox->ui_element = NULL;
        textbox->colour = false;
        textbox->bg_texture = NULL;
        textbox->text = NULL;
    }

    return textbox;

}

void ui_textbox_delete (void *textbox_ptr) {

    if (textbox_ptr) {
        TextBox *textbox = (TextBox *) textbox_ptr;

        textbox->ui_element = NULL;
        if (textbox->bg_texture) SDL_DestroyTexture (textbox->bg_texture);
        ui_text_component_delete (textbox->text);

        free (textbox);
    }

}

// sets the textbox's UI position
void ui_textbox_set_pos (TextBox *textbox, UIRect *ref_rect, UIPosition pos, Renderer *renderer) {

    if (textbox) {
        ui_transform_component_set_pos (textbox->ui_element->transform, 
            renderer, 
            ref_rect, 
            pos, 
            false);
        ui_textbox_update_text_pos (textbox);
    } 

}

// sets the textbox's position offset
void ui_textbox_set_pos_offset (TextBox *textbox, int x_offset, int y_offset) {

    if (textbox) {
        textbox->ui_element->transform->x_offset = x_offset;
        textbox->ui_element->transform->y_offset = y_offset;
    }

}

// returns the string representing the text in the textbox
String *ui_textbox_get_text (TextBox *textbox) {

    if (textbox) return textbox->text->text;

}

// sets the textbox's text with options
void ui_textbox_set_text (TextBox *textbox, Renderer *renderer, const char *text, 
    Font *font, u32 size, RGBA_Color color, bool adjust_to_text) {

    if (textbox) {
        if (textbox->text) ui_text_component_delete (textbox->text);

        textbox->text = text ? ui_text_component_new () : NULL;
        if (textbox->text) {
            ui_text_component_init (textbox->text,
                font, size, color, text);

            // set the text position inside the textbox
            textbox->text->transform->rect.x = textbox->ui_element->transform->rect.x;
            textbox->text->transform->rect.y = textbox->ui_element->transform->rect.y;

            ui_text_component_draw (textbox->text, renderer);

            if (adjust_to_text) {
                textbox->ui_element->transform->rect.w = textbox->text->transform->rect.w;
                textbox->ui_element->transform->rect.h = textbox->text->transform->rect.h;
                ui_position_update (renderer, textbox->ui_element->transform, NULL, true);
            }
        }
    }

}

// updates the textbox's text
void ui_textbox_update_text (TextBox *textbox, Renderer *renderer, const char *text) {

    if (textbox) {
        if (textbox->text) {
            ui_text_component_update (textbox->text, text);
            ui_text_component_draw (textbox->text, renderer);
            // ui_transform_component_set_pos (textbox->text->transform, 
            //     &textbox->transform->rect, textbox->text->transform->pos, true);
        }
    }

}

// sets the textbox's text position
void ui_textbox_set_text_pos (TextBox *textbox, UIPosition pos) {

    if (textbox) {
        if (textbox->text) {
            ui_transform_component_set_pos (textbox->text->transform, 
                NULL, 
                &textbox->ui_element->transform->rect, 
                pos, 
                true);
        }
    }

}

// sets the textbox's text offset
void ui_textbox_set_text_pos_offset (TextBox *textbox, int x_offset, int y_offset) {

    if (textbox) {
        if (textbox->text) {
            textbox->text->transform->x_offset = x_offset;
            textbox->text->transform->y_offset = y_offset;
        }
    }

}

// 04/02/2020 -- 12:03 -- used when the parent component's position has been updated
// NOTE: to work properly, a text pos needs to be set before calling this method
void ui_textbox_update_text_pos (TextBox *textbox) {

    if (textbox) {
        if (textbox->text) {
            // reset pos to 0
            textbox->text->transform->rect.x = 0;
            textbox->text->transform->rect.y = 0;

            // use the previous set position
            ui_position_update (NULL, 
                textbox->text->transform, 
                &textbox->ui_element->transform->rect, 
                false);
        }
    }

}

// sets the textbox font
void ui_textbox_set_font (TextBox *textbox, Renderer *renderer, Font *font) {

    if (textbox) {
        textbox->text->font = font;
        ui_text_component_draw (textbox->text, renderer);
    }

}

// sets the textbox's text color
void ui_textbox_set_text_color (TextBox *textbox, Renderer *renderer, RGBA_Color color) {

    if (textbox) {
        if (textbox->text) {
            textbox->text->text_color = color;
            ui_text_component_draw (textbox->text, renderer);
        }
    }

}

// sets the textbox's outline colour
void ui_textbox_set_ouline_colour (TextBox *textbox, RGBA_Color colour) {

    if (textbox) {
        textbox->outline = true;
        textbox->outline_colour = colour;
    }

}

// sets the textbox's outline scale
void ui_textbox_set_ouline_scale (TextBox *textbox, float x_scale, float y_scale) {

    if (textbox) {
        textbox->outline_scale_x = x_scale;
        textbox->outline_scale_y = y_scale;
    }

}

// removes the ouline form the textbox
void ui_textbox_remove_outline (TextBox *textbox) {

    if (textbox) {
        memset (&textbox->outline_colour, 0, sizeof (RGBA_Color));
        textbox->outline = false;
    }

}

// sets the textbox's background color
void ui_textbox_set_bg_color (TextBox *textbox, Renderer *renderer, RGBA_Color color) {

    if (textbox) {
        textbox->bg_colour = color;
        if (color.a < 255) {
            render_complex_transparent_rect (renderer, &textbox->bg_texture, &textbox->ui_element->transform->rect, color);
            textbox->bg_texture_rect.w = textbox->ui_element->transform->rect.w;
            textbox->bg_texture_rect.h = textbox->ui_element->transform->rect.h;
        }

        textbox->colour = true;
    } 

}

// removes the background from the textbox
void ui_textbox_remove_background (TextBox *textbox, Renderer *renderer) {

    if (textbox) {
        if (textbox->bg_texture) {
            // SDL_DestroyTexture (textbox->bg_texture);
            texture_destroy (renderer, textbox->bg_texture);
            textbox->bg_texture = NULL;
        }

        memset (&textbox->bg_colour, 0, sizeof (RGBA_Color));
        textbox->colour = false;
    }

}

// creates a new textbox
TextBox *ui_textbox_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer) {

    TextBox *textbox = NULL;

    UIElement *ui_element = ui_element_create (renderer->ui, UI_TEXTBOX);
    if (ui_element) {
        textbox = ui_textbox_new ();
        if (textbox) {
            textbox->ui_element = ui_element;
            ui_transform_component_set_values (ui_element->transform, x, y, w, h);
            ui_transform_component_set_pos (textbox->ui_element->transform, renderer, NULL, pos, true);
            ui_element->element = textbox;

            textbox->outline_scale_x = 1;
            textbox->outline_scale_y = 1;

            textbox->original_w = w;
            textbox->original_h = h;
        }

        else ui_element_destroy (ui_element);
    }

    return textbox;

}

// rezises the textbox based on window size
void ui_textbox_resize (TextBox *textbox, WindowSize window_original_size, WindowSize window_new_size) {

    if (textbox) {
        if ((window_original_size.width == window_new_size.width) && window_original_size.height == window_new_size.height) {
            textbox->ui_element->transform->rect.w = textbox->original_w;
            textbox->ui_element->transform->rect.h = textbox->original_h;
        }

        else {
            u32 new_width = (window_new_size.width * textbox->ui_element->transform->rect.w) / window_original_size.width;
            u32 new_height = (window_new_size.height * textbox->ui_element->transform->rect.h) / window_original_size.height;
            textbox->ui_element->transform->rect.w = new_width;
            textbox->ui_element->transform->rect.h = new_height;
        }
    }

}

// draws the textbox
void ui_textbox_draw (TextBox *textbox, Renderer *renderer) {

    if (textbox && renderer) {
        if (SDL_HasIntersection (&textbox->ui_element->transform->rect, &renderer->window->screen_rect)) {
            // render the background
            if (textbox->bg_texture) {
                SDL_RenderCopyEx (renderer->renderer, textbox->bg_texture, 
                    &textbox->bg_texture_rect, &textbox->ui_element->transform->rect, 
                    0, 0, SDL_FLIP_NONE);
            }

            else if (textbox->colour) 
                render_basic_filled_rect (renderer, &textbox->ui_element->transform->rect, textbox->bg_colour);

            // render the outline border
            if (textbox->outline) 
                render_basic_outline_rect (renderer, &textbox->ui_element->transform->rect, textbox->outline_colour,
                    textbox->outline_scale_x, textbox->outline_scale_y);

            // render the text
            ui_text_component_render (textbox->text, renderer);

            renderer->render_count += 1;
        }
    }

}
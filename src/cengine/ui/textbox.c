#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/renderer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/textbox.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

static TextBox *ui_textbox_new (void) {

    TextBox *textbox = (TextBox *) malloc (sizeof (TextBox));
    if (textbox) {
        memset (textbox, 0, sizeof (TextBox));
        textbox->ui_element = NULL;
        textbox->transform = NULL;
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
        ui_transform_component_delete (textbox->transform);
        if (textbox->bg_texture) SDL_DestroyTexture (textbox->bg_texture);
        ui_text_component_delete (textbox->text);

        free (textbox);
    }

}

// returns the string representing the text in the textbox
String *ui_textbox_get_text (TextBox *textbox) {

    if (textbox) return textbox->text->text;

}

// sets the textbox's text with options
void ui_textbox_set_text (TextBox *textbox, const char *text, 
    Font *font, u32 size, RGBA_Color color, bool adjust_to_text) {

    if (textbox) {
        if (textbox->text) ui_text_component_delete (textbox->text);

        textbox->text = text ? ui_text_component_new () : NULL;
        if (textbox->text) {
            ui_text_component_init (textbox->text,
                font, size, color, text);

            // set the text position inside the textbox
            textbox->text->transform->rect.x = textbox->transform->rect.x;
            textbox->text->transform->rect.y = textbox->transform->rect.y;

            ui_text_component_draw (textbox->text);

            if (adjust_to_text) {
                textbox->transform->rect.w = textbox->text->transform->rect.w;
                textbox->transform->rect.h = textbox->text->transform->rect.h;
                ui_position_update (textbox->transform, NULL, true);
            }
        }
    }

}

// updates the textbox's text
void ui_textbox_update_text (TextBox *textbox, const char *text) {

    if (textbox) {
        if (textbox->text) {
            ui_text_component_update (textbox->text, text);
            ui_text_component_draw (textbox->text);
            // ui_transform_component_set_pos (textbox->text->transform, 
            //     &textbox->transform->rect, textbox->text->transform->pos, true);
        }
    }

}

// sets the textbox's text position
void ui_textbox_set_text_pos (TextBox *textbox, UIPosition pos) {

    if (textbox) {
        if (textbox->text) {}
            ui_transform_component_set_pos (textbox->text->transform, &textbox->transform->rect, pos, true);
    }

}

// sets the textbox font
void ui_textbox_set_font (TextBox *textbox, Font *font) {

    if (textbox) {
        textbox->text->font = font;
        ui_text_component_draw (textbox->text);
    }

}

// sets the textbox's text color
void ui_textbox_set_text_color (TextBox *textbox, RGBA_Color color) {

    if (textbox) {
        if (textbox->text) {
            textbox->text->text_color = color;
            ui_text_component_draw (textbox->text);
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

// removes the ouline form the textbox
void ui_textbox_remove_outline (TextBox *textbox) {

    if (textbox) {
        memset (&textbox->outline_colour, 0, sizeof (RGBA_Color));
        textbox->outline = false;
    }

}

// sets the textbox's background color
void ui_textbox_set_bg_color (TextBox *textbox, RGBA_Color color) {

    if (textbox) {
        textbox->bg_colour = color;
        if (color.a < 255) {
            textbox->bg_texture = render_complex_transparent_rect (&textbox->transform->rect, color);
            textbox->bg_texture_rect.w = textbox->transform->rect.w;
            textbox->bg_texture_rect.h = textbox->transform->rect.h;
        }

        textbox->colour = true;
    } 

}

// removes the background from the textbox
void ui_textbox_remove_background (TextBox *textbox) {

    if (textbox) {
        if (textbox->bg_texture) {
            SDL_DestroyTexture (textbox->bg_texture);
            textbox->bg_texture = NULL;
        }

        memset (&textbox->bg_colour, 0, sizeof (RGBA_Color));
        textbox->colour = false;
    }

}

// creates a new textbox
TextBox *ui_textbox_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos) {

    TextBox *textbox = NULL;

    UIElement *ui_element = ui_element_new (UI_TEXTBOX);
    if (ui_element) {
        textbox = ui_textbox_new ();
        if (textbox) {
            textbox->ui_element = ui_element;
            textbox->transform = ui_transform_component_create (x, y, w, h);
            ui_transform_component_set_pos (textbox->transform, NULL, pos, true);
            ui_element->element = textbox;
        }

        else ui_element_delete (ui_element);
    }

    return textbox;

}

// draws the textbox
void ui_textbox_draw (TextBox *textbox) {

    if (textbox) {
        // render the background
        if (textbox->bg_texture) {
            SDL_RenderCopyEx (main_renderer->renderer, textbox->bg_texture, 
                &textbox->bg_texture_rect, &textbox->transform->rect, 
                0, 0, SDL_FLIP_NONE);
        }

        else if (textbox->colour) 
            render_basic_filled_rect (&textbox->transform->rect, textbox->bg_colour);

        // render the outline border
        if (textbox->outline) 
            render_basic_outline_rect (&textbox->transform->rect, textbox->outline_colour);

        // render the text
        ui_text_component_render (textbox->text);
    }

}
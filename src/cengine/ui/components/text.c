#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/renderer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

Text *ui_text_component_new (void) {

    Text *text = (Text *) malloc (sizeof (Text));
    if (text) {
        memset (text, 0, sizeof (Text));
        text->transform = NULL;
        text->font = NULL;
        text->font_source = NULL;
        text->text = NULL;
        text->wrap_text = false;
        text->texture = NULL;
    }

    return text;

}

void ui_text_component_delete (void *text_ptr) {

    if (text_ptr) {
        Text *text = (Text *) text_ptr;

        ui_transform_component_delete (text->transform);
        text->font = NULL;
        text->font_source = NULL;
        str_delete (text->text);
        if (text->texture) SDL_DestroyTexture (text->texture);

        free (text);
    }

}

// inits the text component with the correct values
void ui_text_component_init (Text *text, Font *font, unsigned int size, RGBA_Color color, 
    const char *text_str) {

    if (text) {
        if (!text->transform) text->transform = ui_transform_component_create (0, 0, 0, 0);

        text->font = font;
        text->size = size;

        // get the most likely font source by size
        text->font_source = font_source_get_by_size (text->font, text->size);

        text->text_color = color;
        text->text = text_str ? str_new (text_str) : NULL;
    }

}

// sets a new text for the component
void ui_text_component_set_text (Text *text, const char *text_str) {

    if (text) {
        str_delete (text->text);
        text->text = text_str ? str_new (text_str) : NULL;
    } 

}

// updates the text in the component
void ui_text_component_update (Text *text, const char *text_str) {

    if (text && text_str) str_replace (text->text, text_str);

}

// sets the option to wrap text (create new lines) if to big
void ui_text_component_set_wrap (Text *text, u32 wrap_lenght) {

    if (text) {
        text->wrap_length = wrap_lenght;
        text->wrap_text = (wrap_lenght > 0) ? true : false;
    }

}

// creates / updates the text texture
void ui_text_component_draw (Text *text) {

    if (text) {
        if (text->texture) {
            SDL_DestroyTexture (text->texture);
            text->texture = NULL;
        }

        if (text->text->len > 0) {
            SDL_Surface *surface = NULL;

            if (text->wrap_text) {
                surface = TTF_RenderText_Blended_Wrapped (text->font_source->ttf_source,
                    text->text->str, text->text_color, text->wrap_length);
            }

            else {
                surface = TTF_RenderText_Blended (text->font_source->ttf_source, 
                    text->text->str, text->text_color);
            }

            text->transform->rect.w = surface->w;
            text->transform->rect.h = surface->h;
            text->texture = SDL_CreateTextureFromSurface (main_renderer->renderer, surface);

            SDL_FreeSurface (surface);
        }
    }

}

// renders the text component to the screen
void ui_text_component_render (Text *text) {

    if (text) {
        if (text->texture) {
            SDL_RenderCopy (main_renderer->renderer, text->texture, 
                NULL, &text->transform->rect);
        }
    }

}
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/types/vector2d.h"

#include "cengine/renderer.h"
#include "cengine/textures.h"
#include "cengine/sprites.h"
#include "cengine/input.h"

#include "cengine/game/camera.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/button.h"

static Button *button_new (void) {

    Button *button = (Button *) malloc (sizeof (Button));
    if (button) {
        memset (button, 0, sizeof (Button));

        button->font = NULL;
        button->text = NULL;

        button->sprites = (Sprite **) calloc (BUTTON_STATE_TOTAL, sizeof (Sprite *));
        for (unsigned int  i = 0; i < BUTTON_STATE_TOTAL; i++)
            button->sprites[i] = NULL;

        button->ref_sprites = (bool *) calloc (BUTTON_STATE_TOTAL, sizeof (bool));
        for (unsigned int  i = 0; i < BUTTON_STATE_TOTAL; i++)
            button->ref_sprites[i] = false;

        button->texture = NULL;

        button->event = NULL;
    }

    return button;

}

void button_delete (void *button_ptr) {

    if (button_ptr) {
        Button *button = (Button *) button_ptr;

        button->font = NULL;
        str_delete (button->text);

        if (button->sprites && button->ref_sprites) {
            for (unsigned int  i = 0; i < BUTTON_STATE_TOTAL; i++) {
                if (!button->ref_sprites[i]) sprite_destroy (button->sprites[i]);
                else button->sprites[i] = NULL;
            }

            free (button->sprites);
            free (button->ref_sprites);
        }

        if (button->texture) SDL_DestroyTexture (button->texture);

        free (button);
    }

}

// sets the button text
void ui_button_set_text (Button *button, const char *text) {

    if (button) {
        str_delete (button->text);      // delete previous text
        button->text = text ? str_new (text) : NULL;
    }

}

// sets the button's text color
void ui_button_set_text_color (Button *button, RGBA_Color color) {

    if (button) {
        button->text_color = color;

        if (button->text) {
            if (!button->is_volatile) {
                SDL_Surface *surface = TTF_RenderText_Solid (button->font->ttf_source,
                    button->text->str, button->text_color);
                button->texture = SDL_CreateTextureFromSurface (main_renderer->renderer, surface);

                button->bgrect.w = surface->w;
                button->bgrect.h = surface->h;

                SDL_FreeSurface (surface);
            }
        }
    }

}

// sets the background color of the button
void ui_button_set_bg_color (Button *button, RGBA_Color color) {

    if (button) button->bgcolor = color;

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

// sets an event to be triggered when the button is clicked
void ui_button_set_event (Button *button, Action event) {

    if (button) button->event = event;

}

// draws a volatile button text
void ui_button_text_draw (Button *button) {

    if (button) {
        const char *c = button->text->str;

        UIRect src_rect, dest_rect,
            dirty_rect = ui_rect_create (button->bgrect.x, button->bgrect.y, 0, 0);

        GlyphData glyph;
        u32 code_point;

        u32 destX = button->bgrect.x;
        u32 destY = button->bgrect.y;
        float destH, dest_line_spacing, dest_letter_spacing;

        // TODO: add scale here!
        destH = button->font->height;
        dest_line_spacing = button->font->lineSpacing;
        dest_letter_spacing = button->font->letterSpacing;

        int new_line_x = button->bgrect.x;

        for (; *c != '\0'; c++) {
            if (*c == '\n') {
                destX = new_line_x;
                destY += destH + dest_line_spacing;
                continue;
            }

            code_point = get_code_point_from_UTF8 (&c, 1);
            if (glyph_get_data (button->font, &glyph, code_point)) {
                // FIXME: handle bad characters
            }

            if (code_point == ' ') {
                destX += glyph.rect.w + dest_letter_spacing;
                continue;
            }

            src_rect = glyph.rect;

            dest_rect = ui_rect_render (glyph_get_cache_level (button->font, glyph.cacheLevel),
                &src_rect, destX, destY);
            
            if (dirty_rect.w == 0 || dirty_rect.h == 0) dirty_rect = dest_rect;
            else dirty_rect = ui_rect_union (dirty_rect, dest_rect);

            destX += glyph.rect.w + dest_letter_spacing;
        }
    }

}

// draws a button
void ui_button_draw (Button *button) {

    if (button) {
        Sprite *selected_sprite = NULL;

        // check if the mouse is in the button
        if (mousePos.x >= button->bgrect.x && mousePos.x <= (button->bgrect.x + button->bgrect.w) && 
            mousePos.y >= button->bgrect.y && mousePos.y <= (button->bgrect.y + button->bgrect.h)) {
            // check if the user pressed the left button over the mouse
            if (input_get_mouse_button_state (MOUSE_LEFT))
                selected_sprite = button->sprites[BUTTON_STATE_MOUSE_DOWN];

            // if not, the user is hovering the mouse over the button
            else selected_sprite = button->sprites[BUTTON_STATE_MOUSE_OVER_MOTION];
        }

        // the mouse is NOT hovering over the button OR we dont have any other sprite available
        if (!selected_sprite) selected_sprite = button->sprites[BUTTON_STATE_MOUSE_OUT];

        if (selected_sprite) {
            texture_draw (main_camera, 
            selected_sprite, button->bgrect.x, button->bgrect.y, NO_FLIP);
        } 

        // draw button text
        if (button->text) ui_button_text_draw (button);
    }

}
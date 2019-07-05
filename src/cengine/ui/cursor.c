#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/renderer.h"
#include "cengine/sprites.h"
#include "cengine/input.h"
#include "cengine/ui/cursor.h"
#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

Cursor *main_cursor = NULL;

Cursor *ui_cursor_new (void) {

    Cursor *c = (Cursor *) malloc (sizeof (Cursor));
    if (c) {
        memset (c, 0, sizeof (Cursor));
        c->cursor = NULL;
        c->sprite = NULL;
        c->sprite_sheet = NULL;
    }

    return c;

}

// create a new ui cursor with a custom sprite or pass a spirtesheet to load different images
// when performing different actions
Cursor *ui_cursor_create (const char *sprite, const char *sprite_sheet, int w, int h, int hot_x, int hot_y) {

    Cursor *c = NULL;

    if (sprite || sprite_sheet) {
        // hide the system cursor inside the window
        i32 cursorData[2] = { 0, 0 };
        
        c = ui_cursor_new ();
        
        int width = w <= 0 ? CURSOR_DEFAULT_WIDTH : w;
        int height = h <= 0 ? CURSOR_DEFAULT_HEIGHT : h;
        int hotX = hot_x <= 0 ? CURSOR_DEFAULT_HOT_X : hot_x;    
        int hotY = hot_y <= 0 ? CURSOR_DEFAULT_HOT_Y : hot_y;

        c->cursor = SDL_CreateCursor ((Uint8 *) cursorData, (Uint8 *) cursorData, width, height, hotX, hotY);
        SDL_SetCursor (c->cursor);

        if (sprite) {
            c->sprite = sprite_load (sprite, main_renderer);
            if (!c->sprite) {
                cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to load cursor sprite!");
                ui_cursor_delete (c);
                return NULL;
            } 
            c->multipleSprites = false;
            c->sprite_sheet = NULL;
        }

        else if (sprite_sheet) {
            c->sprite_sheet = sprite_sheet_load (sprite_sheet, main_renderer);
            if (!c->sprite_sheet) {
                cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to load cursor sprite sheet!");
                ui_cursor_delete (c);
                return NULL;
            }
            c->multipleSprites = true;
            c->sprite = NULL;
        }
    }

    return c;

}

void ui_cursor_delete (Cursor *cursor) {

    if (cursor) {
        SDL_FreeCursor (cursor->cursor);

        if (cursor->refSprite) {
            cursor->sprite = NULL;
            cursor->sprite_sheet = NULL;
        }

        else {
            sprite_destroy (cursor->sprite);
            sprite_sheet_destroy (cursor->sprite_sheet);
        }

        free (cursor);
    }

}

void ui_cursor_draw (Cursor *cursor) {

    if (cursor) {
        cursor->sprite->dest_rect.x = mousePos.x;
        cursor->sprite->dest_rect.y = mousePos.y;

        SDL_RenderCopyEx (main_renderer->renderer, cursor->sprite->texture, 
            &cursor->sprite->src_rect, &cursor->sprite->dest_rect, 
            0, 0, NO_FLIP);   
    }

}

// update cursor with new values
int ui_cursor_update (Cursor *c, int w, int h, int hot_x, int hot_y) {

    if (c) {
        if (c->cursor) SDL_FreeCursor (c->cursor);

        // hide the system cursor inside the window
        i32 cursorData[2] = { 0, 0 };
        c->cursor = SDL_CreateCursor ((Uint8 *) cursorData, (Uint8 *) cursorData, w, h, hot_x, hot_y);
        SDL_SetCursor (c->cursor);

        return 0;
    }

    return 1;

}


/*** Getters & Setters ***/

int ui_cursor_get_width (Cursor *c) { if (c) return c->w; }

int ui_cursor_set_width (Cursor *c, int new_width) {

    if (c) {
        int width = new_width <= 0 ? CURSOR_DEFAULT_WIDTH : new_width;
        return ui_cursor_update (c, width, c->h, c->hot_x, c->hot_y);
    }

    return 1;

}

int ui_cursor_get_height (Cursor *c) { if (c) return c->h; }

int ui_cursor_set_height (Cursor *c, int new_height) {

    if (c) {
        int height = new_height <= 0 ? CURSOR_DEFAULT_WIDTH : new_height;
        return ui_cursor_update (c, c->w, height, c->hot_x, c->hot_y);
    }

    return 1;

}

int ui_cursor_get_hot_x (Cursor *c) { if (c) return c->hot_x; }

int ui_cursor_get_hot_y (Cursor *c) { if (c) return c->hot_y; }
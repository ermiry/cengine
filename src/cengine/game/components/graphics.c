#include <stdlib.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/sprites.h"
#include "cengine/renderer.h"
#include "cengine/game/components/graphics.h"

Graphics *graphics_new (u32 objectID) {

    Graphics *new_graphics = (Graphics *) malloc (sizeof (Graphics));
    if (new_graphics) {
        new_graphics->go_id = objectID;
        new_graphics->sprite = NULL;
        new_graphics->spriteSheet = NULL;
        new_graphics->refSprite = false;
        new_graphics->multipleSprites = false;
        new_graphics->x_sprite_offset = 0;
        new_graphics->y_sprite_offset = 0;
        new_graphics->flip = NO_FLIP;
    }

    return new_graphics;

}

void graphics_destroy (Graphics *graphics) {

    if (graphics) {
        if (graphics->refSprite) {
            graphics->sprite = NULL;
            graphics->spriteSheet = NULL;
        }

        else {
            if (graphics->sprite) sprite_destroy (graphics->sprite);
            if (graphics->spriteSheet) sprite_sheet_destroy (graphics->spriteSheet);
        }

        free (graphics);
    }

}

void graphics_set_sprite (Graphics *graphics, const char *filename) {

    if (graphics && filename) {
        graphics->sprite = sprite_load (filename, main_renderer);
        graphics->spriteSheet = NULL;
        graphics->multipleSprites = false;
    }

}

void graphics_set_sprite_sheet (Graphics *graphics, const char *filename) {

    if (graphics && filename) {
        graphics->sprite = NULL;
        graphics->spriteSheet = sprite_sheet_load (filename, main_renderer);
        graphics->multipleSprites = true;
    }

}

void graphics_ref_sprite (Graphics *graphics, Sprite *sprite) {

    if (graphics && sprite) {
        graphics->sprite = sprite;
        graphics->spriteSheet = NULL;
        graphics->multipleSprites = false;
        graphics->refSprite = true;
    }

}

void graphics_ref_sprite_sheet (Graphics *graphics, SpriteSheet *spriteSheet) {

    if (graphics && spriteSheet) {
        graphics->sprite = NULL;
        graphics->spriteSheet = spriteSheet;
        graphics->multipleSprites = true;
        graphics->refSprite = true;
    }

}
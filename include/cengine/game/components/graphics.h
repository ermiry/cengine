#ifndef _CENGINE_COMPONENTS_GRAPHICS_H_
#define _CENGINE_COMPONENTS_GRAPHICS_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/sprites.h"

typedef struct Graphics {

    u32 go_id;

    Sprite *sprite;
    SpriteSheet *spriteSheet;
    bool refSprite;

    u32 x_sprite_offset, y_sprite_offset;
    bool multipleSprites;
    Flip flip;
    bool hasBeenSeen;
    bool visibleOutsideFov;

} Graphics;

extern Graphics *graphics_new (u32 objectID);
extern void graphics_destroy (Graphics *graphics);

extern void graphics_set_sprite (Graphics *graphics, const char *filename);
extern void graphics_set_sprite_sheet (Graphics *graphics, const char *filename);

extern void graphics_ref_sprite (Graphics *graphics, Sprite *sprite);
extern void graphics_ref_sprite_sheet (Graphics *graphics, SpriteSheet *spriteSheet);

#endif
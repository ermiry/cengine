#ifndef _CENGINE_COMPONENTS_GRAPHICS_H_
#define _CENGINE_COMPONENTS_GRAPHICS_H_

#include <stdbool.h>

#include "cengine/types/types.h"

#include "cengine/graphics.h"
#include "cengine/renderer.h"
#include "cengine/sprites.h"

struct _Sprite;
struct _SpriteSheet;

typedef struct Graphics {

    u32 go_id;

    struct _Sprite *sprite;
    struct _SpriteSheet *spriteSheet;
    bool refSprite;

    u32 x_sprite_offset, y_sprite_offset;
    bool multipleSprites;
    Flip flip;
    bool hasBeenSeen;
    bool visibleOutsideFov;

} Graphics;

extern Graphics *graphics_new (u32 objectID);

extern void graphics_destroy (Graphics *graphics);

extern void graphics_set_sprite (Graphics *graphics, Renderer *renderer, const char *filename);

extern void graphics_set_sprite_sheet (Graphics *graphics, Renderer *renderer, const char *filename);

extern void graphics_ref_sprite (Graphics *graphics, struct _Sprite *sprite);

extern void graphics_ref_sprite_sheet (Graphics *graphics, struct _SpriteSheet *spriteSheet);

#endif
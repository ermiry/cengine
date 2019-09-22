#ifndef _CENGINE_SPRITES_H_
#define _CENGINE_SPRITES_H_

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/renderer.h"

typedef enum Flip {

    NO_FLIP = 0x00000000,
    FLIP_HORIZONTAL = 0x00000001,
    FLIP_VERTICAL = 0x00000002

} Flip;

typedef struct Sprite {

    u32 w, h;
    SDL_Texture *texture;
    i32 scale_factor;
    SDL_Rect src_rect, dest_rect;

} Sprite;

extern void sprite_destroy (Sprite *sprite);

typedef struct IndividualSprite {

    u32 col, row;

} IndividualSprite;

typedef struct SpriteSheet {

    u32 w, h;
    SDL_Texture *texture;

    SDL_Rect src_rect, dest_rect;

    u32 sprite_w, sprite_h;

    i32 scale_factor;

    IndividualSprite ***individual_sprites;

} SpriteSheet;

extern SpriteSheet *sprite_sheet_new (void);
extern void sprite_destroy (Sprite *sprite);
extern Sprite *sprite_load (const char *filename, Renderer *renderer);

extern void sprite_sheet_destroy (SpriteSheet *sprite_sheet);
extern SpriteSheet *sprite_sheet_load (const char *filename, Renderer *rendererr);
extern void sprite_sheet_set_sprite_size (SpriteSheet *sprite_sheet, u32 w, u32 h);
extern void sprite_sheet_set_scale_factor (SpriteSheet *sprite_sheet, i32 scale_factor);
extern void sprite_sheet_crop (SpriteSheet *sprite_sheet);

#endif
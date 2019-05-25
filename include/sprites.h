#ifndef _CENGINE_SPRITES_H_
#define _CENGINE_SPRITES_H_

#include <SDL2/SDL.h>

#include "blackrock.h"

typedef struct Sprite {

    u32 w, h;
    SDL_Texture *texture;
    i32 scaleFactor;
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

    i32 scaleFactor;

    IndividualSprite ***individualSprites;

} SpriteSheet;

extern SpriteSheet *sprite_sheet_new (void);
extern void sprite_destroy (Sprite *sprite);
extern Sprite *sprite_load (const char *filename, SDL_Renderer *renderer);

extern void sprite_sheet_destroy (SpriteSheet *spriteSheet);
extern SpriteSheet *sprite_sheet_load (const char *filename, SDL_Renderer *renderer);
extern void sprite_sheet_set_sprite_size (SpriteSheet *spriteSheet, u32 w, u32 h);
extern void sprite_sheet_set_scale_factor (SpriteSheet *spriteSheet, i32 scaleFactor);
extern void sprite_sheet_crop (SpriteSheet *spriteSheet);

#endif
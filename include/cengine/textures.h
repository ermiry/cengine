#ifndef _CENGINE_TEXTURES_H_
#define _CENGINE_TEXTURES_H_

#include <SDL2/SDL.h>

#include "cengine/types/types.h"

#include "cengine/sprites.h"

extern SDL_Texture *texture_load (const char *filename, Renderer *renderer);
extern void texture_get_dimensions (SDL_Texture *texture, u32 *w, u32 *h);

#include "cengine/game/camera.h"

extern void texture_draw (Camera *cam, Sprite *sprite, i32 x, i32 y, SDL_RendererFlip flip);
extern void texture_draw_frame (Camera *cam, SpriteSheet *spriteSheet, 
    i32 x, i32 y, u32 col, u32 row, SDL_RendererFlip flip);

#endif
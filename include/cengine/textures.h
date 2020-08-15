#ifndef _CENGINE_TEXTURES_H_
#define _CENGINE_TEXTURES_H_

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/config.h"
#include "cengine/graphics.h"
#include "cengine/renderer.h"
#include "cengine/sprites.h"

struct _Sprite;
struct _SpriteSheet;

// solves issue of loading texture in a different thread, see texture load
// creates a new texture based on a surface
CENGINE_PUBLIC void texture_create_from_surface (Renderer *renderer, SDL_Texture **texture, SDL_Surface *surface);

// creates a new texture based on a memory buffer represeinting an image
CENGINE_PUBLIC void texture_create_from_mem_image (Renderer *renderer, SDL_Texture **texture, 
    const void *mem, int mem_size, 
    const char *image_type);

// loads an image file into a texture
// NOTE: uses the renderer to create a new texture from a surface
// but only works from the thread where you created the renderer, you can call this method from 
// another thread and you will still get the image data (recovered from the created surface)
// and the texture will be created in the next render loop, given the illusion of a background charge
// the image data msut be freed using image_data_delete
CENGINE_PUBLIC ImageData *texture_load (Renderer *renderer, const char *filename, SDL_Texture **texture);

// thread safe wrapper method for SDL_UpdateTexture () using SDL_LockTexture () and SDL_UnlockTexture ()
// mostly intended for internal cengine usage
// previous texture is not destroyed, surface is destroyed
CENGINE_PUBLIC void texture_update (Renderer *renderer, SDL_Texture **texture, SDL_Surface *surface);

// 27/01/2020 -- 7:11 -- wrapper method for SDL_DestroyTexture () as it is not thread safe
// this method should be called instead of SDL_DestroyTexture () to avoid video memory leaks
CENGINE_PUBLIC void texture_destroy (Renderer *renderer, SDL_Texture *texture);

CENGINE_PUBLIC void texture_destroy_wrapper (void *texture_ptr);

// gets the texture's width and height
CENGINE_PUBLIC void texture_get_dimensions (SDL_Texture *texture, int *w, int *h);

#include "cengine/game/camera.h"

struct _Camera;

CENGINE_PUBLIC void texture_draw (struct _Camera *cam, Renderer *renderer, struct _Sprite *sprite, i32 x, i32 y, SDL_RendererFlip flip);

CENGINE_PUBLIC void texture_draw_frame (struct _Camera *cam, Renderer *renderer, struct _SpriteSheet *spriteSheet, 
    i32 x, i32 y, u32 col, u32 row, SDL_RendererFlip flip);

#endif
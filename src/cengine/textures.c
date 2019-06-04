#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "cengine/renderer.h"
#include "cengine/sprites.h"
#include "cengine/game/camera.h"
#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

/*** TEXTURE MANAGER ***/

SDL_Texture *texture_load (const char *filename, Renderer *renderer) {

    if (filename && renderer) {
        SDL_Surface *tmpSurface = IMG_Load (filename);
        if (tmpSurface) {
            SDL_Texture *texture = SDL_CreateTextureFromSurface (renderer->renderer, tmpSurface);
            SDL_FreeSurface (tmpSurface);

            return texture;
        }

        cengine_log_msg (stderr, ERROR, NO_TYPE, c_string_create ("Failed to load asset: %s!", filename));
    }

    return NULL;

}

void texture_get_dimensions (SDL_Texture *texture, u32 *w, u32 *h) {

    if (texture) SDL_QueryTexture (texture, NULL, NULL, w, h);

}

void texture_draw (Camera *cam, Sprite *sprite, i32 x, i32 y, SDL_RendererFlip flip) {

    if (cam && sprite) {
        sprite->dest_rect.x = x;
        sprite->dest_rect.y = y;

        CamRect screenRect = camera_world_to_screen (cam, sprite->dest_rect);

        SDL_RenderCopyEx (main_renderer->renderer, sprite->texture, &sprite->src_rect, &screenRect, 
            0, 0, flip);    
    }

}

void texture_draw_frame (Camera *cam, SpriteSheet *spriteSheet, 
    i32 x, i32 y, u32 col, u32 row, SDL_RendererFlip flip) {

    if (cam && spriteSheet) {
        spriteSheet->src_rect.x = spriteSheet->sprite_w * col;
        spriteSheet->src_rect.y = spriteSheet->sprite_h * row;

        spriteSheet->dest_rect.x = x;
        spriteSheet->dest_rect.y = y;

        CamRect screenRect = camera_world_to_screen (cam, spriteSheet->dest_rect);

        SDL_RenderCopyEx (main_renderer->renderer, spriteSheet->texture, 
            &spriteSheet->src_rect, &screenRect,
            0, 0, flip);
    }

}
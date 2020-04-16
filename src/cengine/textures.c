#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/renderer.h"
#include "cengine/graphics.h"
#include "cengine/sprites.h"
#include "cengine/textures.h"

#include "cengine/game/camera.h"

#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

void texture_create_from_surface (Renderer *renderer, SDL_Texture **texture, SDL_Surface *surface) {

    if (renderer && texture && surface) {
        pthread_t thread_id = pthread_self ();
        // printf ("Loading texture in thread: %ld\n", thread_id);
        if (thread_id == renderer->thread_id) {
            // load texture as always
            *texture = SDL_CreateTextureFromSurface (renderer->renderer, surface);
            SDL_FreeSurface (surface);
        }

        else {
            // send texture to renderer queue
            renderer_load_queue_push (renderer, surface_texture_new (surface, texture, false));
        }
    }

}

ImageData *texture_load (Renderer *renderer, const char *filename, SDL_Texture **texture) {

    ImageData *image_data = NULL;

    if (filename && renderer && texture) {
        SDL_Surface *temp_surface = IMG_Load (filename);
        if (temp_surface) {
            image_data = image_data_new (temp_surface->w, temp_surface->h, str_new (filename));

            pthread_t thread_id = pthread_self ();
            // printf ("Loading texture in thread: %ld\n", thread_id);
            if (thread_id == renderer->thread_id) {
                // load texture as always
                *texture = SDL_CreateTextureFromSurface (renderer->renderer, temp_surface);
                SDL_FreeSurface (temp_surface);
            }

            else {
                // send texture to renderer queue
                renderer_load_queue_push (renderer, surface_texture_new (temp_surface, texture, false));
            }
        }

        else {
            cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
                c_string_create ("Failed to load asset: %s!", filename));
        } 
    }

    return image_data;

}

void texture_update (Renderer *renderer, SDL_Texture **texture, SDL_Surface *surface) {

    if (renderer && texture && surface) {
        pthread_t thread_id = pthread_self ();
        // printf ("Updating texture in thread: %ld\n", thread_id);
        if (thread_id == renderer->thread_id) {
            SDL_UpdateTexture (*texture, NULL, surface->pixels, surface->pitch);
            SDL_FreeSurface (surface);
        }

        else {
            // send texture to renderer queue
            renderer_load_queue_push (renderer, surface_texture_new (surface, texture, true));
        }
    }

}

// 27/01/2020 -- 7:11 -- wrapper method for SDL_DestroyTexture () as it is not thread safe
// this method should be called instead of SDL_DestroyTexture () to avoid video memory leaks
void texture_destroy (Renderer *renderer, SDL_Texture *texture) {

    if (renderer && texture) {
        pthread_t thread_id = pthread_self ();
        // printf ("Destroying texture in thread: %ld\n", thread_id);
        if (thread_id == renderer->thread_id) {
            SDL_DestroyTexture (texture);
        }

        else {
            // send texture to destroy queue
            renderer_destroy_queue_push (renderer, texture);
        }
    }

}

void texture_destroy_wrapper (void *texture_ptr) {

    if (texture_ptr) SDL_DestroyTexture ((SDL_Texture *) texture_ptr);

}

void texture_get_dimensions (SDL_Texture *texture, int *w, int *h) {

    if (texture) SDL_QueryTexture (texture, NULL, NULL, w, h);

}

void texture_draw (Camera *cam, Renderer *renderer, Sprite *sprite, i32 x, i32 y, SDL_RendererFlip flip) {

    if (cam && sprite) {
        sprite->dest_rect.x = x;
        sprite->dest_rect.y = y;

        CamRect screenRect = camera_world_to_screen (cam, sprite->dest_rect);

        SDL_RenderCopyEx (renderer->renderer, sprite->texture, &sprite->src_rect, &screenRect, 
            0, 0, flip);    
    }

}

void texture_draw_frame (Camera *cam, Renderer *renderer, SpriteSheet *spriteSheet, 
    i32 x, i32 y, u32 col, u32 row, SDL_RendererFlip flip) {

    if (cam && spriteSheet) {
        spriteSheet->src_rect.x = spriteSheet->sprite_w * col;
        spriteSheet->src_rect.y = spriteSheet->sprite_h * row;

        spriteSheet->dest_rect.x = x;
        spriteSheet->dest_rect.y = y;

        CamRect screenRect = camera_world_to_screen (cam, spriteSheet->dest_rect);

        SDL_RenderCopyEx (renderer->renderer, spriteSheet->texture, 
            &spriteSheet->src_rect, &screenRect,
            0, 0, flip);
    }

}
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "cengine/sprites.h"
#include "cengine/textures.h"

/*** SPRITES ***/

Sprite *sprite_load (const char *filename, Renderer *renderer) {

    if (filename && renderer) {
        Sprite *new_sprite = (Sprite *) malloc (sizeof (Sprite));
        if (new_sprite) {
            new_sprite->texture = texture_load (filename, renderer);
            if (new_sprite->texture) {
                texture_get_dimensions (new_sprite->texture, &new_sprite->w, &new_sprite->h) ;

                // dimensions
                new_sprite->src_rect.w = new_sprite->dest_rect.w = new_sprite->w;
                new_sprite->src_rect.h = new_sprite->dest_rect.h = new_sprite->h;

                // positions
                new_sprite->src_rect.x = new_sprite->dest_rect.x = 0;
                new_sprite->src_rect.y = new_sprite->dest_rect.y = 0;

                return new_sprite;
            }

            free (new_sprite);
        }
    }

    return NULL;

}

void sprite_destroy (Sprite *sprite) {

    if (sprite) {
        if (sprite->texture) SDL_DestroyTexture (sprite->texture);
        free (sprite);
    }

}

/*** SPRITES SHEET ***/

SpriteSheet *sprite_sheet_new (void) {

    SpriteSheet *sp = (SpriteSheet *) malloc (sizeof (SpriteSheet));
    if (sp) {
        memset (sp, 0, sizeof (SpriteSheet));
        sp->texture = NULL;
        sp->individual_sprites = NULL;
    }

    return sp;

}

void sprite_sheet_destroy (SpriteSheet *sprite_sheet) {

    if (sprite_sheet) {
        if (sprite_sheet->individual_sprites) {
            u8 x_num_sprites = sprite_sheet->w / sprite_sheet->sprite_w;
            u8 y_num_sprites = sprite_sheet->h / sprite_sheet->sprite_h;

            for (u8 i = 0; i < x_num_sprites; i++)
                for (u8 j = 0; j < y_num_sprites; j++)
                    if (sprite_sheet->individual_sprites[i][j])
                        free (sprite_sheet->individual_sprites[i][j]);

            free (sprite_sheet->individual_sprites);
        }

        if (sprite_sheet->texture) SDL_DestroyTexture (sprite_sheet->texture);

        free (sprite_sheet);
    }

}

SpriteSheet *sprite_sheet_load (const char *filename, Renderer *renderer) {

    if (filename && renderer) {
        SpriteSheet *new_sprite_sheet = sprite_sheet_new ();
        if (new_sprite_sheet) {
            new_sprite_sheet->texture = texture_load (filename, renderer);
            if (new_sprite_sheet->texture) {
                texture_get_dimensions (new_sprite_sheet->texture, &new_sprite_sheet->w,
                    &new_sprite_sheet->h);

                // dimensions
                new_sprite_sheet->src_rect.w = new_sprite_sheet->dest_rect.w = 0;
                new_sprite_sheet->src_rect.h = new_sprite_sheet->dest_rect.h = 0;

                // positions
                new_sprite_sheet->src_rect.x = new_sprite_sheet->dest_rect.x = 0;
                new_sprite_sheet->src_rect.y = new_sprite_sheet->dest_rect.y = 0;

                return new_sprite_sheet;
            }
        }
    }

    return NULL;

}

void sprite_sheet_set_sprite_size (SpriteSheet *sprite_sheet, u32 w, u32 h) {

    if (sprite_sheet) {
        sprite_sheet->sprite_w = w;
        sprite_sheet->sprite_h = h;

        sprite_sheet->src_rect.w = sprite_sheet->sprite_w;
        sprite_sheet->src_rect.h = sprite_sheet->sprite_h;
    }

}

void sprite_sheet_set_scale_factor (SpriteSheet *sprite_sheet, i32 scale_factor) {

    if (sprite_sheet) {
        sprite_sheet->scale_factor = scale_factor;

        sprite_sheet->dest_rect.w = sprite_sheet->src_rect.w * sprite_sheet->scale_factor;
        sprite_sheet->dest_rect.h = sprite_sheet->src_rect.h * sprite_sheet->scale_factor;
    } 

}

void sprite_sheet_crop (SpriteSheet *sprite_sheet) {

    if (sprite_sheet) {
        u8 x_num_sprites = sprite_sheet->w / sprite_sheet->sprite_w;
        u8 y_num_sprites = sprite_sheet->h / sprite_sheet->sprite_h;

        u32 max_sprites_expected = x_num_sprites * y_num_sprites;

        sprite_sheet->individual_sprites = (IndividualSprite ***) calloc (x_num_sprites, sizeof (Sprite **));
        for (u8 i = 0; i < x_num_sprites; i++)
            sprite_sheet->individual_sprites[i] = (IndividualSprite **) calloc (y_num_sprites, sizeof (Sprite *));

        for (u8 y = 0; y < y_num_sprites; y++) {
            for (u8 x = 0; x < x_num_sprites; x++) {
                IndividualSprite *new_sprite = (IndividualSprite *) malloc (sizeof (Sprite));
                new_sprite->col = x;
                new_sprite->row = y;
                sprite_sheet->individual_sprites[x][y] = new_sprite;
            }
        }
    }

}
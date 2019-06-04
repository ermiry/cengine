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
        sp->individualSprites = NULL;
    }

    return sp;

}

void sprite_sheet_destroy (SpriteSheet *spriteSheet) {

    if (spriteSheet) {
        if (spriteSheet->individualSprites) {
            u8 x_num_sprites = spriteSheet->w / spriteSheet->sprite_w;
            u8 y_num_sprites = spriteSheet->h / spriteSheet->sprite_h;

            for (u8 i = 0; i < x_num_sprites; i++)
                for (u8 j = 0; j < y_num_sprites; j++)
                    if (spriteSheet->individualSprites[i][j])
                        free (spriteSheet->individualSprites[i][j]);

            free (spriteSheet->individualSprites);
        }

        if (spriteSheet->texture) SDL_DestroyTexture (spriteSheet->texture);

        free (spriteSheet);
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

void sprite_sheet_set_sprite_size (SpriteSheet *spriteSheet, u32 w, u32 h) {

    if (spriteSheet) {
        spriteSheet->sprite_w = w;
        spriteSheet->sprite_h = h;

        spriteSheet->src_rect.w = spriteSheet->sprite_w;
        spriteSheet->src_rect.h = spriteSheet->sprite_h;
    }

}

void sprite_sheet_set_scale_factor (SpriteSheet *spriteSheet, i32 scaleFactor) {

    if (spriteSheet) {
        spriteSheet->scaleFactor = scaleFactor;

        spriteSheet->dest_rect.w = spriteSheet->src_rect.w * spriteSheet->scaleFactor;
        spriteSheet->dest_rect.h = spriteSheet->src_rect.h * spriteSheet->scaleFactor;
    } 

}

void sprite_sheet_crop (SpriteSheet *spriteSheet) {

    if (spriteSheet) {
        u8 x_num_sprites = spriteSheet->w / spriteSheet->sprite_w;
        u8 y_num_sprites = spriteSheet->h / spriteSheet->sprite_h;

        u32 max_sprites_expected = x_num_sprites * y_num_sprites;

        spriteSheet->individualSprites = (IndividualSprite ***) calloc (x_num_sprites, sizeof (Sprite **));
        for (u8 i = 0; i < x_num_sprites; i++)
            spriteSheet->individualSprites[i] = (IndividualSprite **) calloc (y_num_sprites, sizeof (Sprite *));

        for (u8 y = 0; y < y_num_sprites; y++) {
            for (u8 x = 0; x < x_num_sprites; x++) {
                IndividualSprite *new_sprite = (IndividualSprite *) malloc (sizeof (Sprite));
                new_sprite->col = x;
                new_sprite->row = y;
                spriteSheet->individualSprites[x][y] = new_sprite;
            }
        }
    }

}
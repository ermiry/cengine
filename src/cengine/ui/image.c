#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>

#include "cengine/types/types.h"
#include "cengine/types/vector2d.h"

#include "cengine/sprites.h"
#include "cengine/renderer.h"
#include "cengine/input.h"
#include "cengine/timer.h"

#include "cengine/ui/types/types.h"
#include "cengine/ui/ui.h"
#include "cengine/ui/image.h"

static Image *ui_image_new (void) {

    Image *image = (Image *) malloc (sizeof (Image));
    if (image) {
        memset (image, 0, sizeof (Image));

        image->ui_element = NULL;

        image->texture = NULL;
        image->texture_src_rect = NULL;

        image->sprite = NULL;
        image->sprite_sheet = NULL;

        image->active = NULL;
        image->pressed = NULL;
        image->action = NULL;
        image->args = NULL;

        image->overlay_texture = NULL;
        image->selected_texture = NULL;

        image->double_click_timer = NULL;
        image->double_click_action = NULL;
        image->double_click_args = NULL;
        image->double_click_delay = IMAGE_DEFAULT_DOUBLE_CLICK_DELAY;
    }

    return image;

}

void ui_image_delete (void *image_ptr) {

    if (image_ptr) {
        Image *image = (Image *) image_ptr;

        image->ui_element = NULL;

        if (image->texture && !image->texture_reference) {
            SDL_DestroyTexture (image->texture);
            // texture_destroy (renderer_get_by_name ("main"), image->texture);
        }

        if (image->texture_src_rect) free (image->texture_src_rect);

        if (image->ref_sprite) {
            image->sprite = NULL;
            image->sprite_sheet = NULL;
        }

        else {
            sprite_destroy (image->sprite);
            sprite_sheet_destroy (image->sprite_sheet);
        }

        if (image->overlay_texture && !image->overlay_reference) 
            SDL_DestroyTexture (image->overlay_texture);

        if (image->selected_texture && !image->selected_reference)
            SDL_DestroyTexture (image->selected_texture);

        timer_destroy (image->double_click_timer);

        free (image);
    }

}

// sets the image's UI position
void ui_image_set_pos (Image *image, UIRect *ref_rect, UIPosition pos, Renderer *renderer) {

    if (image) ui_transform_component_set_pos (image->ui_element->transform, renderer, ref_rect, pos, false);

}

// directly sets the image's texture
void ui_image_set_texture (Image *image, SDL_Texture *texture) {

    if (image && texture) {
        // detsroy prevuis texture
        if (image->texture) {
            if (!image->texture_reference) {
                SDL_DestroyTexture (image->texture);
                image->texture = NULL;
            }
        }

        image->texture = texture;
    }

}

// sets the image's texture using a refrence to another texture; when the image gets destroyted,
// the texture won't be deleted
void ui_image_set_texture_ref (Image *image, SDL_Texture *texture_ref) {

    if (image && texture_ref) {
        ui_image_set_texture (image, texture_ref);
        image->texture_reference = true;
    }

}

// sets the image's texture's source rect (used to give an offset to the texture)
void ui_image_set_texture_src_rect (Image *image,
    int x, int y, int w, int h) {

    if (image) {
        if (image->texture_src_rect) free (image->texture_src_rect);

        image->texture_src_rect = (SDL_Rect *) malloc (sizeof (SDL_Rect));
        if (image->texture_src_rect) {
            image->texture_src_rect->x = x;
            image->texture_src_rect->y = y;
            image->texture_src_rect->w = w;
            image->texture_src_rect->h = h;
        }
    }

}

// sets the image's render dimensions
void ui_image_set_dimensions (Image *image, unsigned int width, unsigned int height) {

    if (image) {
        image->ui_element->transform->rect.w = width;
        image->ui_element->transform->rect.h = height;
    }

}

// sets the image's scale factor
void ui_image_set_scale (Image *image, int x_scale, int y_scale) {

    if (image) {
        image->ui_element->transform->x_scale = x_scale;
        image->ui_element->transform->y_scale = y_scale;

        image->ui_element->transform->rect.w *= image->ui_element->transform->x_scale;
        image->ui_element->transform->rect.h *= image->ui_element->transform->y_scale;
    }

}

// sets the image's sprite to be rendered and loads its
// returns 0 on success loading sprite, 1 on error
u8 ui_image_set_sprite (Image *image, Renderer *renderer, const char *filename) {

    u8 retval = 1;

    if (image && renderer && filename) {
        if (!image->ref_sprite) sprite_destroy (image->sprite);

        image->sprite = sprite_load (filename, renderer);
        if (image->sprite) {
            image->ui_element->transform->rect.w = image->sprite->w;
            image->ui_element->transform->rect.h = image->sprite->h;
            retval = 0;
        }
    }

    return retval;

}

// sets the image's sprite sheet to be rendered and loads it
// returns 0 on success loading sprite sheet, 1 on error
u8 ui_image_set_sprite_sheet (Image *image, Renderer *renderer, const char *filename) {

    u8 retval = 1;

    if (image && renderer && filename) {
        if (!image->ref_sprite) sprite_sheet_destroy (image->sprite_sheet);

        image->sprite_sheet = sprite_sheet_load (filename, renderer);
        if (image->sprite_sheet) retval = 0;
    }

    return retval;

}

// sets the image's sprite from a refrence from an already loaded sprite
void ui_image_ref_sprite (Image *image, Sprite *sprite) {

    if (image && sprite) {
        if (!image->ref_sprite) sprite_destroy (image->sprite);

        image->sprite = sprite;
        image->ref_sprite = true;
    }

}

// sets the image's sprite sheet from a refrence from an already loaded sprite sheet
void ui_image_ref_sprite_sheet (Image *image, SpriteSheet *sprite_sheet) {

    if (image && sprite_sheet) {
        if (!image->ref_sprite) sprite_sheet_destroy (image->sprite_sheet);

        image->sprite_sheet = sprite_sheet;
        image->ref_sprite = true;
    }

}

// sets the image's sprite sheet offset
void ui_image_set_sprite_sheet_offset (Image *image, u32 x_offset, u32 y_offset) {

    if (image) {
        image->x_sprite_offset = x_offset;
        image->y_sprite_offset = y_offset;
    }

}

// sets the image's outline colour
void ui_image_set_ouline_colour (Image *image, RGBA_Color colour) {

    if (image) {
        image->outline = true;
        image->outline_colour = colour;
    }

}

// sets the image's outline scale
void ui_image_set_outline_scale (Image *image, float x_scale, float y_scale) {

    if (image) {
        image->outline_scale_x = x_scale;
        image->outline_scale_y = y_scale;
    }

}

// removes the ouline form the image
void ui_image_remove_outline (Image *image) {

    if (image) {
        memset (&image->outline_colour, 0, sizeof (RGBA_Color));
        image->outline = false;
    }

}

// sets the image to be active depending on value
// action listerner working
void ui_image_set_active (Image *image, bool active) {

    if (image) image->active = active;

}

// toggles the image to be active or not
// action listerner working
void ui_image_toggle_active (Image *image) {

    if (image) image->active = !image->active;

}

// sets an action to be triggered when the image is clicked
void ui_image_set_action (Image *image, Action action, void *args) {

    if (image) {
        image->action = action;
        image->args = args;
    } 

}

// sets an action to be executed if double click is dected
void ui_image_set_double_click_action (Image *image, Action action, void *args) {

    if (image) {
        image->double_click_action = action;
        image->double_click_args = args;
    }

}

// sets the max delay between two clicks to count as a double click (in mili secs), the default value is 500
void ui_image_set_double_click_delay (Image *image, u32 double_click_delay) {

    if (image) image->double_click_delay = double_click_delay;

}

// sets an overlay to the image that only renders when you hover the image
void ui_image_set_overlay (Image *image, Renderer *renderer, RGBA_Color color) {

    if (image) {
        if (image->overlay_texture) {
            if (!image->overlay_reference) {
                SDL_DestroyTexture (image->overlay_texture);
                image->overlay_texture = NULL;
            }
        }

        render_complex_transparent_rect (renderer, &image->overlay_texture, &image->ui_element->transform->rect, color); 
        image->overlay_reference = false;   
    }

}

// sets an overlay to the image that only renders when you hover the image
// you need to pass a reference to the texture
void ui_image_set_overlay_ref (Image *image, SDL_Texture *overlay_ref) {

    if (image && overlay_ref) {
        if (image->overlay_texture) {
            if (!image->overlay_reference) {
                SDL_DestroyTexture (image->overlay_texture);
                image->overlay_texture = NULL;
            }
        }

        image->overlay_texture = overlay_ref;
        image->overlay_reference = true;
    }

}

// removes the overlay from the image
void ui_image_remove_overlay (Image *image) {

    if (image) {
        if (image->overlay_texture) {
            if (!image->overlay_reference) {
                SDL_DestroyTexture (image->overlay_texture);
            }

            image->overlay_texture = NULL;
            image->overlay_reference = false;
        }
    }

}

// sets an overlay to the image that only renders when you select the image (1 left click)
void ui_image_set_selected (Image *image, Renderer *renderer, RGBA_Color color) {

    if (image) {
        if (image->selected_texture) {
            if (!image->selected_reference) {
                SDL_DestroyTexture (image->selected_texture);
                image->selected_texture = NULL;
            }
        }

        render_complex_transparent_rect (renderer, &image->selected_texture, &image->ui_element->transform->rect, color); 
        image->selected_reference = false;   
    }

}

// sets an overlay to the image that only renders when you select the image
// you need to pass a reference to the texture
void ui_image_set_selected_ref (Image *image, SDL_Texture *selected_ref) {

    if (image && selected_ref) {
        if (image->selected_texture) {
            if (!image->selected_reference) {
                SDL_DestroyTexture (image->selected_texture);
                image->selected_texture = NULL;
            }
        }

        image->selected_texture = selected_ref;
        image->selected_reference = true;
    }

}

// removes the select overlay from the image
void ui_image_remove_selected (Image *image) {

    if (image) {
        if (image->selected_texture) {
            if (!image->selected_reference) {
                SDL_DestroyTexture (image->selected_texture);
            }

            image->selected_texture = NULL;
            image->selected_reference = false;
        }
    }

}

static Image *ui_image_create_common (Renderer *renderer) {

    Image *image = NULL;

    UIElement *ui_element = ui_element_create (renderer->ui, UI_IMAGE);
    if (ui_element) {
        image = ui_image_new ();
        if (image) {
            image->ui_element = ui_element;
            ui_element->element = image;

            image->outline_scale_x = image->outline_scale_y = 1;

            image->double_click_timer = timer_new ();
        }
    }

    return image;

}

// creates a new image to be displayed from a constant source, like using a sprite loaded from a file
// x and y for position
// w and h for dimensions
Image *ui_image_create (u32 x, u32 y, u32 w, u32 h, Renderer *renderer) {

    Image *image = ui_image_create_common (renderer);
    if (image) {
        ui_transform_component_set_values (image->ui_element->transform, x, y, w, h);
    }

    return image;

}

// FIXME: 17/04/2020 -- only loading jpeg images
u8 ui_image_update_texture_from_mem (Image *image, Renderer *renderer, void *mem, int mem_size) {

    u8 retval = 1;

    if (image && mem) {
        SDL_RWops *rw = SDL_RWFromConstMem (mem, mem_size);
        if (rw) {
            // printf ("Is jpeg? %d\n", IMG_isJPG (rw));
            // SDL_Surface *surface = IMG_Load_RW (rw, 1);
            SDL_Surface *surface = IMG_LoadTyped_RW (rw, 0, "JPG");
            if (surface) {
                // printf ("Pixel format: %s\n", SDL_GetPixelFormatName (surface->format->format));
                // Uint32 format = 0;
                // SDL_QueryTexture (image->texture, &format, NULL, NULL, NULL);
                // printf ("%s\n", SDL_GetPixelFormatName (format));
                // printf ("surface pitch: %d\n", surface->pitch);

                if (image->texture) texture_destroy (renderer, image->texture);

                texture_create_from_surface (renderer, &image->texture, surface);

                retval = 0;
            }

            SDL_FreeRW (rw);
        }
    }

    return retval;

}

// manually creates a streaming access texture, usefull for constant updates
u8 ui_image_create_streaming_texture (Image *image, Renderer *renderer, Uint32 sdl_pixel_format) {

    u8 retval = 1;

    if (image && renderer) {
        image->texture = SDL_CreateTexture (renderer->renderer, sdl_pixel_format,
            SDL_TEXTUREACCESS_STREAMING, image->ui_element->transform->rect.w, image->ui_element->transform->rect.h);
        if (image->texture) retval = 0;
    }

    return retval;

}

// FIXME: 27/01/2020 -- 1:05 --video memory leak -- or at least memory is filling up preatty quickly
// TODO: 27/01/2020 -- 00:17 -- refernce this again https://wiki.libsdl.org/SDL_UpdateTexture
// updates the streaming texture using an in memory buffer representing an image
// NOTE: buffer is not freed
u8 ui_image_update_streaming_texture_mem (Image *image, Renderer *renderer, void *mem, int mem_size) {

    u8 retval = 1;

    if (image && mem) {
        SDL_RWops *rw = SDL_RWFromConstMem (mem, mem_size);
        if (rw) {
            // printf ("Is jpeg? %d\n", IMG_isJPG (rw));
            // SDL_Surface *surface = IMG_Load_RW (rw, 1);
            SDL_Surface *surface = IMG_LoadTyped_RW (rw, 0, "JPG");
            if (surface) {
                // printf ("Pixel format: %s\n", SDL_GetPixelFormatName (surface->format->format));
                // Uint32 format = 0;
                // SDL_QueryTexture (image->texture, &format, NULL, NULL, NULL);
                // printf ("%s\n", SDL_GetPixelFormatName (format));
                // printf ("surface pitch: %d\n", surface->pitch);

                // SDL_DestroyTexture (image->texture);
                // texture_create_from_surface (renderer, &image->texture, surface);
                texture_update (renderer, &image->texture, surface);

                retval = 0;
            }

            SDL_FreeRW (rw);
        }
    }

    return retval;

}

// creates an image that is ment to be updated directly and constantly using its texture
// usefull for streaming video
// x and y for position
// w and h for dimensions
Image *ui_image_create_dynamic (u32 x, u32 y, u32 w, u32 h, Renderer *renderer) {

    Image *image = ui_image_create_common (renderer);
    if (image) ui_transform_component_set_values (image->ui_element->transform, x, y, w, h);

    return image;

}

// draws the image to the screen
void ui_image_draw (Image *image, Renderer *renderer) {

    if (image && renderer) {
        if (SDL_HasIntersection (&image->ui_element->transform->rect, &renderer->window->screen_rect)) {
            if (image->texture) {
                SDL_RenderCopyEx (renderer->renderer, image->texture, 
                    image->texture_src_rect, &image->ui_element->transform->rect, 
                    0, 0, (const SDL_RendererFlip) image->flip);
            }

            else {
                if (image->sprite) {
                    SDL_RenderCopyEx (renderer->renderer, image->sprite->texture, 
                        &image->sprite->src_rect, &image->ui_element->transform->rect, 
                        0, 0, (const SDL_RendererFlip) image->flip);
                }
                
                else if (image->sprite_sheet) {
                    image->sprite_sheet->src_rect.x = image->sprite_sheet->sprite_w * image->x_sprite_offset;
                    image->sprite_sheet->src_rect.y = image->sprite_sheet->sprite_h * image->y_sprite_offset;

                    SDL_RenderCopyEx (renderer->renderer, image->sprite_sheet->texture, 
                        &image->sprite_sheet->src_rect, &image->ui_element->transform->rect, 
                        0, 0, (const SDL_RendererFlip) image->flip);
                }
            }

            // render the outline border
            if (image->outline) 
                render_basic_outline_rect (renderer, &image->ui_element->transform->rect, image->outline_colour, 
                    image->outline_scale_x, image->outline_scale_y);

            // check for action listener
            if (image->active) {
                if (renderer->window->mouse) {
                    // check if the mouse is in the image
                    if (mousePos.x >= image->ui_element->transform->rect.x + image->ui_element->abs_offset_x && mousePos.x <= (image->ui_element->transform->rect.x + image->ui_element->transform->rect.w + image->ui_element->abs_offset_x) && 
                        mousePos.y >= image->ui_element->transform->rect.y + image->ui_element->abs_offset_y && mousePos.y <= (image->ui_element->transform->rect.y + image->ui_element->transform->rect.h + image->ui_element->abs_offset_y)) {
                        renderer->ui->ui_element_hover = image->ui_element;
                            
                        if (image->overlay_texture && !image->selected) {
                            SDL_RenderCopyEx (renderer->renderer, image->overlay_texture, 
                                NULL, &image->ui_element->transform->rect, 
                                0, 0, (const SDL_RendererFlip) image->flip);
                        }

                        // check if the user pressed the left button over the image
                        if (input_get_mouse_button_state (MOUSE_LEFT)) {
                            image->pressed = true;
                        }
                        
                        else if (!input_get_mouse_button_state (MOUSE_LEFT)) {
                            if (image->pressed) {
                                if (!image->one_click) {
                                    image->one_click = true;
                                    timer_start (image->double_click_timer);
                                    image->selected = !image->selected;
                                    if (image->action) image->action (image->args);
                                    // printf ("One click!\n");
                                }

                                else {
                                    u32 ticks = timer_get_ticks (image->double_click_timer);
                                    if (ticks <= image->double_click_delay) {
                                        image->one_click = false;
                                        if (image->double_click_action) 
                                            image->double_click_action (image->double_click_args);

                                        // image->selected = !image->selected;
                                        // printf ("Double click!\n");
                                    }

                                    else {
                                        image->one_click = true;
                                        timer_start (image->double_click_timer);
                                        image->selected = !image->selected;
                                        if (image->action) image->action (image->args);
                                        // printf ("One click again!\n");
                                    }
                                }
                                
                                image->pressed = false;
                            }
                        }
                    }
                
                    else image->pressed = false;
                }
                
                else image->pressed = false;
            }

            if (image->selected && image->selected_texture) {
                SDL_RenderCopyEx (renderer->renderer, image->selected_texture, 
                    NULL, &image->ui_element->transform->rect, 
                    0, 0, (const SDL_RendererFlip) image->flip);
            }

            renderer->render_count += 1;
        }
    }

}
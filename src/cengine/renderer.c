#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"
#include "cengine/collections/queue.h"

#include "cengine/renderer.h"
#include "cengine/window.h"
#include "cengine/textures.h"
#include "cengine/threads/thread.h"

#include "cengine/manager/manager.h"

#include "cengine/game/go.h"
#include "cengine/game/camera.h"

#include "cengine/ui/ui.h"

#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

static u64 next_renderer_id = 0;

SurfaceTexture *surface_texture_new (SDL_Surface *surface, SDL_Texture **texture) {

    SurfaceTexture *st = (SurfaceTexture *) malloc (sizeof (SurfaceTexture));
    if (st) {
        st->surface = surface;
        st->texture = texture;
    }

    return st;

}

void surface_texture_delete (void *st_ptr) {

    if (st_ptr) {
        SurfaceTexture *st = (SurfaceTexture *) st_ptr;
        if (st->surface) SDL_FreeSurface (st->surface);
        st->texture = NULL;

        free (st);
    }

}

#pragma region Renderer

DoubleList *renderers = NULL;

int renderer_window_attach (Renderer *renderer, Uint32 render_flags, int display_idx,
    const char *window_title, WindowSize window_size, Uint32 window_flags);

static Renderer *renderer_new (void) {

    Renderer *renderer = (Renderer *) malloc (sizeof (Renderer));
    if (renderer) {
        memset (renderer, 0, sizeof (Renderer));
        renderer->name = NULL;
        renderer->textures_queue = NULL;

        renderer->renderer = NULL;
        renderer->window = NULL;

        renderer->ui = NULL;

        renderer->update = NULL;
        renderer->update_args = NULL;
    }

    return renderer;

}

void renderer_delete (void *ptr) {

    if (ptr) {
        Renderer *renderer = (Renderer *) ptr;

        str_delete (renderer->name);
        if (renderer->renderer) SDL_DestroyRenderer (renderer->renderer);

        if (renderer->textures_queue) 
            queue_destroy_complete (renderer->textures_queue, surface_texture_delete);

        ui_delete (renderer->ui);

        free (renderer);
    }

}

static int renderer_comparator (const void *a, const void *b) {

    if (a && b) {
        Renderer *ren_a = (Renderer *) a;
        Renderer *ren_b = (Renderer *) b;

        if (ren_a->id < ren_b->id) return -1;
        if (ren_a->id == ren_b->id) return 0;
        else return 1;
    }

    return -1;

}

// gets the renderer by its name
Renderer *renderer_get_by_name (const char *name) {

    Renderer *retval = NULL;

    if (name) {
        Renderer *renderer = NULL;
        for (ListElement *le = dlist_start (renderers); le; le = le->next) {
            renderer = (Renderer *) le->data;
            if (!strcmp (renderer->name->str, name)) {
                retval = renderer;
                break;
            }
        }
    }

    return retval;

}

// creates a new empty renderer without a window attached to it
Renderer *renderer_create_empty (const char *name, int display_idx) {

    Renderer *renderer = renderer_new ();
    if (renderer) {
        renderer->id = next_renderer_id;
        next_renderer_id += 1;

        renderer->name = name ? str_new (name) : NULL;
        // renderer->display_index = display_idx;
        renderer->textures_queue = queue_create ();
        renderer->bg_loading_factor = DEFAULT_BG_LOADING_FACTOR;

        renderer->ui = ui_create ();

        dlist_insert_after (renderers, dlist_end (renderers), renderer);
    }

    return renderer;

}

// creates a new renderer with a window attached to it
Renderer *renderer_create_with_window (const char *name, int display_idx,
    Uint32 render_flags,
    const char *window_title, WindowSize window_size, Uint32 window_flags) {

    Renderer *renderer = renderer_create_empty (name, display_idx);
    if (renderer) {
        renderer->render_flags = render_flags;
        renderer_window_attach (renderer, render_flags, display_idx,
            window_title, window_size, window_flags);
    }

    return renderer;

}

// attaches a new window to a renderer
// creates a new window and then a new render (SDL_Renderer) for it
// retunrs 0 on success, 1 on error
int renderer_window_attach (Renderer *renderer, Uint32 render_flags, int display_idx,
    const char *window_title, WindowSize window_size, Uint32 window_flags) {

    int retval = 0;

    if (renderer) {
        renderer->window = window_create (window_title, window_size, window_flags, display_idx);
        if (renderer->window) {
            renderer->window->renderer = renderer;

            // SDL_CreateRenderer (main_window, 0, SDL_RENDERER_SOFTWARE | SDL_RENDERER_ACCELERATED);
            renderer->renderer = SDL_CreateRenderer (renderer->window->window, renderer->window->display_index, render_flags);
            if (renderer->renderer) {
                renderer->thread_id = pthread_self ();
                // printf ("Renderer created in thread: %ld\n", renderer->thread_id);

                SDL_SetRenderDrawColor (renderer->renderer, 0, 0, 0, 255);
                SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "0");
                SDL_RenderSetLogicalSize (renderer->renderer, 
                    renderer->window->window_size.width, renderer->window->window_size.height);
                
                retval = 0;
            }

            else {
                cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create renderer!"); 
                if (renderer->window) window_delete (renderer->window);
                renderer->window = NULL;
            }
        }

        else cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create window!"); 
    }

    return retval;

}

void renderer_queue_push (Renderer *renderer, SurfaceTexture *st) {

    if (renderer) {
        if (renderer->textures_queue) {
            queue_put (renderer->textures_queue, st);
        }
    }

}

// sets how many textures the renderer can create in the background every loop
// example: if you have a frame rate of 30, the default loading factor is 1,
// so you will load 30 textures in a second, 1 for each frame
void renderer_set_background_texture_loading_factor (Renderer *renderer, u32 bg_loading_factor) {

    if (renderer) renderer->bg_loading_factor = bg_loading_factor;

}

// sets an action to executed on every renderer update
// you can use this if you want to perform action son ui elements, like checking for 
// the current ui element under the mouse using the ui_element_hover in UI
void renderer_set_update (Renderer *renderer, Action update, void *update_args) {

    if (renderer) {
        renderer->update = update;
        renderer->update_args = update_args;
    }

}

#pragma endregion

/*** Layers ***/

#pragma region Layers

DoubleList *gos_layers = NULL;              // render layers for the gameobjects

Layer *layer_get_by_pos (DoubleList *layers, int pos) {

    Layer *layer = NULL;

    if (layers) {
        for (ListElement *le = dlist_start (layers); le; le = le->next) {
            layer = (Layer *) le->data;
            if (layer->pos == pos) return layer;
        }
    }

    return layer;

}

Layer *layer_get_by_name (DoubleList *layers, const char *name) {

    Layer *layer = NULL;

    if (name && layers) {
        for (ListElement *le = dlist_start (layers); le; le = le->next) {
            layer = (Layer *) le->data;
            if (!strcmp (layer->name->str, name)) return layer;
        }
    }

    return layer;

}

static Layer *layer_new (DoubleList *layers, const char *name, int pos, bool gos) {

    Layer *layer = (Layer *) malloc (sizeof (Layer));
    if (layer) {
        if (name) layer->name = str_new (name);
        else layer->name = NULL;

        layer->elements = gos ? dlist_init (game_object_destroy_dummy, game_object_comparator) : 
            dlist_init (ui_element_delete_dummy, ui_element_comparator);

        if (pos >= 0) {
            layer->pos = pos;

            // check for a layer with that pos
            // Layer *l = layer_get_by_pos (layers, pos);
            // if (l) {
            //     printf ("hola!!\n");
            //     pos += 1;
            //     l->pos = pos;

            //     // we need to update the other layers pos
            //     Layer *update_layer = NULL;
            //     do {
            //         update_layer = layer_get_by_pos (layers, pos);
            //         if (update_layer) {
            //             update_layer->pos = pos;
            //             pos += 1;
            //         }
            //     } while (update_layer);
            // }
        }
    }

    return layer;

}

static void layer_delete (void *ptr) {

    if (ptr) {
        Layer *layer = (Layer *) ptr;
        str_delete (layer->name);
        dlist_delete (layer->elements);

        free (layer);
    }

}

static int layer_comparator (const void *one, const void *two) {

    if (one && two) {
        Layer *layer_one = (Layer *) one;
        Layer *layer_two = (Layer *) two;

        if (layer_one->pos < layer_two->pos) return -1;
        else if (layer_one->pos == layer_two->pos) return 0;
        else return 1;
    }

}

// creates a new layer; 
// takes the layer name and the layer pos, -1 for last layer
// pos 0 renders first
// returns 0 on success, 1 on error
int layer_create (DoubleList *layers, const char *name, int pos, bool gos) {

    int retval = 1;

    if (name) {
        Layer *l = layer_new (layers, name, pos, gos);
        dlist_insert_after (layers, dlist_end (layers), l);

        // after we have inserted a new layer, we want to sort the layers
        // to render in the correct order
        retval = dlist_sort (layers);
    }

    return retval;

}

// adds an element to a layer
// returns 0 on succes, 1 on error
int layer_add_element (Layer *layer, void *ptr) {

    int retval = 1;

    if (layer && ptr) {
        if (!dlist_insert_after (layer->elements, dlist_end (layer->elements), ptr)) {
            printf ("Added element to layer %s\n", layer->name->str);
        }

        retval = 0;
    }

    return retval;

}

// adds an element to a layer that is gotten by its name 
// returns 0 on succes, 1 on error
int layer_add_element_by_name (DoubleList *layers, const char *layer_name, void *ptr) {

    int retval = 1;

    if (layer_name && ptr) {
        Layer *layer = layer_get_by_name (layers, layer_name);
        retval = layer_add_element (layer, ptr);
    }

    return retval;

}

// removes an element from a layer
// returns 0 on succes, 1 on error
int layer_remove_element (Layer *layer, void *ptr) {

    int retval = 0;

    if (layer && ptr) {
        void *element = dlist_remove_element (layer->elements, dlist_get_element (layer->elements, ptr));
        // void *element = dlist_remove (layer->elements, ptr);
        retval = element ? 0 : 1;
    }

    return retval;

}

// removes an element from a layer taht is gotten by its name
// returns 0 on succes, 1 on error
int layer_remove_element_by_name (DoubleList *layers, const char *layer_name, void *ptr) {

    int retval = 0;

    if (layer_name && ptr) {
        Layer *layer = layer_get_by_name (layers, layer_name);
        retval = layer_remove_element (layer, ptr);
    }

    return retval;

}

// init the ui elements layers
DoubleList *ui_layers_init (void) {

    DoubleList *ui_elements_layers = dlist_init (layer_delete, layer_comparator);
    if (ui_elements_layers) {
        // add the default layers to the list
        Layer *back_layer = layer_new (ui_elements_layers, "back", 0, false);
        dlist_insert_after (ui_elements_layers, dlist_end (ui_elements_layers), back_layer);

        Layer *middle_layer = layer_new (ui_elements_layers, "middle", 1, false);
        dlist_insert_after (ui_elements_layers, dlist_end (ui_elements_layers), middle_layer);

        Layer *top_layer = layer_new (ui_elements_layers, "top", 2, false);
        dlist_insert_after (ui_elements_layers, dlist_end (ui_elements_layers), top_layer);
    }

    return ui_elements_layers;

}

// FIXME: 22/11/2019 -- we are not calling this any more!!
static u8 layers_init (void) {

    // ini the game objects layers
    gos_layers = dlist_init (layer_delete, layer_comparator);
    if (gos_layers) {
        // add the default layer to the list
        Layer *default_layer = layer_new (gos_layers, "default", 0, true);
        dlist_insert_after (gos_layers, dlist_end (gos_layers), default_layer);
    }

    return (gos_layers ? 0 : 1);

}

// FIXME: 22/11/2019 -- we are not calling this any more!!
static void layers_end (void) { 
    
    dlist_delete (gos_layers);
    gos_layers = NULL;

    // dlist_delete (ui_elements_layers);
    // ui_elements_layers = NULL;
    
}

#pragma endregion

#pragma region Surfaces

// creates a new empty surface
SDL_Surface *surface_create (int width, int height) {

    uint32_t rmask , gmask , bmask , amask ;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    #else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
    #endif

    return SDL_CreateRGBSurface (0, width, height, 32, rmask, gmask, bmask, amask);

}

// loads an image into a new surface
SDL_Surface *surface_load_image (const char *filename) {

    return filename ? IMG_Load (filename) : NULL;

}

// wrapper function to destroy a sdl surface
void surface_delete (SDL_Surface *surface) { if (surface) SDL_FreeSurface (surface); }

#pragma endregion

#pragma region Basic

// renders a dot
void render_basic_dot (Renderer *renderer, int x, int y, SDL_Color color,
    float x_scale, float y_scale) {

    if (renderer) {
        float original_scale_x = 0;
        float original_scale_y = 0;

        SDL_RenderGetScale (renderer->renderer, &original_scale_x, &original_scale_y);

        SDL_RenderSetScale (renderer->renderer, x_scale, y_scale);
        SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);

        SDL_RenderDrawPoint (renderer->renderer, x / x_scale, y / y_scale);

        SDL_RenderSetScale (renderer->renderer, original_scale_x, original_scale_y);
    }

}

// renders a horizontal line of dots
void render_basic_dot_line_horizontal (Renderer *renderer, int start, int end, int y, int offset, SDL_Color color,
    float x_scale, float y_scale) {

    if (renderer) {
        float original_scale_x = 0;
        float original_scale_y = 0;

        SDL_RenderGetScale (renderer->renderer, &original_scale_x, &original_scale_y);

        SDL_RenderSetScale (renderer->renderer, x_scale, y_scale);
        SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);

        int scale_offset = (offset * x_scale);
        for (unsigned int i = start; i < end; i += scale_offset)
            SDL_RenderDrawPoint (renderer->renderer, i / x_scale, y / y_scale);

        SDL_RenderSetScale (renderer->renderer, original_scale_x, original_scale_y);
    }

}

// renders a vertical line of dots
void render_basic_dot_line_vertical (Renderer *renderer, int start, int end, int x, int offset, SDL_Color color,
    float x_scale, float y_scale) {

    if (renderer) {
        float original_scale_x = 0;
        float original_scale_y = 0;

        SDL_RenderGetScale (renderer->renderer, &original_scale_x, &original_scale_y);

        SDL_RenderSetScale (renderer->renderer, x_scale, y_scale);
        SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);

        int scale_offset = (offset * x_scale);
        for (unsigned int i = start; i < end; i += scale_offset)
            SDL_RenderDrawPoint (renderer->renderer, x / x_scale, i / y_scale);

        SDL_RenderSetScale (renderer->renderer, original_scale_x, original_scale_y);
    }

}

// renders a filled rect
void render_basic_filled_rect (Renderer *renderer, SDL_Rect *rect, SDL_Color color) {

    if (renderer && rect) {
        SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);        
        SDL_RenderFillRect (renderer->renderer, rect);
    }

}

// renders an outline rect
// scale works better with even numbers
void render_basic_outline_rect (Renderer *renderer, SDL_Rect *rect, SDL_Color color, float scale_x, float scale_y) {

    if (renderer && rect) {
        float original_scale_x = 0;
        float original_scale_y = 0;

        SDL_RenderGetScale (renderer->renderer, &original_scale_x, &original_scale_y);

        SDL_RenderSetScale (renderer->renderer, scale_x, scale_y);
        SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);      

        SDL_Rect temp_rect = { .x = rect->x / scale_x, .y = rect->y / scale_y, .w = rect->w / scale_x, .h = rect->h / scale_y };
        SDL_RenderDrawRect (renderer->renderer, &temp_rect);

        SDL_RenderSetScale (renderer->renderer, original_scale_x, original_scale_y);
    }

}

// renders a line
// scale works better with even numbers
void render_basic_line (Renderer *renderer, int x1, int x2, int y1, int y2, SDL_Color color, float scale_x, float scale_y) {

    if (renderer) {
        float original_scale_x = 0;
        float original_scale_y = 0;

        SDL_RenderGetScale (renderer->renderer, &original_scale_x, &original_scale_y);

        SDL_RenderSetScale (renderer->renderer, scale_x, scale_y);
        SDL_SetRenderDrawColor (renderer->renderer, 255, 255, 255, 255);        

        SDL_RenderDrawLine (renderer->renderer, x1 / scale_x, y1 / scale_y, x2 / scale_x, y2 / scale_y);

        SDL_RenderSetScale (renderer->renderer, original_scale_x, original_scale_y);
    }

}

#pragma endregion

#pragma region Complex

// renders a rect with transparency
void render_complex_transparent_rect (Renderer *renderer, SDL_Texture **texture, SDL_Rect *rect, SDL_Color color) {

    if (renderer && texture && rect) {
        SDL_Surface *surface = surface_create (rect->w, rect->h);
        if (surface) {
            (void) SDL_FillRect (surface, NULL, convert_rgba_to_hex (color.r, color.g, color.b, color.a));

            pthread_t thread_id = pthread_self ();
            // printf ("Loading texture in thread: %ld\n", thread_id);
            if (thread_id == renderer->thread_id) {
                // load texture as always
                *texture = SDL_CreateTextureFromSurface (renderer->renderer, surface);
                SDL_FreeSurface (surface);
            }

            else {
                // send image to renderer queue
                renderer_queue_push (renderer, surface_texture_new (surface, texture));
            }
        }
    }

}

#pragma endregion

#pragma region Render

// FIXME: we need to implement occlusion culling!
void render (Renderer *renderer) {

    if (renderer) {
        renderer->render_count = 0;

        // load any texture in background queue
        if (renderer->textures_queue->num_els > 0) {
            u32 count = 0;
            while (count < renderer->bg_loading_factor) {
                SurfaceTexture *st = NULL;
                queue_get (renderer->textures_queue, (void **) &st);
                if (st) {
                    *st->texture = SDL_CreateTextureFromSurface (renderer->renderer, st->surface);
                    surface_texture_delete (st);
                } 

                count++;
            }
        }

        SDL_SetRenderDrawColor (renderer->renderer, 0, 0, 0, 255);
        SDL_RenderClear (renderer->renderer);

        // FIXME: 20/11/2019 --- we are no longer creating the camera!!
        // render by layers
        // Layer *layer = NULL;
        // GameObject *go = NULL;
        // Transform *transform = NULL;
        // Graphics *graphics = NULL;
        // for (ListElement *layer_le = dlist_start (gos_layers); layer_le; layer_le = layer_le->next) {
        //     layer = (Layer *) layer_le->data;

        //     for (ListElement *le = dlist_start (layer->elements); le; le = le->next) {
        //         go = (GameObject *) le->data;

        //         transform = (Transform *) game_object_get_component (go, TRANSFORM_COMP);
        //         graphics = (Graphics *) game_object_get_component (go, GRAPHICS_COMP);
        //         if (transform && graphics) {
        //             if (graphics->multipleSprites) {
        //                 texture_draw_frame (main_camera, 
        //                     renderer,
        //                     graphics->spriteSheet, 
        //                     transform->position.x, transform->position.y, 
        //                     graphics->x_sprite_offset, graphics->y_sprite_offset,
        //                     graphics->flip);
        //             }
                    
        //             else {
        //                 texture_draw (main_camera, 
        //                     renderer,
        //                     graphics->sprite, 
        //                     transform->position.x, transform->position.y, 
        //                     graphics->flip);
        //             }
        //         }
        //     }
        // }

        ui_render (renderer);       // render ui elements

        SDL_RenderPresent (renderer->renderer);

        #ifdef CENGINE_DEBUG
        // printf ("Renderer: %s render count: %d\n", renderer->name->str, renderer->render_count);
        #endif
    }

}

#pragma endregion

#pragma region public

// inits cengine render capabilities
u8 render_init (void) {

    u8 errors = 0;

    // errors |= layers_init ();

    renderers = dlist_init (renderer_delete, renderer_comparator);
    u8 retval = renderers ? 0 : 1;

    errors |= retval;

    windows = dlist_init (window_delete, window_comparator);
    retval = windows ? 0 : 1;

    return errors;

}

void render_end (void) {

    // layers_end ();

    dlist_delete (renderers);

    dlist_delete (windows);

}

#pragma endregion
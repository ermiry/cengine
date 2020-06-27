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

SurfaceTexture *surface_texture_new (SDL_Surface *surface, SDL_Texture **texture, bool update) {

    SurfaceTexture *st = (SurfaceTexture *) malloc (sizeof (SurfaceTexture));
    if (st) {
        st->surface = surface;
        st->texture = texture;
        st->update = update;
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

        renderer->load_textures_queue = NULL;
        renderer->destroy_textures_queue = NULL;

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

        if (renderer->load_textures_queue)
            queue_delete (renderer->load_textures_queue);

        if (renderer->destroy_textures_queue)
            queue_delete (renderer->destroy_textures_queue);

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

        renderer->load_textures_queue = queue_create (surface_texture_delete);
        renderer->bg_loading_factor = DEFAULT_BG_LOADING_FACTOR;

        renderer->destroy_textures_queue = queue_create (texture_destroy_wrapper);
        renderer->bg_destroying_factor = DEFAULT_BG_DESTROYING_FACTOR;

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

                SDL_Rect viewport = { 
                    .x = 0, 
                    .y = 0, 
                    .w = (int) renderer->window->window_size.width, 
                    .h = (int) renderer->window->window_size.height 
                };

                memcpy (&renderer->previous_viewport, &viewport, sizeof (SDL_Rect));
                memcpy (&renderer->current_viewport, &viewport, sizeof (SDL_Rect));
                
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

void renderer_load_queue_push (Renderer *renderer, SurfaceTexture *st) {

    if (renderer) {
        if (renderer->load_textures_queue) {
            queue_push (renderer->load_textures_queue, st);
        }
    }

}

void renderer_destroy_queue_push (Renderer *renderer, SDL_Texture *texture) {

    if (renderer) {
        if (renderer->destroy_textures_queue) {
            queue_push (renderer->destroy_textures_queue, texture);
        }
    }

}

// sets how many textures the renderer can create in the background every loop
// example: if you have a frame rate of 30, the default loading factor is 1,
// so you will load 30 textures in a second, 1 for each frame
void renderer_set_background_texture_loading_factor (Renderer *renderer, u32 bg_loading_factor) {

    if (renderer) renderer->bg_loading_factor = bg_loading_factor;

}

// sets how many textures the renderer can destroy in the background every loop
void renderer_set_background_texture_destroying_factor (Renderer *renderer, u32 bg_destroying_factor) {

    if (renderer) renderer->bg_destroying_factor = bg_destroying_factor;

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

// sets the renderer's viewport to be of the specified size
void renderer_set_viewport (Renderer *renderer, u32 x, u32 y, u32 width, u32 height) {

    if (renderer) {
        memcpy (&renderer->previous_viewport, &renderer->current_viewport, sizeof (SDL_Rect));

        SDL_Rect viewport = { 
            .x = (int) x, 
            .y = (int) y, 
            .w = (int) width, 
            .h = (int) height 
        };

        SDL_RenderSetViewport (renderer->renderer, &viewport);

        // set the new viewport
        renderer->current_viewport.x = viewport.x;
        renderer->current_viewport.y = viewport.y;
        renderer->current_viewport.w = viewport.w;
        renderer->current_viewport.h = viewport.h;
    }

}

// sets the renderer's viewport to the previous size
void renderer_set_viewport_to_previous_size (Renderer *renderer) {

    if (renderer)     {
        renderer_set_viewport (renderer, 
            renderer->current_viewport.x, renderer->current_viewport.y, 
            renderer->current_viewport.w, renderer->current_viewport.h);
    }

}

// sets the renderer's viewport to be the size of the window
void renderer_set_viewport_to_window_size (Renderer *renderer) {

    if (renderer) {
        // SDL_Rect viewport = { 
        //     .x = 0, 
        //     .y = 0, 
        //     .w = renderer->window->window_size.width, 
        //     .h = renderer->window->window_size.height 
        // };

        renderer_set_viewport (renderer, 
            0, 0, 
            renderer->window->window_size.width, renderer->window->window_size.height);
    }

}

#pragma endregion

#pragma region Layers

DoubleList *gos_layers = NULL;              // render layers for the gameobjects

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

    return -1;

}

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
        retval = dlist_sort (layers, NULL);
    }

    return retval;

}

// adds an element to a layer
// returns 0 on succes, 1 on error
int layer_add_element (Layer *layer, void *ptr) {

    int retval = 1;

    if (layer && ptr) {
        retval = dlist_insert_after (layer->elements, dlist_end (layer->elements), ptr);
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
        // printf ("%s\n", layer->name->str);
        // void *element = dlist_remove_element (layer->elements, 
        //     dlist_get_element (layer->elements, ptr, NULL));
        void *element = dlist_remove (layer->elements, ptr, NULL);
        // if (element) printf ("hola!\n");
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

// draws a pixel in currently set color
// returns 0 on success, -1 on error
int render_basic_pixel (Renderer *renderer, int x, int y) {

    return renderer ? SDL_RenderDrawPoint (renderer->renderer, x, y) : -1;

}

// draws a pixel with blending enabled if a < 255
// returns 0 on success, -1 on failure
int render_basic_pixel_rgba (Renderer *renderer, 
    int x, int y,
    u8 r, u8 g, u8 b, u8 a) {

    if (renderer) {
        int result = 0;
        result |= SDL_SetRenderDrawBlendMode (renderer->renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
        result |= SDL_SetRenderDrawColor (renderer->renderer, r, g, b, a);
        result |= SDL_RenderDrawPoint (renderer->renderer, x, y);
        return result;
    }

    return -1;

}

// draws a pixel with blending enabled if a < 255
// the color value of the pixel to draw must be in format 0xRRGGBBAA
// returns 0 on success, -1 on failure
int render_basic_pixel_color (Renderer *renderer, int x, int y, u32 color) {

    if (renderer) {
        Uint8 *c = (Uint8 *) &color;
        return render_basic_pixel_rgba (renderer, 
            x, y,
            c[0], c[1], c[2], c[3]);
    }

    return -1;

}

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

// draws a box (filled rectangle) with blending
// returns 0 on success, -1 on failure
int render_basic_box (Renderer *renderer, int x1, int y1, int x2, int y2, SDL_Color color) {

    if (renderer) {
        int result;
        int tmp;
        SDL_Rect rect;

        if (x1 > x2) {
            tmp = x1;
            x1 = x2;
            x2 = tmp;
        }

        if (y1 > y2) {
            tmp = y1;
            y1 = y2;
            y2 = tmp;
	    }

        rect.x = x1;
        rect.y = y1;
        rect.w = x2 - x1 + 1;
        rect.h = y2 - y1 + 1;

        result = 0;
        result |= SDL_SetRenderDrawBlendMode (renderer->renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
        result |= SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);	
        result |= SDL_RenderFillRect (renderer->renderer, &rect);

        return result;
    }

    return -1;

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

        SDL_Rect temp_rect = { 
            .x = (int) (rect->x / scale_x), 
            .y = (int) (rect->y / scale_y), 
            .w = (int) (rect->w / scale_x), 
            .h = (int) (rect->h / scale_y)
        };
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

// draws a horizontal line with blending
// returns 0 on success, -1 on failure
int render_basic_horizontal_line (Renderer *renderer, 
    int x1, int x2, int y,
    SDL_Color color) {

    if (renderer) {
        int result = 0;
        result |= SDL_SetRenderDrawBlendMode (renderer->renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
        result |= SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);
        result |= SDL_RenderDrawLine (renderer->renderer, x1, y, x2, y);
        return result;
    }

    return -1;

}

// draws a vertical line with blending
// returns 0 on success, -1 on failure
int render_basic_vertical_line (Renderer *renderer,
    int x, int y1, int y2,
    SDL_Color color) {

    if (renderer) {
        int result = 0;
        result |= SDL_SetRenderDrawBlendMode (renderer->renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
        result |= SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);
        result |= SDL_RenderDrawLine (renderer->renderer, x, y1, x, y2);
        return result;
    }

    return -1;

}

#pragma endregion

#pragma region Complex

#include <math.h>

#ifndef M_PI
#define M_PI	3.1415926535897932384626433832795
#endif

#define DEFAULT_ELLIPSE_OVERSCAN	    4

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
                renderer_load_queue_push (renderer, surface_texture_new (surface, texture, false));
            }
        }
    }

}

// draws and arc with blending
// returns 0 on success, -1 on error
int render_complex_arc (Renderer *renderer, 
    int x, int y, int rad,
    int start, int end,
    SDL_Color color) {

    if (renderer) {
        int result;
        int cx = 0;
        int cy = rad;
        int df = 1 - rad;
        int d_e = 3;
        int d_se = -2 * rad + 5;
        int xpcx, xmcx, xpcy, xmcy;
        int ypcy, ymcy, ypcx, ymcx;
        Uint8 drawoct;
        int startoct, endoct, oct, stopval_start = 0, stopval_end = 0;
        double dstart, dend, temp = 0;

        if (rad < 0) return -1;

        if (rad == 0) return render_basic_pixel_rgba (renderer, 
            x, y, 
            color.r, color.g, color.b, color.a);

        drawoct = 0; 

        start %= 360;
        end %= 360;
        while (start < 0) start += 360;
        while (end < 0) end += 360;
        start %= 360;
        end %= 360;

        startoct = start / 45;
        endoct = end / 45;
        oct = startoct - 1;

        do {
            oct = (oct + 1) % 8;
            if (oct == startoct) {
                dstart = (double)start;
                switch (oct) 
                {
                case 0:
                case 3:
                    temp = sin(dstart * M_PI / 180.);
                    break;
                case 1:
                case 6:
                    temp = cos(dstart * M_PI / 180.);
                    break;
                case 2:
                case 5:
                    temp = -cos(dstart * M_PI / 180.);
                    break;
                case 4:
                case 7:
                    temp = -sin(dstart * M_PI / 180.);
                    break;
                }
                temp *= rad;
                stopval_start = (int)temp;

                if (oct % 2) drawoct |= (1 << oct);			/* this is basically like saying drawoct[oct] = true, if drawoct were a bool array */
                else		 drawoct &= 255 - (1 << oct);	/* this is basically like saying drawoct[oct] = false */
            }

            if (oct == endoct) {
                dend = (double)end;
                switch (oct) {
                case 0:
                case 3:
                    temp = sin(dend * M_PI / 180);
                    break;
                case 1:
                case 6:
                    temp = cos(dend * M_PI / 180);
                    break;
                case 2:
                case 5:
                    temp = -cos(dend * M_PI / 180);
                    break;
                case 4:
                case 7:
                    temp = -sin(dend * M_PI / 180);
                    break;
                }
                
                temp *= rad;
                stopval_end = (int)temp;

                /* and whether to draw in this octant initially */
                if (startoct == endoct)	{
                    if (start > end) {
                        drawoct = 255;
                    } else {
                        drawoct &= 255 - (1 << oct);
                    }
                } 
                else if (oct % 2) drawoct &= 255 - (1 << oct);
                else			  drawoct |= (1 << oct);
            } 
            else if (oct != startoct) {
                drawoct |= (1 << oct);
            }
        } while (oct != endoct);

        // set color
        result = 0;
        result |= SDL_SetRenderDrawBlendMode(renderer->renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
        result |= SDL_SetRenderDrawColor(renderer->renderer, color.r, color.g, color.b, color.a);

        // draw arc
        do {
            ypcy = y + cy;
            ymcy = y - cy;
            if (cx > 0) {
                xpcx = x + cx;
                xmcx = x - cx;

                if (drawoct & 4)  result |= render_basic_pixel (renderer, xmcx, ypcy);
                if (drawoct & 2)  result |= render_basic_pixel (renderer, xpcx, ypcy);
                if (drawoct & 32) result |= render_basic_pixel (renderer, xmcx, ymcy);
                if (drawoct & 64) result |= render_basic_pixel (renderer, xpcx, ymcy);
            } 
            
            else {
                if (drawoct & 96) result |= render_basic_pixel (renderer, x, ymcy);
                if (drawoct & 6)  result |= render_basic_pixel (renderer, x, ypcy);
            }

            xpcy = x + cy;
            xmcy = x - cy;
            if (cx > 0 && cx != cy) {
                ypcx = y + cx;
                ymcx = y - cx;
                if (drawoct & 8)   result |= render_basic_pixel (renderer, xmcy, ypcx);
                if (drawoct & 1)   result |= render_basic_pixel (renderer, xpcy, ypcx);
                if (drawoct & 16)  result |= render_basic_pixel (renderer, xmcy, ymcx);
                if (drawoct & 128) result |= render_basic_pixel (renderer, xpcy, ymcx);
            }
            
            else if (cx == 0) {
                if (drawoct & 24)  result |= render_basic_pixel (renderer, xmcy, y);
                if (drawoct & 129) result |= render_basic_pixel (renderer, xpcy, y);
            }

            if (stopval_start == cx) {
                if (drawoct & (1 << startoct)) drawoct &= 255 - (1 << startoct);		
                else						   drawoct |= (1 << startoct);
            }

            if (stopval_end == cx) {
                if (drawoct & (1 << endoct)) drawoct &= 255 - (1 << endoct);
                else						 drawoct |= (1 << endoct);
            }

            if (df < 0) {
                df += d_e;
                d_e += 2;
                d_se += 2;
            } 
            
            else {
                df += d_se;
                d_e += 2;
                d_se += 4;
                cy--;
            }
            
            cx++;
        } while (cx <= cy);

        return result;
    }

    return -1;

}

// draws rounded-corner rectangle (outline) with blending
// returns 0 on success, -1 on failure
int render_complex_rounded_rect (Renderer *renderer, 
    int x1, int y1, int x2, int y2, int rad,
    SDL_Color color) {

    if (renderer) {
        if (rad < 1) return -1;
        else {
            int result = 0;
            int tmp;
            int w, h;
            int xx1, xx2;
            int yy1, yy2;

            if (x1 > x2) {
                tmp = x1;
                x1 = x2;
                x2 = tmp;
            }

            if (y1 > y2) {
                tmp = y1;
                y1 = y2;
                y2 = tmp;
            }

            w = x2 - x1;
            h = y2 - y1;

            if ((rad * 2) > w) rad = w / 2;
            if ((rad * 2) > h) rad = h / 2;

            // draw corners
            xx1 = x1 + rad;
            xx2 = x2 - rad;
            yy1 = y1 + rad;
            yy2 = y2 - rad;
            result |= render_complex_arc(renderer, xx1, yy1, rad, 180, 270, color);
            result |= render_complex_arc(renderer, xx2, yy1, rad, 270, 360, color);
            result |= render_complex_arc(renderer, xx1, yy2, rad,  90, 180, color);
            result |= render_complex_arc(renderer, xx2, yy2, rad,   0,  90, color);

            // draw lines
            if (xx1 <= xx2) {
                result |= render_basic_horizontal_line(renderer, xx1, xx2, y1, color);
                result |= render_basic_horizontal_line(renderer, xx1, xx2, y2, color);
            }
            if (yy1 <= yy2) {
                result |= render_basic_vertical_line(renderer, x1, yy1, yy2, color);
                result |= render_basic_vertical_line(renderer, x2, yy1, yy2, color);
            }

            return result;
        }
    }

    return -1; 

}

// draws rounded-corner box (filled rectangle) with blending
// returns 0 on success, -1 on failure
int render_complex_rounded_box (Renderer *renderer, 
    int x1, int y1, int x2, int y2, int rad,
    SDL_Color color) {

    if (renderer) {
        if (rad > 0) {
            int result;
            int w, h, r2, tmp;
            int cx = 0;
            int cy = rad;
            int ocx = (int) 0xffff;
            int ocy = (int) 0xffff;
            int df = 1 - rad;
            int d_e = 3;
            int d_se = -2 * rad + 5;
            int xpcx, xmcx, xpcy, xmcy;
            int ypcy, ymcy, ypcx, ymcx;
            int x, y, dx, dy;

            if (x1 > x2) {
                tmp = x1;
                x1 = x2;
                x2 = tmp;
            }

            if (y1 > y2) {
                tmp = y1;
                y1 = y2;
                y2 = tmp;
            }

            w = x2 - x1 + 1;
	        h = y2 - y1 + 1;

            r2 = rad + rad;
            if (r2 > w) {
                rad = w / 2;
                r2 = rad + rad;
            }

            if (r2 > h) rad = h / 2;

            x = x1 + rad;
            y = y1 + rad;
            dx = x2 - x1 - rad - rad;
            dy = y2 - y1 - rad - rad;

            // set color
            result = 0;
            result |= SDL_SetRenderDrawBlendMode (renderer->renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
            result |= SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);

            // draw corners
            do {
                xpcx = x + cx;
                xmcx = x - cx;
                xpcy = x + cy;
                xmcy = x - cy;
                if (ocy != cy) {
                    if (cy > 0) {
                        ypcy = y + cy;
                        ymcy = y - cy;
                        result |= render_basic_horizontal_line (renderer, xmcx, xpcx + dx, ypcy + dy, color);
                        result |= render_basic_horizontal_line (renderer, xmcx, xpcx + dx, ymcy, color);
                    } 
                    else {
                        result |= render_basic_horizontal_line (renderer, xmcx, xpcx + dx, y, color);
                    }
                    ocy = cy;
                }

                if (ocx != cx) {
                    if (cx != cy) {
                        if (cx > 0) {
                            ypcx = y + cx;
                            ymcx = y - cx;
                            result |= render_basic_horizontal_line (renderer, xmcy, xpcy + dx, ymcx, color);
                            result |= render_basic_horizontal_line (renderer, xmcy, xpcy + dx, ypcx + dy, color);
                        } 
                        else {
                            result |= render_basic_horizontal_line (renderer, xmcy, xpcy + dx, y, color);
                        }
                    }
                    ocx = cx;
                }

                if (df < 0) {
                    df += d_e;
                    d_e += 2;
                    d_se += 2;
                } 
                
                else {
                    df += d_se;
                    d_e += 2;
                    d_se += 4;
                    cy--;
                }

                cx++;
            } while (cx <= cy);

            // inside
            // SDL_Rect inside_rect = { .x = x1, .y = y1 + rad + 1, x2, y2 - rad };
            if (dx > 0 && dy > 0) result |= render_basic_box (renderer, x1, y1 + rad + 1, x2, y2 - rad, color);

            return result;
        }
    }

    return -1;

}

// internal function to draw pixels or lines in 4 quadrants
// returns 0 on success, -1 on failure
static int render_complex_ellipse_draw_quadrants (Renderer *renderer, 
    int x, int y,
    int dx, int dy, 
    SDL_Color color, bool filled) {

    int result = -1;

    if (renderer) {
        int xpdx, xmdx;
        int ypdy, ymdy;

        if (dx == 0) {
            if (dy == 0) result |= render_basic_pixel (renderer, x, y);
            
            else {
                ypdy = y + dy;
                ymdy = y - dy;
                if (filled) result |= render_basic_vertical_line (renderer, x, ymdy, ypdy, color); 
                
                else {
                    result |= render_basic_pixel (renderer, x, ypdy);
                    result |= render_basic_pixel (renderer, x, ymdy);
                }
            }
        } 
        
        else {	
            xpdx = x + dx;
            xmdx = x - dx;
            ypdy = y + dy;
            ymdy = y - dy;
            if (filled) {
                result |= render_basic_vertical_line (renderer, xpdx, ymdy, ypdy, color);
                result |= render_basic_vertical_line (renderer, xmdx, ymdy, ypdy, color);
            } 
            
            else {
                result |= render_basic_pixel (renderer, xpdx, ypdy);
                result |= render_basic_pixel (renderer, xmdx, ypdy);
                result |= render_basic_pixel (renderer, xpdx, ymdy);
                result |= render_basic_pixel (renderer, xmdx, ymdy);
            }
        }
    }

    return result;

}

// draws an ellipse or a filled ellipse with blending
// returns 0 on success, -1 on failure
int render_complex_ellipse (Renderer *renderer,
    int x, int y,
    int rx, int ry,
    SDL_Color color, bool filled) {

    int result = -1;

    if (renderer) {
        if ((rx < 0) || (ry < 0)) {
            return (-1);
        }

        int rxi, ryi;
        int rx2, ry2, rx22, ry22; 
        int error;
        int curX, curY, curXp1, curYm1;
        int scrX, scrY, oldX, oldY;
        int deltaX, deltaY;
        int ellipseOverscan;

        // set color
        result = 0;
        result |= SDL_SetRenderDrawBlendMode (renderer->renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
        result |= SDL_SetRenderDrawColor (renderer->renderer, color.r, color.g, color.b, color.a);

        rxi = rx;
        ryi = ry;
        if (rxi >= 512 || ryi >= 512) ellipseOverscan = DEFAULT_ELLIPSE_OVERSCAN / 4;
        else if (rxi >= 256 || ryi >= 256) ellipseOverscan = DEFAULT_ELLIPSE_OVERSCAN / 2;
        else ellipseOverscan = DEFAULT_ELLIPSE_OVERSCAN / 1;

        oldX = scrX = 0;
        oldY = scrY = ryi;
        result |= render_complex_ellipse_draw_quadrants (renderer, x, y, 0, ry, color, filled);

        /* Midpoint ellipse algorithm with overdraw */
        rxi *= ellipseOverscan;
        ryi *= ellipseOverscan;
        rx2 = rxi * rxi;
        rx22 = rx2 + rx2;
        ry2 = ryi * ryi;
        ry22 = ry2 + ry2;
        curX = 0;
        curY = ryi;
        deltaX = 0;
        deltaY = rx22 * curY;

        // Points in segment 1
        error = ry2 - rx2 * ryi + rx2 / 4;
        while (deltaX <= deltaY) {
            curX++;
            deltaX += ry22;
    
            error +=  deltaX + ry2; 
            if (error >= 0) {
                curY--;
                deltaY -= rx22; 
                error -= deltaY;
            }

            scrX = curX / ellipseOverscan;
            scrY = curY / ellipseOverscan;
            if ((scrX != oldX && scrY == oldY) || (scrX != oldX && scrY != oldY)) {
                result |= render_complex_ellipse_draw_quadrants (renderer, x, y, scrX, scrY, color, filled);
                oldX = scrX;
                oldY = scrY;
            }
        }

        // Points in segment 2
        if (curY > 0) {
            curXp1 = curX + 1;
            curYm1 = curY - 1;
            error = ry2 * curX * curXp1 + ((ry2 + 3) / 4) + rx2 * curYm1 * curYm1 - rx2 * ry2;
            while (curY > 0) {
                curY--;
                deltaY -= rx22;

                error += rx2;
                error -= deltaY;
    
                if (error <= 0) {
                    curX++;
                    deltaX += ry22;
                    error += deltaX;
                }

                scrX = curX / ellipseOverscan;
                scrY = curY / ellipseOverscan;
                if ((scrX != oldX && scrY == oldY) || (scrX != oldX && scrY != oldY)) {
                    oldY--;
                    for (;oldY >= scrY; oldY--) {
                        result |= render_complex_ellipse_draw_quadrants (renderer, x, y, scrX, oldY, color, filled);
                        if (filled) {
                            oldY = scrY - 1;
                        }
                    }
                    oldX = scrX;
                    oldY = scrY;
                }		
            }

            /* Remaining points in vertical */
            if (!filled) {
                oldY--;
                for (;oldY >= 0; oldY--) {
                    result |= render_complex_ellipse_draw_quadrants (renderer, x, y, scrX, oldY, color, filled);
                }
            }
        }
    }

    return result;

}

// draws a circle with blending
// returns 0 on success, -1 on failure
int render_complex_circle (Renderer *renderer,
    int x, int y,
    int rad, 
    SDL_Color color, bool filled) {

    return renderer ? render_complex_ellipse (renderer, x, y, rad, rad, color, filled) : -1;

}

#pragma endregion

#pragma region Render

static void renderer_bg_destroy_textures (Renderer *renderer) {

    if (renderer) {
        size_t current_count = queue_size (renderer->destroy_textures_queue);

        size_t max_count = current_count > renderer->bg_destroying_factor ? 
            renderer->bg_destroying_factor : current_count;
            
        size_t count = 0;
        while (count < max_count) {
            SDL_Texture *texture = queue_pop (renderer->destroy_textures_queue);
            if (texture) {
                SDL_DestroyTexture (texture);
            } 

            count++;
        }
    }

}

static void renderer_bg_load_textures (Renderer *renderer) {

    if (renderer) {
        size_t current_count = queue_size (renderer->load_textures_queue);

        size_t max_count = current_count > renderer->bg_loading_factor ? 
            renderer->bg_loading_factor : current_count;

        size_t count = 0;
        while (count < max_count) {
            SurfaceTexture *st = (SurfaceTexture *) queue_pop (renderer->load_textures_queue);
            if (st) {
                if (st->update) {
                    // TODO: 27/01/2020 -- 01:01 -- check if SDL_UpdateTexture () is thread safe, maybe that why we are not rendering correctly

                    // 27/01/2020 -- 00:27 -- SDL_UpdateTexture () does not render to screen, we get only back texture
                    // so changed to use create texture and for streaming image, we first destroy the texture

                    SDL_LockTexture (*st->texture, NULL, &st->surface->pixels, &st->surface->pitch);
                    SDL_UpdateTexture (*st->texture, NULL, st->surface->pixels, st->surface->pitch);
                    // *st->texture = SDL_CreateTextureFromSurface (renderer->renderer, st->surface);
                    SDL_UnlockTexture (*st->texture);
                }

                else *st->texture = SDL_CreateTextureFromSurface (renderer->renderer, st->surface);

                surface_texture_delete (st);
            } 

            count++;
        }
    }

}

// TODO: 08/02/2020 -- 22:30 - implement occlusion culling using the same intersection checks as the ui
static void renderer_render_game_objects (Renderer *renderer) {

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

}

void render (Renderer *renderer) {

    if (renderer) {
        renderer->render_count = 0;

        // destroy any texture in background queue
        if (queue_size (renderer->destroy_textures_queue) > 0) {
            renderer_bg_destroy_textures (renderer);
        }

        // load any texture in background queue
        if (queue_size (renderer->load_textures_queue) > 0) {
            renderer_bg_load_textures (renderer);
        }

        SDL_SetRenderDrawColor (renderer->renderer, 0, 0, 0, 255);
        SDL_RenderClear (renderer->renderer);

        renderer_render_game_objects (renderer);

        ui_render (renderer);

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

    renderers = dlist_init (renderer_delete, renderer_comparator);
    u8 retval = renderers ? 0 : 1;

    errors |= retval;

    windows = dlist_init (window_delete, window_comparator);
    retval = windows ? 0 : 1;

    return errors;

}

u8 render_end (void) {

    dlist_delete (renderers);

    dlist_delete (windows);

    #ifdef CENGINE_DEBUG
    cengine_log_msg (stdout, LOG_SUCCESS, LOG_NO_TYPE, "Done cleaning cengine renderers and windows.");
    #endif

    return 0;

}

#pragma endregion
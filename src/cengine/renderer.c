#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/renderer.h"
#include "cengine/textures.h"

#include "cengine/manager/manager.h"

#include "cengine/game/go.h"
#include "cengine/game/camera.h"

#include "cengine/ui/ui.h"

#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

#pragma region Window

// gets window size into renderer data struct
int window_get_size (SDL_Window *window, WindowSize *window_size) {

    int retval = 1;

    if (window) {
        SDL_GetWindowSize (window, &window_size->width, &window_size->height);
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, 
            c_string_create ("Window size: %dx%dpx.", window_size->width, window_size->height));
        #endif
        retval = 0;
    }

    return retval;

}

// toggle full screen on and off
int window_toggle_full_screen (Renderer *renderer) {

    int retval = 1;

    if (renderer) {
        renderer->full_screen = SDL_GetWindowFlags (renderer->window) & SDL_WINDOW_FULLSCREEN;
        retval = SDL_SetWindowFullscreen (renderer->window, renderer->full_screen ? 0 : SDL_WINDOW_FULLSCREEN);
        renderer->full_screen = SDL_GetWindowFlags (renderer->window) & SDL_WINDOW_FULLSCREEN;
    }

    return retval;

}

// FIXME: do we need to update the renderer?
// resizes the window asscoaited with a renderer
int window_resize (Renderer *renderer, u32 new_width, u32 new_height) {

    int retval = 1;

    if (renderer) {
        // check if we have a valid new size
        if (new_width <= renderer->display_mode.w && new_width > 0 &&
            new_height <= renderer->display_mode.h && new_height > 0) {
            SDL_SetWindowSize (renderer->window, new_width, new_height);
            window_get_size (renderer->window, &renderer->window_size);
            retval = 0;
        }
    }

    return retval;

}

static SDL_Window *window_create (const char *title, WindowSize window_size, bool full_screen) {

    SDL_Window *window = NULL;

    // creates a window of the size of the screen
    window = SDL_CreateWindow (title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        window_size.width, window_size.height,
        full_screen ? SDL_WINDOW_FULLSCREEN : 0);

    return window;

}

#pragma endregion

#pragma region Renderer

// TODO: as of 03/06/2019 we only have support for one renderer, the main one
// the plan is to have as many as you want in order to support multiple windows 
Renderer *main_renderer = NULL;

static Renderer *renderer_new (void) {

    Renderer *renderer = (Renderer *) malloc (sizeof (Renderer));
    if (renderer) {
        memset (renderer, 0, sizeof (Renderer));
        renderer->name = NULL;
        renderer->window_title = NULL;
        renderer->window = NULL;
        renderer->renderer = NULL;
    }

    return renderer;

}

static void *renderer_delete (void *ptr) {

    if (ptr) {
        Renderer *renderer = (Renderer *) ptr;

        str_delete (renderer->name);
        str_delete (renderer->window_title);
        if (renderer->window) SDL_DestroyWindow (renderer->window);
        if (renderer->renderer) SDL_DestroyRenderer (renderer->renderer);

        free (renderer);
    }

}

// FIXME: players with higher resolution have an advantage -> they see more of the world
// TODO: check SDL_GetCurrentDisplayMode
// TODO: get refresh rate -> do we need vsync?
Renderer *render_create_renderer (const char *renderer_name, Uint32 flags, int display_index,
    const char *window_title, WindowSize window_size, bool full_screen) {

    Renderer *renderer = renderer_new ();

    renderer->display_index = display_index;
    if (!SDL_GetCurrentDisplayMode (display_index, &renderer->display_mode)) {
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE,
            c_string_create ("Display with idx %i mode is %dx%dpx @ %dhz.",
            renderer->display_index, 
            renderer->display_mode.w, renderer->display_mode.h, 
            renderer->display_mode.refresh_rate));
        #endif

        // first init the window
        renderer->window = window_create (window_title, window_size, full_screen);
        if (renderer->window) {
            window_get_size (renderer->window, &renderer->window_size);

            // init the sdl renderer
            // SDL_CreateRenderer (main_window, 0, SDL_RENDERER_SOFTWARE | SDL_RENDERER_ACCELERATED);
            renderer->renderer = SDL_CreateRenderer (renderer->window, display_index, flags);
            if (renderer->renderer) {
                SDL_SetRenderDrawColor (renderer->renderer, 0, 0, 0, 255);
                SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "0");
                SDL_RenderSetLogicalSize (renderer->renderer, 
                    renderer->window_size.width, renderer->window_size.height);

                renderer->name = str_new (renderer_name);
                renderer->flags = flags;
                renderer->window_title = str_new (window_title);
                renderer->full_screen = full_screen;
            }

            else {
                cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create renderer!"); 
                renderer_delete (renderer);
                renderer = NULL;
            }
        }

        else {
            cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create window!"); 
            renderer_delete (renderer);
            renderer = NULL;
        }
    }

    else {
        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
            c_string_create ("Failed to get display mode for display with idx %i", display_index));
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, SDL_GetError ());
        #endif
        renderer_delete (renderer);
        renderer = NULL;
    }

    return renderer;

}

int renderer_init_main (Uint32 flags,
    const char *window_title, WindowSize window_size, bool full_screen) {

    return ((main_renderer = render_create_renderer ("main", flags, 0, 
        window_title, window_size, full_screen)) ? 0 : 1);

}

void renderer_delete_main (void) { renderer_delete (main_renderer); }

#pragma endregion

/*** Layers ***/

#pragma region Layers

DoubleList *gos_layers = NULL;              // render layers for the gameobjects
DoubleList *ui_elements_layers = NULL;      // render layers for the ui elements

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
        dlist_insert_after (layer->elements, dlist_end (layer->elements), ptr);
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

static u8 layers_init (void) {

    // ini the game objects layers
    gos_layers = dlist_init (layer_delete, layer_comparator);
    if (gos_layers) {
        // add the default layer to the list
        Layer *default_layer = layer_new (gos_layers, "default", 0, true);
        dlist_insert_after (gos_layers, dlist_end (gos_layers), default_layer);
    }

    // init the ui elements layers
    ui_elements_layers = dlist_init (layer_delete, layer_comparator);
    if (ui_elements_layers) {
        // add the default layers to the list
        Layer *back_layer = layer_new (ui_elements_layers, "back", 0, false);
        dlist_insert_after (ui_elements_layers, dlist_end (ui_elements_layers), back_layer);

        Layer *middle_layer = layer_new (ui_elements_layers, "middle", 1, false);
        dlist_insert_after (ui_elements_layers, dlist_end (ui_elements_layers), middle_layer);

        Layer *top_layer = layer_new (ui_elements_layers, "top", 2, false);
        dlist_insert_after (ui_elements_layers, dlist_end (ui_elements_layers), top_layer);
    }

    return (gos_layers && ui_elements_layers ? 0 : 1);

}

static void layers_end (void) { 
    
    dlist_delete (gos_layers);
    gos_layers = NULL;

    dlist_delete (ui_elements_layers);
    ui_elements_layers = NULL;
    
}

#pragma endregion

#pragma region Surfaces

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

#pragma endregion

#pragma region Basic

// renders a dot
void render_basic_dot (int x, int y, SDL_Color color) {

    SDL_SetRenderDrawColor (main_renderer->renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint (main_renderer->renderer, x ,y);

}

// renders a horizontal line of dots
void render_basic_dot_line_horizontal (int start, int y, int length, int offset, SDL_Color color) {

    SDL_SetRenderDrawColor (main_renderer->renderer, color.r, color.g, color.b, color.a);
    for (unsigned int i = start; i < length; i += offset)
        SDL_RenderDrawPoint (main_renderer->renderer, i, y);

}

// renders a vertical line of dots
void render_basic_dot_line_vertical (int x, int start, int length, int offset, SDL_Color color) {

    SDL_SetRenderDrawColor (main_renderer->renderer, color.r, color.g, color.b, color.a);
    for (unsigned int i = start; i < length; i += offset)
        SDL_RenderDrawPoint (main_renderer->renderer, x, i);

}

// renders a filled rect
void render_basic_filled_rect (SDL_Rect *rect, SDL_Color color) {

    if (rect) {
        SDL_SetRenderDrawColor (main_renderer->renderer, color.r, color.g, color.b, color.a);        
        SDL_RenderFillRect (main_renderer->renderer, rect);
    }

}

// renders an outline rect
void render_basic_outline_rect (SDL_Rect *rect, SDL_Color color) {

    SDL_SetRenderDrawColor (main_renderer->renderer, color.r, color.g, color.b, color.a);        
    SDL_RenderDrawRect (main_renderer->renderer, rect);

}

// renders a line
void render_basic_line (int x1, int x2, int y1, int y2, SDL_Color color) {

    SDL_SetRenderDrawColor (main_renderer->renderer, color.r, color.g, color.b, color.a);        
    SDL_RenderDrawLine (main_renderer->renderer, x1, y1, x2, y2);

}

#pragma endregion

#pragma region Complex

// renders a rect with transparency
SDL_Texture *render_complex_transparent_rect (SDL_Rect *rect, SDL_Color color) {

    SDL_Texture *texture = NULL;

    SDL_Surface *surface = surface_create (rect->w, rect->h);
    if (surface) {
        (void) SDL_FillRect (surface, NULL, 
            convert_rgba_to_hex (color.r, color.g, color.b, color.a));
        texture = SDL_CreateTextureFromSurface (main_renderer->renderer, surface);
        SDL_FreeSurface (surface); 
    }

    return texture;

}

#pragma endregion

#pragma region Render

// FIXME: we need to implement occlusion culling!
// renders the game objects to the screen
void render (void) {

    SDL_SetRenderDrawColor (main_renderer->renderer, 0, 0, 0, 255);
    SDL_RenderClear (main_renderer->renderer);

    // render by layers
    Layer *layer = NULL;
    GameObject *go = NULL;
    Transform *transform = NULL;
    Graphics *graphics = NULL;
    for (ListElement *layer_le = dlist_start (gos_layers); layer_le; layer_le = layer_le->next) {
        layer = (Layer *) layer_le->data;

        for (ListElement *le = dlist_start (layer->elements); le; le = le->next) {
            go = (GameObject *) le->data;

            transform = (Transform *) game_object_get_component (go, TRANSFORM_COMP);
            graphics = (Graphics *) game_object_get_component (go, GRAPHICS_COMP);
            if (transform && graphics) {
                if (graphics->multipleSprites) {
                    texture_draw_frame (main_camera, graphics->spriteSheet, 
                        transform->position.x, transform->position.y, 
                        graphics->x_sprite_offset, graphics->y_sprite_offset,
                        graphics->flip);
                }
                
                else {
                    texture_draw (main_camera, graphics->sprite, 
                        transform->position.x, transform->position.y, 
                        graphics->flip);
                }
            }
        }
    }

    ui_render ();       // render ui elements

    SDL_RenderPresent (main_renderer->renderer);

}

#pragma endregion

#pragma region public

// inits cengine render capabilities
u8 render_init (void) {

    return layers_init ();

}

void render_end (void) {

    layers_end ();

}

#pragma endregion